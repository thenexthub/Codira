//===--- TargetBuiltins.h - Target specific builtin IDs ---------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Enumerates target-specific builtins in their own namespaces within
/// namespace ::clang.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_BASIC_TARGETBUILTINS_H
#define LANGUAGE_CORE_BASIC_TARGETBUILTINS_H

#include <algorithm>
#include <stdint.h>
#include "language/Core/Basic/Builtins.h"
#include "toolchain/Support/MathExtras.h"
#undef PPC

namespace language::Core {

  namespace NEON {
  enum {
    LastTIBuiltin = language::Core::Builtin::FirstTSBuiltin - 1,
#define GET_NEON_BUILTIN_ENUMERATORS
#include "language/Core/Basic/arm_neon.inc"
    FirstFp16Builtin,
    LastNeonBuiltin = FirstFp16Builtin - 1,
#include "language/Core/Basic/arm_fp16.inc"
#undef GET_NEON_BUILTIN_ENUMERATORS
    FirstTSBuiltin
  };
  }

  /// ARM builtins
  namespace ARM {
  enum {
    LastTIBuiltin = language::Core::Builtin::FirstTSBuiltin - 1,
    LastNEONBuiltin = NEON::FirstTSBuiltin - 1,
#define GET_MVE_BUILTIN_ENUMERATORS
#include "language/Core/Basic/arm_mve_builtins.inc"
#undef GET_MVE_BUILTIN_ENUMERATORS
    FirstCDEBuiltin,
    LastMVEBuiltin = FirstCDEBuiltin - 1,
#define GET_CDE_BUILTIN_ENUMERATORS
#include "language/Core/Basic/arm_cde_builtins.inc"
#undef GET_CDE_BUILTIN_ENUMERATORS
    FirstARMBuiltin,
    LastCDEBuiltin = FirstARMBuiltin - 1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "language/Core/Basic/BuiltinsARM.def"
    LastTSBuiltin
  };
  }

  namespace SVE {
  enum {
    LastNEONBuiltin = NEON::FirstTSBuiltin - 1,
#define GET_SVE_BUILTIN_ENUMERATORS
#include "language/Core/Basic/arm_sve_builtins.inc"
#undef GET_SVE_BUILTIN_ENUMERATORS
    FirstNeonBridgeBuiltin,
    LastSveBuiltin = FirstNeonBridgeBuiltin - 1,
#define GET_SVE_BUILTINS
#define TARGET_BUILTIN(ID, TYPE, ATTRS, FEATURE) BI##ID,
#include "language/Core/Basic/BuiltinsAArch64NeonSVEBridge.def"
#undef TARGET_BUILTIN
#undef GET_SVE_BUILTINS
    FirstTSBuiltin,
  };
  }

  namespace SME {
  enum {
    LastSVEBuiltin = SVE::FirstTSBuiltin - 1,
#define GET_SME_BUILTIN_ENUMERATORS
#include "language/Core/Basic/arm_sme_builtins.inc"
#undef GET_SME_BUILTIN_ENUMERATORS
    FirstTSBuiltin,
  };
  }

  /// AArch64 builtins
  namespace AArch64 {
  enum {
    LastTIBuiltin = language::Core::Builtin::FirstTSBuiltin - 1,
    LastNEONBuiltin = NEON::FirstTSBuiltin - 1,
    FirstSVEBuiltin = NEON::FirstTSBuiltin,
    LastSVEBuiltin = SVE::FirstTSBuiltin - 1,
    FirstSMEBuiltin = SVE::FirstTSBuiltin,
    LastSMEBuiltin = SME::FirstTSBuiltin - 1,
  #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
  #include "language/Core/Basic/BuiltinsAArch64.def"
    LastTSBuiltin
  };
  }

  /// BPF builtins
  namespace BPF {
  enum {
    LastTIBuiltin = language::Core::Builtin::FirstTSBuiltin - 1,
#define GET_BUILTIN_ENUMERATORS
#include "language/Core/Basic/BuiltinsBPF.inc"
#undef GET_BUILTIN_ENUMERATORS
    LastTSBuiltin
  };
  }

  /// PPC builtins
  namespace PPC {
    enum {
        LastTIBuiltin = language::Core::Builtin::FirstTSBuiltin-1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "language/Core/Basic/BuiltinsPPC.def"
        LastTSBuiltin
    };
  }

