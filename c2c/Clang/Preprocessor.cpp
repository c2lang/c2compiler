//===- Preprocess.cpp - C Language Family Preprocessor Implementation -----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements the Preprocessor interface.
//
//===----------------------------------------------------------------------===//
//
// Options to support:
//   -H       - Print the name of each header file used.
//   -d[DNI] - Dump various things.
//   -fworking-directory - #line's with preprocessor's working dir.
//   -fpreprocessed
//   -dependency-file,-M,-MM,-MF,-MG,-MP,-MT,-MQ,-MD,-MMD
//   -W*
//   -w
//
// Messages to emit:
//   "Multiple include guards may be useful for:\n"
//
//===----------------------------------------------------------------------===//

#include "Utils/Errors.h"
#include "Clang/Preprocessor.h"
#include "Clang/FileManager.h"
#include "Clang/FileSystemStatCache.h"
#include "Clang/IdentifierTable.h"
#include "Clang/LLVM.h"
#include "Clang/SourceLocation.h"
#include "Clang/SourceManager.h"
#include "Clang/CodeCompletionHandler.h"
#include "Clang/HeaderSearch.h"
#include "Clang/LexDiagnostic.h"
#include "Clang/Lexer.h"
#include "Clang/LiteralSupport.h"
#include "Clang/MacroArgs.h"
#include "Clang/MacroInfo.h"
#include "Clang/PreprocessorLexer.h"
#include "Clang/PreprocessorOptions.h"
#include "Clang/ScratchBuffer.h"
#include "Clang/Token.h"
#include "Clang/TokenLexer.h"
#include "Clang/CharInfo.h"
#include "Clang/FileManager.h"
#include "Clang/TokenKinds.h"
#include "Clang/VariadicMacroSupport.h"

#include <llvm/ADT/APInt.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSwitch.h>
#include <llvm/Support/Capacity.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <cstring>
#include <new>

#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/StringSwitch.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/AlignOf.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/Path.h>

using namespace c2lang;



Preprocessor::Preprocessor(std::shared_ptr<PreprocessorOptions> PPOpts,
                           DiagnosticsEngine &diags,
                           SourceManager &SM,
                           MemoryBufferCache &PCMCache,
                           HeaderSearch &Headers,
                           bool OwnsHeaders)
    : PPOpts(std::move(PPOpts)), Diags(&diags),
      SourceMgr(SM), PCMCache(PCMCache),
      ScratchBuf(new ScratchBuffer(SourceMgr)), HeaderInfo(Headers),
      // As the language options may have not been loaded yet (when
      // deserializing an ASTUnit), adding keywords to the identifier table is
      // deferred to Preprocessor::Initialize().
      Identifiers(),
      SkipMainFilePreamble(0, true),
      CurSubmoduleState(&NullSubmoduleState) {
  OwnsHeaderSearch = OwnsHeaders;

  // Default to discarding comments.
  KeepComments = false;
  KeepMacroComments = false;
  SuppressIncludeNotFoundError = false;

  // Macro expansion is enabled.
  DisableMacroExpansion = false;
    InMacroArgs = false;
  InMacroArgPreExpansion = false;
  NumCachedTokenLexers = 0;
  ParsingIfOrElifDirective = false;
  PreprocessedOutput = false;


  // "Poison" __VA_ARGS__, __VA_OPT__ which can only appear in the expansion of
  // a macro. They get unpoisoned where it is allowed.
  (Ident__VA_ARGS__ = getIdentifierInfo("__VA_ARGS__"))->setIsPoisoned();
  SetPoisonReason(Ident__VA_ARGS__,diag::ext_pp_bad_vaargs_use);
  Ident__VA_OPT__ = nullptr;


  // Initialize builtin macros like __LINE__ and friends.
  RegisterBuiltinMacros();

  {
    Ident__exception_info = Ident__exception_code = nullptr;
    Ident__abnormal_termination = Ident___exception_info = nullptr;
    Ident___exception_code = Ident___abnormal_termination = nullptr;
    Ident_GetExceptionInfo = Ident_GetExceptionCode = nullptr;
    Ident_AbnormalTermination = nullptr;
  }


  if (this->PPOpts->GeneratePreamble)
    PreambleConditionalStack.startRecording();
}

Preprocessor::~Preprocessor() {
  assert(BacktrackPositions.empty() && "EnableBacktrack/Backtrack imbalance!");

  IncludeMacroStack.clear();

  // Destroy any macro definitions.
  while (MacroInfoChain *I = MIChainHead) {
    MIChainHead = I->Next;
    I->~MacroInfoChain();
  }

  // Free any cached macro expanders.
  // This populates MacroArgCache, so all TokenLexers need to be destroyed
  // before the code below that frees up the MacroArgCache list.
  std::fill(TokenLexerCache, TokenLexerCache + NumCachedTokenLexers, nullptr);
  CurTokenLexer.reset();

  // Free any cached MacroArgs.
  for (MacroArgs *ArgList = MacroArgCache; ArgList;)
    ArgList = ArgList->deallocate();

  // Delete the header search info, if we own it.
  if (OwnsHeaderSearch)
    delete &HeaderInfo;
}

void Preprocessor::Initialize(const C2::TargetInfo &Target,
                              const C2::TargetInfo *AuxTarget) {
  assert((!this->Target || this->Target == &Target) &&
         "Invalid override of target information");
  this->Target = &Target;

  assert((!this->AuxTarget || this->AuxTarget == AuxTarget) &&
         "Invalid override of aux target information.");
  this->AuxTarget = AuxTarget;


  // Populate the identifier table with info about keywords for the current language.
  Identifiers.AddKeywords();
}


void Preprocessor::DumpToken(const Token &Tok, bool DumpFlags) const {
  llvm::errs() << tok::getTokenName(Tok.getKind()) << " '"
               << getSpelling(Tok) << "'";

  if (!DumpFlags) return;

  llvm::errs() << "\t";
  if (Tok.isAtStartOfLine())
    llvm::errs() << " [StartOfLine]";
  if (Tok.hasLeadingSpace())
    llvm::errs() << " [LeadingSpace]";
  if (Tok.isExpandDisabled())
    llvm::errs() << " [ExpandDisabled]";
  if (Tok.needsCleaning()) {
    const char *Start = SourceMgr.getCharacterData(Tok.getLocation());
    llvm::errs() << " [UnClean='" << StringRef(Start, Tok.getLength())
                 << "']";
  }

  llvm::errs() << "\tLoc=<";
  DumpLocation(Tok.getLocation());
  llvm::errs() << ">";
}

void Preprocessor::DumpLocation(SourceLocation Loc) const {
  Loc.dump(SourceMgr);
}

void Preprocessor::DumpMacro(const MacroInfo &MI) const {
  llvm::errs() << "MACRO: ";
  for (unsigned i = 0, e = MI.getNumTokens(); i != e; ++i) {
    DumpToken(MI.getReplacementToken(i));
    llvm::errs() << "  ";
  }
  llvm::errs() << "\n";
}

void Preprocessor::PrintStats() {
  llvm::errs() << "\n*** Preprocessor Stats:\n";
  llvm::errs() << NumDirectives << " directives found:\n";
  llvm::errs() << "  " << NumDefined << " #define.\n";
  llvm::errs() << "  " << NumUndefined << " #undef.\n";
  llvm::errs() << "  #include/#include_next/#import:\n";
  llvm::errs() << "    " << NumEnteredSourceFiles << " source files entered.\n";
  llvm::errs() << "    " << MaxIncludeStackDepth << " max include stack depth\n";
  llvm::errs() << "  " << NumIf << " #if/#ifndef/#ifdef.\n";
  llvm::errs() << "  " << NumElse << " #else/#elif.\n";
  llvm::errs() << "  " << NumEndif << " #endif.\n";
  llvm::errs() << NumSkipped << " #if/#ifndef#ifdef regions skipped\n";

  llvm::errs() << NumMacroExpanded << "/" << NumFnMacroExpanded << "/"
             << NumBuiltinMacroExpanded << " obj/fn/builtin macros expanded, "
             << NumFastMacroExpanded << " on the fast path.\n";
  llvm::errs() << (NumFastTokenPaste+NumTokenPaste)
             << " token paste (##) operations performed, "
             << NumFastTokenPaste << " on the fast path.\n";

  llvm::errs() << "\nPreprocessor Memory: " << getTotalMemory() << "B total";

  llvm::errs() << "\n  BumpPtr: " << BP.getTotalMemory();
  llvm::errs() << "\n  Macro Expanded Tokens: "
               << llvm::capacity_in_bytes(MacroExpandedTokens);
  llvm::errs() << "\n  Predefines Buffer: " << Predefines.capacity();
  // FIXME: List information for all submodules.
  llvm::errs() << "\n  Macros: "
               << llvm::capacity_in_bytes(CurSubmoduleState->Macros);
  llvm::errs() << "\n  Poison Reasons: "
               << llvm::capacity_in_bytes(PoisonReasons);
  llvm::errs() << "\n  Comment Handlers: "
               << llvm::capacity_in_bytes(CommentHandlers) << "\n";
}

Preprocessor::macro_iterator
Preprocessor::macro_begin(bool IncludeExternalMacros) const {

  // Make sure we cover all macros in visible modules.

  return CurSubmoduleState->Macros.begin();
}

size_t Preprocessor::getTotalMemory() const {
  return BP.getTotalMemory()
    + llvm::capacity_in_bytes(MacroExpandedTokens)
    + Predefines.capacity() /* Predefines buffer. */
    // FIXME: Include sizes from all submodules, and include MacroInfo sizes,
    // and ModuleMacros.
    + llvm::capacity_in_bytes(CurSubmoduleState->Macros)
    + llvm::capacity_in_bytes(PoisonReasons)
    + llvm::capacity_in_bytes(CommentHandlers);
}

Preprocessor::macro_iterator
Preprocessor::macro_end(bool IncludeExternalMacros) const {

  return CurSubmoduleState->Macros.end();
}


void Preprocessor::recomputeCurLexerKind() {
  if (CurLexer)
    CurLexerKind = CLK_Lexer;
  else if (CurTokenLexer)
    CurLexerKind = CLK_TokenLexer;
  else
    CurLexerKind = CLK_CachingLexer;
}

bool Preprocessor::SetCodeCompletionPoint(const FileEntry *File,
                                          unsigned CompleteLine,
                                          unsigned CompleteColumn) {
  assert(File);
  assert(CompleteLine && CompleteColumn && "Starts from 1:1");
  assert(!CodeCompletionFile && "Already set");

  using llvm::MemoryBuffer;

  // Load the actual file's contents.
  bool Invalid = false;
  const MemoryBuffer *Buffer = SourceMgr.getMemoryBufferForFile(File, &Invalid);
  if (Invalid)
    return true;

  // Find the byte position of the truncation point.
  const char *Position = Buffer->getBufferStart();
  for (unsigned Line = 1; Line < CompleteLine; ++Line) {
    for (; *Position; ++Position) {
      if (*Position != '\r' && *Position != '\n')
        continue;

      // Eat \r\n or \n\r as a single line.
      if ((Position[1] == '\r' || Position[1] == '\n') &&
          Position[0] != Position[1])
        ++Position;
      ++Position;
      break;
    }
  }

  Position += CompleteColumn - 1;

  // If pointing inside the preamble, adjust the position at the beginning of
  // the file after the preamble.
  if (SkipMainFilePreamble.first &&
      SourceMgr.getFileEntryForID(SourceMgr.getMainFileID()) == File) {
    if (Position - Buffer->getBufferStart() < SkipMainFilePreamble.first)
      Position = Buffer->getBufferStart() + SkipMainFilePreamble.first;
  }

  if (Position > Buffer->getBufferEnd())
    Position = Buffer->getBufferEnd();

  CodeCompletionFile = File;
  CodeCompletionOffset = Position - Buffer->getBufferStart();

  auto NewBuffer = llvm::WritableMemoryBuffer::getNewUninitMemBuffer(
      Buffer->getBufferSize() + 1, Buffer->getBufferIdentifier());
  char *NewBuf = NewBuffer->getBufferStart();
  char *NewPos = std::copy(Buffer->getBufferStart(), Position, NewBuf);
  *NewPos = '\0';
  std::copy(Position, Buffer->getBufferEnd(), NewPos+1);
  SourceMgr.overrideFileContents(File, std::move(NewBuffer));

  return false;
}

void Preprocessor::CodeCompleteNaturalLanguage() {
  if (CodeComplete)
    CodeComplete->CodeCompleteNaturalLanguage();
  setCodeCompletionReached();
}

/// getSpelling - This method is used to get the spelling of a token into a
/// SmallVector. Note that the returned StringRef may not point to the
/// supplied buffer if a copy can be avoided.
StringRef Preprocessor::getSpelling(const Token &Tok,
                                          SmallVectorImpl<char> &Buffer,
                                          bool *Invalid) const {
  // NOTE: this has to be checked *before* testing for an IdentifierInfo.
  if (Tok.isNot(tok::raw_identifier) && !Tok.hasUCN()) {
    // Try the fast path.
    if (const IdentifierInfo *II = Tok.getIdentifierInfo())
      return II->getName();
  }

  // Resize the buffer if we need to copy into it.
  if (Tok.needsCleaning())
    Buffer.resize(Tok.getLength());

  const char *Ptr = Buffer.data();
  unsigned Len = getSpelling(Tok, Ptr, Invalid);
  return StringRef(Ptr, Len);
}

/// CreateString - Plop the specified string into a scratch buffer and return a
/// location for it.  If specified, the source location provides a source
/// location for the token.
void Preprocessor::CreateString(StringRef Str, Token &Tok,
                                SourceLocation ExpansionLocStart,
                                SourceLocation ExpansionLocEnd) {
  Tok.setLength(Str.size());

  const char *DestPtr;
  SourceLocation Loc = ScratchBuf->getToken(Str.data(), Str.size(), DestPtr);

  if (ExpansionLocStart.isValid())
    Loc = SourceMgr.createExpansionLoc(Loc, ExpansionLocStart,
                                       ExpansionLocEnd, Str.size());
  Tok.setLocation(Loc);

  // If this is a raw identifier or a literal token, set the pointer data.
  if (Tok.is(tok::raw_identifier))
    Tok.setRawIdentifierData(DestPtr);
  else if (Tok.isLiteral())
    Tok.setLiteralData(DestPtr);
}

//===----------------------------------------------------------------------===//
// Preprocessor Initialization Methods
//===----------------------------------------------------------------------===//


void Preprocessor::replayPreambleConditionalStack() {
  // Restore the conditional stack from the preamble, if there is one.
  if (PreambleConditionalStack.isReplaying()) {
    assert(CurPPLexer &&
           "CurPPLexer is null when calling replayPreambleConditionalStack.");
    CurPPLexer->setConditionalLevels(PreambleConditionalStack.getStack());
    PreambleConditionalStack.doneReplaying();
    if (PreambleConditionalStack.reachedEOFWhileSkipping())
      SkipExcludedConditionalBlock(
          PreambleConditionalStack.SkipInfo->HashTokenLoc,
          PreambleConditionalStack.SkipInfo->IfTokenLoc,
          PreambleConditionalStack.SkipInfo->FoundNonSkipPortion,
          PreambleConditionalStack.SkipInfo->FoundElse,
          PreambleConditionalStack.SkipInfo->ElseLoc);
  }
}

void Preprocessor::EndSourceFile() {
  // Notify the client that we reached the end of the source file.
}

//===----------------------------------------------------------------------===//
// Lexer Event Handling.
//===----------------------------------------------------------------------===//

