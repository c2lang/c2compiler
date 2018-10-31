//===--- Le64.cpp - Implement Le64 target feature support -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements Le64 TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "Clang/Targets/Le64.h"
#include "Clang/Targets.h"
#include "Clang/Builtins.h"
#include "Clang/MacroBuilder.h"
#include "Clang/TargetBuiltins.h"

using namespace c2lang;
using namespace c2lang::targets;

const Builtin::Info Le64TargetInfo::BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS)                                               \
  {#ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, nullptr},
#include "Clang/BuiltinsLe64.def"
};

ArrayRef<Builtin::Info> Le64TargetInfo::getTargetBuiltins() const {
  return llvm::makeArrayRef(BuiltinInfo, c2lang::Le64::LastTSBuiltin -
                                             Builtin::FirstTSBuiltin);
}

void Le64TargetInfo::getTargetDefines(const LangOptions &Opts,
                                      MacroBuilder &Builder) const {
  DefineStd(Builder, "unix", Opts);
  defineCPUMacros(Builder, "le64", /*Tuning=*/false);
  Builder.defineMacro("__ELF__");
}
