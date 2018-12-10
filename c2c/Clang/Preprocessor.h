//===- Preprocessor.h - C Language Family Preprocessor ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Defines the c2lang::Preprocessor interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LEX_PREPROCESSOR_H
#define LLVM_CLANG_LEX_PREPROCESSOR_H

#include "Utils/TargetInfo.h"
#include "Clang/Diagnostic.h"
#include "Clang/IdentifierTable.h"
#include "Clang/LLVM.h"
#include "Clang/SourceLocation.h"
#include "Clang/SourceManager.h"
#include "Clang/TokenKinds.h"
#include "Clang/Lexer.h"
#include "Clang/MacroInfo.h"
#include "Clang/Token.h"
#include "Clang/TokenLexer.h"
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/FoldingSet.h>
#include <llvm/ADT/None.h>
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/PointerUnion.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/TinyPtrVector.h>
#include <llvm/ADT/iterator_range.h>
#include <llvm/Support/Allocator.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Registry.h>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace llvm {

template<unsigned InternalLen> class SmallString;

} // namespace llvm

namespace c2lang {

class CommentHandler;
class DirectoryEntry;
class DirectoryLookup;
class ExternalPreprocessorSource;
class FileEntry;
class FileManager;
class HeaderSearch;
class MacroArgs;
class MemoryBufferCache;
class PreprocessorLexer;
class PTHManager;
class ScratchBuffer;
class TargetInfo;

/// Stores token information for comparing actual tokens with
/// predefined values.  Only handles simple tokens and identifiers.
class TokenValue {
  tok::TokenKind Kind;
  IdentifierInfo *II;

public:
  TokenValue(tok::TokenKind Kind) : Kind(Kind), II(nullptr) {
    assert(Kind != tok::raw_identifier && "Raw identifiers are not supported.");
    assert(Kind != tok::identifier &&
           "Identifiers should be created by TokenValue(IdentifierInfo *)");
    assert(!tok::isLiteral(Kind) && "Literals are not supported.");
  }

  TokenValue(IdentifierInfo *II) : Kind(tok::identifier), II(II) {}

  bool operator==(const Token &Tok) const {
    return Tok.getKind() == Kind &&
        (!II || II == Tok.getIdentifierInfo());
  }
};

/// Context in which macro name is used.
enum MacroUse {
  // other than #define or #undef
  MU_Other  = 0,

  // macro name specified in #define
  MU_Define = 1,

  // macro name specified in #undef
  MU_Undef  = 2
};

/// Engages in a tight little dance with the lexer to efficiently
/// preprocess tokens.
///
/// Lexers know only about tokens within a single source file, and don't
/// know anything about preprocessor-level issues like the \#include stack,
/// token expansion, etc.
class Preprocessor {
  friend class VAOptDefinitionContext;
  friend class VariadicMacroScopeGuard;

  DiagnosticsEngine        *Diags;
  const C2::TargetInfo *Target = nullptr;
  const C2::TargetInfo *AuxTarget = nullptr;
  FileManager       &FileMgr;
  SourceManager     &SourceMgr;
  MemoryBufferCache &PCMCache;
  std::unique_ptr<ScratchBuffer> ScratchBuf;
  HeaderSearch      &HeaderInfo;

  /// External source of macros.
  ExternalPreprocessorSource *ExternalSource;


  /// A BumpPtrAllocator object used to quickly allocate and release
  /// objects internal to the Preprocessor.
  llvm::BumpPtrAllocator BP;

  /// Identifiers for builtin macros and other builtins.
  IdentifierInfo *Ident__LINE__, *Ident__FILE__;   // __LINE__, __FILE__
  IdentifierInfo *Ident__DATE__, *Ident__TIME__;   // __DATE__, __TIME__
  IdentifierInfo *Ident__INCLUDE_LEVEL__;          // __INCLUDE_LEVEL__
  IdentifierInfo *Ident__BASE_FILE__;              // __BASE_FILE__
  IdentifierInfo *Ident__TIMESTAMP__;              // __TIMESTAMP__
  IdentifierInfo *Ident__COUNTER__;                // __COUNTER__
  IdentifierInfo *Ident__identifier;               // __identifier
  IdentifierInfo *Ident__VA_ARGS__;                // __VA_ARGS__
  IdentifierInfo *Ident__VA_OPT__;                 // __VA_OPT__

  SourceLocation DATELoc, TIMELoc;

  // Next __COUNTER__ value, starts at 0.
  unsigned CounterValue = 0;

  enum {
    /// Maximum depth of \#includes.
    MaxAllowedIncludeStackDepth = 200
  };

  // State that is set before the preprocessor begins.
  bool SuppressIncludeNotFoundError : 1;

  // State that changes while the preprocessor runs:
  bool InMacroArgs : 1;            // True if parsing fn macro invocation args.

  /// Whether the preprocessor owns the header search object.
  bool OwnsHeaderSearch : 1;

  /// True if macro expansion is disabled.
  bool DisableMacroExpansion : 1;

  /// Temporarily disables DisableMacroExpansion (i.e. enables expansion)
  /// when parsing preprocessor directives.
  bool MacroExpansionInDirectivesOverride : 1;

  class ResetMacroExpansionHelper;

  /// True if pragmas are enabled.
  bool PragmasEnabled : 1;

  /// True if the current build action is a preprocessing action.
  bool PreprocessedOutput : 1;

  /// True if we are currently preprocessing a #if or #elif directive
  bool ParsingIfOrElifDirective;

  /// True if we are pre-expanding macro arguments.
  bool InMacroArgPreExpansion;

  /// Mapping/lookup information for all identifiers in
  /// the program, including program keywords.
  mutable IdentifierTable Identifiers;




  /// Tracks all of the comment handlers that the client registered
  /// with this preprocessor.
  std::vector<CommentHandler *> CommentHandlers;

  /// True if we want to ignore EOF token and continue later on (thus
  /// avoid tearing the Lexer and etc. down).
  bool IncrementalProcessing = false;



  /// Whether the last token we lexed was an '@'.
  bool LastTokenWasAt = false;



  /// The directory that the main file should be considered to occupy,
  /// if it does not correspond to a real file (as happens when building a
  /// module).
  const DirectoryEntry *MainFileDir = nullptr;

public:


private:
    friend class MacroArgs;


  /// The current top of the stack that we're lexing from if
  /// not expanding a macro and we are lexing directly from source code.
  ///
  /// Only one of CurLexer, CurPTHLexer, or CurTokenLexer will be non-null.
  std::unique_ptr<Lexer> CurLexer;


  /// The current top of the stack what we're lexing from
  /// if not expanding a macro.
  ///
  /// This is an alias for either CurLexer or  CurPTHLexer.
  PreprocessorLexer *CurPPLexer = nullptr;

  /// Used to find the current FileEntry, if CurLexer is non-null
  /// and if applicable.
  ///
  /// This allows us to implement \#include_next and find directory-specific
  /// properties.
  const DirectoryLookup *CurDirLookup = nullptr;

  /// The current macro we are expanding, if we are expanding a macro.
  ///
  /// One of CurLexer and CurTokenLexer must be null.
  std::unique_ptr<TokenLexer> CurTokenLexer;

