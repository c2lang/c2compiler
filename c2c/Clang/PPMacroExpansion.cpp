//===--- MacroExpansion.cpp - Top level Macro Expansion -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the top level handling of macro expansion for the
// preprocessor.
//
//===----------------------------------------------------------------------===//

#include "Clang/FileManager.h"
#include "Clang/IdentifierTable.h"
#include "Clang/LLVM.h"
#include "Clang/SourceLocation.h"
#include "Clang/DirectoryLookup.h"
#include "Clang/ExternalPreprocessorSource.h"
#include "Clang/LexDiagnostic.h"
#include "Clang/MacroArgs.h"
#include "Clang/MacroInfo.h"
#include "Clang/Preprocessor.h"
#include "Clang/PreprocessorLexer.h"
#include "Clang/Token.h"
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/DenseSet.h>
#include <llvm/ADT/FoldingSet.h>
#include <llvm/ADT/None.h>
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSwitch.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/Format.h>
#include <llvm/Support/raw_ostream.h>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <ctime>
#include <string>
#include <tuple>
#include <utility>

using namespace c2lang;

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

void Preprocessor::setLoadedMacroDirective(IdentifierInfo *II,
                                           MacroDirective *ED,
                                           MacroDirective *MD) {
  // Normally, when a macro is defined, it goes through appendMacroDirective()
  // above, which chains a macro to previous defines, undefs, etc.
  // However, in a pch, the whole macro history up to the end of the pch is
  // stored, so ASTReader goes through this function instead.
  // However, built-in macros are already registered in the Preprocessor
  // ctor, and ASTWriter stops writing the macro chain at built-in macros,
  // so in that case the chain from the pch needs to be spliced to the existing
  // built-in.

  assert(II && MD);
  MacroState &StoredMD = CurSubmoduleState->Macros[II];

  if (auto *OldMD = StoredMD.getLatest()) {
    // shouldIgnoreMacro() in ASTWriter also stops at macros from the
    // predefines buffer in module builds. However, in module builds, modules
    // are loaded completely before predefines are processed, so StoredMD
    // will be nullptr for them when they're loaded. StoredMD should only be
    // non-nullptr for builtins read from a pch file.
    assert(OldMD->getMacroInfo()->isBuiltinMacro() &&
           "only built-ins should have an entry here");
    assert(!OldMD->getPrevious() && "builtin should only have a single entry");
    ED->setPrevious(OldMD);
    StoredMD.setLatest(MD);
  } else {
    StoredMD = MD;
  }

  // Setup the identifier as having associated macro history.
  II->setHasMacroDefinition(true);
  if (!MD->isDefined())
    II->setHasMacroDefinition(false);
}


void Preprocessor::dumpMacroInfo(const IdentifierInfo *II) {
  const MacroState *State = nullptr;
  auto Pos = CurSubmoduleState->Macros.find(II);
  if (Pos != CurSubmoduleState->Macros.end())
    State = &Pos->second;

  llvm::errs() << "MacroState " << State << " " << II->getNameStart();
  if (State && State->isAmbiguous(*this, II))
    llvm::errs() << " ambiguous";
  llvm::errs() << "\n";

  // Dump local macro directives.
  for (auto *MD = State ? State->getLatest() : nullptr; MD;
       MD = MD->getPrevious()) {
    llvm::errs() << " ";
    MD->dump();
  }


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
  bool FoundElidedComma = false;

  SourceLocation TooManyArgsLoc;

  unsigned NumActuals = 0;
  while (Tok.isNot(tok::r_paren)) {
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
          Diag(MacroName, diag::err_unterm_macro_invoc);
          Diag(MI->getDefinitionLoc(), diag::note_macro_here)
              << MacroName.getIdentifierInfo();
          // Do not lose the EOF/EOD.  Return it to the client.
          MacroName = Tok;
          return nullptr;

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
      } else if (Tok.is(tok::comment)) {
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
    if (NumFixedArgsLeft != 0)
      --NumFixedArgsLeft;
  }

  // Okay, we either found the r_paren.  Check to see if we parsed too few
  // arguments.
  unsigned MinArgsExpected = MI->getNumParams();

  // If this is not a variadic macro, and too many args were specified, emit
  // an error.
  if (!isVariadic && NumActuals > MinArgsExpected) {
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
    } else  {
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

  } else if (NumActuals > MinArgsExpected && !MI->isVariadic()) {
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
