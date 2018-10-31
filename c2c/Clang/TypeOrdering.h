//===-------------- TypeOrdering.h - Total ordering for types ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Allows QualTypes to be sorted and hence used in maps and sets.
///
/// Defines c2lang::QualTypeOrdering, a total ordering on c2lang::QualType,
/// and hence enables QualType values to be sorted and to be used in
/// std::maps, std::sets, llvm::DenseMaps, and llvm::DenseSets.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_TYPEORDERING_H
#define LLVM_CLANG_AST_TYPEORDERING_H

#include "Clang/CanonicalType.h"
#include "Clang/Type.h"
#include <functional>

namespace c2lang {

/// Function object that provides a total ordering on QualType values.
struct QualTypeOrdering {
  bool operator()(QualType T1, QualType T2) const {
    return std::less<void*>()(T1.getAsOpaquePtr(), T2.getAsOpaquePtr());
  }
};

}

namespace llvm {
  template<class> struct DenseMapInfo;

  template<> struct DenseMapInfo<c2lang::QualType> {
    static inline c2lang::QualType getEmptyKey() { return c2lang::QualType(); }

    static inline c2lang::QualType getTombstoneKey() {
      using c2lang::QualType;
      return QualType::getFromOpaquePtr(reinterpret_cast<c2lang::Type *>(-1));
    }

    static unsigned getHashValue(c2lang::QualType Val) {
      return (unsigned)((uintptr_t)Val.getAsOpaquePtr()) ^
            ((unsigned)((uintptr_t)Val.getAsOpaquePtr() >> 9));
    }

    static bool isEqual(c2lang::QualType LHS, c2lang::QualType RHS) {
      return LHS == RHS;
    }
  };

  template<> struct DenseMapInfo<c2lang::CanQualType> {
    static inline c2lang::CanQualType getEmptyKey() {
      return c2lang::CanQualType();
    }

    static inline c2lang::CanQualType getTombstoneKey() {
      using c2lang::CanQualType;
      return CanQualType::getFromOpaquePtr(reinterpret_cast<c2lang::Type *>(-1));
    }

    static unsigned getHashValue(c2lang::CanQualType Val) {
      return (unsigned)((uintptr_t)Val.getAsOpaquePtr()) ^
      ((unsigned)((uintptr_t)Val.getAsOpaquePtr() >> 9));
    }

    static bool isEqual(c2lang::CanQualType LHS, c2lang::CanQualType RHS) {
      return LHS == RHS;
    }
  };
}

#endif
