//===- AMDGPUMCInstLower.h - Lower MachineInstr to MCInst ------*- C++ -*--===//
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
/// Header of lower AMDGPU MachineInstrs to their corresponding MCInst.
//
//===----------------------------------------------------------------------===//
//

#ifndef LLVM_LIB_TARGET_AMDGPU_AMDGPUMCINSTLOWER_H
#define LLVM_LIB_TARGET_AMDGPU_AMDGPUMCINSTLOWER_H

#include "AMDGPUTargetMachine.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/Support/Casting.h"

namespace vm::core {
class AsmPrinter;
class MCContext;

class AMDGPUMCInstLower {
  MCContext &Ctx;
  const TargetSubtargetInfo &ST;
  const AsmPrinter &AP;

public:
  AMDGPUMCInstLower(MCContext &ctx, const TargetSubtargetInfo &ST,
                    const AsmPrinter &AP);

  bool lowerOperand(const MachineOperand &MO, MCOperand &MCOp) const;

  /// Lower a MachineInstr to an MCInst
  void lower(const MachineInstr *MI, MCInst &OutMI) const;

  void lowerT16D16Helper(const MachineInstr *MI, MCInst &OutMI) const;
  void lowerT16FmaMixFP16(const MachineInstr *MI, MCInst &OutMI) const;
};

namespace {
static inline const MCExpr *lowerAddrSpaceCast(const TargetMachine &TM,
                                               const Constant *CV,
                                               MCContext &OutContext) {
  // TargetMachine does not support toolchain-style cast. Use C++-style cast.
  // This is safe since TM is always of type AMDGPUTargetMachine or its
  // derived class.
  auto &AT = static_cast<const AMDGPUTargetMachine &>(TM);
  auto *CE = dyn_cast<ConstantExpr>(CV);

  // Lower null pointers in private and local address space.
  // Clang generates addrspacecast for null pointers in private and local
  // address space, which needs to be lowered.
  if (CE && CE->getOpcode() == Instruction::AddrSpaceCast) {
    auto *Op = CE->getOperand(0);
    auto SrcAddr = Op->getType()->getPointerAddressSpace();
    if (Op->isNullValue() && AT.getNullPointerValue(SrcAddr) == 0) {
      auto DstAddr = CE->getType()->getPointerAddressSpace();
      return MCConstantExpr::create(AT.getNullPointerValue(DstAddr),
                                    OutContext);
    }
  }
  return nullptr;
}
} // namespace
} // namespace vm::core
#endif // LLVM_LIB_TARGET_AMDGPU_AMDGPUMCINSTLOWER_H