/// LookUpIdentifierInfo - Given a tok::raw_identifier token, look up the
/// identifier information for the token and install it into the token,
/// updating the token kind accordingly.
IdentifierInfo *Preprocessor::LookUpIdentifierInfo(Token &Identifier) const {
  assert(!Identifier.getRawIdentifier().empty() && "No raw identifier data!");

  // Look up this token, see if it is a macro, or if it is a language keyword.
  IdentifierInfo *II;
  if (!Identifier.needsCleaning() && !Identifier.hasUCN()) {
    // No cleaning needed, just use the characters from the lexed buffer.
    II = getIdentifierInfo(Identifier.getRawIdentifier());
  } else {
    // Cleaning needed, alloca a buffer, clean into it, then use the buffer.
    SmallString<64> IdentifierBuffer;
    StringRef CleanedStr = getSpelling(Identifier, IdentifierBuffer);

    if (Identifier.hasUCN()) {
      SmallString<64> UCNIdentifierBuffer;
      expandUCNs(UCNIdentifierBuffer, CleanedStr);
      II = getIdentifierInfo(UCNIdentifierBuffer);
    } else {
      II = getIdentifierInfo(CleanedStr);
    }
  }

  // Update the token info (identifier info and appropriate token kind).
  Identifier.setIdentifierInfo(II);
  Identifier.setKind(II->getTokenID());

  return II;
}

void Preprocessor::SetPoisonReason(IdentifierInfo *II, unsigned DiagID) {
  PoisonReasons[II] = DiagID;
}


void Preprocessor::HandlePoisonedIdentifier(Token & Identifier) {
  assert(Identifier.getIdentifierInfo() &&
         "Can't handle identifiers without identifier info!");
  llvm::DenseMap<IdentifierInfo*,unsigned>::const_iterator it =
    PoisonReasons.find(Identifier.getIdentifierInfo());
  if(it == PoisonReasons.end())
    Diag(Identifier, diag::err_pp_used_poisoned_id);
  else
    Diag(Identifier,it->second) << Identifier.getIdentifierInfo();
}

/// Returns a diagnostic message kind for reporting a future keyword as
/// appropriate for the identifier and specified language.
static diag::kind getFutureCompatDiagKind(const IdentifierInfo &II) {
  assert(II.isFutureCompatKeyword() && "diagnostic should not be needed");


  llvm_unreachable(
      "Keyword not known to come from a newer Standard or proposed Standard");
}


/// HandleIdentifier - This callback is invoked when the lexer reads an
/// identifier.  This callback looks up the identifier in the map and/or
/// potentially macro expands it or turns it into a named token (like 'for').
///
/// Note that callers of this method are guarded by checking the
/// IdentifierInfo's 'isHandleIdentifierCase' bit.  If this method changes, the
/// IdentifierInfo methods that compute these properties will need to change to
/// match.
bool Preprocessor::HandleIdentifier(Token &Identifier) {
  assert(Identifier.getIdentifierInfo() &&
         "Can't handle identifiers without identifier info!");

  IdentifierInfo &II = *Identifier.getIdentifierInfo();

  // If the information about this identifier is out of date, update it from
  // the external source.
  // We have to treat __VA_ARGS__ in a special way, since it gets
  // serialized with isPoisoned = true, but our preprocessor may have
  // unpoisoned it if we're defining a C99 macro.
  if (II.isOutOfDate()) {
    bool CurrentIsPoisoned = false;
    const bool IsSpecialVariadicMacro =
        &II == Ident__VA_ARGS__ || &II == Ident__VA_OPT__;
    if (IsSpecialVariadicMacro)
      CurrentIsPoisoned = II.isPoisoned();

    Identifier.setKind(II.getTokenID());

    if (IsSpecialVariadicMacro)
      II.setIsPoisoned(CurrentIsPoisoned);
  }

  // If this identifier was poisoned, and if it was not produced from a macro
  // expansion, emit an error.
  if (II.isPoisoned() && CurPPLexer) {
    HandlePoisonedIdentifier(Identifier);
  }

  // If this is a macro to be expanded, do it.
  if (MacroDefinition MD = getMacroDefinition(&II)) {
    auto *MI = MD.getMacroInfo();
    assert(MI && "macro definition with no macro info?");
    if (!DisableMacroExpansion) {
      if (!Identifier.isExpandDisabled() && MI->isEnabled()) {
        // C99 6.10.3p10: If the preprocessing token immediately after the
        // macro name isn't a '(', this macro should not be expanded.
        if (!MI->isFunctionLike() || isNextPPTokenLParen())
          return HandleMacroExpandedIdentifier(Identifier, MD);
      } else {
        // C99 6.10.3.4p2 says that a disabled macro may never again be
        // expanded, even if it's in a context where it could be expanded in the
        // future.
        Identifier.setFlag(Token::DisableExpand);
        if (MI->isObjectLike() || isNextPPTokenLParen())
          Diag(Identifier, diag::pp_disabled_macro_expansion);
      }
    }
  }

  // If this identifier is a keyword in a newer Standard or proposed Standard,
  // produce a warning. Don't warn if we're not considering macro expansion,
  // since this identifier might be the name of a macro.
  // FIXME: This warning is disabled in cases where it shouldn't be, like
  //   "#define constexpr constexpr", "int constexpr;"
  if (II.isFutureCompatKeyword() && !DisableMacroExpansion) {
    Diag(Identifier, getFutureCompatDiagKind(II))
        << II.getName();
    // Don't diagnose this keyword again in this translation unit.
    II.setIsFutureCompatKeyword(false);
  }

  // If this is an extension token, diagnose its use.
  // We avoid diagnosing tokens that originate from macro definitions.
  // FIXME: This warning is disabled in cases where it shouldn't be,
  // like "#define TY typeof", "TY(1) x".
  if (II.isExtensionToken() && !DisableMacroExpansion)
    Diag(Identifier, diag::ext_token_used);

  return true;
}

void Preprocessor::Lex(Token &Result) {
  // We loop here until a lex function returns a token; this avoids recursion.
  bool ReturnedToken;
  do {
    switch (CurLexerKind) {
    case CLK_Lexer:
      ReturnedToken = CurLexer->Lex(Result);
      break;
    case CLK_TokenLexer:
      ReturnedToken = CurTokenLexer->Lex(Result);
      break;
    case CLK_CachingLexer:
      CachingLex(Result);
      ReturnedToken = true;
      break;
    }
  } while (!ReturnedToken);

  if (Result.is(tok::code_completion) && Result.getIdentifierInfo()) {
    // Remember the identifier before code completion token.
    setCodeCompletionIdentifierInfo(Result.getIdentifierInfo());
    // Set IdenfitierInfo to null to avoid confusing code that handles both
    // identifiers and completion tokens.
    Result.setIdentifierInfo(nullptr);
  }

  LastTokenWasAt = Result.is(tok::at);
}


bool Preprocessor::FinishLexStringLiteral(Token &Result, std::string &String,
                                          const char *DiagnosticTag,
                                          bool AllowMacroExpansion) {
  // We need at least one string literal.
  if (Result.isNot(tok::string_literal)) {
    Diag(Result, diag::err_expected_string_literal)
      << /*Source='in...'*/0 << DiagnosticTag;
    return false;
  }

  // Lex string literal tokens, optionally with macro expansion.
  SmallVector<Token, 4> StrToks;
  do {
    StrToks.push_back(Result);

    if (Result.hasUDSuffix())
      Diag(Result, diag::err_invalid_string_udl);

    if (AllowMacroExpansion)
      Lex(Result);
    else
      LexUnexpandedToken(Result);
  } while (Result.is(tok::string_literal));

  // Concatenate and parse the strings.
  StringLiteralParser Literal(StrToks, *this);
  assert(Literal.isAscii() && "Didn't allow wide strings in");

  if (Literal.hadError)
    return false;

  if (Literal.Pascal) {
    Diag(StrToks[0].getLocation(), diag::err_expected_string_literal)
      << /*Source='in...'*/0 << DiagnosticTag;
    return false;
  }

  String = Literal.GetString();
  return true;
}

bool Preprocessor::parseSimpleIntegerLiteral(Token &Tok, uint64_t &Value) {
  assert(Tok.is(tok::numeric_constant));
  SmallString<8> IntegerBuffer;
  bool NumberInvalid = false;
  StringRef Spelling = getSpelling(Tok, IntegerBuffer, &NumberInvalid);
  if (NumberInvalid)
    return false;
  NumericLiteralParser Literal(Spelling, Tok.getLocation(), *this);
  if (Literal.hadError || !Literal.isIntegerLiteral() || Literal.hasUDSuffix())
    return false;
  llvm::APInt APVal(64, 0);
  if (Literal.GetIntegerValue(APVal))
    return false;
  Lex(Tok);
  Value = APVal.getLimitedValue();
  return true;
}


bool Preprocessor::HandleComment(Token &result, SourceRange Comment) {
  bool AnyPendingTokens = false;
  for (std::vector<CommentHandler *>::iterator H = CommentHandlers.begin(),
       HEnd = CommentHandlers.end();
       H != HEnd; ++H) {
    if ((*H)->HandleComment(*this, Comment))
      AnyPendingTokens = true;
  }
  if (!AnyPendingTokens || getCommentRetentionState())
    return false;
  Lex(result);
  return true;
}


CommentHandler::~CommentHandler() = default;

CodeCompletionHandler::~CodeCompletionHandler() = default;

//===----------------------------------------------------------------------===//
// Miscellaneous Methods.
//===----------------------------------------------------------------------===//

/// isInPrimaryFile - Return true if we're in the top-level file, not in a
/// \#include.  This looks through macro expansions and active _Pragma lexers.
bool Preprocessor::isInPrimaryFile() const {
  if (IsFileLexer())
    return IncludeMacroStack.empty();

  // If there are any stacked lexers, we're in a #include.
  assert(IsFileLexer(IncludeMacroStack[0]) &&
         "Top level include stack isn't our primary lexer?");
  return std::none_of(
      IncludeMacroStack.begin() + 1, IncludeMacroStack.end(),
      [&](const IncludeStackInfo &ISI) -> bool { return IsFileLexer(ISI); });
}

/// getCurrentLexer - Return the current file lexer being lexed from.  Note
/// that this ignores any potentially active macro expansions and _Pragma
/// expansions going on at the time.
PreprocessorLexer *Preprocessor::getCurrentFileLexer() const {
  if (IsFileLexer())
    return CurPPLexer;

  // Look for a stacked lexer.
  for (const IncludeStackInfo &ISI : llvm::reverse(IncludeMacroStack)) {
    if (IsFileLexer(ISI))
      return ISI.ThePPLexer;
  }
  return nullptr;
}


//===----------------------------------------------------------------------===//
// Methods for Entering and Callbacks for leaving various contexts
//===----------------------------------------------------------------------===//

/// EnterSourceFile - Add a source file to the top of the include stack and
/// start lexing tokens from it instead of the current buffer.
bool Preprocessor::EnterSourceFile(FileID FID, const DirectoryLookup *CurDir,
                                   SourceLocation Loc) {
  assert(!CurTokenLexer && "Cannot #include a file inside a macro!");
  ++NumEnteredSourceFiles;

  if (MaxIncludeStackDepth < IncludeMacroStack.size())
    MaxIncludeStackDepth = IncludeMacroStack.size();

  // Get the MemoryBuffer for this FID, if it fails, we fail.
  bool Invalid = false;
  const llvm::MemoryBuffer *InputFile =
    getSourceManager().getBuffer(FID, Loc, &Invalid);
  if (Invalid) {
    SourceLocation FileStart = SourceMgr.getLocForStartOfFile(FID);
    Diag(Loc, diag::err_pp_error_opening_file)
      << std::string(SourceMgr.getBufferName(FileStart)) << "";
    return true;
  }

  if (isCodeCompletionEnabled() &&
      SourceMgr.getFileEntryForID(FID) == CodeCompletionFile) {
    CodeCompletionFileLoc = SourceMgr.getLocForStartOfFile(FID);
    CodeCompletionLoc =
        CodeCompletionFileLoc.getLocWithOffset(CodeCompletionOffset);
  }

  EnterSourceFileWithLexer(new Lexer(FID, InputFile, *this), CurDir);
  return false;
}

/// EnterSourceFileWithLexer - Add a source file to the top of the include stack
///  and start lexing tokens from it instead of the current buffer.
void Preprocessor::EnterSourceFileWithLexer(Lexer *TheLexer,
                                            const DirectoryLookup *CurDir) {

  // Add the current lexer to the include stack.
  if (CurPPLexer || CurTokenLexer)
    PushIncludeMacroStack();

  CurLexer.reset(TheLexer);
  CurPPLexer = TheLexer;
  CurDirLookup = CurDir;
  CurLexerKind = CLK_Lexer;

  // Notify the client, if desired, that we are in a new source file.
}


/// EnterMacro - Add a Macro to the top of the include stack and start lexing
/// tokens from it instead of the current buffer.
void Preprocessor::EnterMacro(Token &Tok, SourceLocation ILEnd,
                              MacroInfo *Macro, MacroArgs *Args) {
  std::unique_ptr<TokenLexer> TokLexer;
  if (NumCachedTokenLexers == 0) {
    TokLexer = llvm::make_unique<TokenLexer>(Tok, ILEnd, Macro, Args, *this);
  } else {
    TokLexer = std::move(TokenLexerCache[--NumCachedTokenLexers]);
    TokLexer->Init(Tok, ILEnd, Macro, Args);
  }

  PushIncludeMacroStack();
  CurDirLookup = nullptr;
  CurTokenLexer = std::move(TokLexer);
  CurLexerKind = CLK_TokenLexer;
}

/// EnterTokenStream - Add a "macro" context to the top of the include stack,
/// which will cause the lexer to start returning the specified tokens.
///
/// If DisableMacroExpansion is true, tokens lexed from the token stream will
/// not be subject to further macro expansion.  Otherwise, these tokens will
/// be re-macro-expanded when/if expansion is enabled.
///
/// If OwnsTokens is false, this method assumes that the specified stream of
/// tokens has a permanent owner somewhere, so they do not need to be copied.
/// If it is true, it assumes the array of tokens is allocated with new[] and
/// must be freed.
///
void Preprocessor::EnterTokenStream(const Token *Toks, unsigned NumToks,
                                    bool DisableMacroExpansion,
                                    bool OwnsTokens) {
  if (CurLexerKind == CLK_CachingLexer) {
    if (CachedLexPos < CachedTokens.size()) {
      // We're entering tokens into the middle of our cached token stream. We
      // can't represent that, so just insert the tokens into the buffer.
      CachedTokens.insert(CachedTokens.begin() + CachedLexPos,
                          Toks, Toks + NumToks);
      if (OwnsTokens)
        delete [] Toks;
      return;
    }

    // New tokens are at the end of the cached token sequnece; insert the
    // token stream underneath the caching lexer.
    ExitCachingLexMode();
    EnterTokenStream(Toks, NumToks, DisableMacroExpansion, OwnsTokens);
    EnterCachingLexMode();
    return;
  }

  // Create a macro expander to expand from the specified token stream.
  std::unique_ptr<TokenLexer> TokLexer;
  if (NumCachedTokenLexers == 0) {
    TokLexer = llvm::make_unique<TokenLexer>(
        Toks, NumToks, DisableMacroExpansion, OwnsTokens, *this);
  } else {
    TokLexer = std::move(TokenLexerCache[--NumCachedTokenLexers]);
    TokLexer->Init(Toks, NumToks, DisableMacroExpansion, OwnsTokens);
  }

  // Save our current state.
  PushIncludeMacroStack();
  CurDirLookup = nullptr;
  CurTokenLexer = std::move(TokLexer);
  CurLexerKind = CLK_TokenLexer;
}