  /// The kind of lexer we're currently working with.
  enum CurLexerKind {
    CLK_Lexer,
    CLK_TokenLexer,
    CLK_CachingLexer,
  } CurLexerKind = CLK_Lexer;


  /// Keeps track of the stack of files currently
  /// \#included, and macros currently being expanded from, not counting
  /// CurLexer/CurTokenLexer.
  struct IncludeStackInfo {
    enum CurLexerKind           CurLexerKind;
    std::unique_ptr<Lexer>      TheLexer;
    PreprocessorLexer          *ThePPLexer;
    std::unique_ptr<TokenLexer> TheTokenLexer;
    const DirectoryLookup      *TheDirLookup;

    // The following constructors are completely useless copies of the default
    // versions, only needed to pacify MSVC.
    IncludeStackInfo(enum CurLexerKind CurLexerKind,
                     std::unique_ptr<Lexer> &&TheLexer,
                     PreprocessorLexer *ThePPLexer,
                     std::unique_ptr<TokenLexer> &&TheTokenLexer,
                     const DirectoryLookup *TheDirLookup)
        : CurLexerKind(std::move(CurLexerKind)),
          TheLexer(std::move(TheLexer)),
          ThePPLexer(std::move(ThePPLexer)),
          TheTokenLexer(std::move(TheTokenLexer)),
          TheDirLookup(std::move(TheDirLookup)) {}
  };
  std::vector<IncludeStackInfo> IncludeMacroStack;


  struct MacroExpandsInfo {
    Token Tok;
    MacroDefinition MD;
    SourceRange Range;

    MacroExpandsInfo(Token Tok, MacroDefinition MD, SourceRange Range)
        : Tok(Tok), MD(MD), Range(Range) {}
  };
  SmallVector<MacroExpandsInfo, 2> DelayedMacroExpandsCallbacks;

  /// Information about a name that has been used to define a module macro.
  struct ModuleMacroInfo {
    /// The most recent macro directive for this identifier.
    MacroDirective *MD;


    /// Whether this macro name is ambiguous.
    bool IsAmbiguous = false;


  };

  /// The state of a macro for an identifier.
  class MacroState {
    MacroDirective * State;


  public:
    MacroState() : MacroState(nullptr) {}
    MacroState(MacroDirective *MD) : State(MD) {}

    MacroState(MacroState &&O) noexcept : State(O.State) {
      O.State = (MacroDirective *)nullptr;
    }

    MacroState &operator=(MacroState &&O) noexcept {
      auto S = O.State;
      O.State = (MacroDirective *)nullptr;
      State = S;
      return *this;
    }

    ~MacroState() {
    }

    MacroDirective *getLatest() const {
      return State;
    }

    void setLatest(MacroDirective *MD) {
        State = MD;
    }

    bool isAmbiguous(Preprocessor &PP, const IdentifierInfo *II) const {
      return false;
    }


    MacroDirective::DefInfo findDirectiveAtLoc(SourceLocation Loc,
                                               SourceManager &SourceMgr) const {
      // FIXME: Incorporate module macros into the result of this.
      if (auto *Latest = getLatest())
        return Latest->findDirectiveAtLoc(Loc, SourceMgr);
      return {};
    }


  };

  /// For each IdentifierInfo that was associated with a macro, we
  /// keep a mapping to the history of all macro definitions and #undefs in
  /// the reverse order (the latest one is in the head of the list).
  ///
  /// This mapping lives within the \p CurSubmoduleState.
  using MacroMap = llvm::DenseMap<const IdentifierInfo *, MacroState>;

  struct SubmoduleState;


  /// Information about a submodule's preprocessor state.
  struct SubmoduleState {
    /// The macros for the submodule.
    MacroMap Macros;


    // FIXME: CounterValue?
    // FIXME: PragmaPushMacroInfo?
  };

  /// The preprocessor state for preprocessing outside of any submodule.
  SubmoduleState NullSubmoduleState;

  /// The current submodule state. Will be \p NullSubmoduleState if we're not
  /// in a submodule.
  SubmoduleState *CurSubmoduleState;



  /// Macros that we want to warn because they are not used at the end
  /// of the translation unit.
  ///
  /// We store just their SourceLocations instead of
  /// something like MacroInfo*. The benefit of this is that when we are
  /// deserializing from PCH, we don't need to deserialize identifier & macros
  /// just so that we can report that they are unused, we just warn using
  /// the SourceLocations of this set (that will be filled by the ASTReader).
  /// We are using SmallPtrSet instead of a vector for faster removal.
  using WarnUnusedMacroLocsTy = llvm::SmallPtrSet<SourceLocation, 32>;
  WarnUnusedMacroLocsTy WarnUnusedMacroLocs;

  /// A "freelist" of MacroArg objects that can be
  /// reused for quick allocation.
  MacroArgs *MacroArgCache = nullptr;

  /// For each IdentifierInfo used in a \#pragma push_macro directive,
  /// we keep a MacroInfo stack used to restore the previous macro value.
  llvm::DenseMap<IdentifierInfo *, std::vector<MacroInfo *>>
      PragmaPushMacroInfo;

  // Various statistics we track for performance analysis.
  unsigned NumDirectives = 0;
  unsigned NumDefined = 0;
  unsigned NumUndefined = 0;
  unsigned NumIf = 0;
  unsigned NumElse = 0;
  unsigned NumEndif = 0;
  unsigned NumEnteredSourceFiles = 0;
  unsigned MaxIncludeStackDepth = 0;
  unsigned NumMacroExpanded = 0;
  unsigned NumFnMacroExpanded = 0;
  unsigned NumBuiltinMacroExpanded = 0;
  unsigned NumFastMacroExpanded = 0;
  unsigned NumTokenPaste = 0;
  unsigned NumFastTokenPaste = 0;
  unsigned NumSkipped = 0;

  /// The predefined macros that preprocessor should use from the
  /// command line etc.
  std::string Predefines;

  /// The file ID for the preprocessor predefines.
  FileID PredefinesFileID;

    /// \{
  /// Cache of macro expanders to reduce malloc traffic.
  enum { TokenLexerCacheSize = 8 };
  unsigned NumCachedTokenLexers;
  std::unique_ptr<TokenLexer> TokenLexerCache[TokenLexerCacheSize];
  /// \}

  /// Keeps macro expanded tokens for TokenLexers.
  //
  /// Works like a stack; a TokenLexer adds the macro expanded tokens that is
  /// going to lex in the cache and when it finishes the tokens are removed
  /// from the end of the cache.
  SmallVector<Token, 16> MacroExpandedTokens;
  std::vector<std::pair<TokenLexer *, size_t>> MacroExpandingLexersStack;


  /// Cached tokens state.
  using CachedTokensTy = SmallVector<Token, 1>;

  /// Cached tokens are stored here when we do backtracking or
  /// lookahead. They are "lexed" by the CachingLex() method.
  CachedTokensTy CachedTokens;

