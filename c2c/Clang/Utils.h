//===- Utils.h - Misc utilities for the front-end ---------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This header contains miscellaneous utilities for various front-end actions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_FRONTEND_UTILS_H
#define LLVM_CLANG_FRONTEND_UTILS_H

#include "Clang/Diagnostic.h"
#include "Clang/LLVM.h"
#include "Clang/VirtualFileSystem.h"
#include <llvm/ADT/IntrusiveRefCntPtr.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/Option/OptSpecifier.h>
#include <cstdint>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace llvm {

class Triple;

namespace opt {


} // namespace opt

} // namespace llvm

namespace c2lang {

class DiagnosticsEngine;
class FrontendOptions;
class HeaderSearch;
class HeaderSearchOptions;
class PCHContainerReader;
class Preprocessor;
class PreprocessorOptions;

/// Apply the header search options to get given HeaderSearch object.
void ApplyHeaderSearchOptions(HeaderSearch &HS, const HeaderSearchOptions &HSOpts);

/// InitializePreprocessor - Initialize the preprocessor getting it and the
/// environment ready to process a single file.
void InitializePreprocessor(Preprocessor &PP, const PreprocessorOptions &PPOpts,
                            const PCHContainerReader &PCHContainerRdr,
                            const FrontendOptions &FEOpts);

// Frontend timing utils

} // namespace c2lang

#endif // LLVM_CLANG_FRONTEND_UTILS_H