void Preprocessor::PropagateLineStartLeadingSpaceInfo(Token &Result) {
  if (CurTokenLexer) {
    CurTokenLexer->PropagateLineStartLeadingSpaceInfo(Result);
    return;
  }
  if (CurLexer) {
    CurLexer->PropagateLineStartLeadingSpaceInfo(Result);
    return;
  }
  // FIXME: Handle other kinds of lexers?  It generally shouldn't matter,
  // but it might if they're empty?
}

/// Determine the location to use as the end of the buffer for a lexer.
///
/// If the file ends with a newline, form the EOF token on the newline itself,
/// rather than "on the line following it", which doesn't exist.  This makes
/// diagnostics relating to the end of file include the last file that the user
/// actually typed, which is goodness.
const char *Preprocessor::getCurLexerEndPos() {
  const char *EndPos = CurLexer->BufferEnd;
  if (EndPos != CurLexer->BufferStart &&
      (EndPos[-1] == '\n' || EndPos[-1] == '\r')) {
    --EndPos;

    // Handle \n\r and \r\n:
    if (EndPos != CurLexer->BufferStart &&
        (EndPos[-1] == '\n' || EndPos[-1] == '\r') &&
        EndPos[-1] != EndPos[0])
      --EndPos;
  }

  return EndPos;
}


/// HandleEndOfFile - This callback is invoked when the lexer hits the end of
/// the current file.  This either returns the EOF token or pops a level off
/// the include stack and keeps going.
bool Preprocessor::HandleEndOfFile(Token &Result, bool isEndOfMacro) {
  assert(!CurTokenLexer &&
         "Ending a file when currently in a macro!");


  // See if this file had a controlling macro.
  if (CurPPLexer) {  // Not ending a macro, ignore it.
    if (const IdentifierInfo *ControllingMacro =
          CurPPLexer->MIOpt.GetControllingMacroAtEndOfFile()) {
      // Okay, this has a controlling macro, remember in HeaderFileInfo.
      if (const FileEntry *FE = CurPPLexer->getFileEntry()) {
        HeaderInfo.SetFileControllingMacro(FE, ControllingMacro);
        if (MacroInfo *MI =
              getMacroInfo(const_cast<IdentifierInfo*>(ControllingMacro)))
          MI->setUsedForHeaderGuard(true);
        if (const IdentifierInfo *DefinedMacro =
              CurPPLexer->MIOpt.GetDefinedMacro()) {
          if (!isMacroDefined(ControllingMacro) &&
              DefinedMacro != ControllingMacro &&
              HeaderInfo.FirstTimeLexingFile(FE)) {

            // If the edit distance between the two macros is more than 50%,
            // DefinedMacro may not be header guard, or can be header guard of
            // another header file. Therefore, it maybe defining something
            // completely different. This can be observed in the wild when
            // handling feature macros or header guards in different files.

            const StringRef ControllingMacroName = ControllingMacro->getName();
            const StringRef DefinedMacroName = DefinedMacro->getName();
            const size_t MaxHalfLength = std::max(ControllingMacroName.size(),
                                                  DefinedMacroName.size()) / 2;
            const unsigned ED = ControllingMacroName.edit_distance(
                DefinedMacroName, true, MaxHalfLength);
            if (ED <= MaxHalfLength) {
              // Emit a warning for a bad header guard.
              Diag(CurPPLexer->MIOpt.GetMacroLocation(),
                   diag::warn_header_guard)
                  << CurPPLexer->MIOpt.GetMacroLocation() << ControllingMacro;
              Diag(CurPPLexer->MIOpt.GetDefinedLocation(),
                   diag::note_header_guard)
                  << CurPPLexer->MIOpt.GetDefinedLocation() << DefinedMacro
                  << ControllingMacro
                  << FixItHint::CreateReplacement(
                         CurPPLexer->MIOpt.GetDefinedLocation(),
                         ControllingMacro->getName());
            }
          }
        }
      }
    }
  }



  // If this is a #include'd file, pop it off the include stack and continue
  // lexing the #includer file.
  if (!IncludeMacroStack.empty()) {

    // If we lexed the code-completion file, act as if we reached EOF.
    if (isCodeCompletionEnabled() && CurPPLexer &&
        SourceMgr.getLocForStartOfFile(CurPPLexer->getFileID()) ==
            CodeCompletionFileLoc) {
      if (CurLexer) {
        Result.startToken();
        CurLexer->FormTokenWithChars(Result, CurLexer->BufferEnd, tok::eof);
        CurLexer.reset();
      } else {
        FATAL_ERROR("Got EOF but no current lexer set!");
      }

      CurPPLexer = nullptr;
      recomputeCurLexerKind();
      return true;
    }

    if (!isEndOfMacro && CurPPLexer &&
        SourceMgr.getIncludeLoc(CurPPLexer->getFileID()).isValid()) {
      // Notify SourceManager to record the number of FileIDs that were created
      // during lexing of the #include'd file.
      unsigned NumFIDs =
          SourceMgr.local_sloc_entry_size() -
          CurPPLexer->getInitialNumSLocEntries() + 1/*#include'd file*/;
      SourceMgr.setNumCreatedFIDsForFileID(CurPPLexer->getFileID(), NumFIDs);
    }

    bool ExitedFromPredefinesFile = false;
    FileID ExitedFID;
    if (!isEndOfMacro && CurPPLexer) {
      ExitedFID = CurPPLexer->getFileID();

      assert(PredefinesFileID.isValid() &&
             "HandleEndOfFile is called before PredefinesFileId is set");
      ExitedFromPredefinesFile = (PredefinesFileID == ExitedFID);
    }



    // We're done with the #included file.
    RemoveTopOfLexerStack();

    // Propagate info about start-of-line/leading white-space/etc.
    PropagateLineStartLeadingSpaceInfo(Result);


    // Restore conditional stack from the preamble right after exiting from the
    // predefines file.
    if (ExitedFromPredefinesFile)
      replayPreambleConditionalStack();

    return false;
  }

  // If this is the end of the main file, form an EOF token.
  if (CurLexer) {
    const char *EndPos = getCurLexerEndPos();
    Result.startToken();
    CurLexer->BufferPtr = EndPos;
    CurLexer->FormTokenWithChars(Result, EndPos, tok::eof);

    if (isCodeCompletionEnabled()) {
      // Inserting the code-completion point increases the source buffer by 1,
      // but the main FileID was created before inserting the point.
      // Compensate by reducing the EOF location by 1, otherwise the location
      // will point to the next FileID.
      // FIXME: This is hacky, the code-completion point should probably be
      // inserted before the main FileID is created.
      if (CurLexer->getFileLoc() == CodeCompletionFileLoc)
        Result.setLocation(Result.getLocation().getLocWithOffset(-1));
    }


    if (!isIncrementalProcessingEnabled())
      // We're done with lexing.
      CurLexer.reset();
  } else {
    FATAL_ERROR("Got EOF but no current lexer set!");
  }

  if (!isIncrementalProcessingEnabled())
    CurPPLexer = nullptr;

  for (WarnUnusedMacroLocsTy::iterator
         I=WarnUnusedMacroLocs.begin(), E=WarnUnusedMacroLocs.end();
         I!=E; ++I)
    Diag(*I, diag::pp_macro_not_used);

  return true;
}

/// HandleEndOfTokenLexer - This callback is invoked when the current TokenLexer
/// hits the end of its token stream.
bool Preprocessor::HandleEndOfTokenLexer(Token &Result) {
  assert(CurTokenLexer && !CurPPLexer &&
         "Ending a macro when currently in a #include file!");

  if (!MacroExpandingLexersStack.empty() &&
      MacroExpandingLexersStack.back().first == CurTokenLexer.get())
    removeCachedMacroExpandedTokensOfLastLexer();

  // Delete or cache the now-dead macro expander.
  if (NumCachedTokenLexers == TokenLexerCacheSize)
    CurTokenLexer.reset();
  else
    TokenLexerCache[NumCachedTokenLexers++] = std::move(CurTokenLexer);

  // Handle this like a #include file being popped off the stack.
  return HandleEndOfFile(Result, true);
}

/// RemoveTopOfLexerStack - Pop the current lexer/macro exp off the top of the
/// lexer stack.  This should only be used in situations where the current
/// state of the top-of-stack lexer is unknown.
void Preprocessor::RemoveTopOfLexerStack() {
  assert(!IncludeMacroStack.empty() && "Ran out of stack entries to load");

  if (CurTokenLexer) {
    // Delete or cache the now-dead macro expander.
    if (NumCachedTokenLexers == TokenLexerCacheSize)
      CurTokenLexer.reset();
    else
      TokenLexerCache[NumCachedTokenLexers++] = std::move(CurTokenLexer);
  }

  PopIncludeMacroStack();
}

/// HandleMicrosoftCommentPaste - When the macro expander pastes together a
/// comment (/##/) in microsoft mode, this method handles updating the current
/// state, returning the token on the next source line.
void Preprocessor::HandleMicrosoftCommentPaste(Token &Tok) {
  assert(CurTokenLexer && !CurPPLexer &&
         "Pasted comment can only be formed from macro");
  // We handle this by scanning for the closest real lexer, switching it to
  // raw mode and preprocessor mode.  This will cause it to return \n as an
  // explicit EOD token.
  PreprocessorLexer *FoundLexer = nullptr;
  bool LexerWasInPPMode = false;
  for (const IncludeStackInfo &ISI : llvm::reverse(IncludeMacroStack)) {
    if (ISI.ThePPLexer == nullptr) continue;  // Scan for a real lexer.

    // Once we find a real lexer, mark it as raw mode (disabling macro
    // expansions) and preprocessor mode (return EOD).  We know that the lexer
    // was *not* in raw mode before, because the macro that the comment came
    // from was expanded.  However, it could have already been in preprocessor
    // mode (#if COMMENT) in which case we have to return it to that mode and
    // return EOD.
    FoundLexer = ISI.ThePPLexer;
    FoundLexer->LexingRawMode = true;
    LexerWasInPPMode = FoundLexer->ParsingPreprocessorDirective;
    FoundLexer->ParsingPreprocessorDirective = true;
    break;
  }

  // Okay, we either found and switched over the lexer, or we didn't find a
  // lexer.  In either case, finish off the macro the comment came from, getting
  // the next token.
  if (!HandleEndOfTokenLexer(Tok)) Lex(Tok);

  // Discarding comments as long as we don't have EOF or EOD.  This 'comments
  // out' the rest of the line, including any tokens that came from other macros
  // that were active, as in:
  //  #define submacro a COMMENT b
  //    submacro c
  // which should lex to 'a' only: 'b' and 'c' should be removed.
  while (Tok.isNot(tok::eod) && Tok.isNot(tok::eof))
    Lex(Tok);

  // If we got an eod token, then we successfully found the end of the line.
  if (Tok.is(tok::eod)) {
    assert(FoundLexer && "Can't get end of line without an active lexer");
    // Restore the lexer back to normal mode instead of raw mode.
    FoundLexer->LexingRawMode = false;

    // If the lexer was already in preprocessor mode, just return the EOD token
    // to finish the preprocessor line.
    if (LexerWasInPPMode) return;

    // Otherwise, switch out of PP mode and return the next lexed token.
    FoundLexer->ParsingPreprocessorDirective = false;
    return Lex(Tok);
  }

  // If we got an EOF token, then we reached the end of the token stream but
  // didn't find an explicit \n.  This can only happen if there was no lexer
  // active (an active lexer would return EOD at EOF if there was no \n in
  // preprocessor directive mode), so just return EOF as our token.
  assert(!FoundLexer && "Lexer should return EOD before EOF in PP mode");
}



//===----------------------------------------------------------------------===//
// Utility Methods for Preprocessor Directive Handling.
//===----------------------------------------------------------------------===//

MacroInfo *Preprocessor::AllocateMacroInfo(SourceLocation L) {
  auto *MIChain = new (BP) MacroInfoChain{L, MIChainHead};
  MIChainHead = MIChain;
  return &MIChain->MI;
}

DefMacroDirective *Preprocessor::AllocateDefMacroDirective(MacroInfo *MI,
                                                           SourceLocation Loc) {
  return new (BP) DefMacroDirective(MI, Loc);
}

UndefMacroDirective *
Preprocessor::AllocateUndefMacroDirective(SourceLocation UndefLoc) {
  return new (BP) UndefMacroDirective(UndefLoc);
}

VisibilityMacroDirective *
Preprocessor::AllocateVisibilityMacroDirective(SourceLocation Loc,
                                               bool isPublic) {
  return new (BP) VisibilityMacroDirective(Loc, isPublic);
}

/// Read and discard all tokens remaining on the current line until
/// the tok::eod token is found.
void Preprocessor::DiscardUntilEndOfDirective() {
  Token Tmp;
  do {
    LexUnexpandedToken(Tmp);
    assert(Tmp.isNot(tok::eof) && "EOF seen while discarding directive tokens");
  } while (Tmp.isNot(tok::eod));
}

/// Enumerates possible cases of #define/#undef a reserved identifier.
enum MacroDiag {
  MD_NoWarn,        //> Not a reserved identifier
  MD_KeywordDef,    //> Macro hides keyword, enabled by default
  MD_ReservedMacro  //> #define of #undef reserved id, disabled by default
};

/// Checks if the specified identifier is reserved in the specified
/// language.
/// This function does not check if the identifier is a keyword.
static bool isReservedId(StringRef Text) {
  // C++ [macro.names], C11 7.1.3:
  // All identifiers that begin with an underscore and either an uppercase
  // letter or another underscore are always reserved for any use.
  if (Text.size() >= 2 && Text[0] == '_' &&
      (isUppercase(Text[1]) || Text[1] == '_'))
      return true;
  // C++ [global.names]
  // Each name that contains a double underscore ... is reserved to the
  // implementation for any use.
  return false;
}

static MacroDiag shouldWarnOnMacroDef(Preprocessor &PP, IdentifierInfo *II) {
  StringRef Text = II->getName();
  if (isReservedId(Text))
    return MD_ReservedMacro;
  if (II->isKeyword())
    return MD_KeywordDef;
  return MD_NoWarn;
}

static MacroDiag shouldWarnOnMacroUndef(Preprocessor &PP, IdentifierInfo *II) {
  StringRef Text = II->getName();
  // Do not warn on keyword undef.  It is generally harmless and widely used.
  if (isReservedId(Text))
    return MD_ReservedMacro;
  return MD_NoWarn;
}


bool Preprocessor::CheckMacroName(Token &MacroNameTok, MacroUse isDefineUndef,
                                  bool *ShadowFlag) {
  // Missing macro name?
  if (MacroNameTok.is(tok::eod))
    return Diag(MacroNameTok, diag::err_pp_missing_macro_name);

  IdentifierInfo *II = MacroNameTok.getIdentifierInfo();
  if (!II)
    return Diag(MacroNameTok, diag::err_pp_macro_not_identifier);



  if (isDefineUndef == MU_Undef) {
    auto *MI = getMacroInfo(II);
    if (MI && MI->isBuiltinMacro()) {
      // Warn if undefining "__LINE__" and other builtins, per C99 6.10.8/4
      // and C++ [cpp.predefined]p4], but allow it as an extension.
      Diag(MacroNameTok, diag::ext_pp_undef_builtin_macro);
    }
  }

  // If defining/undefining reserved identifier or a keyword, we need to issue
  // a warning.
  SourceLocation MacroNameLoc = MacroNameTok.getLocation();
  if (ShadowFlag)
    *ShadowFlag = false;
  if (!SourceMgr.isInSystemHeader(MacroNameLoc) &&
      (SourceMgr.getBufferName(MacroNameLoc) != "<built-in>")) {
    MacroDiag D = MD_NoWarn;
    if (isDefineUndef == MU_Define) {
      D = shouldWarnOnMacroDef(*this, II);
    }
    else if (isDefineUndef == MU_Undef)
      D = shouldWarnOnMacroUndef(*this, II);
    if (D == MD_KeywordDef) {
      // We do not want to warn on some patterns widely used in configuration
      // scripts.  This requires analyzing next tokens, so do not issue warnings
      // now, only inform caller.
      if (ShadowFlag)
        *ShadowFlag = true;
    }
    if (D == MD_ReservedMacro)
      Diag(MacroNameTok, diag::warn_pp_macro_is_reserved_id);
  }

  // Okay, we got a good identifier.
  return false;
}

