//===--- AllDiagnostics.h - Aggregate Diagnostic headers --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Includes all the separate Diagnostic headers & some related helpers.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_BASIC_ALLDIAGNOSTICS_H
#define LLVM_CLANG_BASIC_ALLDIAGNOSTICS_H

#include "Clang/ASTDiagnostic.h"
#include "Clang/CommentDiagnostic.h"
#include "Clang/AnalysisDiagnostic.h"
#include "Clang/CrossTUDiagnostic.h"
#include "Clang/DriverDiagnostic.h"
#include "Clang/FrontendDiagnostic.h"
#include "Clang/LexDiagnostic.h"
#include "Clang/ParseDiagnostic.h"
#include "Clang/SemaDiagnostic.h"
#include "Clang/SerializationDiagnostic.h"
#include "Clang/RefactoringDiagnostic.h"

namespace c2lang {
template <size_t SizeOfStr, typename FieldType>
class StringSizerHelper {
  static_assert(SizeOfStr <= FieldType(~0U), "Field too small!");
public:
  enum { Size = SizeOfStr };
};
} // end namespace c2lang

#define STR_SIZE(str, fieldTy) c2lang::StringSizerHelper<sizeof(str)-1, \
                                                        fieldTy>::Size

#endif
