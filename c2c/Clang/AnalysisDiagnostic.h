//===--- DiagnosticAnalysis.h - Diagnostics for libanalysis -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_ANALYSIS_ANALYSISDIAGNOSTIC_H
#define LLVM_CLANG_ANALYSIS_ANALYSISDIAGNOSTIC_H

#include "Clang/Diagnostic.h"

namespace c2lang {
  namespace diag {
    enum {
#define DIAG(ENUM,FLAGS,DEFAULT_MAPPING,DESC,GROUP,\
             SFINAE,NOWERROR,SHOWINSYSHEADER,CATEGORY) ENUM,
#define ANALYSISSTART
#include "Clang/DiagnosticAnalysisKinds.inc"
#undef DIAG
      NUM_BUILTIN_ANALYSIS_DIAGNOSTICS
    };
  }  // end namespace diag
}  // end namespace c2lang

#endif