/// Lex and validate a macro name, which occurs after a
/// \#define or \#undef.
///
/// This sets the token kind to eod and discards the rest of the macro line if
/// the macro name is invalid.
///
/// \param MacroNameTok Token that is expected to be a macro name.
/// \param isDefineUndef Context in which macro is used.
/// \param ShadowFlag Points to a flag that is set if macro shadows a keyword.
void Preprocessor::ReadMacroName(Token &MacroNameTok, MacroUse isDefineUndef,
                                 bool *ShadowFlag) {
  // Read the token, don't allow macro expansion on it.
  LexUnexpandedToken(MacroNameTok);

  if (MacroNameTok.is(tok::code_completion)) {
    if (CodeComplete)
      CodeComplete->CodeCompleteMacroName(isDefineUndef == MU_Define);
    setCodeCompletionReached();
    LexUnexpandedToken(MacroNameTok);
  }

  if (!CheckMacroName(MacroNameTok, isDefineUndef, ShadowFlag))
    return;

  // Invalid macro name, read and discard the rest of the line and set the
  // token kind to tok::eod if necessary.
  if (MacroNameTok.isNot(tok::eod)) {
    MacroNameTok.setKind(tok::eod);
    DiscardUntilEndOfDirective();
  }
}

/// Ensure that the next token is a tok::eod token.
///
/// If not, emit a diagnostic and consume up until the eod.  If EnableMacros is
/// true, then we consider macros that expand to zero tokens as being ok.
void Preprocessor::CheckEndOfDirective(const char *DirType, bool EnableMacros) {
  Token Tmp;
  // Lex unexpanded tokens for most directives: macros might expand to zero
  // tokens, causing us to miss diagnosing invalid lines.  Some directives (like
  // #line) allow empty macros.
  if (EnableMacros)
    Lex(Tmp);
  else
    LexUnexpandedToken(Tmp);

  // There should be no tokens after the directive, but we allow them as an
  // extension.
  while (Tmp.is(tok::comment))  // Skip comments in -C mode.
    LexUnexpandedToken(Tmp);

  if (Tmp.isNot(tok::eod)) {
    // Add a fixit in GNU/C99/C++ mode.  Don't offer a fixit for strict-C89,
    // or if this is a macro-style preprocessing directive, because it is more
    // trouble than it is worth to insert /**/ and check that there is no /**/
    // in the range also.
    FixItHint Hint;
    if (!CurTokenLexer) {
        Hint = FixItHint::CreateInsertion(Tmp.getLocation(), "//");
    }
    Diag(Tmp, diag::ext_pp_extra_tokens_at_eol) << DirType << Hint;
    DiscardUntilEndOfDirective();
  }
}

/// SkipExcludedConditionalBlock - We just read a \#if or related directive and
/// decided that the subsequent tokens are in the \#if'd out portion of the
/// file.  Lex the rest of the file, until we see an \#endif.  If
/// FoundNonSkipPortion is true, then we have already emitted code for part of
/// this \#if directive, so \#else/\#elif blocks should never be entered.
/// If ElseOk is true, then \#else directives are ok, if not, then we have
/// already seen one so a \#else directive is a duplicate.  When this returns,
/// the caller can lex the first valid token.
void Preprocessor::SkipExcludedConditionalBlock(SourceLocation HashTokenLoc,
                                                SourceLocation IfTokenLoc,
                                                bool FoundNonSkipPortion,
                                                bool FoundElse,
                                                SourceLocation ElseLoc) {
  ++NumSkipped;
  assert(!CurTokenLexer && CurPPLexer && "Lexing a macro, not a file?");

  if (PreambleConditionalStack.reachedEOFWhileSkipping())
    PreambleConditionalStack.clearSkipInfo();
  else
    CurPPLexer->pushConditionalLevel(IfTokenLoc, /*isSkipping*/ false,
                                     FoundNonSkipPortion, FoundElse);


  // Enter raw mode to disable identifier lookup (and thus macro expansion),
  // disabling warnings, etc.
  CurPPLexer->LexingRawMode = true;
  Token Tok;
  while (true) {
    CurLexer->Lex(Tok);

    if (Tok.is(tok::code_completion)) {
      if (CodeComplete)
        CodeComplete->CodeCompleteInConditionalExclusion();
      setCodeCompletionReached();
      continue;
    }

    // If this is the end of the buffer, we have an error.
    if (Tok.is(tok::eof)) {
      // We don't emit errors for unterminated conditionals here,
      // Lexer::LexEndOfFile can do that propertly.
      // Just return and let the caller lex after this #include.
      if (PreambleConditionalStack.isRecording())
        PreambleConditionalStack.SkipInfo.emplace(
            HashTokenLoc, IfTokenLoc, FoundNonSkipPortion, FoundElse, ElseLoc);
      break;
    }

    // If this token is not a preprocessor directive, just skip it.
    if (Tok.isNot(tok::hash) || !Tok.isAtStartOfLine())
      continue;

    // We just parsed a # character at the start of a line, so we're in
    // directive mode.  Tell the lexer this so any newlines we see will be
    // converted into an EOD token (this terminates the macro).
    CurPPLexer->ParsingPreprocessorDirective = true;
    if (CurLexer) CurLexer->SetKeepWhitespaceMode(false);


    // Read the next token, the directive flavor.
    LexUnexpandedToken(Tok);

    // If this isn't an identifier directive (e.g. is "# 1\n" or "#\n", or
    // something bogus), skip it.
    if (Tok.isNot(tok::raw_identifier)) {
      CurPPLexer->ParsingPreprocessorDirective = false;
      // Restore comment saving mode.
      if (CurLexer) CurLexer->resetExtendedTokenMode();
      continue;
    }

    // If the first letter isn't i or e, it isn't intesting to us.  We know that
    // this is safe in the face of spelling differences, because there is no way
    // to spell an i/e in a strange way that is another letter.  Skipping this
    // allows us to avoid looking up the identifier info for #define/#undef and
    // other common directives.
    StringRef RI = Tok.getRawIdentifier();

    char FirstChar = RI[0];
    if (FirstChar >= 'a' && FirstChar <= 'z' &&
        FirstChar != 'i' && FirstChar != 'e') {
      CurPPLexer->ParsingPreprocessorDirective = false;
      // Restore comment saving mode.
      if (CurLexer) CurLexer->resetExtendedTokenMode();
      continue;
    }

    // Get the identifier name without trigraphs or embedded newlines.  Note
    // that we can't use Tok.getIdentifierInfo() because its lookup is disabled
    // when skipping.
    char DirectiveBuf[20];
    StringRef Directive;
    if (!Tok.needsCleaning() && RI.size() < 20) {
      Directive = RI;
    } else {
      std::string DirectiveStr = getSpelling(Tok);
      size_t IdLen = DirectiveStr.size();
      if (IdLen >= 20) {
        CurPPLexer->ParsingPreprocessorDirective = false;
        // Restore comment saving mode.
        if (CurLexer) CurLexer->resetExtendedTokenMode();
        continue;
      }
      memcpy(DirectiveBuf, &DirectiveStr[0], IdLen);
      Directive = StringRef(DirectiveBuf, IdLen);
    }

    if (Directive.startswith("if")) {
      StringRef Sub = Directive.substr(2);
      if (Sub.empty() ||   // "if"
          Sub == "def" ||   // "ifdef"
          Sub == "ndef") {  // "ifndef"
        // We know the entire #if/#ifdef/#ifndef block will be skipped, don't
        // bother parsing the condition.
        DiscardUntilEndOfDirective();
        CurPPLexer->pushConditionalLevel(Tok.getLocation(), /*wasskipping*/true,
                                       /*foundnonskip*/false,
                                       /*foundelse*/false);
      }
    } else if (Directive[0] == 'e') {
      StringRef Sub = Directive.substr(1);
      if (Sub == "ndif") {  // "endif"
        PPConditionalInfo CondInfo;
        CondInfo.WasSkipping = true; // Silence bogus warning.
        bool InCond = CurPPLexer->popConditionalLevel(CondInfo);
        (void)InCond;  // Silence warning in no-asserts mode.
        assert(!InCond && "Can't be skipping if not in a conditional!");

        // If we popped the outermost skipping block, we're done skipping!
        if (!CondInfo.WasSkipping) {
          // Restore the value of LexingRawMode so that trailing comments
          // are handled correctly, if we've reached the outermost block.
          CurPPLexer->LexingRawMode = false;
          CheckEndOfDirective("endif");
          CurPPLexer->LexingRawMode = true;
          break;
        } else {
          DiscardUntilEndOfDirective();
        }
      } else if (Sub == "lse") { // "else".
        // #else directive in a skipping conditional.  If not in some other
        // skipping conditional, and if #else hasn't already been seen, enter it
        // as a non-skipping conditional.
        PPConditionalInfo &CondInfo = CurPPLexer->peekConditionalLevel();

        // If this is a #else with a #else before it, report the error.
        if (CondInfo.FoundElse) Diag(Tok, diag::pp_err_else_after_else);

        // Note that we've seen a #else in this conditional.
        CondInfo.FoundElse = true;

        // If the conditional is at the top level, and the #if block wasn't
        // entered, enter the #else block now.
        if (!CondInfo.WasSkipping && !CondInfo.FoundNonSkip) {
          CondInfo.FoundNonSkip = true;
          // Restore the value of LexingRawMode so that trailing comments
          // are handled correctly.
          CurPPLexer->LexingRawMode = false;
          CheckEndOfDirective("else");
          CurPPLexer->LexingRawMode = true;
          break;
        } else {
          DiscardUntilEndOfDirective();  // C99 6.10p4.
        }
      } else if (Sub == "lif") {  // "elif".
        PPConditionalInfo &CondInfo = CurPPLexer->peekConditionalLevel();

        // If this is a #elif with a #else before it, report the error.
        if (CondInfo.FoundElse) Diag(Tok, diag::pp_err_elif_after_else);

        // If this is in a skipping block or if we're already handled this #if
        // block, don't bother parsing the condition.
        if (CondInfo.WasSkipping || CondInfo.FoundNonSkip) {
          DiscardUntilEndOfDirective();
        } else {
          CurPPLexer->getSourceLocation();
          // Restore the value of LexingRawMode so that identifiers are
          // looked up, etc, inside the #elif expression.
          assert(CurPPLexer->LexingRawMode && "We have to be skipping here!");
          CurPPLexer->LexingRawMode = false;
          IdentifierInfo *IfNDefMacro = nullptr;
          const bool CondValue = EvaluateDirectiveExpression(IfNDefMacro).Conditional;
          CurPPLexer->LexingRawMode = true;
          // If this condition is true, enter it!
          if (CondValue) {
            CondInfo.FoundNonSkip = true;
            break;
          }
        }
      }
    }

    CurPPLexer->ParsingPreprocessorDirective = false;
    // Restore comment saving mode.
    if (CurLexer) CurLexer->resetExtendedTokenMode();
  }

  // Finally, if we are out of the conditional (saw an #endif or ran off the end
  // of the file, just stop skipping and return to lexing whatever came after
  // the #if block.
  CurPPLexer->LexingRawMode = false;


}


//===----------------------------------------------------------------------===//
// Preprocessor Directive Handling.
//===----------------------------------------------------------------------===//

class Preprocessor::ResetMacroExpansionHelper {
public:
  ResetMacroExpansionHelper(Preprocessor *pp)
    : PP(pp), save(pp->DisableMacroExpansion) {
    if (pp->MacroExpansionInDirectivesOverride)
      pp->DisableMacroExpansion = false;
  }

  ~ResetMacroExpansionHelper() {
    PP->DisableMacroExpansion = save;
  }

private:
  Preprocessor *PP;
  bool save;
};


/// HandleDirective - This callback is invoked when the lexer sees a # token
/// at the start of a line.  This consumes the directive, modifies the
/// lexer/preprocessor state, and advances the lexer(s) so that the next token
/// read is the correct one.
void Preprocessor::HandleDirective(Token &Result) {
  // FIXME: Traditional: # with whitespace before it not recognized by K&R?

  // We just parsed a # character at the start of a line, so we're in directive
  // mode.  Tell the lexer this so any newlines we see will be converted into an
  // EOD token (which terminates the directive).
  CurPPLexer->ParsingPreprocessorDirective = true;
  if (CurLexer) CurLexer->SetKeepWhitespaceMode(false);

  bool ImmediatelyAfterTopLevelIfndef =
      CurPPLexer->MIOpt.getImmediatelyAfterTopLevelIfndef();
  CurPPLexer->MIOpt.resetImmediatelyAfterTopLevelIfndef();

  ++NumDirectives;

  // We are about to read a token.  For the multiple-include optimization FA to
  // work, we have to remember if we had read any tokens *before* this
  // pp-directive.
  bool ReadAnyTokensBeforeDirective =CurPPLexer->MIOpt.getHasReadAnyTokensVal();

  // Save the '#' token in case we need to return it later.
  Token SavedHash = Result;

  // Read the next token, the directive flavor.  This isn't expanded due to
  // C99 6.10.3p8.
  LexUnexpandedToken(Result);

  // C99 6.10.3p11: Is this preprocessor directive in macro invocation?  e.g.:
  //   #define A(x) #x
  //   A(abc
  //     #warning blah
  //   def)
  // If so, the user is relying on undefined behavior, emit a diagnostic. Do
  // not support this for #include-like directives, since that can result in
  // terrible diagnostics, and does not work in GCC.
  if (InMacroArgs) {
    Diag(Result, diag::ext_embedded_directive);
  }

  // Temporarily enable macro expansion if set so
  // and reset to previous state when returning from this function.
  ResetMacroExpansionHelper helper(this);


  switch (Result.getKind()) {
  case tok::eod:
    return;   // null directive.
  case tok::code_completion:
    if (CodeComplete)
      CodeComplete->CodeCompleteDirective(
                                    CurPPLexer->getConditionalStackDepth() > 0);
    setCodeCompletionReached();
    return;
  default:
    IdentifierInfo *II = Result.getIdentifierInfo();
    if (!II) break; // Not an identifier.

    // Ask what the preprocessor keyword ID is.
    switch (II->getPPKeywordID()) {
    default: break;
    // C99 6.10.1 - Conditional Inclusion.
    case tok::pp_if:
      return HandleIfDirective(Result, SavedHash, ReadAnyTokensBeforeDirective);
    case tok::pp_ifdef:
      return HandleIfdefDirective(Result, SavedHash, false,
                                  true /*not valid for miopt*/);
    case tok::pp_ifndef:
      return HandleIfdefDirective(Result, SavedHash, true,
                                  ReadAnyTokensBeforeDirective);
    case tok::pp_elif:
      return HandleElifDirective(Result, SavedHash);
    case tok::pp_else:
      return HandleElseDirective(Result, SavedHash);
    case tok::pp_endif:
      return HandleEndifDirective(Result);


    // C99 6.10.3 - Macro Replacement.
    case tok::pp_define:
      return HandleDefineDirective(Result, ImmediatelyAfterTopLevelIfndef);
    case tok::pp_undef:
      return HandleUndefDirective();

    }
    break;
  }


  // If we reached here, the preprocessing token is not valid!
  Diag(Result, diag::err_pp_invalid_directive);

  // Read the rest of the PP line.
  DiscardUntilEndOfDirective();

  // Okay, we're done parsing the directive.
}



