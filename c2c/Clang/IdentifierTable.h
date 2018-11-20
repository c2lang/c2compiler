//===- IdentifierTable.h - Hash table for identifier lookup -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Defines the c2lang::IdentifierInfo, c2lang::IdentifierTable, and
/// c2lang::Selector interfaces.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_BASIC_IDENTIFIERTABLE_H
#define LLVM_CLANG_BASIC_IDENTIFIERTABLE_H

#include "Clang/LLVM.h"
#include "Clang/TokenKinds.h"
#include <llvm/ADT/DenseMapInfo.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Allocator.h>
#include <llvm/Support/PointerLikeTypeTraits.h>
#include <llvm/Support/type_traits.h>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <utility>

namespace c2lang {

class IdentifierInfo;
class LangOptions;
class SourceLocation;

/// A simple pair of identifier info and location.
using IdentifierLocPair = std::pair<IdentifierInfo *, SourceLocation>;

/// One of these records is kept for each identifier that
/// is lexed.  This contains information about whether the token was \#define'd,
/// is a language keyword, or if it is a front-end token of some sort (e.g. a
/// variable or function name).  The preprocessor keeps this information in a
/// set, and all tok::identifier tokens have a pointer to one of these.
class IdentifierInfo {
  friend class IdentifierTable;

  unsigned TokenID            : 9; // Front-end token ID or tok::identifier.
  bool HasMacro               : 1; // True if there is a #define for this.
  bool HadMacro               : 1; // True if there was a #define for this.
  bool IsExtension            : 1; // True if identifier is a lang extension.
  bool IsFutureCompatKeyword  : 1; // True if identifier is a keyword in a
                                   // newer Standard or proposed Standard.
  bool IsPoisoned             : 1; // True if identifier is poisoned.
  bool NeedsHandleIdentifier  : 1; // See "RecomputeNeedsHandleIdentifier".
  bool IsFromAST              : 1; // True if identifier was loaded (at least
                                   // partially) from an AST file.
  // 29 bit left in 64-bit word.


  llvm::StringMapEntry<IdentifierInfo *> *Entry = nullptr;

public:
  IdentifierInfo();
  IdentifierInfo(const IdentifierInfo &) = delete;
  IdentifierInfo &operator=(const IdentifierInfo &) = delete;

  /// Return true if this is the identifier for the specified string.
  ///
  /// This is intended to be used for string literals only: II->isStr("foo").
  template <std::size_t StrLen>
  bool isStr(const char (&Str)[StrLen]) const {
    return getLength() == StrLen-1 &&
           memcmp(getNameStart(), Str, StrLen-1) == 0;
  }

  /// Return true if this is the identifier for the specified StringRef.
  bool isStr(llvm::StringRef Str) const {
    llvm::StringRef ThisStr(getNameStart(), getLength());
    return ThisStr == Str;
  }

  /// Return the beginning of the actual null-terminated string for this
  /// identifier.
  const char *getNameStart() const {
    if (Entry) return Entry->getKeyData();
    // FIXME: This is gross. It would be best not to embed specific details
    // of the PTH file format here.
    // The 'this' pointer really points to a
    // std::pair<IdentifierInfo, const char*>, where internal pointer
    // points to the external string data.
    using actualtype = std::pair<IdentifierInfo, const char *>;

    return ((const actualtype*) this)->second;
  }

  /// Efficiently return the length of this identifier info.
  unsigned getLength() const {
    if (Entry) return Entry->getKeyLength();
    // FIXME: This is gross. It would be best not to embed specific details
    // of the PTH file format here.
    // The 'this' pointer really points to a
    // std::pair<IdentifierInfo, const char*>, where internal pointer
    // points to the external string data.
    using actualtype = std::pair<IdentifierInfo, const char *>;

    const char* p = ((const actualtype*) this)->second - 2;
    return (((unsigned) p[0]) | (((unsigned) p[1]) << 8)) - 1;
  }

  /// Return the actual identifier string.
  StringRef getName() const {
    return StringRef(getNameStart(), getLength());
  }

  /// Return true if this identifier is \#defined to some other value.
  /// \note The current definition may be in a module and not currently visible.
  bool hasMacroDefinition() const {
    return HasMacro;
  }
  void setHasMacroDefinition(bool Val) {
    if (HasMacro == Val) return;

    HasMacro = Val;
    if (Val) {
      NeedsHandleIdentifier = true;
      HadMacro = true;
    } else {
      RecomputeNeedsHandleIdentifier();
    }
  }
  /// Returns true if this identifier was \#defined to some value at any
  /// moment. In this case there should be an entry for the identifier in the
  /// macro history table in Preprocessor.
  bool hadMacroDefinition() const {
    return HadMacro;
  }