  /// The position of the cached token that CachingLex() should
  /// "lex" next.
  ///
  /// If it points beyond the CachedTokens vector, it means that a normal
  /// Lex() should be invoked.
  CachedTokensTy::size_type CachedLexPos = 0;

  /// Stack of backtrack positions, allowing nested backtracks.
  ///
  /// The EnableBacktrackAtThisPos() method pushes a position to
  /// indicate where CachedLexPos should be set when the BackTrack() method is
  /// invoked (at which point the last position is popped).
  std::vector<CachedTokensTy::size_type> BacktrackPositions;

  struct MacroInfoChain {
    MacroInfo MI;
    MacroInfoChain *Next;
  };

  /// MacroInfos are managed as a chain for easy disposal.  This is the head
  /// of that list.
  MacroInfoChain *MIChainHead = nullptr;

public:
  Preprocessor(DiagnosticsEngine &diags, SourceManager &SM,
               MemoryBufferCache &PCMCache,
               HeaderSearch &Headers,
               IdentifierInfoLookup *IILookup = nullptr,
               bool OwnsHeaderSearch = false);

  ~Preprocessor();

  /// Initialize the preprocessor using information about the target.
  ///
  /// \param Target is owned by the caller and must remain valid for the
  /// lifetime of the preprocessor.
  /// \param AuxTarget is owned by the caller and must remain valid for
  /// the lifetime of the preprocessor.
  void Initialize(const C2::TargetInfo &Target,
                  const C2::TargetInfo *AuxTarget = nullptr);

  /// Initialize the preprocessor to parse a model file
  ///
  /// To parse model files the preprocessor of the original source is reused to
  /// preserver the identifier table. However to avoid some duplicate
  /// information in the preprocessor some cleanup is needed before it is used
  /// to parse model files. This method does that cleanup.
  void InitializeForModelFile();

  /// Cleanup after model file parsing
  void FinalizeForModelFile();


  DiagnosticsEngine &getDiagnostics() const { return *Diags; }
  void setDiagnostics(DiagnosticsEngine &D) { Diags = &D; }

  const C2::TargetInfo &getTargetInfo() const { return *Target; }
  const C2::TargetInfo *getAuxTargetInfo() const { return AuxTarget; }
  FileManager &getFileManager() const { return FileMgr; }
  SourceManager &getSourceManager() const { return SourceMgr; }
  MemoryBufferCache &getPCMCache() const { return PCMCache; }
  HeaderSearch &getHeaderSearchInfo() const { return HeaderInfo; }

  IdentifierTable &getIdentifierTable() { return Identifiers; }
  const IdentifierTable &getIdentifierTable() const { return Identifiers; }
  llvm::BumpPtrAllocator &getPreprocessorAllocator() { return BP; }


    ExternalPreprocessorSource *getExternalSource() const {
    return ExternalSource;
  }


  /// True if we are currently preprocessing a #if or #elif directive
  bool isParsingIfOrElifDirective() const {
    return ParsingIfOrElifDirective;
  }


    void SetSuppressIncludeNotFoundError(bool Suppress) {
    SuppressIncludeNotFoundError = Suppress;
  }

  bool GetSuppressIncludeNotFoundError() {
    return SuppressIncludeNotFoundError;
  }

  /// Sets whether the preprocessor is responsible for producing output or if
  /// it is producing tokens to be consumed by Parse and Sema.
  void setPreprocessedOutput(bool IsPreprocessedOutput) {
    PreprocessedOutput = IsPreprocessedOutput;
  }

  /// Returns true if the preprocessor is responsible for generating output,
  /// false if it is producing tokens to be consumed by Parse and Sema.
  bool isPreprocessedOutput() const { return PreprocessedOutput; }

  /// Return true if we are lexing directly from the specified lexer.
  bool isCurrentLexer(const PreprocessorLexer *L) const {
    return CurPPLexer == L;
  }

  /// Return the current lexer being lexed from.
  ///
  /// Note that this ignores any potentially active macro expansions and _Pragma
  /// expansions going on at the time.
  PreprocessorLexer *getCurrentLexer() const { return CurPPLexer; }

  /// Return the current file lexer being lexed from.
  ///
  /// Note that this ignores any potentially active macro expansions and _Pragma
  /// expansions going on at the time.
  PreprocessorLexer *getCurrentFileLexer() const;


  bool isMacroDefined(StringRef Id) {
    return isMacroDefined(&Identifiers.get(Id));
  }
  bool isMacroDefined(const IdentifierInfo *II) {
    return II->hasMacroDefinition();
  }

  /// Determine whether II is defined as a macro within the module M,
  /// if that is a module that we've already preprocessed. Does not check for
  /// macros imported into M.
  bool isMacroDefinedInLocalModule(const IdentifierInfo *II) {
      return II->hasMacroDefinition();
  }

  MacroDefinition getMacroDefinition(const IdentifierInfo *II) {
    if (!II->hasMacroDefinition())
      return {};

    MacroState &S = CurSubmoduleState->Macros[II];
    auto *MD = S.getLatest();
    while (MD && isa<VisibilityMacroDirective>(MD))
      MD = MD->getPrevious();
    return MacroDefinition(dyn_cast_or_null<DefMacroDirective>(MD),
                           S.isAmbiguous(*this, II));
  }

  MacroDefinition getMacroDefinitionAtLoc(const IdentifierInfo *II,
                                          SourceLocation Loc) {
    if (!II->hadMacroDefinition())
      return {};

    MacroState &S = CurSubmoduleState->Macros[II];
    MacroDirective::DefInfo DI;
    if (auto *MD = S.getLatest())
      DI = MD->findDirectiveAtLoc(Loc, getSourceManager());
    // FIXME: Compute the set of active module macros at the specified location.
    return MacroDefinition(DI.getDirective(),
                           S.isAmbiguous(*this, II));
  }

  /// Given an identifier, return its latest non-imported MacroDirective
  /// if it is \#define'd and not \#undef'd, or null if it isn't \#define'd.
  MacroDirective *getLocalMacroDirective(const IdentifierInfo *II) const {
    if (!II->hasMacroDefinition())
      return nullptr;

    auto *MD = getLocalMacroDirectiveHistory(II);
    if (!MD || MD->getDefinition().isUndefined())
      return nullptr;

    return MD;
  }

  const MacroInfo *getMacroInfo(const IdentifierInfo *II) const {
    return const_cast<Preprocessor*>(this)->getMacroInfo(II);
  }

  MacroInfo *getMacroInfo(const IdentifierInfo *II) {
    if (!II->hasMacroDefinition())
      return nullptr;
    if (auto MD = getMacroDefinition(II))
      return MD.getMacroInfo();
    return nullptr;
  }

  /// Given an identifier, return the latest non-imported macro
  /// directive for that identifier.
  ///
  /// One can iterate over all previous macro directives from the most recent
  /// one.
  MacroDirective *getLocalMacroDirectiveHistory(const IdentifierInfo *II) const;

