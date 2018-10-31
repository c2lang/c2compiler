//===- StmtGraphTraits.h - Graph Traits for the class Stmt ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines a template specialization of llvm::GraphTraits to
//  treat ASTs (Stmt*) as graphs
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_STMTGRAPHTRAITS_H
#define LLVM_CLANG_AST_STMTGRAPHTRAITS_H

#include "Clang/Stmt.h"
#include <llvm/ADT/DepthFirstIterator.h>
#include <llvm/ADT/GraphTraits.h>

namespace llvm {

template <> struct GraphTraits<c2lang::Stmt *> {
  using NodeRef = c2lang::Stmt *;
  using ChildIteratorType = c2lang::Stmt::child_iterator;
  using nodes_iterator = llvm::df_iterator<c2lang::Stmt *>;

  static NodeRef getEntryNode(c2lang::Stmt *S) { return S; }

  static ChildIteratorType child_begin(NodeRef N) {
    if (N) return N->child_begin();
    else return ChildIteratorType();
  }

  static ChildIteratorType child_end(NodeRef N) {
    if (N) return N->child_end();
    else return ChildIteratorType();
  }

  static nodes_iterator nodes_begin(c2lang::Stmt* S) {
    return df_begin(S);
  }

  static nodes_iterator nodes_end(c2lang::Stmt* S) {
    return df_end(S);
  }
};

template <> struct GraphTraits<const c2lang::Stmt *> {
  using NodeRef = const c2lang::Stmt *;
  using ChildIteratorType = c2lang::Stmt::const_child_iterator;
  using nodes_iterator = llvm::df_iterator<const c2lang::Stmt *>;

  static NodeRef getEntryNode(const c2lang::Stmt *S) { return S; }

  static ChildIteratorType child_begin(NodeRef N) {
    if (N) return N->child_begin();
    else return ChildIteratorType();
  }

  static ChildIteratorType child_end(NodeRef N) {
    if (N) return N->child_end();
    else return ChildIteratorType();
  }

  static nodes_iterator nodes_begin(const c2lang::Stmt* S) {
    return df_begin(S);
  }

  static nodes_iterator nodes_end(const c2lang::Stmt* S) {
    return df_end(S);
  }
};

} // namespace llvm

#endif // LLVM_CLANG_AST_STMTGRAPHTRAITS_H