//===----------------------------------------------------------------------===//
// Preprocessor Macro Directive Handling.
//===----------------------------------------------------------------------===//

/// ReadMacroParameterList - The ( starting a parameter list of a macro
/// definition has just been read.  Lex the rest of the parameters and the
/// closing ), updating MI with what we learn.  Return true if an error occurs
/// parsing the param list.
bool Preprocessor::ReadMacroParameterList(MacroInfo *MI, Token &Tok) {
  SmallVector<IdentifierInfo*, 32> Parameters;

  while (true) {
    LexUnexpandedToken(Tok);
    switch (Tok.getKind()) {
    case tok::r_paren:
      // Found the end of the parameter list.
      if (Parameters.empty())  // #define FOO()
        return false;
      // Otherwise we have #define FOO(A,)
      Diag(Tok, diag::err_pp_expected_ident_in_arg_list);
      return true;
    case tok::ellipsis:  // #define X(... -> C99 varargs


      // Lex the token after the identifier.
      LexUnexpandedToken(Tok);
      if (Tok.isNot(tok::r_paren)) {
        Diag(Tok, diag::err_pp_missing_rparen_in_macro_def);
        return true;
      }
      // Add the __VA_ARGS__ identifier as a parameter.
      Parameters.push_back(Ident__VA_ARGS__);
      MI->setIsC99Varargs();
      MI->setParameterList(Parameters, BP);
      return false;
    case tok::eod:  // #define X(
      Diag(Tok, diag::err_pp_missing_rparen_in_macro_def);
      return true;
    default:
      // Handle keywords and identifiers here to accept things like
      // #define Foo(for) for.
      IdentifierInfo *II = Tok.getIdentifierInfo();
      if (!II) {
        // #define X(1
        Diag(Tok, diag::err_pp_invalid_tok_in_arg_list);
        return true;
      }

      // If this is already used as a parameter, it is used multiple times (e.g.
      // #define X(A,A.
      if (std::find(Parameters.begin(), Parameters.end(), II) !=
          Parameters.end()) {  // C99 6.10.3p6
        Diag(Tok, diag::err_pp_duplicate_name_in_arg_list) << II;
        return true;
      }

      // Add the parameter to the macro info.
      Parameters.push_back(II);

      // Lex the token after the identifier.
      LexUnexpandedToken(Tok);

      switch (Tok.getKind()) {
      default:          // #define X(A B
        Diag(Tok, diag::err_pp_expected_comma_in_arg_list);
        return true;
      case tok::r_paren: // #define X(A)
        MI->setParameterList(Parameters, BP);
        return false;
      case tok::comma:  // #define X(A,
        break;
      case tok::ellipsis:  // #define X(A... -> GCC extension
        // Diagnose extension.
        Diag(Tok, diag::ext_named_variadic_macro);

        // Lex the token after the identifier.
        LexUnexpandedToken(Tok);
        if (Tok.isNot(tok::r_paren)) {
          Diag(Tok, diag::err_pp_missing_rparen_in_macro_def);
          return true;
        }

        MI->setIsGNUVarargs();
        MI->setParameterList(Parameters, BP);
        return false;
      }
    }
  }
}

static bool isConfigurationPattern(Token &MacroName, MacroInfo *MI) {
  if (MI->getNumTokens() == 1) {
    const Token &Value = MI->getReplacementToken(0);

    // Macro that is identity, like '#define inline inline' is a valid pattern.
    if (MacroName.getKind() == Value.getKind())
      return true;

    // Macro that maps a keyword to the same keyword decorated with leading/
    // trailing underscores is a valid pattern:
    //    #define inline __inline
    //    #define inline __inline__
    //    #define inline _inline (in MS compatibility mode)
    StringRef MacroText = MacroName.getIdentifierInfo()->getName();
    if (IdentifierInfo *II = Value.getIdentifierInfo()) {
      if (!II->isKeyword())
        return false;
      StringRef ValueText = II->getName();
      StringRef TrimmedValue = ValueText;
      if (!ValueText.startswith("__")) {
        if (ValueText.startswith("_"))
          TrimmedValue = TrimmedValue.drop_front(1);
        else
          return false;
      } else {
        TrimmedValue = TrimmedValue.drop_front(2);
        if (TrimmedValue.endswith("__"))
          TrimmedValue = TrimmedValue.drop_back(2);
      }
      return TrimmedValue.equals(MacroText);
    } else {
      return false;
    }
  }

  // #define inline
  return MacroName.is(tok::kw_const) &&
         MI->getNumTokens() == 0;
}

// ReadOptionalMacroParameterListAndBody - This consumes all (i.e. the
// entire line) of the macro's tokens and adds them to MacroInfo, and while
// doing so performs certain validity checks including (but not limited to):
//   - # (stringization) is followed by a macro parameter
//
//  Returns a nullptr if an invalid sequence of tokens is encountered or returns
//  a pointer to a MacroInfo object.

MacroInfo *Preprocessor::ReadOptionalMacroParameterListAndBody(
    const Token &MacroNameTok, const bool ImmediatelyAfterHeaderGuard) {

  Token LastTok = MacroNameTok;
  // Create the new macro.
  MacroInfo *const MI = AllocateMacroInfo(MacroNameTok.getLocation());

  Token Tok;
  LexUnexpandedToken(Tok);

  // Used to un-poison and then re-poison identifiers of the __VA_ARGS__ ilk
  // within their appropriate context.
  VariadicMacroScopeGuard VariadicMacroScopeGuard(*this);

  // If this is a function-like macro definition, parse the argument list,
  // marking each of the identifiers as being used as macro arguments.  Also,
  // check other constraints on the first token of the macro body.
  if (Tok.is(tok::eod)) {
    if (ImmediatelyAfterHeaderGuard) {
      // Save this macro information since it may part of a header guard.
      CurPPLexer->MIOpt.SetDefinedMacro(MacroNameTok.getIdentifierInfo(),
                                        MacroNameTok.getLocation());
    }
    // If there is no body to this macro, we have no special handling here.
  } else if (Tok.hasLeadingSpace()) {
    // This is a normal token with leading space.  Clear the leading space
    // marker on the first token to get proper expansion.
    Tok.clearFlag(Token::LeadingSpace);
  } else if (Tok.is(tok::l_paren)) {
    // This is a function-like macro definition.  Read the argument list.
    MI->setIsFunctionLike();
    if (ReadMacroParameterList(MI, LastTok)) {
      // Throw away the rest of the line.
      if (CurPPLexer->ParsingPreprocessorDirective)
        DiscardUntilEndOfDirective();
      return nullptr;
    }

    // If this is a definition of an ISO C/C++ variadic function-like macro (not
    // using the GNU named varargs extension) inform our variadic scope guard
    // which un-poisons and re-poisons certain identifiers (e.g. __VA_ARGS__)
    // allowed only within the definition of a variadic macro.

    if (MI->isC99Varargs()) {
      VariadicMacroScopeGuard.enterScope();
    }

    // Read the first token after the arg list for down below.
    LexUnexpandedToken(Tok);
  } else  {
    // C99 requires whitespace between the macro definition and the body.  Emit
    // a diagnostic for something like "#define X+".
    Diag(Tok, diag::ext_c99_whitespace_required_after_macro_name);
  }

  if (!Tok.is(tok::eod))
    LastTok = Tok;

  // Read the rest of the macro body.
  if (MI->isObjectLike()) {
    // Object-like macros are very simple, just read their body.
    while (Tok.isNot(tok::eod)) {
      LastTok = Tok;
      MI->AddTokenToBody(Tok);
      // Get the next token of the macro.
      LexUnexpandedToken(Tok);
    }
  } else {
    // Otherwise, read the body of a function-like macro.  While we are at it,
    // check C99 6.10.3.2p1: ensure that # operators are followed by macro
    // parameters in function-like macro expansions.

    VAOptDefinitionContext VAOCtx(*this);

    while (Tok.isNot(tok::eod)) {
      LastTok = Tok;

      if (!Tok.isOneOf(tok::hash, tok::hashat, tok::hashhash)) {
        MI->AddTokenToBody(Tok);

        if (VAOCtx.isVAOptToken(Tok)) {
          // If we're already within a VAOPT, emit an error.
          if (VAOCtx.isInVAOpt()) {
            Diag(Tok, diag::err_pp_vaopt_nested_use);
            return nullptr;
          }
          // Ensure VAOPT is followed by a '(' .
          LexUnexpandedToken(Tok);
          if (Tok.isNot(tok::l_paren)) {
            Diag(Tok, diag::err_pp_missing_lparen_in_vaopt_use);
            return nullptr;
          }
          MI->AddTokenToBody(Tok);
          VAOCtx.sawVAOptFollowedByOpeningParens(Tok.getLocation());
          LexUnexpandedToken(Tok);
          if (Tok.is(tok::hashhash)) {
            Diag(Tok, diag::err_vaopt_paste_at_start);
            return nullptr;
          }
          continue;
        } else if (VAOCtx.isInVAOpt()) {
          if (Tok.is(tok::r_paren)) {
            if (VAOCtx.sawClosingParen()) {
              const unsigned NumTokens = MI->getNumTokens();
              assert(NumTokens >= 3 && "Must have seen at least __VA_OPT__( "
                                       "and a subsequent tok::r_paren");
              if (MI->getReplacementToken(NumTokens - 2).is(tok::hashhash)) {
                Diag(Tok, diag::err_vaopt_paste_at_end);
                return nullptr;
              }
            }
          } else if (Tok.is(tok::l_paren)) {
            VAOCtx.sawOpeningParen(Tok.getLocation());
          }
        }
        // Get the next token of the macro.
        LexUnexpandedToken(Tok);
        continue;
      }



      if (Tok.is(tok::hashhash)) {
        // If we see token pasting, check if it looks like the gcc comma
        // pasting extension.  We'll use this information to suppress
        // diagnostics later on.

        // Get the next token of the macro.
        LexUnexpandedToken(Tok);

        if (Tok.is(tok::eod)) {
          MI->AddTokenToBody(LastTok);
          break;
        }

        unsigned NumTokens = MI->getNumTokens();
        if (NumTokens && Tok.getIdentifierInfo() == Ident__VA_ARGS__ &&
            MI->getReplacementToken(NumTokens-1).is(tok::comma))
          MI->setHasCommaPasting();

        // Things look ok, add the '##' token to the macro.
        MI->AddTokenToBody(LastTok);
        continue;
      }

      // Our Token is a stringization operator.
      // Get the next token of the macro.
      LexUnexpandedToken(Tok);

      // Check for a valid macro arg identifier or __VA_OPT__.
      if (!VAOCtx.isVAOptToken(Tok) &&
          (Tok.getIdentifierInfo() == nullptr ||
           MI->getParameterNum(Tok.getIdentifierInfo()) == -1)) {

        {
          Diag(Tok, diag::err_pp_stringize_not_parameter)
            << LastTok.is(tok::hashat);
          return nullptr;
        }
      }

      // Things look ok, add the '#' and param name tokens to the macro.
      MI->AddTokenToBody(LastTok);

      // If the token following '#' is VAOPT, let the next iteration handle it
      // and check it for correctness, otherwise add the token and prime the
      // loop with the next one.
      if (!VAOCtx.isVAOptToken(Tok)) {
        MI->AddTokenToBody(Tok);
        LastTok = Tok;

        // Get the next token of the macro.
        LexUnexpandedToken(Tok);
      }
    }
    if (VAOCtx.isInVAOpt()) {
      assert(Tok.is(tok::eod) && "Must be at End Of preprocessing Directive");
      Diag(Tok, diag::err_pp_expected_after)
        << LastTok.getKind() << tok::r_paren;
      Diag(VAOCtx.getUnmatchedOpeningParenLoc(), diag::note_matching) << tok::l_paren;
      return nullptr;
    }
  }
  MI->setDefinitionEndLoc(LastTok.getLocation());
  return MI;
}
/// HandleDefineDirective - Implements \#define.  This consumes the entire macro
/// line then lets the caller lex the next real token.
void Preprocessor::HandleDefineDirective(
    Token &DefineTok, const bool ImmediatelyAfterHeaderGuard) {
  ++NumDefined;

  Token MacroNameTok;
  bool MacroShadowsKeyword;
  ReadMacroName(MacroNameTok, MU_Define, &MacroShadowsKeyword);

  // Error reading macro name?  If so, diagnostic already issued.
  if (MacroNameTok.is(tok::eod))
    return;

  // If we are supposed to keep comments in #defines, reenable comment saving
  // mode.
  if (CurLexer) CurLexer->SetCommentRetentionState(KeepMacroComments);

  MacroInfo *const MI = ReadOptionalMacroParameterListAndBody(
      MacroNameTok, ImmediatelyAfterHeaderGuard);

  if (!MI) return;

  if (MacroShadowsKeyword &&
      !isConfigurationPattern(MacroNameTok, MI)) {
    Diag(MacroNameTok, diag::warn_pp_macro_hides_keyword);
  }
  // Check that there is no paste (##) operator at the beginning or end of the
  // replacement list.
  unsigned NumTokens = MI->getNumTokens();
  if (NumTokens != 0) {
    if (MI->getReplacementToken(0).is(tok::hashhash)) {
      Diag(MI->getReplacementToken(0), diag::err_paste_at_start);
      return;
    }
    if (MI->getReplacementToken(NumTokens-1).is(tok::hashhash)) {
      Diag(MI->getReplacementToken(NumTokens-1), diag::err_paste_at_end);
      return;
    }
  }


  // Finally, if this identifier already had a macro defined for it, verify that
  // the macro bodies are identical, and issue diagnostics if they are not.
  if (const MacroInfo *OtherMI=getMacroInfo(MacroNameTok.getIdentifierInfo())) {

    // It is very common for system headers to have tons of macro redefinitions
    // and for warnings to be disabled in system headers.  If this is the case,
    // then don't bother calling MacroInfo::isIdenticalTo.
    if (!getDiagnostics().getSuppressSystemWarnings() ||
        !SourceMgr.isInSystemHeader(DefineTok.getLocation())) {
      if (!OtherMI->isUsed() && OtherMI->isWarnIfUnused())
        Diag(OtherMI->getDefinitionLoc(), diag::pp_macro_not_used);

      // Warn if defining "__LINE__" and other builtins, per C99 6.10.8/4 and
      // C++ [cpp.predefined]p4, but allow it as an extension.
      if (OtherMI->isBuiltinMacro())
        Diag(MacroNameTok, diag::ext_pp_redef_builtin_macro);
      // Macros must be identical.  This means all tokens and whitespace
      // separation must be the same.  C99 6.10.3p2.
      else if (!OtherMI->isAllowRedefinitionsWithoutWarning() &&
               !MI->isIdenticalTo(*OtherMI, *this, /*Syntactic=*/0)) {
        Diag(MI->getDefinitionLoc(), diag::ext_pp_macro_redef)
          << MacroNameTok.getIdentifierInfo();
        Diag(OtherMI->getDefinitionLoc(), diag::note_previous_definition);
      }
    }
    if (OtherMI->isWarnIfUnused())
      WarnUnusedMacroLocs.erase(OtherMI->getDefinitionLoc());
  }

  appendDefMacroDirective(MacroNameTok.getIdentifierInfo(), MI);

  assert(!MI->isUsed());
  // If we need warning for not using the macro, add its location in the
  // warn-because-unused-macro set. If it gets used it will be removed from set.
  if (getSourceManager().isInMainFile(MI->getDefinitionLoc()) &&
      !Diags->isIgnored(diag::pp_macro_not_used, MI->getDefinitionLoc())) {
    MI->setIsWarnIfUnused(true);
    WarnUnusedMacroLocs.insert(MI->getDefinitionLoc());
  }

}