  /// If this is a source-language token (e.g. 'for'), this API
  /// can be used to cause the lexer to map identifiers to source-language
  /// tokens.
  tok::TokenKind getTokenID() const { return (tok::TokenKind)TokenID; }


  /// Return the preprocessor keyword ID for this identifier.
  ///
  /// For example, "define" will return tok::pp_define.
  tok::PPKeywordKind getPPKeywordID() const;


    /// get/setExtension - Initialize information about whether or not this
  /// language token is an extension.  This controls extension warnings, and is
  /// only valid if a custom token ID is set.
  bool isExtensionToken() const { return IsExtension; }
  void setIsExtensionToken(bool Val) {
    IsExtension = Val;
    if (Val)
      NeedsHandleIdentifier = true;
    else
      RecomputeNeedsHandleIdentifier();
  }

  /// is/setIsFutureCompatKeyword - Initialize information about whether or not
  /// this language token is a keyword in a newer or proposed Standard. This
  /// controls compatibility warnings, and is only true when not parsing the
  /// corresponding Standard. Once a compatibility problem has been diagnosed
  /// with this keyword, the flag will be cleared.
  bool isFutureCompatKeyword() const { return IsFutureCompatKeyword; }
  void setIsFutureCompatKeyword(bool Val) {
    IsFutureCompatKeyword = Val;
    if (Val)
      NeedsHandleIdentifier = true;
    else
      RecomputeNeedsHandleIdentifier();
  }

  /// setIsPoisoned - Mark this identifier as poisoned.  After poisoning, the
  /// Preprocessor will emit an error every time this token is used.
  void setIsPoisoned(bool Value = true) {
    IsPoisoned = Value;
    if (Value)
      NeedsHandleIdentifier = true;
    else
      RecomputeNeedsHandleIdentifier();
  }

  /// Return true if this token has been poisoned.
  bool isPoisoned() const { return IsPoisoned; }



    /// Return true if this token is a keyword in the specified language.
  bool isKeyword(const LangOptions &LangOpts) const;



  /// Return true if the Preprocessor::HandleIdentifier must be called
  /// on a token of this identifier.
  ///
  /// If this returns false, we know that HandleIdentifier will not affect
  /// the token.
  bool isHandleIdentifierCase() const { return NeedsHandleIdentifier; }

  /// Return true if the identifier in its current state was loaded
  /// from an AST file.
  bool isFromAST() const { return IsFromAST; }

    /// Provide less than operator for lexicographical sorting.
  bool operator<(const IdentifierInfo &RHS) const {
    return getName() < RHS.getName();
  }

private:
  /// The Preprocessor::HandleIdentifier does several special (but rare)
  /// things to identifiers of various sorts.  For example, it changes the
  /// \c for keyword token from tok::identifier to tok::for.
  ///
  /// This method is very tied to the definition of HandleIdentifier.  Any
  /// change to it should be reflected here.
  void RecomputeNeedsHandleIdentifier() {
    NeedsHandleIdentifier = isPoisoned() || hasMacroDefinition() ||
                            isExtensionToken() || isFutureCompatKeyword();
  }
};

/// An iterator that walks over all of the known identifiers
/// in the lookup table.
///
/// Since this iterator uses an abstract interface via virtual
/// functions, it uses an object-oriented interface rather than the
/// more standard C++ STL iterator interface. In this OO-style
/// iteration, the single function \c Next() provides dereference,
/// advance, and end-of-sequence checking in a single
/// operation. Subclasses of this iterator type will provide the
/// actual functionality.
class IdentifierIterator {
protected:
  IdentifierIterator() = default;

public:
  IdentifierIterator(const IdentifierIterator &) = delete;
  IdentifierIterator &operator=(const IdentifierIterator &) = delete;

  virtual ~IdentifierIterator();

  /// Retrieve the next string in the identifier table and
  /// advances the iterator for the following string.
  ///
  /// \returns The next string in the identifier table. If there is
  /// no such string, returns an empty \c StringRef.
  virtual StringRef Next() = 0;
};

/// Provides lookups to, and iteration over, IdentiferInfo objects.
class IdentifierInfoLookup {
public:
  virtual ~IdentifierInfoLookup();

  /// Return the IdentifierInfo for the specified named identifier.
  ///
  /// Unlike the version in IdentifierTable, this returns a pointer instead
  /// of a reference.  If the pointer is null then the IdentifierInfo cannot
  /// be found.
  virtual IdentifierInfo* get(StringRef Name) = 0;

