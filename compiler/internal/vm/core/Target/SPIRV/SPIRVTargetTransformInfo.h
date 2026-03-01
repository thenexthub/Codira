//===- SPIRVTargetTransformInfo.h - SPIR-V specific TTI ---------*- C++ -*-===//
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
// \file
// This file contains a TargetTransformInfoImplBase conforming object specific
// to the SPIRV target machine. It uses the target's detailed information to
// provide more precise answers to certain TTI queries, while letting the
// target independent and default TTI implementations handle the rest.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPIRV_SPIRVTARGETTRANSFORMINFO_H
#define LLVM_LIB_TARGET_SPIRV_SPIRVTARGETTRANSFORMINFO_H

#include "SPIRV.h"
#include "SPIRVTargetMachine.h"
#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/CodeGen/BasicTTIImpl.h"

namespace vm::core {
class SPIRVTTIImpl final : public BasicTTIImplBase<SPIRVTTIImpl> {
  using BaseT = BasicTTIImplBase<SPIRVTTIImpl>;
  using TTI = TargetTransformInfo;

  friend BaseT;

  const SPIRVSubtarget *ST;
  const SPIRVTargetLowering *TLI;

  const TargetSubtargetInfo *getST() const { return ST; }
  const SPIRVTargetLowering *getTLI() const { return TLI; }

public:
  explicit SPIRVTTIImpl(const SPIRVTargetMachine *TM, const Function &F)
      : BaseT(TM, F.getDataLayout()), ST(TM->getSubtargetImpl(F)),
        TLI(ST->getTargetLowering()) {}

  TTI::PopcntSupportKind getPopcntSupport(unsigned TyWidth) const override {
    // SPIR-V natively supports OpBitcount, per 3.53.14 in the spec, as such it
    // is reasonable to assume the Op is fast / preferable to the expanded loop.
    // Furthermore, this prevents information being lost if transforms are
    // applied to SPIR-V before lowering to a concrete target.
    if (!isPowerOf2_32(TyWidth) || TyWidth > 64)
      return TTI::PSK_Software; // Arbitrary bit-width INT is not core SPIR-V.
    return TTI::PSK_FastHardware;
  }

  unsigned getFlatAddressSpace() const override {
    // Clang has 2 distinct address space maps. One where
    // default=4=Generic, and one with default=0=Function. This depends on the
    // environment.
    return ST->isShader() ? 0 : 4;
  }
  bool collectFlatAddressOperands(SmallVectorImpl<int> &OpIndexes,
                                  Intrinsic::ID IID) const override;
  Value *rewriteIntrinsicWithAddressSpace(IntrinsicInst *II, Value *OldV,
                                          Value *NewV) const override;

  bool allowVectorElementIndexingUsingGEP() const override { return false; }
};

} // namespace vm::core

#endif // LLVM_LIB_TARGET_SPIRV_SPIRVTARGETTRANSFORMINFO_H
