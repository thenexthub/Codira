//=-- BPFMCInstLower.cpp - Convert BPF MachineInstr to an MCInst ------------=//
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
// This file contains code to lower BPF MachineInstrs to their corresponding
// MCInst records.
//
//===----------------------------------------------------------------------===//

#include "BPFMCInstLower.h"
#include "BPFAsmPrinter.h"
#include "BPFISelLowering.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCStreamer.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/raw_ostream.h"
using namespace vm::core;

MCSymbol *
BPFMCInstLower::GetGlobalAddressSymbol(const MachineOperand &MO) const {
  return Printer.getSymbol(MO.getGlobal());
}

MCSymbol *
BPFMCInstLower::GetExternalSymbolSymbol(const MachineOperand &MO) const {
  return Printer.GetExternalSymbolSymbol(MO.getSymbolName());
}

MCOperand BPFMCInstLower::LowerSymbolOperand(const MachineOperand &MO,
                                             MCSymbol *Sym) const {

  const MCExpr *Expr = MCSymbolRefExpr::create(Sym, Ctx);

  if (!MO.isJTI() && MO.getOffset())
    llvm_unreachable("unknown symbol op");

  return MCOperand::createExpr(Expr);
}

void BPFMCInstLower::Lower(const MachineInstr *MI, MCInst &OutMI) const {
  OutMI.setOpcode(MI->getOpcode());

  for (const MachineOperand &MO : MI->operands()) {
    MCOperand MCOp;
    switch (MO.getType()) {
    default:
      MI->print(errs());
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
    case MachineOperand::MO_MachineBasicBlock:
      MCOp = MCOperand::createExpr(
          MCSymbolRefExpr::create(MO.getMBB()->getSymbol(), Ctx));
      break;
    case MachineOperand::MO_RegisterMask:
      continue;
    case MachineOperand::MO_ExternalSymbol:
      MCOp = LowerSymbolOperand(MO, GetExternalSymbolSymbol(MO));
      break;
    case MachineOperand::MO_GlobalAddress:
      MCOp = LowerSymbolOperand(MO, GetGlobalAddressSymbol(MO));
      break;
    case MachineOperand::MO_ConstantPoolIndex:
      MCOp = LowerSymbolOperand(MO, Printer.GetCPISymbol(MO.getIndex()));
      break;
    case MachineOperand::MO_JumpTableIndex:
      MCOp = LowerSymbolOperand(MO, Printer.getJTPublicSymbol(MO.getIndex()));
      break;
    }

    OutMI.addOperand(MCOp);
  }
}