  /// NVPTX builtins
  namespace NVPTX {
  enum {
    LastTIBuiltin = language::Core::Builtin::FirstTSBuiltin - 1,
#define GET_BUILTIN_ENUMERATORS
#include "language/Core/Basic/BuiltinsNVPTX.inc"
#undef GET_BUILTIN_ENUMERATORS
    LastTSBuiltin
  };
  }

  /// AMDGPU builtins
  namespace AMDGPU {
  enum {
    LastTIBuiltin = language::Core::Builtin::FirstTSBuiltin - 1,
  #define BUILTIN(ID, TYPE, ATTRS) BI##ID,
  #include "language/Core/Basic/BuiltinsAMDGPU.def"
    LastTSBuiltin
  };
  }

  /// DirectX builtins
  namespace DirectX {
  enum {
    LastTIBuiltin = language::Core::Builtin::FirstTSBuiltin - 1,
#define GET_BUILTIN_ENUMERATORS
#include "language/Core/Basic/BuiltinsDirectX.inc"
#undef GET_BUILTIN_ENUMERATORS
    LastTSBuiltin
  };
  } // namespace DirectX

  /// SPIRV builtins
  namespace SPIRV {
  enum {
    LastTIBuiltin = language::Core::Builtin::FirstTSBuiltin - 1,
#define GET_BUILTIN_ENUMERATORS
#include "language/Core/Basic/BuiltinsSPIRVCommon.inc"
#undef GET_BUILTIN_ENUMERATORS
    FirstVKBuiltin,
    LastCoreBuiltin = FirstVKBuiltin - 1,
#define GET_BUILTIN_ENUMERATORS
#include "language/Core/Basic/BuiltinsSPIRVVK.inc"
#undef GET_BUILTIN_ENUMERATORS
    FirstCLBuiltin,
    LastVKBuiltin = FirstCLBuiltin - 1,
#define GET_BUILTIN_ENUMERATORS
#include "language/Core/Basic/BuiltinsSPIRVCL.inc"
#undef GET_BUILTIN_ENUMERATORS
    LastTSBuiltin
  };
  } // namespace SPIRV

  /// X86 builtins
  namespace X86 {
  enum {
    LastTIBuiltin = language::Core::Builtin::FirstTSBuiltin - 1,
#define GET_BUILTIN_ENUMERATORS
#include "language/Core/Basic/BuiltinsX86.inc"
#undef GET_BUILTIN_ENUMERATORS
    FirstX86_64Builtin,
    LastX86CommonBuiltin = FirstX86_64Builtin - 1,
#define GET_BUILTIN_ENUMERATORS
#include "language/Core/Basic/BuiltinsX86_64.inc"
#undef GET_BUILTIN_ENUMERATORS
    LastTSBuiltin
  };
  }

  /// VE builtins
  namespace VE {
  enum {
    LastTIBuiltin = language::Core::Builtin::FirstTSBuiltin - 1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "language/Core/Basic/BuiltinsVE.def"
    LastTSBuiltin
  };
  }

  namespace RISCVVector {
  enum {
    LastTIBuiltin = language::Core::Builtin::FirstTSBuiltin - 1,
#define GET_RISCVV_BUILTIN_ENUMERATORS
#include "language/Core/Basic/riscv_vector_builtins.inc"
    FirstSiFiveBuiltin,
    LastRVVBuiltin = FirstSiFiveBuiltin - 1,
#include "language/Core/Basic/riscv_sifive_vector_builtins.inc"
    FirstAndesBuiltin,
    LastSiFiveBuiltin = FirstAndesBuiltin - 1,
#include "language/Core/Basic/riscv_andes_vector_builtins.inc"
#undef GET_RISCVV_BUILTIN_ENUMERATORS
    FirstTSBuiltin,
  };
  }

  /// RISCV builtins
  namespace RISCV {
  enum {
    LastTIBuiltin = language::Core::Builtin::FirstTSBuiltin - 1,
    FirstRVVBuiltin = language::Core::Builtin::FirstTSBuiltin,
    LastRVVBuiltin = RISCVVector::FirstTSBuiltin - 1,
#define GET_BUILTIN_ENUMERATORS
#include "language/Core/Basic/BuiltinsRISCV.inc"
#undef GET_BUILTIN_ENUMERATORS
    LastTSBuiltin
  };
  } // namespace RISCV

