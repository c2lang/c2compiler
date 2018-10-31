//===--- TargetBuiltins.h - Target specific builtin IDs ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Enumerates target-specific builtins in their own namespaces within
/// namespace ::c2lang.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_BASIC_TARGETBUILTINS_H
#define LLVM_CLANG_BASIC_TARGETBUILTINS_H

#include <stdint.h>
#include "Clang/Builtins.h"
#undef PPC

namespace c2lang {

  namespace NEON {
  enum {
    LastTIBuiltin = c2lang::Builtin::FirstTSBuiltin - 1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "Clang/BuiltinsNEON.def"
    FirstTSBuiltin
  };
  }

  /// ARM builtins
  namespace ARM {
    enum {
      LastTIBuiltin = c2lang::Builtin::FirstTSBuiltin-1,
      LastNEONBuiltin = NEON::FirstTSBuiltin - 1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "Clang/BuiltinsARM.def"
      LastTSBuiltin
    };
  }

  /// AArch64 builtins
  namespace AArch64 {
  enum {
    LastTIBuiltin = c2lang::Builtin::FirstTSBuiltin - 1,
    LastNEONBuiltin = NEON::FirstTSBuiltin - 1,
  #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
  #include "Clang/BuiltinsAArch64.def"
    LastTSBuiltin
  };
  }

  /// PPC builtins
  namespace PPC {
    enum {
        LastTIBuiltin = c2lang::Builtin::FirstTSBuiltin-1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "Clang/BuiltinsPPC.def"
        LastTSBuiltin
    };
  }

  /// NVPTX builtins
  namespace NVPTX {
    enum {
        LastTIBuiltin = c2lang::Builtin::FirstTSBuiltin-1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "Clang/BuiltinsNVPTX.def"
        LastTSBuiltin
    };
  }

  /// AMDGPU builtins
  namespace AMDGPU {
  enum {
    LastTIBuiltin = c2lang::Builtin::FirstTSBuiltin - 1,
  #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
  #include "Clang/BuiltinsAMDGPU.def"
    LastTSBuiltin
  };
  }

  /// X86 builtins
  namespace X86 {
  enum {
    LastTIBuiltin = c2lang::Builtin::FirstTSBuiltin - 1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "Clang/BuiltinsX86.def"
    FirstX86_64Builtin,
    LastX86CommonBuiltin = FirstX86_64Builtin - 1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "Clang/BuiltinsX86_64.def"
    LastTSBuiltin
  };
  }

  /// Flags to identify the types for overloaded Neon builtins.
  ///
  /// These must be kept in sync with the flags in utils/TableGen/NeonEmitter.h.
  class NeonTypeFlags {
    enum {
      EltTypeMask = 0xf,
      UnsignedFlag = 0x10,
      QuadFlag = 0x20
    };
    uint32_t Flags;

  public:
    enum EltType {
      Int8,
      Int16,
      Int32,
      Int64,
      Poly8,
      Poly16,
      Poly64,
      Poly128,
      Float16,
      Float32,
      Float64
    };

    NeonTypeFlags(unsigned F) : Flags(F) {}
    NeonTypeFlags(EltType ET, bool IsUnsigned, bool IsQuad) : Flags(ET) {
      if (IsUnsigned)
        Flags |= UnsignedFlag;
      if (IsQuad)
        Flags |= QuadFlag;
    }

    EltType getEltType() const { return (EltType)(Flags & EltTypeMask); }
    bool isPoly() const {
      EltType ET = getEltType();
      return ET == Poly8 || ET == Poly16;
    }
    bool isUnsigned() const { return (Flags & UnsignedFlag) != 0; }
    bool isQuad() const { return (Flags & QuadFlag) != 0; }
  };

  /// Hexagon builtins
  namespace Hexagon {
    enum {
        LastTIBuiltin = c2lang::Builtin::FirstTSBuiltin-1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "Clang/BuiltinsHexagon.def"
        LastTSBuiltin
    };
  }

  /// Nios2 builtins
  namespace Nios2 {
  enum {
    LastTIBuiltin = c2lang::Builtin::FirstTSBuiltin - 1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "Clang/BuiltinsNios2.def"
    LastTSBuiltin
  };
  }

  /// MIPS builtins
  namespace Mips {
    enum {
        LastTIBuiltin = c2lang::Builtin::FirstTSBuiltin-1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "Clang/BuiltinsMips.def"
        LastTSBuiltin
    };
  }

  /// XCore builtins
  namespace XCore {
    enum {
        LastTIBuiltin = c2lang::Builtin::FirstTSBuiltin-1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "Clang/BuiltinsXCore.def"
        LastTSBuiltin
    };
  }

  /// Le64 builtins
  namespace Le64 {
  enum {
    LastTIBuiltin = c2lang::Builtin::FirstTSBuiltin - 1,
  #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
  #include "Clang/BuiltinsLe64.def"
    LastTSBuiltin
  };
  }

  /// SystemZ builtins
  namespace SystemZ {
    enum {
        LastTIBuiltin = c2lang::Builtin::FirstTSBuiltin-1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "Clang/BuiltinsSystemZ.def"
        LastTSBuiltin
    };
  }

  /// WebAssembly builtins
  namespace WebAssembly {
    enum {
      LastTIBuiltin = c2lang::Builtin::FirstTSBuiltin-1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "Clang/BuiltinsWebAssembly.def"
      LastTSBuiltin
    };
  }

} // end namespace c2lang.

#endif
