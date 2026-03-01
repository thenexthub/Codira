//===- VETargetTransformInfo.h - VE specific TTI ------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
/// \file
/// This file a TargetTransformInfoImplBase conforming object specific to the
/// VE target machine. It uses the target's detailed information to
/// provide more precise answers to certain TTI queries, while letting the
/// target independent and default TTI implementations handle the rest.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_VE_VETARGETTRANSFORMINFO_H
#define LLVM_LIB_TARGET_VE_VETARGETTRANSFORMINFO_H

#include "VE.h"
#include "VETargetMachine.h"
#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/CodeGen/BasicTTIImpl.h"

static toolchain::Type *getVectorElementType(toolchain::Type *Ty) {
  return toolchain::cast<toolchain::FixedVectorType>(Ty)->getElementType();
}

static toolchain::Type *getLaneType(toolchain::Type *Ty) {
  using namespace vm::core;
  if (!isa<VectorType>(Ty))
    return Ty;
  return getVectorElementType(Ty);
}

static bool isVectorLaneType(toolchain::Type &ElemTy) {
  // check element sizes for vregs
  if (ElemTy.isIntegerTy()) {
    unsigned ScaBits = ElemTy.getScalarSizeInBits();
    return ScaBits == 1 || ScaBits == 32 || ScaBits == 64;
  }
  if (ElemTy.isPointerTy()) {
    return true;
  }
  if (ElemTy.isFloatTy() || ElemTy.isDoubleTy()) {
    return true;
  }
  return false;
}

namespace vm::core {

class VETTIImpl final : public BasicTTIImplBase<VETTIImpl> {
  using BaseT = BasicTTIImplBase<VETTIImpl>;
  friend BaseT;

  const VESubtarget *ST;
  const VETargetLowering *TLI;

  const VESubtarget *getST() const { return ST; }
  const VETargetLowering *getTLI() const { return TLI; }

  bool enableVPU() const { return getST()->enableVPU(); }

  static bool isSupportedReduction(Intrinsic::ID ReductionID) {
#define VEC_VP_CASE(SUFFIX)                                                    \
  case Intrinsic::vp_reduce_##SUFFIX:                                          \
  case Intrinsic::vector_reduce_##SUFFIX:

    switch (ReductionID) {
      VEC_VP_CASE(add)
      VEC_VP_CASE(and)
      VEC_VP_CASE(or)
      VEC_VP_CASE(xor)
      VEC_VP_CASE(smax)
      return true;

    default:
      return false;
    }
#undef VEC_VP_CASE
  }

public:
  explicit VETTIImpl(const VETargetMachine *TM, const Function &F)
      : BaseT(TM, F.getDataLayout()), ST(TM->getSubtargetImpl(F)),
        TLI(ST->getTargetLowering()) {}

  unsigned getNumberOfRegisters(unsigned ClassID) const override {
    bool VectorRegs = (ClassID == 1);
    if (VectorRegs) {
      // TODO report vregs once vector isel is stable.
      return 0;
    }

    return 64;
  }

  TypeSize
  getRegisterBitWidth(TargetTransformInfo::RegisterKind K) const override {
    switch (K) {
    case TargetTransformInfo::RGK_Scalar:
      return TypeSize::getFixed(64);
    case TargetTransformInfo::RGK_FixedWidthVector:
      // TODO report vregs once vector isel is stable.
      return TypeSize::getFixed(0);
    case TargetTransformInfo::RGK_ScalableVector:
      return TypeSize::getScalable(0);
    }

    llvm_unreachable("Unsupported register kind");
  }

  /// \returns How the target needs this vector-predicated operation to be
  /// transformed.
  TargetTransformInfo::VPLegalization
  getVPLegalizationStrategy(const VPIntrinsic &PI) const override {
    using VPLegalization = TargetTransformInfo::VPLegalization;
    return VPLegalization(VPLegalization::Legal, VPLegalization::Legal);
  }

  unsigned getMinVectorRegisterBitWidth() const override {
    // TODO report vregs once vector isel is stable.
    return 0;
  }

  bool shouldBuildRelLookupTables() const override {
    // NEC nld doesn't support relative lookup tables.  It shows following
    // errors.  So, we disable it at the moment.
    //   /opt/nec/ve/bin/nld: src/CMakeFiles/cxxabi_shared.dir/cxa_demangle.cpp
    //   .o(.rodata+0x17b4): reloc against `.L.str.376': error 2
    //   /opt/nec/ve/bin/nld: final link failed: Nonrepresentable section on
    //   output
    return false;
  }

  // Load & Store {
  bool
  isLegalMaskedLoad(Type *DataType, Align Alignment, unsigned /*AddressSpace*/,
                    TargetTransformInfo::MaskKind /*MaskKind*/) const override {
    return isVectorLaneType(*getLaneType(DataType));
  }
  bool isLegalMaskedStore(
      Type *DataType, Align Alignment, unsigned /*AddressSpace*/,
      TargetTransformInfo::MaskKind /*MaskKind*/) const override {
    return isVectorLaneType(*getLaneType(DataType));
  }
  bool isLegalMaskedGather(Type *DataType, Align Alignment) const override {
    return isVectorLaneType(*getLaneType(DataType));
  };
  bool isLegalMaskedScatter(Type *DataType, Align Alignment) const override {
    return isVectorLaneType(*getLaneType(DataType));
  }
  // } Load & Store

  bool shouldExpandReduction(const IntrinsicInst *II) const override {
    if (!enableVPU())
      return true;
    return !isSupportedReduction(II->getIntrinsicID());
  }
};

} // namespace vm::core

#endif // LLVM_LIB_TARGET_VE_VETARGETTRANSFORMINFO_H
