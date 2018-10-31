//===- BaseSubobject.h - BaseSubobject class --------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides a definition of the BaseSubobject class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_AST_BASESUBOBJECT_H
#define LLVM_CLANG_AST_BASESUBOBJECT_H

#include "Clang/CharUnits.h"
#include <llvm/ADT/DenseMapInfo.h>
#include <llvm/Support/type_traits.h>
#include <cstdint>
#include <utility>

namespace c2lang {

class CXXRecordDecl;

// BaseSubobject - Uniquely identifies a direct or indirect base class.
// Stores both the base class decl and the offset from the most derived class to
// the base class. Used for vtable and VTT generation.
class BaseSubobject {
  /// Base - The base class declaration.
  const CXXRecordDecl *Base;

  /// BaseOffset - The offset from the most derived class to the base class.
  CharUnits BaseOffset;

public:
  BaseSubobject() = default;
  BaseSubobject(const CXXRecordDecl *Base, CharUnits BaseOffset)
      : Base(Base), BaseOffset(BaseOffset) {}

  /// getBase - Returns the base class declaration.
  const CXXRecordDecl *getBase() const { return Base; }

  /// getBaseOffset - Returns the base class offset.
  CharUnits getBaseOffset() const { return BaseOffset; }

  friend bool operator==(const BaseSubobject &LHS, const BaseSubobject &RHS) {
    return LHS.Base == RHS.Base && LHS.BaseOffset == RHS.BaseOffset;
 }
};

} // namespace c2lang

namespace llvm {

template<> struct DenseMapInfo<c2lang::BaseSubobject> {
  static c2lang::BaseSubobject getEmptyKey() {
    return c2lang::BaseSubobject(
      DenseMapInfo<const c2lang::CXXRecordDecl *>::getEmptyKey(),
      c2lang::CharUnits::fromQuantity(DenseMapInfo<int64_t>::getEmptyKey()));
  }

  static c2lang::BaseSubobject getTombstoneKey() {
    return c2lang::BaseSubobject(
      DenseMapInfo<const c2lang::CXXRecordDecl *>::getTombstoneKey(),
      c2lang::CharUnits::fromQuantity(DenseMapInfo<int64_t>::getTombstoneKey()));
  }

  static unsigned getHashValue(const c2lang::BaseSubobject &Base) {
    using PairTy = std::pair<const c2lang::CXXRecordDecl *, c2lang::CharUnits>;

    return DenseMapInfo<PairTy>::getHashValue(PairTy(Base.getBase(),
                                                     Base.getBaseOffset()));
  }

  static bool isEqual(const c2lang::BaseSubobject &LHS,
                      const c2lang::BaseSubobject &RHS) {
    return LHS == RHS;
  }
};

// It's OK to treat BaseSubobject as a POD type.
template <> struct isPodLike<c2lang::BaseSubobject> {
  static const bool value = true;
};

} // namespace llvm

#endif // LLVM_CLANG_AST_BASESUBOBJECT_H