  /// Add a directive to the macro directive history for this identifier.
  void appendMacroDirective(IdentifierInfo *II, MacroDirective *MD);
  DefMacroDirective *appendDefMacroDirective(IdentifierInfo *II, MacroInfo *MI,
                                             SourceLocation Loc) {
    DefMacroDirective *MD = AllocateDefMacroDirective(MI, Loc);
    appendMacroDirective(II, MD);
    return MD;
  }
  DefMacroDirective *appendDefMacroDirective(IdentifierInfo *II,
                                             MacroInfo *MI) {
    return appendDefMacroDirective(II, MI, MI->getDefinitionLoc());
  }

  /// Set a MacroDirective that was loaded from a PCH file.
  void setLoadedMacroDirective(IdentifierInfo *II, MacroDirective *ED,
                               MacroDirective *MD);


  /// \{
  /// Iterators for the macro history table. Currently defined macros have
  /// IdentifierInfo::hasMacroDefinition() set and an empty
  /// MacroInfo::getUndefLoc() at the head of the list.
  using macro_iterator = MacroMap::const_iterator;

  macro_iterator macro_begin(bool IncludeExternalMacros = true) const;
  macro_iterator macro_end(bool IncludeExternalMacros = true) const;

  llvm::iterator_range<macro_iterator>
  macros(bool IncludeExternalMacros = true) const {
    macro_iterator begin = macro_begin(IncludeExternalMacros);
    macro_iterator end = macro_end(IncludeExternalMacros);
    return llvm::make_range(begin, end);
  }

  /// \}

    const std::string &getPredefines() const { return Predefines; }

  /// Set the predefines for this Preprocessor.
  ///
  /// These predefines are automatically injected when parsing the main file.
  void setPredefines(const char *P) { Predefines = P; }
  void setPredefines(StringRef P) { Predefines = P; }

  /// Return information about the specified preprocessor
  /// identifier token.
  IdentifierInfo *getIdentifierInfo(StringRef Name) const {
    return &Identifiers.get(Name);
  }


    /// Add the specified comment handler to the preprocessor.
  void addCommentHandler(CommentHandler *Handler);

  /// Remove the specified comment handler.
  ///
  /// It is an error to remove a handler that has not been registered.
  void removeCommentHandler(CommentHandler *Handler);




    /// Process directives while skipping until the through header is found.
  void HandleSkippedThroughHeaderDirective(Token &Result,
                                           SourceLocation HashLoc);

  /// Enter the specified FileID as the main source file,
  /// which implicitly adds the builtin defines etc.
  void EnterMainSourceFile();

  /// Inform the preprocessor callbacks that processing is complete.
  void EndSourceFile();

  /// Add a source file to the top of the include stack and
  /// start lexing tokens from it instead of the current buffer.
  ///
  /// Emits a diagnostic, doesn't enter the file, and returns true on error.
  bool EnterSourceFile(FileID CurFileID, const DirectoryLookup *Dir,
                       SourceLocation Loc);

  /// Add a Macro to the top of the include stack and start lexing
  /// tokens from it instead of the current buffer.
  ///
  /// \param Args specifies the tokens input to a function-like macro.
  /// \param ILEnd specifies the location of the ')' for a function-like macro
  /// or the identifier for an object-like macro.
  void EnterMacro(Token &Identifier, SourceLocation ILEnd, MacroInfo *Macro,
                  MacroArgs *Args);

  /// Add a "macro" context to the top of the include stack,
  /// which will cause the lexer to start returning the specified tokens.
  ///
  /// If \p DisableMacroExpansion is true, tokens lexed from the token stream
  /// will not be subject to further macro expansion. Otherwise, these tokens
  /// will be re-macro-expanded when/if expansion is enabled.
  ///
  /// If \p OwnsTokens is false, this method assumes that the specified stream
  /// of tokens has a permanent owner somewhere, so they do not need to be
  /// copied. If it is true, it assumes the array of tokens is allocated with
  /// \c new[] and the Preprocessor will delete[] it.
private:
  void EnterTokenStream(const Token *Toks, unsigned NumToks,
                        bool DisableMacroExpansion, bool OwnsTokens);

public:
  void EnterTokenStream(std::unique_ptr<Token[]> Toks, unsigned NumToks,
                        bool DisableMacroExpansion) {
    EnterTokenStream(Toks.release(), NumToks, DisableMacroExpansion, true);
  }

  void EnterTokenStream(ArrayRef<Token> Toks, bool DisableMacroExpansion) {
    EnterTokenStream(Toks.data(), Toks.size(), DisableMacroExpansion, false);
  }

  /// Pop the current lexer/macro exp off the top of the lexer stack.
  ///
  /// This should only be used in situations where the current state of the
  /// top-of-stack lexer is known.
  void RemoveTopOfLexerStack();

  /// From the point that this method is called, and until
  /// CommitBacktrackedTokens() or Backtrack() is called, the Preprocessor
  /// keeps track of the lexed tokens so that a subsequent Backtrack() call will
  /// make the Preprocessor re-lex the same tokens.
  ///
  /// Nested backtracks are allowed, meaning that EnableBacktrackAtThisPos can
  /// be called multiple times and CommitBacktrackedTokens/Backtrack calls will
  /// be combined with the EnableBacktrackAtThisPos calls in reverse order.
  ///
  /// NOTE: *DO NOT* forget to call either CommitBacktrackedTokens or Backtrack
  /// at some point after EnableBacktrackAtThisPos. If you don't, caching of
  /// tokens will continue indefinitely.
  ///
  void EnableBacktrackAtThisPos();

  /// Disable the last EnableBacktrackAtThisPos call.
  void CommitBacktrackedTokens();

  struct CachedTokensRange {
    CachedTokensTy::size_type Begin, End;
  };

private:
  /// A range of cached tokens that should be erased after lexing
  /// when backtracking requires the erasure of such cached tokens.
  Optional<CachedTokensRange> CachedTokenRangeToErase;

public:
  /// Returns the range of cached tokens that were lexed since
  /// EnableBacktrackAtThisPos() was previously called.
  CachedTokensRange LastCachedTokenRange();

  /// Erase the range of cached tokens that were lexed since
  /// EnableBacktrackAtThisPos() was previously called.
  void EraseCachedTokens(CachedTokensRange TokenRange);

  /// Make Preprocessor re-lex the tokens that were lexed since
  /// EnableBacktrackAtThisPos() was previously called.
  void Backtrack();

  /// True if EnableBacktrackAtThisPos() was called and
  /// caching of tokens is on.
  bool isBacktrackEnabled() const { return !BacktrackPositions.empty(); }

  /// Lex the next token for this preprocessor.
  void Lex(Token &Result);


    /// Lex a string literal, which may be the concatenation of multiple
  /// string literals and may even come from macro expansion.
  /// \returns true on success, false if a error diagnostic has been generated.
  bool LexStringLiteral(Token &Result, std::string &String,
                        const char *DiagnosticTag, bool AllowMacroExpansion) {
    if (AllowMacroExpansion)
      Lex(Result);
    else
      LexUnexpandedToken(Result);
    return FinishLexStringLiteral(Result, String, DiagnosticTag,
                                  AllowMacroExpansion);
  }

  /// Complete the lexing of a string literal where the first token has
  /// already been lexed (see LexStringLiteral).
  bool FinishLexStringLiteral(Token &Result, std::string &String,
                              const char *DiagnosticTag,
                              bool AllowMacroExpansion);

