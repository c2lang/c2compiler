//===- Sanitizers.h - C Language Family Language Options --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Defines the c2lang::SanitizerKind enum.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_BASIC_SANITIZERS_H
#define LLVM_CLANG_BASIC_SANITIZERS_H

#include "Clang/LLVM.h"
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/MathExtras.h>
#include <cassert>
#include <cstdint>

namespace c2lang {

using SanitizerMask = uint64_t;

namespace SanitizerKind {

// Assign ordinals to possible values of -fsanitize= flag, which we will use as
// bit positions.
enum SanitizerOrdinal : uint64_t {
#define SANITIZER(NAME, ID) SO_##ID,
#define SANITIZER_GROUP(NAME, ID, ALIAS) SO_##ID##Group,
#include "Clang/Sanitizers.def"
  SO_Count
};

// Define the set of sanitizer kinds, as well as the set of sanitizers each
// sanitizer group expands into.
#define SANITIZER(NAME, ID) \
  const SanitizerMask ID = 1ULL << SO_##ID;
#define SANITIZER_GROUP(NAME, ID, ALIAS) \
  const SanitizerMask ID = ALIAS; \
  const SanitizerMask ID##Group = 1ULL << SO_##ID##Group;
#include "Clang/Sanitizers.def"

} // namespace SanitizerKind

struct SanitizerSet {
  /// Check if a certain (single) sanitizer is enabled.
  bool has(SanitizerMask K) const {
    assert(llvm::isPowerOf2_64(K));
    return Mask & K;
  }

  /// Check if one or more sanitizers are enabled.
  bool hasOneOf(SanitizerMask K) const { return Mask & K; }

  /// Enable or disable a certain (single) sanitizer.
  void set(SanitizerMask K, bool Value) {
    assert(llvm::isPowerOf2_64(K));
    Mask = Value ? (Mask | K) : (Mask & ~K);
  }

  /// Disable the sanitizers specified in \p K.
  void clear(SanitizerMask K = SanitizerKind::All) { Mask &= ~K; }

  /// Returns true if at least one sanitizer is enabled.
  bool empty() const { return Mask == 0; }

  /// Bitmask of enabled sanitizers.
  SanitizerMask Mask = 0;
};

/// Parse a single value from a -fsanitize= or -fno-sanitize= value list.
/// Returns a non-zero SanitizerMask, or \c 0 if \p Value is not known.
SanitizerMask parseSanitizerValue(StringRef Value, bool AllowGroups);

/// For each sanitizer group bit set in \p Kinds, set the bits for sanitizers
/// this group enables.
SanitizerMask expandSanitizerGroups(SanitizerMask Kinds);

/// Return the sanitizers which do not affect preprocessing.
inline SanitizerMask getPPTransparentSanitizers() {
  return SanitizerKind::CFI | SanitizerKind::Integer |
         SanitizerKind::ImplicitConversion | SanitizerKind::Nullability |
         SanitizerKind::Undefined;
}

} // namespace c2lang

#endif // LLVM_CLANG_BASIC_SANITIZERS_H
