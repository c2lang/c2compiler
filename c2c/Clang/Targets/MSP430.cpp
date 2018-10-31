//===--- MSP430.cpp - Implement MSP430 target feature support -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements MSP430 TargetInfo objects.
//
//===----------------------------------------------------------------------===//

#include "Clang/Targets/MSP430.h"
#include "Clang/MacroBuilder.h"

using namespace c2lang;
using namespace c2lang::targets;

const char *const MSP430TargetInfo::GCCRegNames[] = {
    "r0", "r1", "r2",  "r3",  "r4",  "r5",  "r6",  "r7",
    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
};

ArrayRef<const char *> MSP430TargetInfo::getGCCRegNames() const {
  return llvm::makeArrayRef(GCCRegNames);
}

void MSP430TargetInfo::getTargetDefines(const LangOptions &Opts,
                                        MacroBuilder &Builder) const {
  Builder.defineMacro("MSP430");
  Builder.defineMacro("__MSP430__");
  // FIXME: defines for different 'flavours' of MCU
}
