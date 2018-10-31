//===--- TCE.cpp - Implement TCE target feature support -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements TCE TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "Clang/Targets/TCE.h"
#include "Clang/Targets.h"
#include "Clang/MacroBuilder.h"

using namespace c2lang;
using namespace c2lang::targets;

void TCETargetInfo::getTargetDefines(const LangOptions &Opts,
                                     MacroBuilder &Builder) const {
  DefineStd(Builder, "tce", Opts);
  Builder.defineMacro("__TCE__");
  Builder.defineMacro("__TCE_V1__");
}

void TCELETargetInfo::getTargetDefines(const LangOptions &Opts,
                                       MacroBuilder &Builder) const {
  DefineStd(Builder, "tcele", Opts);
  Builder.defineMacro("__TCE__");
  Builder.defineMacro("__TCE_V1__");
  Builder.defineMacro("__TCELE__");
  Builder.defineMacro("__TCELE_V1__");
}
