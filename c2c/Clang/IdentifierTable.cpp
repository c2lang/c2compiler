//===- IdentifierTable.cpp - Hash table for identifier lookup -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the IdentifierInfo, IdentifierVisitor, and
// IdentifierTable interfaces.
//
//===----------------------------------------------------------------------===//

#include "Clang/IdentifierTable.h"
#include "Clang/CharInfo.h"
#include "Clang/Specifiers.h"
#include "Clang/TokenKinds.h"
#include "llvm/ADT/DenseMapInfo.h"
#include "llvm/ADT/FoldingSet.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <string>

using namespace c2lang;

//===----------------------------------------------------------------------===//
// IdentifierInfo Implementation
//===----------------------------------------------------------------------===//

IdentifierInfo::IdentifierInfo() {
  TokenID = tok::identifier;
  HasMacro = false;
  HadMacro = false;
  IsExtension = false;
  IsFutureCompatKeyword = false;
  IsPoisoned = false;
  NeedsHandleIdentifier = false;
  IsFromAST = false;
  OutOfDate = false;
}

//===----------------------------------------------------------------------===//
// IdentifierTable Implementation
//===----------------------------------------------------------------------===//

IdentifierIterator::~IdentifierIterator() = default;

namespace {


} // namespace


IdentifierTable::IdentifierTable()
    : HashTable(8192) // Start with space for 8K identifiers.
       {
  // Populate the identifier table with info about keywords for the current
  // language.
  AddKeywords();
}

//===----------------------------------------------------------------------===//
// Language Keyword Implementation
//===----------------------------------------------------------------------===//

// Constants for TokenKinds.def
namespace {

  enum {
    KEYALL = (0xffffff)
  };

  /// How a keyword is treated in the selected standard.
  enum KeywordStatus {
    KS_Disabled,    // Disabled
    KS_Extension,   // Is an extension
    KS_Enabled,     // Enabled
    KS_Future       // Is a keyword in future standard
  };

} // namespace

/// Translates flags as specified in TokenKinds.def into keyword status
/// in the given language standard.
static KeywordStatus getKeywordStatus(unsigned Flags) {
  if (Flags == KEYALL) return KS_Enabled;
  return KS_Disabled;
}

/// AddKeyword - This method is used to associate a token ID with specific
/// identifiers because they are language keywords.  This causes the lexer to
/// automatically map matching identifiers to specialized token codes.
static void AddKeyword(StringRef Keyword,
                       tok::TokenKind TokenCode, unsigned Flags,
                       IdentifierTable &Table) {
  KeywordStatus AddResult = getKeywordStatus(Flags);

  // Don't add this keyword if disabled in this language.
  if (AddResult == KS_Disabled) return;

  IdentifierInfo &Info =
      Table.get(Keyword, AddResult == KS_Future ? tok::identifier : TokenCode);
  Info.setIsExtensionToken(AddResult == KS_Extension);
  Info.setIsFutureCompatKeyword(AddResult == KS_Future);
}



/// AddKeywords - Add all keywords to the symbol table.
///
void IdentifierTable::AddKeywords() {
  // Add keywords and tokens for the current language.
#define KEYWORD(NAME, FLAGS) \
  AddKeyword(StringRef(#NAME), tok::kw_ ## NAME,  \
             FLAGS, *this);
#define ALIAS(NAME, TOK, FLAGS) \
  AddKeyword(StringRef(NAME), tok::kw_ ## TOK,  \
             FLAGS, *this);
#define TESTING_KEYWORD(NAME, FLAGS)
#include "Clang/TokenKinds.def"


}

/// Checks if the specified token kind represents a keyword in the
/// specified language.
/// \returns Status of the keyword in the language.
static KeywordStatus getTokenKwStatus(tok::TokenKind K) {
  switch (K) {
#define KEYWORD(NAME, FLAGS) \
  case tok::kw_##NAME: return getKeywordStatus(FLAGS);
#include "Clang/TokenKinds.def"
  default: return KS_Disabled;
  }
}

/// Returns true if the identifier represents a keyword in the
/// specified language.
bool IdentifierInfo::isKeyword() const {
  switch (getTokenKwStatus(getTokenID())) {
  case KS_Enabled:
  case KS_Extension:
    return true;
  default:
    return false;
  }
}


tok::PPKeywordKind IdentifierInfo::getPPKeywordID() const {
  // We use a perfect hash function here involving the length of the keyword,
  // the first and third character.  For preprocessor ID's there are no
  // collisions (if there were, the switch below would complain about duplicate
  // case values).  Note that this depends on 'if' being null terminated.

#define HASH(LEN, FIRST, THIRD) \
  (LEN << 5) + (((FIRST-'a') + (THIRD-'a')) & 31)
#define CASE(LEN, FIRST, THIRD, NAME) \
  case HASH(LEN, FIRST, THIRD): \
    return memcmp(Name, #NAME, LEN) ? tok::pp_not_keyword : tok::pp_ ## NAME

  unsigned Len = getLength();
  if (Len < 2) return tok::pp_not_keyword;
  const char *Name = getNameStart();
  switch (HASH(Len, Name[0], Name[2])) {
  default: return tok::pp_not_keyword;
  CASE( 2, 'i', '\0', if);
  CASE( 4, 'e', 'i', elif);
  CASE( 4, 'e', 's', else);
  CASE( 5, 'e', 'd', endif);
  CASE( 5, 'i', 'd', ifdef);
  CASE( 5, 'u', 'd', undef);

  CASE( 6, 'd', 'f', define);
  CASE( 6, 'i', 'n', ifndef);


#undef CASE
#undef HASH
  }
}

//===----------------------------------------------------------------------===//
// Stats Implementation
//===----------------------------------------------------------------------===//

/// PrintStats - Print statistics about how well the identifier table is doing
/// at hashing identifiers.
void IdentifierTable::PrintStats() const {
  unsigned NumBuckets = HashTable.getNumBuckets();
  unsigned NumIdentifiers = HashTable.getNumItems();
  unsigned NumEmptyBuckets = NumBuckets-NumIdentifiers;
  unsigned AverageIdentifierSize = 0;
  unsigned MaxIdentifierLength = 0;

  // TODO: Figure out maximum times an identifier had to probe for -stats.
  for (llvm::StringMap<IdentifierInfo*, llvm::BumpPtrAllocator>::const_iterator
       I = HashTable.begin(), E = HashTable.end(); I != E; ++I) {
    unsigned IdLen = I->getKeyLength();
    AverageIdentifierSize += IdLen;
    if (MaxIdentifierLength < IdLen)
      MaxIdentifierLength = IdLen;
  }

  fprintf(stderr, "\n*** Identifier Table Stats:\n");
  fprintf(stderr, "# Identifiers:   %d\n", NumIdentifiers);
  fprintf(stderr, "# Empty Buckets: %d\n", NumEmptyBuckets);
  fprintf(stderr, "Hash density (#identifiers per bucket): %f\n",
          NumIdentifiers/(double)NumBuckets);
  fprintf(stderr, "Ave identifier length: %f\n",
          (AverageIdentifierSize/(double)NumIdentifiers));
  fprintf(stderr, "Max identifier length: %d\n", MaxIdentifierLength);

  // Compute statistics about the memory allocated for identifiers.
  HashTable.getAllocator().PrintStats();
}
