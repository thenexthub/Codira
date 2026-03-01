//===- R600MCInstLower.cpp - Lower R600 MachineInstr to an MCInst ---------===//
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
/// Code to lower R600 MachineInstrs to their corresponding MCInst.
//
//===----------------------------------------------------------------------===//
//

#include "AMDGPUMCInstLower.h"
#include "MCTargetDesc/R600MCTargetDesc.h"
#include "R600AsmPrinter.h"
#include "R600Subtarget.h"
#include "vm/core/CodeGen/MachineOperand.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCExpr.h"

using namespace vm::core;

namespace {
class R600MCInstLower : public AMDGPUMCInstLower {
public:
  R600MCInstLower(MCContext &ctx, const R600Subtarget &ST,
                  const AsmPrinter &AP);

  /// Lower a MachineInstr to an MCInst
  void lower(const MachineInstr *MI, MCInst &OutMI) const;
};
} // namespace

R600MCInstLower::R600MCInstLower(MCContext &Ctx, const R600Subtarget &ST,
                                 const AsmPrinter &AP)
    : AMDGPUMCInstLower(Ctx, ST, AP) {}

void R600MCInstLower::lower(const MachineInstr *MI, MCInst &OutMI) const {
  OutMI.setOpcode(MI->getOpcode());
  for (const MachineOperand &MO : MI->explicit_operands()) {
    MCOperand MCOp;
    lowerOperand(MO, MCOp);
    OutMI.addOperand(MCOp);
  }
}

void R600AsmPrinter::emitInstruction(const MachineInstr *MI) {
  R600_MC::verifyInstructionPredicates(MI->getOpcode(),
                                       getSubtargetInfo().getFeatureBits());

  const R600Subtarget &STI = MF->getSubtarget<R600Subtarget>();
  R600MCInstLower MCInstLowering(OutContext, STI, *this);

  StringRef Err;
  if (!STI.getInstrInfo()->verifyInstruction(*MI, Err)) {
    LLVMContext &C = MI->getMF()->getFunction().getContext();
    C.emitError("Illegal instruction detected: " + Err);
    MI->print(errs());
  }

  if (MI->isBundle()) {
    const MachineBasicBlock *MBB = MI->getParent();
    MachineBasicBlock::const_instr_iterator I = ++MI->getIterator();
    while (I != MBB->instr_end() && I->isInsideBundle()) {
      emitInstruction(&*I);
      ++I;
    }
  } else {
    MCInst TmpInst;
    MCInstLowering.lower(MI, TmpInst);
    EmitToStreamer(*OutStreamer, TmpInst);
  }
}

const MCExpr *R600AsmPrinter::lowerConstant(const Constant *CV,
                                            const Constant *BaseCV,
                                            uint64_t Offset) {
  if (const MCExpr *E = lowerAddrSpaceCast(TM, CV, OutContext))
    return E;
  return AsmPrinter::lowerConstant(CV, BaseCV, Offset);
}
