//===--- PPCaching.cpp - Handle caching lexed tokens ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements pieces of the Preprocessor interface that manage the
// caching of lexed tokens.
//
//===----------------------------------------------------------------------===//

#include "Clang/Preprocessor.h"
using namespace c2lang;


// Disable the last EnableBacktrackAtThisPos call.
void Preprocessor::CommitBacktrackedTokens() {
  assert(!BacktrackPositions.empty()
         && "EnableBacktrackAtThisPos was not called!");
  BacktrackPositions.pop_back();
}


void Preprocessor::EraseCachedTokens(CachedTokensRange TokenRange) {
  assert(TokenRange.Begin <= TokenRange.End);
  if (CachedLexPos == TokenRange.Begin && TokenRange.Begin != TokenRange.End) {
    // We have backtracked to the start of the token range as we want to consume
    // them again. Erase the tokens only after consuming then.
    assert(!CachedTokenRangeToErase);
    CachedTokenRangeToErase = TokenRange;
    return;
  }
  // The cached tokens were committed, so they should be erased now.
  assert(TokenRange.End == CachedLexPos);
  CachedTokens.erase(CachedTokens.begin() + TokenRange.Begin,
                     CachedTokens.begin() + TokenRange.End);
  CachedLexPos = TokenRange.Begin;
  ExitCachingLexMode();
}

// Make Preprocessor re-lex the tokens that were lexed since
// EnableBacktrackAtThisPos() was previously called.
void Preprocessor::Backtrack() {
  assert(!BacktrackPositions.empty()
         && "EnableBacktrackAtThisPos was not called!");
  CachedLexPos = BacktrackPositions.back();
  BacktrackPositions.pop_back();
  recomputeCurLexerKind();
}

void Preprocessor::CachingLex(Token &Result) {
  if (!InCachingLexMode())
    return;

  if (CachedLexPos < CachedTokens.size()) {
    Result = CachedTokens[CachedLexPos++];
    // Erase the some of the cached tokens after they are consumed when
    // asked to do so.
    if (CachedTokenRangeToErase &&
        CachedTokenRangeToErase->End == CachedLexPos) {
      EraseCachedTokens(*CachedTokenRangeToErase);
      CachedTokenRangeToErase = None;
    }
    return;
  }

  ExitCachingLexMode();
  Lex(Result);

  if (isBacktrackEnabled()) {
    // Cache the lexed token.
    EnterCachingLexMode();
    CachedTokens.push_back(Result);
    ++CachedLexPos;
    return;
  }

  if (CachedLexPos < CachedTokens.size()) {
    EnterCachingLexMode();
  } else {
    // All cached tokens were consumed.
    CachedTokens.clear();
    CachedLexPos = 0;
  }
}

void Preprocessor::EnterCachingLexMode() {
  if (InCachingLexMode()) {
    assert(CurLexerKind == CLK_CachingLexer && "Unexpected lexer kind");
    return;
  }

  PushIncludeMacroStack();
  CurLexerKind = CLK_CachingLexer;
}


const Token &Preprocessor::PeekAhead(unsigned N) {
  assert(CachedLexPos + N > CachedTokens.size() && "Confused caching.");
  ExitCachingLexMode();
  for (size_t C = CachedLexPos + N - CachedTokens.size(); C > 0; --C) {
    CachedTokens.push_back(Token());
    Lex(CachedTokens.back());
  }
  EnterCachingLexMode();
  return CachedTokens.back();
}


bool Preprocessor::IsPreviousCachedToken(const Token &Tok) const {
  // There's currently no cached token...
  if (!CachedLexPos)
    return false;

  const Token LastCachedTok = CachedTokens[CachedLexPos - 1];
  if (LastCachedTok.getKind() != Tok.getKind())
    return false;

  int RelOffset = 0;
  if ((!getSourceManager().isInSameSLocAddrSpace(
          Tok.getLocation(), getLastCachedTokenLocation(), &RelOffset)) ||
      RelOffset)
    return false;

  return true;
}

void Preprocessor::ReplacePreviousCachedToken(ArrayRef<Token> NewToks) {
  assert(CachedLexPos != 0 && "Expected to have some cached tokens");
  CachedTokens.insert(CachedTokens.begin() + CachedLexPos - 1, NewToks.begin(),
                      NewToks.end());
  CachedTokens.erase(CachedTokens.begin() + CachedLexPos - 1 + NewToks.size());
  CachedLexPos += NewToks.size() - 1;
}