  /// Lex a token.  If it's a comment, keep lexing until we get
  /// something not a comment.
  ///
  /// This is useful in -E -C mode where comments would foul up preprocessor
  /// directive handling.
  void LexNonComment(Token &Result) {
    do
      Lex(Result);
    while (Result.getKind() == tok::comment);
  }

  /// Just like Lex, but disables macro expansion of identifier tokens.
  void LexUnexpandedToken(Token &Result) {
    // Disable macro expansion.
    bool OldVal = DisableMacroExpansion;
    DisableMacroExpansion = true;
    // Lex the token.
    Lex(Result);

    // Reenable it.
    DisableMacroExpansion = OldVal;
  }

  /// Like LexNonComment, but this disables macro expansion of
  /// identifier tokens.
  void LexUnexpandedNonComment(Token &Result) {
    do
      LexUnexpandedToken(Result);
    while (Result.getKind() == tok::comment);
  }

  /// Parses a simple integer literal to get its numeric value.  Floating
  /// point literals and user defined literals are rejected.  Used primarily to
  /// handle pragmas that accept integer arguments.
  bool parseSimpleIntegerLiteral(Token &Tok, uint64_t &Value);

  /// Disables macro expansion everywhere except for preprocessor directives.
  void SetMacroExpansionOnlyInDirectives() {
    DisableMacroExpansion = true;
    MacroExpansionInDirectivesOverride = true;
  }

  /// Peeks ahead N tokens and returns that token without consuming any
  /// tokens.
  ///
  /// LookAhead(0) returns the next token that would be returned by Lex(),
  /// LookAhead(1) returns the token after it, etc.  This returns normal
  /// tokens after phase 5.  As such, it is equivalent to using
  /// 'Lex', not 'LexUnexpandedToken'.
  const Token &LookAhead(unsigned N) {
    if (CachedLexPos + N < CachedTokens.size())
      return CachedTokens[CachedLexPos+N];
    else
      return PeekAhead(N+1);
  }

  /// When backtracking is enabled and tokens are cached,
  /// this allows to revert a specific number of tokens.
  ///
  /// Note that the number of tokens being reverted should be up to the last
  /// backtrack position, not more.
  void RevertCachedTokens(unsigned N) {
    assert(isBacktrackEnabled() &&
           "Should only be called when tokens are cached for backtracking");
    assert(signed(CachedLexPos) - signed(N) >= signed(BacktrackPositions.back())
         && "Should revert tokens up to the last backtrack position, not more");
    assert(signed(CachedLexPos) - signed(N) >= 0 &&
           "Corrupted backtrack positions ?");
    CachedLexPos -= N;
  }

  /// Enters a token in the token stream to be lexed next.
  ///
  /// If BackTrack() is called afterwards, the token will remain at the
  /// insertion point.
  void EnterToken(const Token &Tok) {
    EnterCachingLexMode();
    CachedTokens.insert(CachedTokens.begin()+CachedLexPos, Tok);
  }


    /// Get the location of the last cached token, suitable for setting the end
  /// location of an annotation token.
  SourceLocation getLastCachedTokenLocation() const {
    assert(CachedLexPos != 0);
    return CachedTokens[CachedLexPos-1].getLastLoc();
  }

  /// Whether \p Tok is the most recent token (`CachedLexPos - 1`) in
  /// CachedTokens.
  bool IsPreviousCachedToken(const Token &Tok) const;

  /// Replace token in `CachedLexPos - 1` in CachedTokens by the tokens
  /// in \p NewToks.
  ///
  /// Useful when a token needs to be split in smaller ones and CachedTokens
  /// most recent token must to be updated to reflect that.
  void ReplacePreviousCachedToken(ArrayRef<Token> NewToks);


    /// Update the current token to represent the provided
  /// identifier, in order to cache an action performed by typo correction.
  void TypoCorrectToken(const Token &Tok) {
    assert(Tok.getIdentifierInfo() && "Expected identifier token");
    if (CachedLexPos != 0 && isBacktrackEnabled())
      CachedTokens[CachedLexPos-1] = Tok;
  }

  /// Recompute the current lexer kind based on the CurLexer/CurPTHLexer/
  /// CurTokenLexer pointers.
  void recomputeCurLexerKind();

  /// Returns true if incremental processing is enabled
  bool isIncrementalProcessingEnabled() const { return IncrementalProcessing; }

  /// Enables the incremental processing
  void enableIncrementalProcessing(bool value = true) {
    IncrementalProcessing = value;
  }




  /// Set the directory in which the main file should be considered
  /// to have been found, if it is not a real file.
  void setMainFileDir(const DirectoryEntry *Dir) {
    MainFileDir = Dir;
  }


    /// Forwarding function for diagnostics.  This emits a diagnostic at
  /// the specified Token's location, translating the token's start
  /// position in the current buffer into a SourcePosition object for rendering.
  DiagnosticBuilder Diag(SourceLocation Loc, unsigned DiagID) const {
    return Diags->Report(Loc, DiagID);
  }

  DiagnosticBuilder Diag(const Token &Tok, unsigned DiagID) const {
    return Diags->Report(Tok.getLocation(), DiagID);
  }

  /// Return the 'spelling' of the token at the given
  /// location; does not go up to the spelling location or down to the
  /// expansion location.
  ///
  /// \param buffer A buffer which will be used only if the token requires
  ///   "cleaning", e.g. if it contains trigraphs or escaped newlines
  /// \param invalid If non-null, will be set \c true if an error occurs.
  StringRef getSpelling(SourceLocation loc,
                        SmallVectorImpl<char> &buffer,
                        bool *invalid = nullptr) const {
    return Lexer::getSpelling(loc, buffer, SourceMgr, invalid);
  }

  /// Return the 'spelling' of the Tok token.
  ///
  /// The spelling of a token is the characters used to represent the token in
  /// the source file after trigraph expansion and escaped-newline folding.  In
  /// particular, this wants to get the true, uncanonicalized, spelling of
  /// things like digraphs, UCNs, etc.
  ///
  /// \param Invalid If non-null, will be set \c true if an error occurs.
  std::string getSpelling(const Token &Tok, bool *Invalid = nullptr) const {
    return Lexer::getSpelling(Tok, SourceMgr, Invalid);
  }

  /// Get the spelling of a token into a preallocated buffer, instead
  /// of as an std::string.
  ///
  /// The caller is required to allocate enough space for the token, which is
  /// guaranteed to be at least Tok.getLength() bytes long. The length of the
  /// actual result is returned.
  ///
  /// Note that this method may do two possible things: it may either fill in
  /// the buffer specified with characters, or it may *change the input pointer*
  /// to point to a constant buffer with the data already in it (avoiding a
  /// copy).  The caller is not allowed to modify the returned buffer pointer
  /// if an internal buffer is returned.
  unsigned getSpelling(const Token &Tok, const char *&Buffer,
                       bool *Invalid = nullptr) const {
    return Lexer::getSpelling(Tok, Buffer, SourceMgr, Invalid);
  }

