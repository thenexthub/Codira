//==- WebAssemblyTargetTransformInfo.h - WebAssembly-specific TTI -*- C++ -*-=//
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
///
/// \file
/// This file a TargetTransformInfoImplBase conforming object specific
/// to the WebAssembly target machine.
///
/// It uses the target's detailed information to provide more precise answers to
/// certain TTI queries, while letting the target independent and default TTI
/// implementations handle the rest.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_WEBASSEMBLY_WEBASSEMBLYTARGETTRANSFORMINFO_H
#define LLVM_LIB_TARGET_WEBASSEMBLY_WEBASSEMBLYTARGETTRANSFORMINFO_H

#include "WebAssemblyTargetMachine.h"
#include "vm/core/CodeGen/BasicTTIImpl.h"

namespace vm::core {

class WebAssemblyTTIImpl final : public BasicTTIImplBase<WebAssemblyTTIImpl> {
  typedef BasicTTIImplBase<WebAssemblyTTIImpl> BaseT;
  typedef TargetTransformInfo TTI;
  friend BaseT;

  const WebAssemblySubtarget *ST;
  const WebAssemblyTargetLowering *TLI;

  const WebAssemblySubtarget *getST() const { return ST; }
  const WebAssemblyTargetLowering *getTLI() const { return TLI; }

public:
  WebAssemblyTTIImpl(const WebAssemblyTargetMachine *TM, const Function &F)
      : BaseT(TM, F.getDataLayout()), ST(TM->getSubtargetImpl(F)),
        TLI(ST->getTargetLowering()) {}

  /// \name Scalar TTI Implementations
  /// @{

  // TODO: Implement more Scalar TTI for WebAssembly

  TTI::PopcntSupportKind getPopcntSupport(unsigned TyWidth) const override;

  void getUnrollingPreferences(Loop *L, ScalarEvolution &SE,
                               TTI::UnrollingPreferences &UP,
                               OptimizationRemarkEmitter *ORE) const override;

  /// @}

  /// \name Vector TTI Implementations
  /// @{

  bool enableInterleavedAccessVectorization() const override { return true; }

  unsigned getNumberOfRegisters(unsigned ClassID) const override;
  TypeSize
  getRegisterBitWidth(TargetTransformInfo::RegisterKind K) const override;
  InstructionCost getArithmeticInstrCost(
      unsigned Opcode, Type *Ty, TTI::TargetCostKind CostKind,
      TTI::OperandValueInfo Op1Info = {TTI::OK_AnyValue, TTI::OP_None},
      TTI::OperandValueInfo Op2Info = {TTI::OK_AnyValue, TTI::OP_None},
      ArrayRef<const Value *> Args = {},
      const Instruction *CxtI = nullptr) const override;

  InstructionCost
  getCastInstrCost(unsigned Opcode, Type *Dst, Type *Src,
                   TTI::CastContextHint CCH, TTI::TargetCostKind CostKind,
                   const Instruction *I = nullptr) const override;

  TTI::MemCmpExpansionOptions
  enableMemCmpExpansion(bool OptSize, bool IsZeroCmp) const override;

  InstructionCost getMemoryOpCost(
      unsigned Opcode, Type *Src, Align Alignment, unsigned AddressSpace,
      TTI::TargetCostKind CostKind,
      TTI::OperandValueInfo OpInfo = {TTI::OK_AnyValue, TTI::OP_None},
      const Instruction *I = nullptr) const override;
  InstructionCost getInterleavedMemoryOpCost(
      unsigned Opcode, Type *Ty, unsigned Factor, ArrayRef<unsigned> Indices,
      Align Alignment, unsigned AddressSpace, TTI::TargetCostKind CostKind,
      bool UseMaskForCond, bool UseMaskForGaps) const override;
  using BaseT::getVectorInstrCost;
  InstructionCost getVectorInstrCost(unsigned Opcode, Type *Val,
                                     TTI::TargetCostKind CostKind,
                                     unsigned Index, const Value *Op0,
                                     const Value *Op1) const override;
  InstructionCost getPartialReductionCost(
      unsigned Opcode, Type *InputTypeA, Type *InputTypeB, Type *AccumType,
      ElementCount VF, TTI::PartialReductionExtendKind OpAExtend,
      TTI::PartialReductionExtendKind OpBExtend, std::optional<unsigned> BinOp,
      TTI::TargetCostKind CostKind) const override;
  TTI::ReductionShuffle
  getPreferredExpandedReductionShuffle(const IntrinsicInst *II) const override;

  bool supportsTailCalls() const override;

  bool isProfitableToSinkOperands(Instruction *I,
                                  SmallVectorImpl<Use *> &Ops) const override;

  std::optional<Instruction *>
  instCombineIntrinsic(InstCombiner &IC, IntrinsicInst &II) const override;

  /// @}
};

} // end namespace vm::core

#endif