  /// Retrieve an iterator into the set of all identifiers
  /// known to this identifier lookup source.
  ///
  /// This routine provides access to all of the identifiers known to
  /// the identifier lookup, allowing access to the contents of the
  /// identifiers without introducing the overhead of constructing
  /// IdentifierInfo objects for each.
  ///
  /// \returns A new iterator into the set of known identifiers. The
  /// caller is responsible for deleting this iterator.
  virtual IdentifierIterator *getIdentifiers();
};

/// Implements an efficient mapping from strings to IdentifierInfo nodes.
///
/// This has no other purpose, but this is an extremely performance-critical
/// piece of the code, as each occurrence of every identifier goes through
/// here when lexed.
class IdentifierTable {
  // Shark shows that using MallocAllocator is *much* slower than using this
  // BumpPtrAllocator!
  using HashTableTy = llvm::StringMap<IdentifierInfo *, llvm::BumpPtrAllocator>;
  HashTableTy HashTable;

  IdentifierInfoLookup* ExternalLookup;

public:
  /// Create the identifier table.
  explicit IdentifierTable(IdentifierInfoLookup *ExternalLookup = nullptr);

  /// Create the identifier table, populating it with info about the
  /// language keywords for the language specified by \p LangOpts.
  explicit IdentifierTable(const LangOptions &LangOpts,
                           IdentifierInfoLookup *ExternalLookup = nullptr);


    llvm::BumpPtrAllocator& getAllocator() {
    return HashTable.getAllocator();
  }

  /// Return the identifier token info for the specified named
  /// identifier.
  IdentifierInfo &get(StringRef Name) {
    auto &Entry = *HashTable.insert(std::make_pair(Name, nullptr)).first;

    IdentifierInfo *&II = Entry.second;
    if (II) return *II;

    // No entry; if we have an external lookup, look there first.
    if (ExternalLookup) {
      II = ExternalLookup->get(Name);
      if (II)
        return *II;
    }

    // Lookups failed, make a new IdentifierInfo.
    void *Mem = getAllocator().Allocate<IdentifierInfo>();
    II = new (Mem) IdentifierInfo();

    // Make sure getName() knows how to find the IdentifierInfo
    // contents.
    II->Entry = &Entry;

    return *II;
  }

  IdentifierInfo &get(StringRef Name, tok::TokenKind TokenCode) {
    IdentifierInfo &II = get(Name);
    II.TokenID = TokenCode;
    assert(II.TokenID == (unsigned) TokenCode && "TokenCode too large");
    return II;
  }

  /// Gets an IdentifierInfo for the given name without consulting
  ///        external sources.
  ///
  /// This is a version of get() meant for external sources that want to
  /// introduce or modify an identifier. If they called get(), they would
  /// likely end up in a recursion.
  IdentifierInfo &getOwn(StringRef Name) {
    auto &Entry = *HashTable.insert(std::make_pair(Name, nullptr)).first;

    IdentifierInfo *&II = Entry.second;
    if (II)
      return *II;

    // Lookups failed, make a new IdentifierInfo.
    void *Mem = getAllocator().Allocate<IdentifierInfo>();
    II = new (Mem) IdentifierInfo();

    // Make sure getName() knows how to find the IdentifierInfo
    // contents.
    II->Entry = &Entry;


    return *II;
  }

  using iterator = HashTableTy::const_iterator;
  using const_iterator = HashTableTy::const_iterator;

  iterator begin() const { return HashTable.begin(); }
  iterator end() const   { return HashTable.end(); }
  unsigned size() const  { return HashTable.size(); }

  /// Print some statistics to stderr that indicate how well the
  /// hashing is doing.
  void PrintStats() const;

  /// Populate the identifier table with info about the language keywords
  /// for the language specified by \p LangOpts.
  void AddKeywords(const LangOptions &LangOpts);
};


}  // namespace c2lang

namespace llvm {


// Provide PointerLikeTypeTraits for IdentifierInfo pointers, which
// are not guaranteed to be 8-byte aligned.
template<>
struct PointerLikeTypeTraits<c2lang::IdentifierInfo*> {
  static void *getAsVoidPointer(c2lang::IdentifierInfo* P) {
    return P;
  }

  static c2lang::IdentifierInfo *getFromVoidPointer(void *P) {
    return static_cast<c2lang::IdentifierInfo*>(P);
  }

  enum { NumLowBitsAvailable = 1 };
};

template<>
struct PointerLikeTypeTraits<const c2lang::IdentifierInfo*> {
  static const void *getAsVoidPointer(const c2lang::IdentifierInfo* P) {
    return P;
  }

  static const c2lang::IdentifierInfo *getFromVoidPointer(const void *P) {
    return static_cast<const c2lang::IdentifierInfo*>(P);
  }

  enum { NumLowBitsAvailable = 1 };
};

} // namespace llvm

#endif // LLVM_CLANG_BASIC_IDENTIFIERTABLE_H
