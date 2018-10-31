//===--- RefactoringDiagnostic.h - ------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLING_REFACTORING_REFACTORINGDIAGNOSTIC_H
#define LLVM_CLANG_TOOLING_REFACTORING_REFACTORINGDIAGNOSTIC_H

#include "Clang/Diagnostic.h"
#include "Clang/PartialDiagnostic.h"

namespace c2lang {
namespace diag {
enum {
#define DIAG(ENUM, FLAGS, DEFAULT_MAPPING, DESC, GROUP, SFINAE, NOWERROR,      \
             SHOWINSYSHEADER, CATEGORY)                                        \
  ENUM,
#define REFACTORINGSTART
#include "Clang/DiagnosticRefactoringKinds.inc"
#undef DIAG
  NUM_BUILTIN_REFACTORING_DIAGNOSTICS
};
} // end namespace diag
} // end namespace c2lang

#endif // LLVM_CLANG_TOOLING_REFACTORING_REFACTORINGDIAGNOSTIC_H