/// HandleUndefDirective - Implements \#undef.
///
void Preprocessor::HandleUndefDirective() {
  ++NumUndefined;

  Token MacroNameTok;
  ReadMacroName(MacroNameTok, MU_Undef);

  // Error reading macro name?  If so, diagnostic already issued.
  if (MacroNameTok.is(tok::eod))
    return;

  // Check to see if this is the last token on the #undef line.
  CheckEndOfDirective("undef");

  // Okay, we have a valid identifier to undef.
  auto *II = MacroNameTok.getIdentifierInfo();
  auto MD = getMacroDefinition(II);
  UndefMacroDirective *Undef = nullptr;

  // If the macro is not defined, this is a noop undef.
  if (const MacroInfo *MI = MD.getMacroInfo()) {
    if (!MI->isUsed() && MI->isWarnIfUnused())
      Diag(MI->getDefinitionLoc(), diag::pp_macro_not_used);

    if (MI->isWarnIfUnused())
      WarnUnusedMacroLocs.erase(MI->getDefinitionLoc());

    Undef = AllocateUndefMacroDirective(MacroNameTok.getLocation());
  }


  if (Undef)
    appendMacroDirective(II, Undef);
}

//===----------------------------------------------------------------------===//
// Preprocessor Conditional Directive Handling.
//===----------------------------------------------------------------------===//

/// HandleIfdefDirective - Implements the \#ifdef/\#ifndef directive.  isIfndef
/// is true when this is a \#ifndef directive.  ReadAnyTokensBeforeDirective is
/// true if any tokens have been returned or pp-directives activated before this
/// \#ifndef has been lexed.
///
void Preprocessor::HandleIfdefDirective(Token &Result,
                                        const Token &HashToken,
                                        bool isIfndef,
                                        bool ReadAnyTokensBeforeDirective) {
  ++NumIf;
  Token DirectiveTok = Result;

  Token MacroNameTok;
  ReadMacroName(MacroNameTok);

  // Error reading macro name?  If so, diagnostic already issued.
  if (MacroNameTok.is(tok::eod)) {
    // Skip code until we get to #endif.  This helps with recovery by not
    // emitting an error when the #endif is reached.
    SkipExcludedConditionalBlock(HashToken.getLocation(),
                                 DirectiveTok.getLocation(),
                                 /*Foundnonskip*/ false, /*FoundElse*/ false);
    return;
  }

  // Check to see if this is the last token on the #if[n]def line.
  CheckEndOfDirective(isIfndef ? "ifndef" : "ifdef");

  IdentifierInfo *MII = MacroNameTok.getIdentifierInfo();
  auto MD = getMacroDefinition(MII);
  MacroInfo *MI = MD.getMacroInfo();

  if (CurPPLexer->getConditionalStackDepth() == 0) {
    // If the start of a top-level #ifdef and if the macro is not defined,
    // inform MIOpt that this might be the start of a proper include guard.
    // Otherwise it is some other form of unknown conditional which we can't
    // handle.
    if (!ReadAnyTokensBeforeDirective && !MI) {
      assert(isIfndef && "#ifdef shouldn't reach here");
      CurPPLexer->MIOpt.EnterTopLevelIfndef(MII, MacroNameTok.getLocation());
    } else
      CurPPLexer->MIOpt.EnterTopLevelConditional();
  }

  // If there is a macro, process it.
  if (MI)  // Mark it used.
    markMacroAsUsed(MI);


  // Should we include the stuff contained by this directive?
  if (PPOpts->SingleFileParseMode && !MI) {
    // In 'single-file-parse mode' undefined identifiers trigger parsing of all
    // the directive blocks.
    CurPPLexer->pushConditionalLevel(DirectiveTok.getLocation(),
                                     /*wasskip*/false, /*foundnonskip*/false,
                                     /*foundelse*/false);
  } else if (!MI == isIfndef) {
    // Yes, remember that we are inside a conditional, then lex the next token.
    CurPPLexer->pushConditionalLevel(DirectiveTok.getLocation(),
                                     /*wasskip*/false, /*foundnonskip*/true,
                                     /*foundelse*/false);
  } else {
    // No, skip the contents of this block.
    SkipExcludedConditionalBlock(HashToken.getLocation(),
                                 DirectiveTok.getLocation(),
                                 /*Foundnonskip*/ false,
                                 /*FoundElse*/ false);
  }
}

/// HandleIfDirective - Implements the \#if directive.
///
void Preprocessor::HandleIfDirective(Token &IfToken,
                                     const Token &HashToken,
                                     bool ReadAnyTokensBeforeDirective) {
  ++NumIf;

  // Parse and evaluate the conditional expression.
  IdentifierInfo *IfNDefMacro = nullptr;
  CurPPLexer->getSourceLocation();
  const DirectiveEvalResult DER = EvaluateDirectiveExpression(IfNDefMacro);
  const bool ConditionalTrue = DER.Conditional;
  CurPPLexer->getSourceLocation();

  // If this condition is equivalent to #ifndef X, and if this is the first
  // directive seen, handle it for the multiple-include optimization.
  if (CurPPLexer->getConditionalStackDepth() == 0) {
    if (!ReadAnyTokensBeforeDirective && IfNDefMacro && ConditionalTrue)
      // FIXME: Pass in the location of the macro name, not the 'if' token.
      CurPPLexer->MIOpt.EnterTopLevelIfndef(IfNDefMacro, IfToken.getLocation());
    else
      CurPPLexer->MIOpt.EnterTopLevelConditional();
  }


  // Should we include the stuff contained by this directive?
  if (PPOpts->SingleFileParseMode && DER.IncludedUndefinedIds) {
    // In 'single-file-parse mode' undefined identifiers trigger parsing of all
    // the directive blocks.
    CurPPLexer->pushConditionalLevel(IfToken.getLocation(), /*wasskip*/false,
                                     /*foundnonskip*/false, /*foundelse*/false);
  } else if (ConditionalTrue) {
    // Yes, remember that we are inside a conditional, then lex the next token.
    CurPPLexer->pushConditionalLevel(IfToken.getLocation(), /*wasskip*/false,
                                   /*foundnonskip*/true, /*foundelse*/false);
  } else {
    // No, skip the contents of this block.
    SkipExcludedConditionalBlock(HashToken.getLocation(), IfToken.getLocation(),
                                 /*Foundnonskip*/ false,
                                 /*FoundElse*/ false);
  }
}

/// HandleEndifDirective - Implements the \#endif directive.
///
void Preprocessor::HandleEndifDirective(Token &EndifToken) {
  ++NumEndif;

  // Check that this is the whole directive.
  CheckEndOfDirective("endif");

  PPConditionalInfo CondInfo;
  if (CurPPLexer->popConditionalLevel(CondInfo)) {
    // No conditionals on the stack: this is an #endif without an #if.
    Diag(EndifToken, diag::err_pp_endif_without_if);
    return;
  }

  // If this the end of a top-level #endif, inform MIOpt.
  if (CurPPLexer->getConditionalStackDepth() == 0)
    CurPPLexer->MIOpt.ExitTopLevelConditional();

  assert(!CondInfo.WasSkipping && !CurPPLexer->LexingRawMode &&
         "This code should only be reachable in the non-skipping case!");

}

/// HandleElseDirective - Implements the \#else directive.
///
void Preprocessor::HandleElseDirective(Token &Result, const Token &HashToken) {
  ++NumElse;

  // #else directive in a non-skipping conditional... start skipping.
  CheckEndOfDirective("else");

  PPConditionalInfo CI;
  if (CurPPLexer->popConditionalLevel(CI)) {
    Diag(Result, diag::pp_err_else_without_if);
    return;
  }

  // If this is a top-level #else, inform the MIOpt.
  if (CurPPLexer->getConditionalStackDepth() == 0)
    CurPPLexer->MIOpt.EnterTopLevelConditional();

  // If this is a #else with a #else before it, report the error.
  if (CI.FoundElse) Diag(Result, diag::pp_err_else_after_else);


  if (PPOpts->SingleFileParseMode && !CI.FoundNonSkip) {
    // In 'single-file-parse mode' undefined identifiers trigger parsing of all
    // the directive blocks.
    CurPPLexer->pushConditionalLevel(CI.IfLoc, /*wasskip*/false,
                                     /*foundnonskip*/false, /*foundelse*/true);
    return;
  }

  // Finally, skip the rest of the contents of this block.
  SkipExcludedConditionalBlock(HashToken.getLocation(), CI.IfLoc,
                               /*Foundnonskip*/ true,
                               /*FoundElse*/ true, Result.getLocation());
}

/// HandleElifDirective - Implements the \#elif directive.
///
void Preprocessor::HandleElifDirective(Token &ElifToken,
                                       const Token &HashToken) {
  ++NumElse;

  // #elif directive in a non-skipping conditional... start skipping.
  // We don't care what the condition is, because we will always skip it (since
  // the block immediately before it was included).
  CurPPLexer->getSourceLocation();
  DiscardUntilEndOfDirective();
  CurPPLexer->getSourceLocation();

  PPConditionalInfo CI;
  if (CurPPLexer->popConditionalLevel(CI)) {
    Diag(ElifToken, diag::pp_err_elif_without_if);
    return;
  }

  // If this is a top-level #elif, inform the MIOpt.
  if (CurPPLexer->getConditionalStackDepth() == 0)
    CurPPLexer->MIOpt.EnterTopLevelConditional();

  // If this is a #elif with a #else before it, report the error.
  if (CI.FoundElse) Diag(ElifToken, diag::pp_err_elif_after_else);


  if (PPOpts->SingleFileParseMode && !CI.FoundNonSkip) {
    // In 'single-file-parse mode' undefined identifiers trigger parsing of all
    // the directive blocks.
    CurPPLexer->pushConditionalLevel(ElifToken.getLocation(), /*wasskip*/false,
                                     /*foundnonskip*/false, /*foundelse*/false);
    return;
  }

  // Finally, skip the rest of the contents of this block.
  SkipExcludedConditionalBlock(
      HashToken.getLocation(), CI.IfLoc, /*Foundnonskip*/ true,
      /*FoundElse*/ CI.FoundElse, ElifToken.getLocation());
}

MacroDirective *
Preprocessor::getLocalMacroDirectiveHistory(const IdentifierInfo *II) const {
  if (!II->hadMacroDefinition())
    return nullptr;
  auto Pos = CurSubmoduleState->Macros.find(II);
  return Pos == CurSubmoduleState->Macros.end() ? nullptr
                                                : Pos->second.getLatest();
}

void Preprocessor::appendMacroDirective(IdentifierInfo *II, MacroDirective *MD){
  assert(MD && "MacroDirective should be non-zero!");
  assert(!MD->getPrevious() && "Already attached to a MacroDirective history.");

  MacroState &StoredMD = CurSubmoduleState->Macros[II];
  auto *OldMD = StoredMD.getLatest();
  MD->setPrevious(OldMD);
  StoredMD.setLatest(MD);


  // Set up the identifier as having associated macro history.
  II->setHasMacroDefinition(true);
  if (!MD->isDefined())
    II->setHasMacroDefinition(false);
}


/// RegisterBuiltinMacro - Register the specified identifier in the identifier
/// table and mark it as a builtin macro to be expanded.
static IdentifierInfo *RegisterBuiltinMacro(Preprocessor &PP, const char *Name){
  // Get the identifier.
  IdentifierInfo *Id = PP.getIdentifierInfo(Name);

  // Mark it as being a macro that is builtin.
  MacroInfo *MI = PP.AllocateMacroInfo(SourceLocation());
  MI->setIsBuiltinMacro();
  PP.appendDefMacroDirective(Id, MI);
  return Id;
}

/// RegisterBuiltinMacros - Register builtin macros, such as __LINE__ with the
/// identifier table.
void Preprocessor::RegisterBuiltinMacros() {
  Ident__LINE__ = RegisterBuiltinMacro(*this, "__LINE__");
  Ident__FILE__ = RegisterBuiltinMacro(*this, "__FILE__");
  Ident__DATE__ = RegisterBuiltinMacro(*this, "__DATE__");
  Ident__TIME__ = RegisterBuiltinMacro(*this, "__TIME__");
  Ident__COUNTER__ = RegisterBuiltinMacro(*this, "__COUNTER__");


  // GCC Extensions.
  Ident__BASE_FILE__     = RegisterBuiltinMacro(*this, "__BASE_FILE__");
  Ident__INCLUDE_LEVEL__ = RegisterBuiltinMacro(*this, "__INCLUDE_LEVEL__");
  Ident__TIMESTAMP__     = RegisterBuiltinMacro(*this, "__TIMESTAMP__");

  Ident__identifier = nullptr;

  // Clang Extensions.
      RegisterBuiltinMacro(*this, "__is_target_environment");

}

/// isTrivialSingleTokenExpansion - Return true if MI, which has a single token
/// in its expansion, currently expands to that token literally.
static bool isTrivialSingleTokenExpansion(const MacroInfo *MI,
                                          const IdentifierInfo *MacroIdent,
                                          Preprocessor &PP) {
  IdentifierInfo *II = MI->getReplacementToken(0).getIdentifierInfo();

  // If the token isn't an identifier, it's always literally expanded.
  if (!II) return true;


  // If the identifier is a macro, and if that macro is enabled, it may be
  // expanded so it's not a trivial expansion.
  if (auto *ExpansionMI = PP.getMacroInfo(II))
    if (ExpansionMI->isEnabled() &&
        // Fast expanding "#define X X" is ok, because X would be disabled.
        II != MacroIdent)
      return false;

  // If this is an object-like macro invocation, it is safe to trivially expand
  // it.
  if (MI->isObjectLike()) return true;

  // If this is a function-like macro invocation, it's safe to trivially expand
  // as long as the identifier is not a macro argument.
  return std::find(MI->param_begin(), MI->param_end(), II) == MI->param_end();
}

/// isNextPPTokenLParen - Determine whether the next preprocessor token to be
/// lexed is a '('.  If so, consume the token and return true, if not, this
/// method should have no observable side-effect on the lexed tokens.
bool Preprocessor::isNextPPTokenLParen() {
  // Do some quick tests for rejection cases.
  unsigned Val;
  if (CurLexer)
    Val = CurLexer->isNextPPTokenLParen();
  else
    Val = CurTokenLexer->isNextTokenLParen();

  if (Val == 2) {
    // We have run off the end.  If it's a source file we don't
    // examine enclosing ones (C99 5.1.1.2p4).  Otherwise walk up the
    // macro stack.
    if (CurPPLexer)
      return false;
    for (const IncludeStackInfo &Entry : llvm::reverse(IncludeMacroStack)) {
      if (Entry.TheLexer)
        Val = Entry.TheLexer->isNextPPTokenLParen();
      else
        Val = Entry.TheTokenLexer->isNextTokenLParen();

      if (Val != 2)
        break;

      // Ran off the end of a source file?
      if (Entry.ThePPLexer)
        return false;
    }
  }

  // Okay, if we know that the token is a '(', lex it and return.  Otherwise we
  // have found something that isn't a '(' or we found the end of the
  // translation unit.  In either case, return false.
  return Val == 1;
}

