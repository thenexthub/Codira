//===-- LanaiTargetTransformInfo.h - Lanai specific TTI ---------*- C++ -*-===//
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
// This file a TargetTransformInfoImplBase conforming object specific to the
// Lanai target machine. It uses the target's detailed information to
// provide more precise answers to certain TTI queries, while letting the
// target independent and default TTI implementations handle the rest.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LANAI_LANAITARGETTRANSFORMINFO_H
#define LLVM_LIB_TARGET_LANAI_LANAITARGETTRANSFORMINFO_H

#include "Lanai.h"
#include "LanaiSubtarget.h"
#include "LanaiTargetMachine.h"
#include "vm/core/Analysis/TargetTransformInfo.h"
#include "vm/core/CodeGen/BasicTTIImpl.h"
#include "vm/core/CodeGen/TargetLowering.h"
#include "vm/core/Support/MathExtras.h"

namespace vm::core {
class LanaiTTIImpl final : public BasicTTIImplBase<LanaiTTIImpl> {
  typedef BasicTTIImplBase<LanaiTTIImpl> BaseT;
  typedef TargetTransformInfo TTI;
  friend BaseT;

  const LanaiSubtarget *ST;
  const LanaiTargetLowering *TLI;

  const LanaiSubtarget *getST() const { return ST; }
  const LanaiTargetLowering *getTLI() const { return TLI; }

public:
  explicit LanaiTTIImpl(const LanaiTargetMachine *TM, const Function &F)
      : BaseT(TM, F.getDataLayout()), ST(TM->getSubtargetImpl(F)),
        TLI(ST->getTargetLowering()) {}

  bool shouldBuildLookupTables() const override { return false; }

  TargetTransformInfo::PopcntSupportKind
  getPopcntSupport(unsigned TyWidth) const override {
    if (TyWidth == 32)
      return TTI::PSK_FastHardware;
    return TTI::PSK_Software;
  }

  InstructionCost getIntImmCost(const APInt &Imm, Type *Ty,
                                TTI::TargetCostKind CostKind) const override {
    assert(Ty->isIntegerTy());
    unsigned BitSize = Ty->getPrimitiveSizeInBits();
    // There is no cost model for constants with a bit size of 0. Return
    // TCC_Free here, so that constant hoisting will ignore this constant.
    if (BitSize == 0)
      return TTI::TCC_Free;
    // No cost model for operations on integers larger than 64 bit implemented
    // yet.
    if (BitSize > 64)
      return TTI::TCC_Free;

    if (Imm == 0)
      return TTI::TCC_Free;
    if (isInt<16>(Imm.getSExtValue()))
      return TTI::TCC_Basic;
    if (isInt<21>(Imm.getZExtValue()))
      return TTI::TCC_Basic;
    if (isInt<32>(Imm.getSExtValue())) {
      if ((Imm.getSExtValue() & 0xFFFF) == 0)
        return TTI::TCC_Basic;
      return 2 * TTI::TCC_Basic;
    }

    return 4 * TTI::TCC_Basic;
  }

  InstructionCost
  getIntImmCostInst(unsigned Opc, unsigned Idx, const APInt &Imm, Type *Ty,
                    TTI::TargetCostKind CostKind,
                    Instruction *Inst = nullptr) const override {
    return getIntImmCost(Imm, Ty, CostKind);
  }

  InstructionCost
  getIntImmCostIntrin(Intrinsic::ID IID, unsigned Idx, const APInt &Imm,
                      Type *Ty, TTI::TargetCostKind CostKind) const override {
    return getIntImmCost(Imm, Ty, CostKind);
  }

  InstructionCost getArithmeticInstrCost(
      unsigned Opcode, Type *Ty, TTI::TargetCostKind CostKind,
      TTI::OperandValueInfo Op1Info = {TTI::OK_AnyValue, TTI::OP_None},
      TTI::OperandValueInfo Op2Info = {TTI::OK_AnyValue, TTI::OP_None},
      ArrayRef<const Value *> Args = {},
      const Instruction *CxtI = nullptr) const override {
    int ISD = TLI->InstructionOpcodeToISD(Opcode);

    switch (ISD) {
    default:
      return BaseT::getArithmeticInstrCost(Opcode, Ty, CostKind, Op1Info,
                                           Op2Info);
    case ISD::MUL:
    case ISD::SDIV:
    case ISD::UDIV:
    case ISD::UREM:
      // This increases the cost associated with multiplication and division
      // to 64 times what the baseline arithmetic cost is. The arithmetic
      // instruction cost was arbitrarily chosen to reduce the desirability
      // of emitting arithmetic instructions that are emulated in software.
      // TODO: Investigate the performance impact given specialized lowerings.
      return 64 * BaseT::getArithmeticInstrCost(Opcode, Ty, CostKind, Op1Info,
                                                Op2Info);
    }
  }
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_LANAI_LANAITARGETTRANSFORMINFO_H