  /// Get the spelling of a token into a SmallVector.
  ///
  /// Note that the returned StringRef may not point to the
  /// supplied buffer if a copy can be avoided.
  StringRef getSpelling(const Token &Tok,
                        SmallVectorImpl<char> &Buffer,
                        bool *Invalid = nullptr) const;

  /// Relex the token at the specified location.
  /// \returns true if there was a failure, false on success.
  bool getRawToken(SourceLocation Loc, Token &Result,
                   bool IgnoreWhiteSpace = false) {
    return Lexer::getRawToken(Loc, Result, SourceMgr, IgnoreWhiteSpace);
  }

  /// Given a Token \p Tok that is a numeric constant with length 1,
  /// return the character.
  char
  getSpellingOfSingleCharacterNumericConstant(const Token &Tok,
                                              bool *Invalid = nullptr) const {
    assert(Tok.is(tok::numeric_constant) &&
           Tok.getLength() == 1 && "Called on unsupported token");
    assert(!Tok.needsCleaning() && "Token can't need cleaning with length 1");

    // If the token is carrying a literal data pointer, just use it.
    if (const char *D = Tok.getLiteralData())
      return *D;

    // Otherwise, fall back on getCharacterData, which is slower, but always
    // works.
    return *SourceMgr.getCharacterData(Tok.getLocation(), Invalid);
  }

  /// Retrieve the name of the immediate macro expansion.
  ///
  /// This routine starts from a source location, and finds the name of the
  /// macro responsible for its immediate expansion. It looks through any
  /// intervening macro argument expansions to compute this. It returns a
  /// StringRef that refers to the SourceManager-owned buffer of the source
  /// where that macro name is spelled. Thus, the result shouldn't out-live
  /// the SourceManager.
  StringRef getImmediateMacroName(SourceLocation Loc) {
    return Lexer::getImmediateMacroName(Loc, SourceMgr);
  }

  /// Plop the specified string into a scratch buffer and set the
  /// specified token's location and length to it.
  ///
  /// If specified, the source location provides a location of the expansion
  /// point of the token.
  void CreateString(StringRef Str, Token &Tok,
                    SourceLocation ExpansionLocStart = SourceLocation(),
                    SourceLocation ExpansionLocEnd = SourceLocation());

  /// Split the first Length characters out of the token starting at TokLoc
  /// and return a location pointing to the split token. Re-lexing from the
  /// split token will return the split token rather than the original.
  SourceLocation SplitToken(SourceLocation TokLoc, unsigned Length);

  /// Computes the source location just past the end of the
  /// token at this source location.
  ///
  /// This routine can be used to produce a source location that
  /// points just past the end of the token referenced by \p Loc, and
  /// is generally used when a diagnostic needs to point just after a
  /// token where it expected something different that it received. If
  /// the returned source location would not be meaningful (e.g., if
  /// it points into a macro), this routine returns an invalid
  /// source location.
  ///
  /// \param Offset an offset from the end of the token, where the source
  /// location should refer to. The default offset (0) produces a source
  /// location pointing just past the end of the token; an offset of 1 produces
  /// a source location pointing to the last character in the token, etc.
  SourceLocation getLocForEndOfToken(SourceLocation Loc, unsigned Offset = 0) {
    return Lexer::getLocForEndOfToken(Loc, Offset, SourceMgr);
  }

  /// Returns true if the given MacroID location points at the first
  /// token of the macro expansion.
  ///
  /// \param MacroBegin If non-null and function returns true, it is set to
  /// begin location of the macro.
  bool isAtStartOfMacroExpansion(SourceLocation loc,
                                 SourceLocation *MacroBegin = nullptr) const {
    return Lexer::isAtStartOfMacroExpansion(loc, SourceMgr,
                                            MacroBegin);
  }

  /// Returns true if the given MacroID location points at the last
  /// token of the macro expansion.
  ///
  /// \param MacroEnd If non-null and function returns true, it is set to
  /// end location of the macro.
  bool isAtEndOfMacroExpansion(SourceLocation loc,
                               SourceLocation *MacroEnd = nullptr) const {
    return Lexer::isAtEndOfMacroExpansion(loc, SourceMgr, MacroEnd);
  }

  /// Print the token to stderr, used for debugging.
  void DumpToken(const Token &Tok, bool DumpFlags = false) const;
  void DumpLocation(SourceLocation Loc) const;
  void DumpMacro(const MacroInfo &MI) const;
  void dumpMacroInfo(const IdentifierInfo *II);

  /// Given a location that specifies the start of a
  /// token, return a new location that specifies a character within the token.
  SourceLocation AdvanceToTokenCharacter(SourceLocation TokStart,
                                         unsigned Char) const {
    return Lexer::AdvanceToTokenCharacter(TokStart, Char, SourceMgr);
  }

  /// Increment the counters for the number of token paste operations
  /// performed.
  ///
  /// If fast was specified, this is a 'fast paste' case we handled.
  void IncrementPasteCounter(bool isFast) {
    if (isFast)
      ++NumFastTokenPaste;
    else
      ++NumTokenPaste;
  }

  void PrintStats();

  size_t getTotalMemory() const;

  /// When the macro expander pastes together a comment (/##/) in Microsoft
  /// mode, this method handles updating the current state, returning the
  /// token on the next source line.
  void HandleMicrosoftCommentPaste(Token &Tok);

  //===--------------------------------------------------------------------===//
  // Preprocessor callback methods.  These are invoked by a lexer as various
  // directives and events are found.

  /// Given a tok::raw_identifier token, look up the
  /// identifier information for the token and install it into the token,
  /// updating the token kind accordingly.
  IdentifierInfo *LookUpIdentifierInfo(Token &Identifier) const;

private:
  llvm::DenseMap<IdentifierInfo*,unsigned> PoisonReasons;

public:
  /// Specifies the reason for poisoning an identifier.
  ///
  /// If that identifier is accessed while poisoned, then this reason will be
  /// used instead of the default "poisoned" diagnostic.
  void SetPoisonReason(IdentifierInfo *II, unsigned DiagID);

  /// Display reason for poisoned identifier.
  void HandlePoisonedIdentifier(Token & Tok);

  void MaybeHandlePoisonedIdentifier(Token & Identifier) {
    if(IdentifierInfo * II = Identifier.getIdentifierInfo()) {
      if(II->isPoisoned()) {
        HandlePoisonedIdentifier(Identifier);
      }
    }
  }

private:
  /// Identifiers used for SEH handling in Borland. These are only
  /// allowed in particular circumstances
  // __except block
  IdentifierInfo *Ident__exception_code,
                 *Ident___exception_code,
                 *Ident_GetExceptionCode;
  // __except filter expression
  IdentifierInfo *Ident__exception_info,
                 *Ident___exception_info,
                 *Ident_GetExceptionInfo;
  // __finally
  IdentifierInfo *Ident__abnormal_termination,
                 *Ident___abnormal_termination,
                 *Ident_AbnormalTermination;

  const char *getCurLexerEndPos();

public:
  void PoisonSEHIdentifiers(bool Poison = true); // Borland

