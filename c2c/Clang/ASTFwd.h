//===--- ASTFwd.h ----------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===--------------------------------------------------------------===//
///
/// \file
/// Forward declaration of all AST node types.
///
//===-------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_ASTFWD_H
#define LLVM_CLANG_AST_ASTFWD_H

namespace c2lang {

class Decl;
#define DECL(DERIVED, BASE) class DERIVED##Decl;
#include "Clang/DeclNodes.inc"
class Stmt;
#define STMT(DERIVED, BASE) class DERIVED;
#include "Clang/StmtNodes.inc"
class Type;
#define TYPE(DERIVED, BASE) class DERIVED##Type;
#include "Clang/TypeNodes.def"
class CXXCtorInitializer;

} // end namespace c2lang

#endif
