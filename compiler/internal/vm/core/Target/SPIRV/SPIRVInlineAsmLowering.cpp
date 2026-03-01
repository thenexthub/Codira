//===--- SPIRVInlineAsmLowering.cpp - Inline Asm lowering -------*- C++ -*-===//
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
// This file implements the lowering of LLVM inline asm calls to machine code
// calls for GlobalISel.
//
//===----------------------------------------------------------------------===//

#include "SPIRVInlineAsmLowering.h"
#include "SPIRVSubtarget.h"
#include "vm/core/IR/IntrinsicInst.h"
#include "vm/core/IR/IntrinsicsSPIRV.h"

using namespace vm::core;

SPIRVInlineAsmLowering::SPIRVInlineAsmLowering(const SPIRVTargetLowering &TLI)
    : InlineAsmLowering(&TLI) {}

bool SPIRVInlineAsmLowering::lowerAsmOperandForConstraint(
    Value *Val, StringRef Constraint, std::vector<MachineOperand> &Ops,
    MachineIRBuilder &MIRBuilder) const {
  Value *ValOp = nullptr;
  if (isa<ConstantInt>(Val)) {
    ValOp = Val;
  } else if (ConstantFP *CFP = dyn_cast<ConstantFP>(Val)) {
    Ops.push_back(MachineOperand::CreateFPImm(CFP));
    return true;
  } else if (auto *II = dyn_cast<IntrinsicInst>(Val)) {
    if (II->getIntrinsicID() == Intrinsic::spv_track_constant) {
      if (isa<ConstantInt>(II->getOperand(0))) {
        ValOp = II->getOperand(0);
      } else if (ConstantFP *CFP = dyn_cast<ConstantFP>(II->getOperand(0))) {
        Ops.push_back(MachineOperand::CreateFPImm(CFP));
        return true;
      }
    }
  }
  return ValOp ? InlineAsmLowering::lowerAsmOperandForConstraint(
                     ValOp, Constraint, Ops, MIRBuilder)
               : false;
}
