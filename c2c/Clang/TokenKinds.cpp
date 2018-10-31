//===--- TokenKinds.cpp - Token Kinds Support -----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements the TokenKind enum and support functions.
//
//===----------------------------------------------------------------------===//

#include "Clang/TokenKinds.h"
#include <llvm/Support/ErrorHandling.h>
using namespace c2lang;

static const char * const TokNames[] = {
#define TOK(X) #X,
#define KEYWORD(X,Y) #X,
#include "Clang/TokenKinds.def"
  nullptr
};

const char *tok::getTokenName(TokenKind Kind) {
  if (Kind < tok::NUM_TOKENS)
    return TokNames[Kind];
  llvm_unreachable("unknown TokenKind");
  return nullptr;
}

const char *tok::getPunctuatorSpelling(TokenKind Kind) {
  switch (Kind) {
#define PUNCTUATOR(X,Y) case X: return Y;
#include "Clang/TokenKinds.def"
  default: break;
  }
  return nullptr;
}

const char *tok::getKeywordSpelling(TokenKind Kind) {
  switch (Kind) {
#define KEYWORD(X,Y) case kw_ ## X: return #X;
#include "Clang/TokenKinds.def"
    default: break;
  }
  return nullptr;
}