  /// Callback invoked when the lexer reads an identifier and has
  /// filled in the tokens IdentifierInfo member.
  ///
  /// This callback potentially macro expands it or turns it into a named
  /// token (like 'for').
  ///
  /// \returns true if we actually computed a token, false if we need to
  /// lex again.
  bool HandleIdentifier(Token &Identifier);

  /// Callback invoked when the lexer hits the end of the current file.
  ///
  /// This either returns the EOF token and returns true, or
  /// pops a level off the include stack and returns false, at which point the
  /// client should call lex again.
  bool HandleEndOfFile(Token &Result, bool isEndOfMacro = false);

  /// Callback invoked when the current TokenLexer hits the end of its
  /// token stream.
  bool HandleEndOfTokenLexer(Token &Result);

  /// Callback invoked when the lexer sees a # token at the start of a
  /// line.
  ///
  /// This consumes the directive, modifies the lexer/preprocessor state, and
  /// advances the lexer(s) so that the next token read is the correct one.
  void HandleDirective(Token &Result);

  /// Ensure that the next token is a tok::eod token.
  ///
  /// If not, emit a diagnostic and consume up until the eod.
  /// If \p EnableMacros is true, then we consider macros that expand to zero
  /// tokens as being ok.
  void CheckEndOfDirective(const char *Directive, bool EnableMacros = false);

  /// Read and discard all tokens remaining on the current line until
  /// the tok::eod token is found.
  void DiscardUntilEndOfDirective();

  /// Returns true if the preprocessor has seen a use of
  /// __DATE__ or __TIME__ in the file so far.
  bool SawDateOrTime() const {
    return DATELoc != SourceLocation() || TIMELoc != SourceLocation();
  }
  unsigned getCounterValue() const { return CounterValue; }
  void setCounterValue(unsigned V) { CounterValue = V; }


    /// Allocate a new MacroInfo object with the provided SourceLocation.
  MacroInfo *AllocateMacroInfo(SourceLocation L);

  /// Turn the specified lexer token into a fully checked and spelled
  /// filename, e.g. as an operand of \#include.
  ///
  /// The caller is expected to provide a buffer that is large enough to hold
  /// the spelling of the filename, but is also expected to handle the case
  /// when this method decides to use a different buffer.
  ///
  /// \returns true if the input filename was in <>'s or false if it was
  /// in ""'s.
  bool GetIncludeFilenameSpelling(SourceLocation Loc,StringRef &Filename);

  /// Given a "foo" or \<foo> reference, look up the indicated file.
  ///
  /// Returns null on failure.  \p isAngled indicates whether the file
  /// reference is for system \#include's or not (i.e. using <> instead of "").
  const FileEntry *LookupFile(SourceLocation FilenameLoc, StringRef Filename,
                              bool isAngled, const DirectoryLookup *FromDir,
                              const FileEntry *FromFile,
                              const DirectoryLookup *&CurDir,
                              SmallVectorImpl<char> *SearchPath,
                              SmallVectorImpl<char> *RelativePath,
                              bool *IsMapped, bool SkipCache = false);

  /// Get the DirectoryLookup structure used to find the current
  /// FileEntry, if CurLexer is non-null and if applicable.
  ///
  /// This allows us to implement \#include_next and find directory-specific
  /// properties.
  const DirectoryLookup *GetCurDirLookup() { return CurDirLookup; }

  /// Return true if we're in the top-level file, not in a \#include.
  bool isInPrimaryFile() const;

  /// Handle cases where the \#include name is expanded
  /// from a macro as multiple tokens, which need to be glued together.
  ///
  /// This occurs for code like:
  /// \code
  ///    \#define FOO <x/y.h>
  ///    \#include FOO
  /// \endcode
  /// because in this case, "<x/y.h>" is returned as 7 tokens, not one.
  ///
  /// This code concatenates and consumes tokens up to the '>' token.  It
  /// returns false if the > was found, otherwise it returns true if it finds
  /// and consumes the EOD marker.
  bool ConcatenateIncludeName(SmallString<128> &FilenameBuffer,
                              SourceLocation &End);

  /// Lex an on-off-switch (C99 6.10.6p2) and verify that it is
  /// followed by EOD.  Return true if the token is not a valid on-off-switch.
  bool LexOnOffSwitch(tok::OnOffSwitch &OOS);

  bool CheckMacroName(Token &MacroNameTok, MacroUse isDefineUndef,
                      bool *ShadowFlag = nullptr);


private:
  friend void TokenLexer::ExpandFunctionArguments();

  void PushIncludeMacroStack() {
    assert(CurLexerKind != CLK_CachingLexer && "cannot push a caching lexer");
    IncludeMacroStack.emplace_back(CurLexerKind,
                                   std::move(CurLexer),
                                   CurPPLexer, std::move(CurTokenLexer),
                                   CurDirLookup);
    CurPPLexer = nullptr;
  }

  void PopIncludeMacroStack() {
    CurLexer = std::move(IncludeMacroStack.back().TheLexer);
    CurPPLexer = IncludeMacroStack.back().ThePPLexer;
    CurTokenLexer = std::move(IncludeMacroStack.back().TheTokenLexer);
    CurDirLookup  = IncludeMacroStack.back().TheDirLookup;
    CurLexerKind = IncludeMacroStack.back().CurLexerKind;
    IncludeMacroStack.pop_back();
  }

  void PropagateLineStartLeadingSpaceInfo(Token &Result);


  DefMacroDirective *AllocateDefMacroDirective(MacroInfo *MI,
                                               SourceLocation Loc);
  UndefMacroDirective *AllocateUndefMacroDirective(SourceLocation UndefLoc);
  VisibilityMacroDirective *AllocateVisibilityMacroDirective(SourceLocation Loc,
                                                             bool isPublic);

  /// Lex and validate a macro name, which occurs after a
  /// \#define or \#undef.
  ///
  /// \param MacroNameTok Token that represents the name defined or undefined.
  /// \param IsDefineUndef Kind if preprocessor directive.
  /// \param ShadowFlag Points to flag that is set if macro name shadows
  ///                   a keyword.
  ///
  /// This emits a diagnostic, sets the token kind to eod,
  /// and discards the rest of the macro line if the macro name is invalid.
  void ReadMacroName(Token &MacroNameTok, MacroUse IsDefineUndef = MU_Other,
                     bool *ShadowFlag = nullptr);

  /// ReadOptionalMacroParameterListAndBody - This consumes all (i.e. the
  /// entire line) of the macro's tokens and adds them to MacroInfo, and while
  /// doing so performs certain validity checks including (but not limited to):
  ///   - # (stringization) is followed by a macro parameter
  /// \param MacroNameTok - Token that represents the macro name
  /// \param ImmediatelyAfterHeaderGuard - Macro follows an #ifdef header guard
  ///
  ///  Either returns a pointer to a MacroInfo object OR emits a diagnostic and
  ///  returns a nullptr if an invalid sequence of tokens is encountered.
  MacroInfo *ReadOptionalMacroParameterListAndBody(
      const Token &MacroNameTok, bool ImmediatelyAfterHeaderGuard);

