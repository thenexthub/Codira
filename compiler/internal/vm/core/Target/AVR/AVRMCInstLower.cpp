//===-- AVRMCInstLower.cpp - Convert AVR MachineInstr to an MCInst --------===//
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
// This file contains code to lower AVR MachineInstrs to their corresponding
// MCInst records.
//
//===----------------------------------------------------------------------===//

#include "AVRMCInstLower.h"
#include "AVRInstrInfo.h"
#include "MCTargetDesc/AVRMCAsmInfo.h"

#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/IR/Mangler.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/Support/ErrorHandling.h"

namespace vm::core {

MCOperand
AVRMCInstLower::lowerSymbolOperand(const MachineOperand &MO, MCSymbol *Sym,
                                   const AVRSubtarget &Subtarget) const {
  unsigned char TF = MO.getTargetFlags();
  const MCExpr *Expr = MCSymbolRefExpr::create(Sym, Ctx);

  bool IsNegated = false;
  if (TF & AVRII::MO_NEG) {
    IsNegated = true;
  }

  if (!MO.isJTI() && MO.getOffset()) {
    Expr = MCBinaryExpr::createAdd(
        Expr, MCConstantExpr::create(MO.getOffset(), Ctx), Ctx);
  }

  bool IsFunction = MO.isGlobal() && isa<Function>(MO.getGlobal());

  if (TF & AVRII::MO_LO) {
    if (IsFunction) {
      Expr = AVRMCExpr::create(Subtarget.hasEIJMPCALL() ? AVR::S_LO8_GS
                                                        : AVR::S_PM_LO8,
                               Expr, IsNegated, Ctx);
    } else {
      Expr = AVRMCExpr::create(AVR::S_LO8, Expr, IsNegated, Ctx);
    }
  } else if (TF & AVRII::MO_HI) {
    if (IsFunction) {
      Expr = AVRMCExpr::create(Subtarget.hasEIJMPCALL() ? AVR::S_HI8_GS
                                                        : AVR::S_PM_HI8,
                               Expr, IsNegated, Ctx);
    } else {
      Expr = AVRMCExpr::create(AVR::S_HI8, Expr, IsNegated, Ctx);
    }
  } else if (TF != 0) {
    llvm_unreachable("Unknown target flag on symbol operand");
  }

  return MCOperand::createExpr(Expr);
}

void AVRMCInstLower::lowerInstruction(const MachineInstr &MI,
                                      MCInst &OutMI) const {
  auto &Subtarget = MI.getParent()->getParent()->getSubtarget<AVRSubtarget>();
  OutMI.setOpcode(MI.getOpcode());

  for (MachineOperand const &MO : MI.operands()) {
    MCOperand MCOp;

    switch (MO.getType()) {
    default:
      MI.print(errs());
      llvm_unreachable("unknown operand type");
    case MachineOperand::MO_Register:
      // Ignore all implicit register operands.
      if (MO.isImplicit())
        continue;
      MCOp = MCOperand::createReg(MO.getReg());
      break;
    case MachineOperand::MO_Immediate:
      MCOp = MCOperand::createImm(MO.getImm());
      break;
    case MachineOperand::MO_GlobalAddress:
      MCOp =
          lowerSymbolOperand(MO, Printer.getSymbol(MO.getGlobal()), Subtarget);
      break;
    case MachineOperand::MO_ExternalSymbol:
      MCOp = lowerSymbolOperand(
          MO, Printer.GetExternalSymbolSymbol(MO.getSymbolName()), Subtarget);
      break;
    case MachineOperand::MO_MachineBasicBlock:
      MCOp = MCOperand::createExpr(
          MCSymbolRefExpr::create(MO.getMBB()->getSymbol(), Ctx));
      break;
    case MachineOperand::MO_RegisterMask:
      continue;
    case MachineOperand::MO_BlockAddress:
      MCOp = lowerSymbolOperand(
          MO, Printer.GetBlockAddressSymbol(MO.getBlockAddress()), Subtarget);
      break;
    case MachineOperand::MO_JumpTableIndex:
      MCOp = lowerSymbolOperand(MO, Printer.GetJTISymbol(MO.getIndex()),
                                Subtarget);
      break;
    case MachineOperand::MO_ConstantPoolIndex:
      MCOp = lowerSymbolOperand(MO, Printer.GetCPISymbol(MO.getIndex()),
                                Subtarget);
      break;
    }

    OutMI.addOperand(MCOp);
  }
}

} // end of namespace vm::core
