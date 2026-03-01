//===------ BPFTargetTransformInfo.h - BPF specific TTI ---------*- C++ -*-===//
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
// This file uses the target's specific information to
// provide more precise answers to certain TTI queries, while letting the
// target independent and default TTI implementations handle the rest.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_BPF_BPFTARGETTRANSFORMINFO_H
#define LLVM_LIB_TARGET_BPF_BPFTARGETTRANSFORMINFO_H

#include "BPFTargetMachine.h"
#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/CodeGen/BasicTTIImpl.h"
#include "vm/core/Transforms/Utils/ScalarEvolutionExpander.h"

namespace vm::core {
class BPFTTIImpl final : public BasicTTIImplBase<BPFTTIImpl> {
  typedef BasicTTIImplBase<BPFTTIImpl> BaseT;
  typedef TargetTransformInfo TTI;
  friend BaseT;

  const BPFSubtarget *ST;
  const BPFTargetLowering *TLI;

  const BPFSubtarget *getST() const { return ST; }
  const BPFTargetLowering *getTLI() const { return TLI; }

public:
  explicit BPFTTIImpl(const BPFTargetMachine *TM, const Function &F)
      : BaseT(TM, F.getDataLayout()), ST(TM->getSubtargetImpl(F)),
        TLI(ST->getTargetLowering()) {}

  InstructionCost getIntImmCost(const APInt &Imm, Type *Ty,
                                TTI::TargetCostKind CostKind) const override {
    if (Imm.getBitWidth() <= 64 && isInt<32>(Imm.getSExtValue()))
      return TTI::TCC_Free;

    return TTI::TCC_Basic;
  }

  InstructionCost getCmpSelInstrCost(
      unsigned Opcode, Type *ValTy, Type *CondTy, CmpInst::Predicate VecPred,
      TTI::TargetCostKind CostKind,
      TTI::OperandValueInfo Op1Info = {TTI::OK_AnyValue, TTI::OP_None},
      TTI::OperandValueInfo Op2Info = {TTI::OK_AnyValue, TTI::OP_None},
      const toolchain::Instruction *I = nullptr) const override {
    if (Opcode == Instruction::Select)
      return SCEVCheapExpansionBudget.getValue();

    return BaseT::getCmpSelInstrCost(Opcode, ValTy, CondTy, VecPred, CostKind,
                                     Op1Info, Op2Info, I);
  }

  InstructionCost getArithmeticInstrCost(
      unsigned Opcode, Type *Ty, TTI::TargetCostKind CostKind,
      TTI::OperandValueInfo Op1Info = {TTI::OK_AnyValue, TTI::OP_None},
      TTI::OperandValueInfo Op2Info = {TTI::OK_AnyValue, TTI::OP_None},
      ArrayRef<const Value *> Args = {},
      const Instruction *CxtI = nullptr) const override {
    int ISD = TLI->InstructionOpcodeToISD(Opcode);
    if (ISD == ISD::ADD && CostKind == TTI::TCK_RecipThroughput)
      return SCEVCheapExpansionBudget.getValue() + 1;

    return BaseT::getArithmeticInstrCost(Opcode, Ty, CostKind, Op1Info,
                                         Op2Info);
  }

  TTI::MemCmpExpansionOptions
  enableMemCmpExpansion(bool OptSize, bool IsZeroCmp) const override {
    TTI::MemCmpExpansionOptions Options;
    Options.LoadSizes = {8, 4, 2, 1};
    Options.MaxNumLoads = TLI->getMaxExpandSizeMemcmp(OptSize);
    return Options;
  }

  unsigned getMaxNumArgs() const override { return 5; }
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_BPF_BPFTARGETTRANSFORMINFO_H