  /// The ( starting an argument list of a macro definition has just been read.
  /// Lex the rest of the parameters and the closing ), updating \p MI with
  /// what we learn and saving in \p LastTok the last token read.
  /// Return true if an error occurs parsing the arg list.
  bool ReadMacroParameterList(MacroInfo *MI, Token& LastTok);

  /// We just read a \#if or related directive and decided that the
  /// subsequent tokens are in the \#if'd out portion of the
  /// file.  Lex the rest of the file, until we see an \#endif.  If \p
  /// FoundNonSkipPortion is true, then we have already emitted code for part of
  /// this \#if directive, so \#else/\#elif blocks should never be entered. If
  /// \p FoundElse is false, then \#else directives are ok, if not, then we have
  /// already seen one so a \#else directive is a duplicate.  When this returns,
  /// the caller can lex the first valid token.
  void SkipExcludedConditionalBlock(SourceLocation HashTokenLoc,
                                    SourceLocation IfTokenLoc,
                                    bool FoundNonSkipPortion, bool FoundElse,
                                    SourceLocation ElseLoc = SourceLocation());

    /// Information about the result for evaluating an expression for a
  /// preprocessor directive.
  struct DirectiveEvalResult {
    /// Whether the expression was evaluated as true or not.
    bool Conditional;

    /// True if the expression contained identifiers that were undefined.
    bool IncludedUndefinedIds;
  };

  /// Evaluate an integer constant expression that may occur after a
  /// \#if or \#elif directive and return a \p DirectiveEvalResult object.
  ///
  /// If the expression is equivalent to "!defined(X)" return X in IfNDefMacro.
  DirectiveEvalResult EvaluateDirectiveExpression(IdentifierInfo *&IfNDefMacro);

    /// Register builtin macros such as __LINE__ with the identifier table.
  void RegisterBuiltinMacros();

  /// If an identifier token is read that is to be expanded as a macro, handle
  /// it and return the next token as 'Tok'.  If we lexed a token, return true;
  /// otherwise the caller should lex again.
  bool HandleMacroExpandedIdentifier(Token &Tok, const MacroDefinition &MD);

  /// Cache macro expanded tokens for TokenLexers.
  //
  /// Works like a stack; a TokenLexer adds the macro expanded tokens that is
  /// going to lex in the cache and when it finishes the tokens are removed
  /// from the end of the cache.
  Token *cacheMacroExpandedTokens(TokenLexer *tokLexer,
                                  ArrayRef<Token> tokens);

  void removeCachedMacroExpandedTokensOfLastLexer();

  /// Determine whether the next preprocessor token to be
  /// lexed is a '('.  If so, consume the token and return true, if not, this
  /// method should have no observable side-effect on the lexed tokens.
  bool isNextPPTokenLParen();

  /// After reading "MACRO(", this method is invoked to read all of the formal
  /// arguments specified for the macro invocation.  Returns null on error.
  MacroArgs *ReadMacroCallArgumentList(Token &MacroName, MacroInfo *MI,
                                       SourceLocation &ExpansionEnd);

  /// If an identifier token is read that is to be expanded
  /// as a builtin macro, handle it and return the next token as 'Tok'.
  void ExpandBuiltinMacro(Token &Tok);
  /// Add a lexer to the top of the include stack and
  /// start lexing tokens from it instead of the current buffer.
  void EnterSourceFileWithLexer(Lexer *TheLexer, const DirectoryLookup *Dir);

public:
  /// Set the FileID for the preprocessor predefines.
  void setPredefinesFileID(FileID FID) {
    assert(PredefinesFileID.isInvalid() && "PredefinesFileID already set!");
    PredefinesFileID = FID;
  }
private:

    /// Returns true if we are lexing from a file and not a
  /// pragma or a macro.
  static bool IsFileLexer(const Lexer* L, const PreprocessorLexer* P) {
    return L ? true : P != nullptr;
  }

  static bool IsFileLexer(const IncludeStackInfo& I) {
    return IsFileLexer(I.TheLexer.get(), I.ThePPLexer);
  }

  bool IsFileLexer() const {
    return IsFileLexer(CurLexer.get(), CurPPLexer);
  }

  //===--------------------------------------------------------------------===//
  // Caching stuff.
  void CachingLex(Token &Result);

  bool InCachingLexMode() const {
    // If the Lexer pointers are 0 and IncludeMacroStack is empty, it means
    // that we are past EOF, not that we are in CachingLex mode.
    return !CurPPLexer && !CurTokenLexer &&
           !IncludeMacroStack.empty();
  }

  void EnterCachingLexMode();

  void ExitCachingLexMode() {
    if (InCachingLexMode())
      RemoveTopOfLexerStack();
  }

  const Token &PeekAhead(unsigned N);

    //===--------------------------------------------------------------------===//
  /// Handle*Directive - implement the various preprocessor directives.  These
  /// should side-effect the current preprocessor object so that the next call
  /// to Lex() will return the appropriate token next.
  void HandleLineDirective();
  void HandleDigitDirective(Token &Tok);
  void HandleUserDiagnosticDirective(Token &Tok, bool isWarning);
  void HandleIdentSCCSDirective(Token &Tok);
  void HandleMacroPublicDirective(Token &Tok);
  void HandleMacroPrivateDirective();

  // File inclusion.
  void HandleIncludeDirective(SourceLocation HashLoc,
                              Token &Tok,
                              const DirectoryLookup *LookupFrom = nullptr,
                              const FileEntry *LookupFromFile = nullptr,
                              bool isImport = false);
  void HandleIncludeNextDirective(SourceLocation HashLoc, Token &Tok);
  void HandleIncludeMacrosDirective(SourceLocation HashLoc, Token &Tok);

public:



private:

  // Macro handling.
  void HandleDefineDirective(Token &Tok, bool ImmediatelyAfterTopLevelIfndef);
  void HandleUndefDirective();

  // Conditional Inclusion.
  void HandleIfdefDirective(Token &Tok, const Token &HashToken,
                            bool isIfndef, bool ReadAnyTokensBeforeDirective);
  void HandleIfDirective(Token &Tok, const Token &HashToken,
                         bool ReadAnyTokensBeforeDirective);
  void HandleEndifDirective(Token &Tok);
  void HandleElseDirective(Token &Tok, const Token &HashToken);
  void HandleElifDirective(Token &Tok, const Token &HashToken);

public:

  // Return true and store the first token only if any CommentHandler
  // has inserted some tokens and getCommentRetentionState() is false.
  bool HandleComment(Token &Token, SourceRange Comment);

  /// A macro is used, update information about macros that need unused
  /// warnings.
  void markMacroAsUsed(MacroInfo *MI);
};

/// Abstract base class that describes a handler that will receive
/// source ranges for each of the comments encountered in the source file.
class CommentHandler {
public:
  virtual ~CommentHandler();

  // The handler shall return true if it has pushed any tokens
  // to be read using e.g. EnterToken or EnterTokenStream.
  virtual bool HandleComment(Preprocessor &PP, SourceRange Comment) = 0;
};


} // namespace c2lang

#endif // LLVM_CLANG_LEX_PREPROCESSOR_H
