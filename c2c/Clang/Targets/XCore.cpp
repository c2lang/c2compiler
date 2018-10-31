//===--- XCore.cpp - Implement XCore target feature support ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements XCore TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "Clang/Targets/XCore.h"
#include "Clang/Builtins.h"
#include "Clang/MacroBuilder.h"
#include "Clang/TargetBuiltins.h"

using namespace c2lang;
using namespace c2lang::targets;

const Builtin::Info XCoreTargetInfo::BuiltinInfo[] = {
#define BUILTIN(ID, TYPE, ATTRS)                                               \
  {#ID, TYPE, ATTRS, nullptr, ALL_LANGUAGES, nullptr},
#define LIBBUILTIN(ID, TYPE, ATTRS, HEADER)                                    \
  {#ID, TYPE, ATTRS, HEADER, ALL_LANGUAGES, nullptr},
#include "Clang/BuiltinsXCore.def"
};

void XCoreTargetInfo::getTargetDefines(const LangOptions &Opts,
                                       MacroBuilder &Builder) const {
  Builder.defineMacro("__XS1B__");
}

ArrayRef<Builtin::Info> XCoreTargetInfo::getTargetBuiltins() const {
  return llvm::makeArrayRef(BuiltinInfo, c2lang::XCore::LastTSBuiltin -
                                             Builtin::FirstTSBuiltin);
}
