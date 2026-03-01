//===- R600TargetTransformInfo.h - R600 specific TTI --------*- C++ -*-===//
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
//
/// \file
/// This file a TargetTransformInfoImplBase conforming object specific to the
/// R600 target machine. It uses the target's detailed information to
/// provide more precise answers to certain TTI queries, while letting the
/// target independent and default TTI implementations handle the rest.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_AMDGPU_R600TARGETTRANSFORMINFO_H
#define LLVM_LIB_TARGET_AMDGPU_R600TARGETTRANSFORMINFO_H

#include "AMDGPUTargetTransformInfo.h"
#include "vm/core/CodeGen/BasicTTIImpl.h"

namespace vm::core {

class R600Subtarget;
class AMDGPUTargetLowering;

class R600TTIImpl final : public BasicTTIImplBase<R600TTIImpl> {
  using BaseT = BasicTTIImplBase<R600TTIImpl>;
  using TTI = TargetTransformInfo;

  friend BaseT;

  const R600Subtarget *ST;
  const AMDGPUTargetLowering *TLI;
  AMDGPUTTIImpl CommonTTI;

public:
  explicit R600TTIImpl(const AMDGPUTargetMachine *TM, const Function &F);

  const R600Subtarget *getST() const { return ST; }
  const AMDGPUTargetLowering *getTLI() const { return TLI; }

  void getUnrollingPreferences(Loop *L, ScalarEvolution &SE,
                               TTI::UnrollingPreferences &UP,
                               OptimizationRemarkEmitter *ORE) const override;
  void getPeelingPreferences(Loop *L, ScalarEvolution &SE,
                             TTI::PeelingPreferences &PP) const override;
  unsigned getHardwareNumberOfRegisters(bool Vec) const;
  unsigned getNumberOfRegisters(unsigned ClassID) const override;
  TypeSize
  getRegisterBitWidth(TargetTransformInfo::RegisterKind Vector) const override;
  unsigned getMinVectorRegisterBitWidth() const override;
  unsigned getLoadStoreVecRegBitWidth(unsigned AddrSpace) const override;
  bool isLegalToVectorizeMemChain(unsigned ChainSizeInBytes, Align Alignment,
                                  unsigned AddrSpace) const;
  bool isLegalToVectorizeLoadChain(unsigned ChainSizeInBytes, Align Alignment,
                                   unsigned AddrSpace) const override;
  bool isLegalToVectorizeStoreChain(unsigned ChainSizeInBytes, Align Alignment,
                                    unsigned AddrSpace) const override;
  unsigned getMaxInterleaveFactor(ElementCount VF) const override;
  InstructionCost getCFInstrCost(unsigned Opcode, TTI::TargetCostKind CostKind,
                                 const Instruction *I = nullptr) const override;
  using BaseT::getVectorInstrCost;
  InstructionCost getVectorInstrCost(unsigned Opcode, Type *ValTy,
                                     TTI::TargetCostKind CostKind,
                                     unsigned Index, const Value *Op0,
                                     const Value *Op1) const override;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_AMDGPU_R600TARGETTRANSFORMINFO_H