  /// LoongArch builtins
  namespace LoongArch {
  enum {
    LastTIBuiltin = language::Core::Builtin::FirstTSBuiltin - 1,
#define TARGET_BUILTIN(ID, TYPE, ATTRS, FEATURE) BI##ID,
#include "language/Core/Basic/BuiltinsLoongArchBase.def"
    FirstLSXBuiltin,
    LastBaseBuiltin = FirstLSXBuiltin - 1,
#define TARGET_BUILTIN(ID, TYPE, ATTRS, FEATURE) BI##ID,
#include "language/Core/Basic/BuiltinsLoongArchLSX.def"
    FirstLASXBuiltin,
    LastLSXBuiltin = FirstLASXBuiltin - 1,
#define TARGET_BUILTIN(ID, TYPE, ATTRS, FEATURE) BI##ID,
#include "language/Core/Basic/BuiltinsLoongArchLASX.def"
    LastTSBuiltin
  };
  } // namespace LoongArch

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
      Float64,
      BFloat16,
      MFloat8
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
      return ET == Poly8 || ET == Poly16 || ET == Poly64;
    }
    bool isFloatingPoint() const {
      EltType ET = getEltType();
      return ET == Float16 || ET == Float32 || ET == Float64 || ET == BFloat16;
    }
    bool isUnsigned() const { return (Flags & UnsignedFlag) != 0; }
    bool isQuad() const { return (Flags & QuadFlag) != 0; }
    unsigned getEltSizeInBits() const {
      switch (getEltType()) {
      case Int8:
      case Poly8:
      case MFloat8:
        return 8;
      case Int16:
      case Float16:
      case Poly16:
      case BFloat16:
        return 16;
      case Int32:
      case Float32:
        return 32;
      case Int64:
      case Float64:
      case Poly64:
        return 64;
      case Poly128:
        return 128;
      }
      toolchain_unreachable("Invalid NeonTypeFlag!");
    }
  };

  // Shared between SVE/SME and NEON
  enum ImmCheckType {
#define LLVM_GET_ARM_INTRIN_IMMCHECKTYPES
#include "language/Core/Basic/arm_immcheck_types.inc"
#undef LLVM_GET_ARM_INTRIN_IMMCHECKTYPES
  };

  /// Flags to identify the types for overloaded SVE builtins.
  class SVETypeFlags {
    uint64_t Flags;
    unsigned EltTypeShift;
    unsigned MemEltTypeShift;
    unsigned MergeTypeShift;
    unsigned SplatOperandMaskShift;

  public:
#define LLVM_GET_SVE_TYPEFLAGS
#include "language/Core/Basic/arm_sve_typeflags.inc"
#undef LLVM_GET_SVE_TYPEFLAGS

    enum EltType {
#define LLVM_GET_SVE_ELTTYPES
#include "language/Core/Basic/arm_sve_typeflags.inc"
#undef LLVM_GET_SVE_ELTTYPES
    };

    enum MemEltType {
#define LLVM_GET_SVE_MEMELTTYPES
#include "language/Core/Basic/arm_sve_typeflags.inc"
#undef LLVM_GET_SVE_MEMELTTYPES
    };

    enum MergeType {
#define LLVM_GET_SVE_MERGETYPES
#include "language/Core/Basic/arm_sve_typeflags.inc"
#undef LLVM_GET_SVE_MERGETYPES
    };

    SVETypeFlags(uint64_t F) : Flags(F) {
      EltTypeShift = toolchain::countr_zero(EltTypeMask);
      MemEltTypeShift = toolchain::countr_zero(MemEltTypeMask);
      MergeTypeShift = toolchain::countr_zero(MergeTypeMask);
      SplatOperandMaskShift = toolchain::countr_zero(SplatOperandMask);
    }

    EltType getEltType() const {
      return (EltType)((Flags & EltTypeMask) >> EltTypeShift);
    }

    MemEltType getMemEltType() const {
      return (MemEltType)((Flags & MemEltTypeMask) >> MemEltTypeShift);
    }

    MergeType getMergeType() const {
      return (MergeType)((Flags & MergeTypeMask) >> MergeTypeShift);
    }

    unsigned getSplatOperand() const {
      return ((Flags & SplatOperandMask) >> SplatOperandMaskShift) - 1;
    }

    bool hasSplatOperand() const {
      return Flags & SplatOperandMask;
    }

    bool isLoad() const { return Flags & IsLoad; }
    bool isStore() const { return Flags & IsStore; }
    bool isGatherLoad() const { return Flags & IsGatherLoad; }
    bool isScatterStore() const { return Flags & IsScatterStore; }
    bool isStructLoad() const { return Flags & IsStructLoad; }
    bool isStructStore() const { return Flags & IsStructStore; }
    bool isZExtReturn() const { return Flags & IsZExtReturn; }
    bool isByteIndexed() const { return Flags & IsByteIndexed; }
    bool isOverloadNone() const { return Flags & IsOverloadNone; }
    bool isOverloadWhileOrMultiVecCvt() const {
      return Flags & IsOverloadWhileOrMultiVecCvt;
    }
    bool isOverloadDefault() const { return !(Flags & OverloadKindMask); }
    bool isOverloadWhileRW() const { return Flags & IsOverloadWhileRW; }
    bool isOverloadCvt() const { return Flags & IsOverloadCvt; }
    bool isPrefetch() const { return Flags & IsPrefetch; }
    bool isReverseCompare() const { return Flags & ReverseCompare; }
    bool isAppendSVALL() const { return Flags & IsAppendSVALL; }
    bool isInsertOp1SVALL() const { return Flags & IsInsertOp1SVALL; }
    bool isGatherPrefetch() const { return Flags & IsGatherPrefetch; }
    bool isReverseUSDOT() const { return Flags & ReverseUSDOT; }
    bool isReverseMergeAnyBinOp() const { return Flags & ReverseMergeAnyBinOp; }
    bool isReverseMergeAnyAccOp() const { return Flags & ReverseMergeAnyAccOp; }
    bool isUndef() const { return Flags & IsUndef; }
    bool isTupleCreate() const { return Flags & IsTupleCreate; }
    bool isTupleGet() const { return Flags & IsTupleGet; }
    bool isTupleSet() const { return Flags & IsTupleSet; }
    bool isReadZA() const { return Flags & IsReadZA; }
    bool isWriteZA() const { return Flags & IsWriteZA; }
    bool setsFPMR() const { return Flags & SetsFPMR; }
    bool isReductionQV() const { return Flags & IsReductionQV; }
    uint64_t getBits() const { return Flags; }
    bool isFlagSet(uint64_t Flag) const { return Flags & Flag; }
  };

  /// Hexagon builtins
  namespace Hexagon {
  enum {
    LastTIBuiltin = language::Core::Builtin::FirstTSBuiltin - 1,
#define GET_BUILTIN_ENUMERATORS
#include "language/Core/Basic/BuiltinsHexagon.inc"
#undef GET_BUILTIN_ENUMERATORS
    LastTSBuiltin
  };
  }

  /// MIPS builtins
  namespace Mips {
    enum {
        LastTIBuiltin = language::Core::Builtin::FirstTSBuiltin-1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "language/Core/Basic/BuiltinsMips.def"
        LastTSBuiltin
    };
  }

  /// XCore builtins
  namespace XCore {
    enum {
        LastTIBuiltin = language::Core::Builtin::FirstTSBuiltin-1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "language/Core/Basic/BuiltinsXCore.def"
        LastTSBuiltin
    };
  }

  /// SystemZ builtins
  namespace SystemZ {
    enum {
        LastTIBuiltin = language::Core::Builtin::FirstTSBuiltin-1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "language/Core/Basic/BuiltinsSystemZ.def"
        LastTSBuiltin
    };
  }

  /// WebAssembly builtins
  namespace WebAssembly {
    enum {
      LastTIBuiltin = language::Core::Builtin::FirstTSBuiltin-1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "language/Core/Basic/BuiltinsWebAssembly.def"
      LastTSBuiltin
    };
  }

  static constexpr uint64_t LargestBuiltinID = std::max<uint64_t>(
      {ARM::LastTSBuiltin, AArch64::LastTSBuiltin, BPF::LastTSBuiltin,
       PPC::LastTSBuiltin, NVPTX::LastTSBuiltin, AMDGPU::LastTSBuiltin,
       X86::LastTSBuiltin, VE::LastTSBuiltin, RISCV::LastTSBuiltin,
       Hexagon::LastTSBuiltin, Mips::LastTSBuiltin, XCore::LastTSBuiltin,
       SystemZ::LastTSBuiltin, WebAssembly::LastTSBuiltin});

} // end namespace language::Core.

#endif
