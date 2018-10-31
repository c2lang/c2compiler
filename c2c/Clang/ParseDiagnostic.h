//===--- DiagnosticParse.h - Diagnostics for libparse -----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_PARSE_PARSEDIAGNOSTIC_H
#define LLVM_CLANG_PARSE_PARSEDIAGNOSTIC_H

#include "Clang/Diagnostic.h"

namespace c2lang {
  namespace diag {
    enum {
#define DIAG(ENUM,FLAGS,DEFAULT_MAPPING,DESC,GROUP,\
             SFINAE,NOWERROR,SHOWINSYSHEADER,CATEGORY) ENUM,
#define PARSESTART
#include "Clang/DiagnosticParseKinds.inc"
#undef DIAG
      NUM_BUILTIN_PARSE_DIAGNOSTICS
    };
  }  // end namespace diag
}  // end namespace c2lang

#endif