/// HandleMacroExpandedIdentifier - If an identifier token is read that is to be
/// expanded as a macro, handle it and return the next token as 'Identifier'.
bool Preprocessor::HandleMacroExpandedIdentifier(Token &Identifier,
                                                 const MacroDefinition &M) {
  MacroInfo *MI = M.getMacroInfo();

  // If this is a macro expansion in the "#if !defined(x)" line for the file,
  // then the macro could expand to different things in other contexts, we need
  // to disable the optimization in this case.
  if (CurPPLexer) CurPPLexer->MIOpt.ExpandedMacro();

  // If this is a builtin macro, like __LINE__ or _Pragma, handle it specially.
  if (MI->isBuiltinMacro()) {
    ExpandBuiltinMacro(Identifier);
    return true;
  }

  /// Args - If this is a function-like macro expansion, this contains,
  /// for each macro argument, the list of tokens that were provided to the
  /// invocation.
  MacroArgs *Args = nullptr;

  // Remember where the end of the expansion occurred.  For an object-like
  // macro, this is the identifier.  For a function-like macro, this is the ')'.
  SourceLocation ExpansionEnd = Identifier.getLocation();

  // If this is a function-like macro, read the arguments.
  if (MI->isFunctionLike()) {
    // Remember that we are now parsing the arguments to a macro invocation.
    // Preprocessor directives used inside macro arguments are not portable, and
    // this enables the warning.
    InMacroArgs = true;
    Args = ReadMacroCallArgumentList(Identifier, MI, ExpansionEnd);

    // Finished parsing args.
    InMacroArgs = false;

    // If there was an error parsing the arguments, bail out.
    if (!Args) return true;

    ++NumFnMacroExpanded;
  } else {
    ++NumMacroExpanded;
  }

  // Notice that this macro has been used.
  markMacroAsUsed(MI);

  // Remember where the token is expanded.
  SourceLocation ExpandLoc = Identifier.getLocation();
  SourceRange ExpansionRange(ExpandLoc, ExpansionEnd);


  // If the macro definition is ambiguous, complain.
  if (M.isAmbiguous()) {
    Diag(Identifier, diag::warn_pp_ambiguous_macro)
      << Identifier.getIdentifierInfo();
    Diag(MI->getDefinitionLoc(), diag::note_pp_ambiguous_macro_chosen)
      << Identifier.getIdentifierInfo();
    M.forAllDefinitions([&](const MacroInfo *OtherMI) {
      if (OtherMI != MI)
        Diag(OtherMI->getDefinitionLoc(), diag::note_pp_ambiguous_macro_other)
          << Identifier.getIdentifierInfo();
    });
  }

  // If we started lexing a macro, enter the macro expansion body.

  // If this macro expands to no tokens, don't bother to push it onto the
  // expansion stack, only to take it right back off.
  if (MI->getNumTokens() == 0) {
    // No need for arg info.
    if (Args) Args->destroy(*this);

    // Propagate whitespace info as if we had pushed, then popped,
    // a macro context.
    Identifier.setFlag(Token::LeadingEmptyMacro);
    PropagateLineStartLeadingSpaceInfo(Identifier);
    ++NumFastMacroExpanded;
    return false;
  } else if (MI->getNumTokens() == 1 &&
             isTrivialSingleTokenExpansion(MI, Identifier.getIdentifierInfo(),
                                           *this)) {
    // Otherwise, if this macro expands into a single trivially-expanded
    // token: expand it now.  This handles common cases like
    // "#define VAL 42".

    // No need for arg info.
    if (Args) Args->destroy(*this);

    // Propagate the isAtStartOfLine/hasLeadingSpace markers of the macro
    // identifier to the expanded token.
    bool isAtStartOfLine = Identifier.isAtStartOfLine();
    bool hasLeadingSpace = Identifier.hasLeadingSpace();

    // Replace the result token.
    Identifier = MI->getReplacementToken(0);

    // Restore the StartOfLine/LeadingSpace markers.
    Identifier.setFlagValue(Token::StartOfLine , isAtStartOfLine);
    Identifier.setFlagValue(Token::LeadingSpace, hasLeadingSpace);

    // Update the tokens location to include both its expansion and physical
    // locations.
    SourceLocation Loc =
      SourceMgr.createExpansionLoc(Identifier.getLocation(), ExpandLoc,
                                   ExpansionEnd,Identifier.getLength());
    Identifier.setLocation(Loc);

    // If this is a disabled macro or #define X X, we must mark the result as
    // unexpandable.
    if (IdentifierInfo *NewII = Identifier.getIdentifierInfo()) {
      if (MacroInfo *NewMI = getMacroInfo(NewII))
        if (!NewMI->isEnabled() || NewMI == MI) {
          Identifier.setFlag(Token::DisableExpand);
          // Don't warn for "#define X X" like "#define bool bool" from
          // stdbool.h.
          if (NewMI != MI || MI->isFunctionLike())
            Diag(Identifier, diag::pp_disabled_macro_expansion);
        }
    }

    // Since this is not an identifier token, it can't be macro expanded, so
    // we're done.
    ++NumFastMacroExpanded;
    return true;
  }

  // Start expanding the macro.
  EnterMacro(Identifier, ExpansionEnd, MI, Args);
  return false;
}

enum Bracket {
  Brace,
  Paren
};

/// CheckMatchedBrackets - Returns true if the braces and parentheses in the
/// token vector are properly nested.
static bool CheckMatchedBrackets(const SmallVectorImpl<Token> &Tokens) {
  SmallVector<Bracket, 8> Brackets;
  for (SmallVectorImpl<Token>::const_iterator I = Tokens.begin(),
                                              E = Tokens.end();
       I != E; ++I) {
    if (I->is(tok::l_paren)) {
      Brackets.push_back(Paren);
    } else if (I->is(tok::r_paren)) {
      if (Brackets.empty() || Brackets.back() == Brace)
        return false;
      Brackets.pop_back();
    } else if (I->is(tok::l_brace)) {
      Brackets.push_back(Brace);
    } else if (I->is(tok::r_brace)) {
      if (Brackets.empty() || Brackets.back() == Paren)
        return false;
      Brackets.pop_back();
    }
  }
  return Brackets.empty();
}

/// GenerateNewArgTokens - Returns true if OldTokens can be converted to a new
/// vector of tokens in NewTokens.  The new number of arguments will be placed
/// in NumArgs and the ranges which need to surrounded in parentheses will be
/// in ParenHints.
/// Returns false if the token stream cannot be changed.  If this is because
/// of an initializer list starting a macro argument, the range of those
/// initializer lists will be place in InitLists.
static bool GenerateNewArgTokens(Preprocessor &PP,
                                 SmallVectorImpl<Token> &OldTokens,
                                 SmallVectorImpl<Token> &NewTokens,
                                 unsigned &NumArgs,
                                 SmallVectorImpl<SourceRange> &ParenHints,
                                 SmallVectorImpl<SourceRange> &InitLists) {
  if (!CheckMatchedBrackets(OldTokens))
    return false;

  // Once it is known that the brackets are matched, only a simple count of the
  // braces is needed.
  unsigned Braces = 0;

  // First token of a new macro argument.
  SmallVectorImpl<Token>::iterator ArgStartIterator = OldTokens.begin();

  // First closing brace in a new macro argument.  Used to generate
  // SourceRanges for InitLists.
  SmallVectorImpl<Token>::iterator ClosingBrace = OldTokens.end();
  NumArgs = 0;
  Token TempToken;
  // Set to true when a macro separator token is found inside a braced list.
  // If true, the fixed argument spans multiple old arguments and ParenHints
  // will be updated.
  bool FoundSeparatorToken = false;
  for (SmallVectorImpl<Token>::iterator I = OldTokens.begin(),
                                        E = OldTokens.end();
       I != E; ++I) {
    if (I->is(tok::l_brace)) {
      ++Braces;
    } else if (I->is(tok::r_brace)) {
      --Braces;
      if (Braces == 0 && ClosingBrace == E && FoundSeparatorToken)
        ClosingBrace = I;
    } else if (I->is(tok::eof)) {
      // EOF token is used to separate macro arguments
      if (Braces != 0) {
        // Assume comma separator is actually braced list separator and change
        // it back to a comma.
        FoundSeparatorToken = true;
        I->setKind(tok::comma);
        I->setLength(1);
      } else { // Braces == 0
        // Separator token still separates arguments.
        ++NumArgs;

        // If the argument starts with a brace, it can't be fixed with
        // parentheses.  A different diagnostic will be given.
        if (FoundSeparatorToken && ArgStartIterator->is(tok::l_brace)) {
          InitLists.push_back(
              SourceRange(ArgStartIterator->getLocation(),
                          PP.getLocForEndOfToken(ClosingBrace->getLocation())));
          ClosingBrace = E;
        }

        // Add left paren
        if (FoundSeparatorToken) {
          TempToken.startToken();
          TempToken.setKind(tok::l_paren);
          TempToken.setLocation(ArgStartIterator->getLocation());
          TempToken.setLength(0);
          NewTokens.push_back(TempToken);
        }

        // Copy over argument tokens
        NewTokens.insert(NewTokens.end(), ArgStartIterator, I);

        // Add right paren and store the paren locations in ParenHints
        if (FoundSeparatorToken) {
          SourceLocation Loc = PP.getLocForEndOfToken((I - 1)->getLocation());
          TempToken.startToken();
          TempToken.setKind(tok::r_paren);
          TempToken.setLocation(Loc);
          TempToken.setLength(0);
          NewTokens.push_back(TempToken);
          ParenHints.push_back(SourceRange(ArgStartIterator->getLocation(),
                                           Loc));
        }

        // Copy separator token
        NewTokens.push_back(*I);

        // Reset values
        ArgStartIterator = I + 1;
        FoundSeparatorToken = false;
      }
    }
  }

  return !ParenHints.empty() && InitLists.empty();
}

/// ReadFunctionLikeMacroArgs - After reading "MACRO" and knowing that the next
/// token is the '(' of the macro, this method is invoked to read all of the
/// actual arguments specified for the macro invocation.  This returns null on
/// error.
MacroArgs *Preprocessor::ReadMacroCallArgumentList(Token &MacroName,
                                                   MacroInfo *MI,
                                                   SourceLocation &MacroEnd) {
  // The number of fixed arguments to parse.
  unsigned NumFixedArgsLeft = MI->getNumParams();
  bool isVariadic = MI->isVariadic();

  // Outer loop, while there are more arguments, keep reading them.
  Token Tok;

  // Read arguments as unexpanded tokens.  This avoids issues, e.g., where
  // an argument value in a macro could expand to ',' or '(' or ')'.
  LexUnexpandedToken(Tok);
  assert(Tok.is(tok::l_paren) && "Error computing l-paren-ness?");

  // ArgTokens - Build up a list of tokens that make up each argument.  Each
  // argument is separated by an EOF token.  Use a SmallVector so we can avoid
  // heap allocations in the common case.
  SmallVector<Token, 64> ArgTokens;
  bool ContainsCodeCompletionTok = false;
  bool FoundElidedComma = false;

  SourceLocation TooManyArgsLoc;

  unsigned NumActuals = 0;
  while (Tok.isNot(tok::r_paren)) {
    if (ContainsCodeCompletionTok && Tok.isOneOf(tok::eof, tok::eod))
      break;

    assert(Tok.isOneOf(tok::l_paren, tok::comma) &&
           "only expect argument separators here");

    size_t ArgTokenStart = ArgTokens.size();
    SourceLocation ArgStartLoc = Tok.getLocation();

    // C99 6.10.3p11: Keep track of the number of l_parens we have seen.  Note
    // that we already consumed the first one.
    unsigned NumParens = 0;

    while (true) {
      // Read arguments as unexpanded tokens.  This avoids issues, e.g., where
      // an argument value in a macro could expand to ',' or '(' or ')'.
      LexUnexpandedToken(Tok);

      if (Tok.isOneOf(tok::eof, tok::eod)) { // "#if f(<eof>" & "#if f(\n"
        if (!ContainsCodeCompletionTok) {
          Diag(MacroName, diag::err_unterm_macro_invoc);
          Diag(MI->getDefinitionLoc(), diag::note_macro_here)
            << MacroName.getIdentifierInfo();
          // Do not lose the EOF/EOD.  Return it to the client.
          MacroName = Tok;
          return nullptr;
        }
        // Do not lose the EOF/EOD.
        auto Toks = llvm::make_unique<Token[]>(1);
        Toks[0] = Tok;
        EnterTokenStream(std::move(Toks), 1, true);
        break;
      } else if (Tok.is(tok::r_paren)) {
        // If we found the ) token, the macro arg list is done.
        if (NumParens-- == 0) {
          MacroEnd = Tok.getLocation();
          if (!ArgTokens.empty() &&
              ArgTokens.back().commaAfterElided()) {
            FoundElidedComma = true;
          }
          break;
        }
      } else if (Tok.is(tok::l_paren)) {
        ++NumParens;
      } else if (Tok.is(tok::comma) && NumParens == 0 &&
                 !(Tok.getFlags() & Token::IgnoredComma)) {
        // In Microsoft-compatibility mode, single commas from nested macro
        // expansions should not be considered as argument separators. We test
        // for this with the IgnoredComma token flag above.

        // Comma ends this argument if there are more fixed arguments expected.
        // However, if this is a variadic macro, and this is part of the
        // variadic part, then the comma is just an argument token.
        if (!isVariadic) break;
        if (NumFixedArgsLeft > 1)
          break;
      } else if (Tok.is(tok::comment) && !KeepMacroComments) {
        // If this is a comment token in the argument list and we're just in
        // -C mode (not -CC mode), discard the comment.
        continue;
      } else if (Tok.getIdentifierInfo() != nullptr) {
        // Reading macro arguments can cause macros that we are currently
        // expanding from to be popped off the expansion stack.  Doing so causes
        // them to be reenabled for expansion.  Here we record whether any
        // identifiers we lex as macro arguments correspond to disabled macros.
        // If so, we mark the token as noexpand.  This is a subtle aspect of
        // C99 6.10.3.4p2.
        if (MacroInfo *MI = getMacroInfo(Tok.getIdentifierInfo()))
          if (!MI->isEnabled())
            Tok.setFlag(Token::DisableExpand);
      } else if (Tok.is(tok::code_completion)) {
        ContainsCodeCompletionTok = true;
        if (CodeComplete)
          CodeComplete->CodeCompleteMacroArgument(MacroName.getIdentifierInfo(),
                                                  MI, NumActuals);
        // Don't mark that we reached the code-completion point because the
        // parser is going to handle the token and there will be another
        // code-completion callback.
      }

      ArgTokens.push_back(Tok);
    }

    // If this was an empty argument list foo(), don't add this as an empty
    // argument.
    if (ArgTokens.empty() && Tok.getKind() == tok::r_paren)
      break;

    // If this is not a variadic macro, and too many args were specified, emit
    // an error.
    if (!isVariadic && NumFixedArgsLeft == 0 && TooManyArgsLoc.isInvalid()) {
      if (ArgTokens.size() != ArgTokenStart)
        TooManyArgsLoc = ArgTokens[ArgTokenStart].getLocation();
      else
        TooManyArgsLoc = ArgStartLoc;
    }


    // Add a marker EOF token to the end of the token list for this argument.
    Token EOFTok;
    EOFTok.startToken();
    EOFTok.setKind(tok::eof);
    EOFTok.setLocation(Tok.getLocation());
    EOFTok.setLength(0);
    ArgTokens.push_back(EOFTok);
    ++NumActuals;
    if (!ContainsCodeCompletionTok && NumFixedArgsLeft != 0)
      --NumFixedArgsLeft;
  }

  // Okay, we either found the r_paren.  Check to see if we parsed too few
  // arguments.
  unsigned MinArgsExpected = MI->getNumParams();

  // If this is not a variadic macro, and too many args were specified, emit
  // an error.
  if (!isVariadic && NumActuals > MinArgsExpected &&
      !ContainsCodeCompletionTok) {
    // Emit the diagnostic at the macro name in case there is a missing ).
    // Emitting it at the , could be far away from the macro name.
    Diag(TooManyArgsLoc, diag::err_too_many_args_in_macro_invoc);
    Diag(MI->getDefinitionLoc(), diag::note_macro_here)
      << MacroName.getIdentifierInfo();

    // Commas from braced initializer lists will be treated as argument
    // separators inside macros.  Attempt to correct for this with parentheses.
    // TODO: See if this can be generalized to angle brackets for templates
    // inside macro arguments.

    SmallVector<Token, 4> FixedArgTokens;
    unsigned FixedNumArgs = 0;
    SmallVector<SourceRange, 4> ParenHints, InitLists;
    if (!GenerateNewArgTokens(*this, ArgTokens, FixedArgTokens, FixedNumArgs,
                              ParenHints, InitLists)) {
      if (!InitLists.empty()) {
        DiagnosticBuilder DB =
            Diag(MacroName,
                 diag::note_init_list_at_beginning_of_macro_argument);
        for (SourceRange Range : InitLists)
          DB << Range;
      }
      return nullptr;
    }
    if (FixedNumArgs != MinArgsExpected)
      return nullptr;

    DiagnosticBuilder DB = Diag(MacroName, diag::note_suggest_parens_for_macro);
    for (SourceRange ParenLocation : ParenHints) {
      DB << FixItHint::CreateInsertion(ParenLocation.getBegin(), "(");
      DB << FixItHint::CreateInsertion(ParenLocation.getEnd(), ")");
    }
    ArgTokens.swap(FixedArgTokens);
    NumActuals = FixedNumArgs;
  }

  // See MacroArgs instance var for description of this.
  bool isVarargsElided = false;

  if (ContainsCodeCompletionTok) {
    // Recover from not-fully-formed macro invocation during code-completion.
    Token EOFTok;
    EOFTok.startToken();
    EOFTok.setKind(tok::eof);
    EOFTok.setLocation(Tok.getLocation());
    EOFTok.setLength(0);
    for (; NumActuals < MinArgsExpected; ++NumActuals)
      ArgTokens.push_back(EOFTok);
  }

  if (NumActuals < MinArgsExpected) {
    // There are several cases where too few arguments is ok, handle them now.
    if (NumActuals == 0 && MinArgsExpected == 1) {
      // #define A(X)  or  #define A(...)   ---> A()

      // If there is exactly one argument, and that argument is missing,
      // then we have an empty "()" argument empty list.  This is fine, even if
      // the macro expects one argument (the argument is just empty).
      isVarargsElided = MI->isVariadic();
    } else if ((FoundElidedComma || MI->isVariadic()) &&
               (NumActuals+1 == MinArgsExpected ||  // A(x, ...) -> A(X)
                (NumActuals == 0 && MinArgsExpected == 2))) {// A(x,...) -> A()
      // Varargs where the named vararg parameter is missing: OK as extension.
      //   #define A(x, ...)
      //   A("blah")
      //
      // If the macro contains the comma pasting extension, the diagnostic
      // is suppressed; we know we'll get another diagnostic later.
      if (!MI->hasCommaPasting()) {
        Diag(Tok, diag::ext_missing_varargs_arg);
        Diag(MI->getDefinitionLoc(), diag::note_macro_here)
          << MacroName.getIdentifierInfo();
      }

      // Remember this occurred, allowing us to elide the comma when used for
      // cases like:
      //   #define A(x, foo...) blah(a, ## foo)
      //   #define B(x, ...) blah(a, ## __VA_ARGS__)
      //   #define C(...) blah(a, ## __VA_ARGS__)
      //  A(x) B(x) C()
      isVarargsElided = true;
    } else if (!ContainsCodeCompletionTok) {
      // Otherwise, emit the error.
      Diag(Tok, diag::err_too_few_args_in_macro_invoc);
      Diag(MI->getDefinitionLoc(), diag::note_macro_here)
        << MacroName.getIdentifierInfo();
      return nullptr;
    }

    // Add a marker EOF token to the end of the token list for this argument.
    SourceLocation EndLoc = Tok.getLocation();
    Tok.startToken();
    Tok.setKind(tok::eof);
    Tok.setLocation(EndLoc);
    Tok.setLength(0);
    ArgTokens.push_back(Tok);

    // If we expect two arguments, add both as empty.
    if (NumActuals == 0 && MinArgsExpected == 2)
      ArgTokens.push_back(Tok);

  } else if (NumActuals > MinArgsExpected && !MI->isVariadic() &&
             !ContainsCodeCompletionTok) {
    // Emit the diagnostic at the macro name in case there is a missing ).
    // Emitting it at the , could be far away from the macro name.
    Diag(MacroName, diag::err_too_many_args_in_macro_invoc);
    Diag(MI->getDefinitionLoc(), diag::note_macro_here)
      << MacroName.getIdentifierInfo();
    return nullptr;
  }

  return MacroArgs::create(MI, ArgTokens, isVarargsElided, *this);
}

/// Keeps macro expanded tokens for TokenLexers.
//
/// Works like a stack; a TokenLexer adds the macro expanded tokens that is
/// going to lex in the cache and when it finishes the tokens are removed
/// from the end of the cache.
Token *Preprocessor::cacheMacroExpandedTokens(TokenLexer *tokLexer,
                                              ArrayRef<Token> tokens) {
  assert(tokLexer);
  if (tokens.empty())
    return nullptr;

  size_t newIndex = MacroExpandedTokens.size();
  bool cacheNeedsToGrow = tokens.size() >
                      MacroExpandedTokens.capacity()-MacroExpandedTokens.size();
  MacroExpandedTokens.append(tokens.begin(), tokens.end());

  if (cacheNeedsToGrow) {
    // Go through all the TokenLexers whose 'Tokens' pointer points in the
    // buffer and update the pointers to the (potential) new buffer array.
    for (const auto &Lexer : MacroExpandingLexersStack) {
      TokenLexer *prevLexer;
      size_t tokIndex;
      std::tie(prevLexer, tokIndex) = Lexer;
      prevLexer->Tokens = MacroExpandedTokens.data() + tokIndex;
    }
  }

  MacroExpandingLexersStack.push_back(std::make_pair(tokLexer, newIndex));
  return MacroExpandedTokens.data() + newIndex;
}

void Preprocessor::removeCachedMacroExpandedTokensOfLastLexer() {
  assert(!MacroExpandingLexersStack.empty());
  size_t tokIndex = MacroExpandingLexersStack.back().second;
  assert(tokIndex < MacroExpandedTokens.size());
  // Pop the cached macro expanded tokens from the end.
  MacroExpandedTokens.resize(tokIndex);
  MacroExpandingLexersStack.pop_back();
}

/// ComputeDATE_TIME - Compute the current time, enter it into the specified
/// scratch buffer, then return DATELoc/TIMELoc locations with the position of
/// the identifier tokens inserted.
static void ComputeDATE_TIME(SourceLocation &DATELoc, SourceLocation &TIMELoc,
                             Preprocessor &PP) {
  time_t TT = time(nullptr);
  struct tm *TM = localtime(&TT);

  static const char * const Months[] = {
    "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
  };

  {
    SmallString<32> TmpBuffer;
    llvm::raw_svector_ostream TmpStream(TmpBuffer);
    TmpStream << llvm::format("\"%s %2d %4d\"", Months[TM->tm_mon],
                              TM->tm_mday, TM->tm_year + 1900);
    Token TmpTok;
    TmpTok.startToken();
    PP.CreateString(TmpStream.str(), TmpTok);
    DATELoc = TmpTok.getLocation();
  }

  {
    SmallString<32> TmpBuffer;
    llvm::raw_svector_ostream TmpStream(TmpBuffer);
    TmpStream << llvm::format("\"%02d:%02d:%02d\"",
                              TM->tm_hour, TM->tm_min, TM->tm_sec);
    Token TmpTok;
    TmpTok.startToken();
    PP.CreateString(TmpStream.str(), TmpTok);
    TIMELoc = TmpTok.getLocation();
  }
}


/// ExpandBuiltinMacro - If an identifier token is read that is to be expanded
/// as a builtin macro, handle it and return the next token as 'Tok'.
void Preprocessor::ExpandBuiltinMacro(Token &Tok) {
  // Figure out which token this is.
  IdentifierInfo *II = Tok.getIdentifierInfo();
  assert(II && "Can't be a macro without id info!");


  ++NumBuiltinMacroExpanded;

  SmallString<128> TmpBuffer;
  llvm::raw_svector_ostream OS(TmpBuffer);

  // Set up the return result.
  Tok.setIdentifierInfo(nullptr);
  Tok.clearFlag(Token::NeedsCleaning);

  if (II == Ident__LINE__) {
    // C99 6.10.8: "__LINE__: The presumed line number (within the current
    // source file) of the current source line (an integer constant)".  This can
    // be affected by #line.
    SourceLocation Loc = Tok.getLocation();

    // Advance to the location of the first _, this might not be the first byte
    // of the token if it starts with an escaped newline.
    Loc = AdvanceToTokenCharacter(Loc, 0);

    // One wrinkle here is that GCC expands __LINE__ to location of the *end* of
    // a macro expansion.  This doesn't matter for object-like macros, but
    // can matter for a function-like macro that expands to contain __LINE__.
    // Skip down through expansion points until we find a file loc for the
    // end of the expansion history.
    Loc = SourceMgr.getExpansionRange(Loc).getEnd();
    PresumedLoc PLoc = SourceMgr.getPresumedLoc(Loc);

    // __LINE__ expands to a simple numeric value.
    OS << (PLoc.isValid()? PLoc.getLine() : 1);
    Tok.setKind(tok::numeric_constant);
  } else if (II == Ident__FILE__ || II == Ident__BASE_FILE__) {
    // C99 6.10.8: "__FILE__: The presumed name of the current source file (a
    // character string literal)". This can be affected by #line.
    PresumedLoc PLoc = SourceMgr.getPresumedLoc(Tok.getLocation());

    // __BASE_FILE__ is a GNU extension that returns the top of the presumed
    // #include stack instead of the current file.
    if (II == Ident__BASE_FILE__ && PLoc.isValid()) {
      SourceLocation NextLoc = PLoc.getIncludeLoc();
      while (NextLoc.isValid()) {
        PLoc = SourceMgr.getPresumedLoc(NextLoc);
        if (PLoc.isInvalid())
          break;

        NextLoc = PLoc.getIncludeLoc();
      }
    }

    // Escape this filename.  Turn '\' -> '\\' '"' -> '\"'
    SmallString<128> FN;
    if (PLoc.isValid()) {
      FN += PLoc.getFilename();
      Lexer::Stringify(FN);
      OS << '"' << FN << '"';
    }
    Tok.setKind(tok::string_literal);
  } else if (II == Ident__DATE__) {
    Diag(Tok.getLocation(), diag::warn_pp_date_time);
    if (!DATELoc.isValid())
      ComputeDATE_TIME(DATELoc, TIMELoc, *this);
    Tok.setKind(tok::string_literal);
    Tok.setLength(strlen("\"Mmm dd yyyy\""));
    Tok.setLocation(SourceMgr.createExpansionLoc(DATELoc, Tok.getLocation(),
                                                 Tok.getLocation(),
                                                 Tok.getLength()));
    return;
  } else if (II == Ident__TIME__) {
    Diag(Tok.getLocation(), diag::warn_pp_date_time);
    if (!TIMELoc.isValid())
      ComputeDATE_TIME(DATELoc, TIMELoc, *this);
    Tok.setKind(tok::string_literal);
    Tok.setLength(strlen("\"hh:mm:ss\""));
    Tok.setLocation(SourceMgr.createExpansionLoc(TIMELoc, Tok.getLocation(),
                                                 Tok.getLocation(),
                                                 Tok.getLength()));
    return;
  } else if (II == Ident__INCLUDE_LEVEL__) {
    // Compute the presumed include depth of this token.  This can be affected
    // by GNU line markers.
    unsigned Depth = 0;

    PresumedLoc PLoc = SourceMgr.getPresumedLoc(Tok.getLocation());
    if (PLoc.isValid()) {
      PLoc = SourceMgr.getPresumedLoc(PLoc.getIncludeLoc());
      for (; PLoc.isValid(); ++Depth)
        PLoc = SourceMgr.getPresumedLoc(PLoc.getIncludeLoc());
    }

    // __INCLUDE_LEVEL__ expands to a simple numeric value.
    OS << Depth;
    Tok.setKind(tok::numeric_constant);
  } else if (II == Ident__TIMESTAMP__) {
    Diag(Tok.getLocation(), diag::warn_pp_date_time);
    // MSVC, ICC, GCC, VisualAge C++ extension.  The generated string should be
    // of the form "Ddd Mmm dd hh::mm::ss yyyy", which is returned by asctime.

    // Get the file that we are lexing out of.  If we're currently lexing from
    // a macro, dig into the include stack.
    const FileEntry *CurFile = nullptr;
    PreprocessorLexer *TheLexer = getCurrentFileLexer();

    if (TheLexer)
      CurFile = SourceMgr.getFileEntryForID(TheLexer->getFileID());

    const char *Result;
    if (CurFile) {
      time_t TT = CurFile->getModificationTime();
      struct tm *TM = localtime(&TT);
      Result = asctime(TM);
    } else {
      Result = "??? ??? ?? ??:??:?? ????\n";
    }
    // Surround the string with " and strip the trailing newline.
    OS << '"' << StringRef(Result).drop_back() << '"';
    Tok.setKind(tok::string_literal);
  } else if (II == Ident__COUNTER__) {
    // __COUNTER__ expands to a simple numeric value.
    OS << CounterValue++;
    Tok.setKind(tok::numeric_constant);
  } else if (II == Ident__identifier) {
    SourceLocation Loc = Tok.getLocation();

    // We're expecting '__identifier' '(' identifier ')'. Try to recover
    // if the parens are missing.
    LexNonComment(Tok);
    if (Tok.isNot(tok::l_paren)) {
      // No '(', use end of last token.
      Diag(getLocForEndOfToken(Loc), diag::err_pp_expected_after)
        << II << tok::l_paren;
      // If the next token isn't valid as our argument, we can't recover.
      if (Tok.getIdentifierInfo())
        Tok.setKind(tok::identifier);
      return;
    }

    SourceLocation LParenLoc = Tok.getLocation();
    LexNonComment(Tok);

    if (Tok.getIdentifierInfo())
      Tok.setKind(tok::identifier);
    else {
      Diag(Tok.getLocation(), diag::err_pp_identifier_arg_not_identifier)
        << Tok.getKind();
      // Don't walk past anything that's not a real token.
      if (Tok.isOneOf(tok::eof, tok::eod))
        return;
    }

    // Discard the ')', preserving 'Tok' as our result.
    Token RParen;
    LexNonComment(RParen);
    if (RParen.isNot(tok::r_paren)) {
      Diag(getLocForEndOfToken(Tok.getLocation()), diag::err_pp_expected_after)
        << Tok.getKind() << tok::r_paren;
      Diag(LParenLoc, diag::note_matching) << tok::l_paren;
    }
    return;
  } else {
    llvm_unreachable("Unknown identifier!");
  }
  CreateString(OS.str(), Tok, Tok.getLocation(), Tok.getLocation());
}

void Preprocessor::markMacroAsUsed(MacroInfo *MI) {
  // If the 'used' status changed, and the macro requires 'unused' warning,
  // remove its SourceLocation from the warn-for-unused-macro locations.
  if (MI->isWarnIfUnused() && !MI->isUsed())
    WarnUnusedMacroLocs.erase(MI->getDefinitionLoc());
  MI->setIsUsed(true);
}
