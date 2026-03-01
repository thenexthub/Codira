//===-- CSKYMCInstLower.cpp - Convert CSKY MachineInstr to an MCInst --------=//
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
// This file contains code to lower CSKY MachineInstrs to their corresponding
// MCInst records.
//
//===----------------------------------------------------------------------===//

#include "CSKYMCInstLower.h"
#include "MCTargetDesc/CSKYBaseInfo.h"
#include "MCTargetDesc/CSKYMCAsmInfo.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/MC/MCExpr.h"

#define DEBUG_TYPE "csky-mcinst-lower"

using namespace vm::core;

CSKYMCInstLower::CSKYMCInstLower(MCContext &Ctx, AsmPrinter &Printer)
    : Ctx(Ctx), Printer(Printer) {}

void CSKYMCInstLower::Lower(const MachineInstr *MI, MCInst &OutMI) const {
  OutMI.setOpcode(MI->getOpcode());

  for (const MachineOperand &MO : MI->operands()) {
    MCOperand MCOp;
    if (lowerOperand(MO, MCOp))
      OutMI.addOperand(MCOp);
  }
}

MCOperand CSKYMCInstLower::lowerSymbolOperand(const MachineOperand &MO,
                                              MCSymbol *Sym) const {
  CSKY::Specifier Spec;
  MCContext &Ctx = Printer.OutContext;

  switch (MO.getTargetFlags()) {
  default:
    llvm_unreachable("Unknown target flag.");
  case CSKYII::MO_None:
    Spec = CSKY::S_None;
    break;
  case CSKYII::MO_GOT32:
    Spec = CSKY::S_GOT;
    break;
  case CSKYII::MO_GOTOFF:
    Spec = CSKY::S_GOTOFF;
    break;
  case CSKYII::MO_ADDR32:
    Spec = CSKY::S_ADDR;
    break;
  case CSKYII::MO_PLT32:
    Spec = CSKY::S_PLT;
    break;
  case CSKYII::MO_ADDR_HI16:
    Spec = CSKY::S_ADDR_HI16;
    break;
  case CSKYII::MO_ADDR_LO16:
    Spec = CSKY::S_ADDR_LO16;
    break;
  }
  const MCExpr *ME = MCSymbolRefExpr::create(Sym, Ctx);

  if (Spec != CSKY::S_None)
    ME = MCSpecifierExpr::create(ME, Spec, Ctx);

  return MCOperand::createExpr(ME);
}

bool CSKYMCInstLower::lowerOperand(const MachineOperand &MO,
                                   MCOperand &MCOp) const {
  switch (MO.getType()) {
  default:
    llvm_unreachable("unknown operand type");
  case MachineOperand::MO_RegisterMask:
    break;
  case MachineOperand::MO_Immediate:
    MCOp = MCOperand::createImm(MO.getImm());
    break;
  case MachineOperand::MO_Register:
    if (MO.isImplicit())
      return false;
    MCOp = MCOperand::createReg(MO.getReg());
    break;
  case MachineOperand::MO_MachineBasicBlock:
    MCOp = MCOperand::createExpr(
        MCSymbolRefExpr::create(MO.getMBB()->getSymbol(), Ctx));
    break;
  case MachineOperand::MO_GlobalAddress:
    MCOp = lowerSymbolOperand(MO, Printer.getSymbol(MO.getGlobal()));
    break;
  case MachineOperand::MO_BlockAddress:
    MCOp = lowerSymbolOperand(
        MO, Printer.GetBlockAddressSymbol(MO.getBlockAddress()));
    break;
  case MachineOperand::MO_ExternalSymbol:
    MCOp = lowerSymbolOperand(
        MO, Printer.GetExternalSymbolSymbol(MO.getSymbolName()));
    break;
  case MachineOperand::MO_ConstantPoolIndex:
    MCOp = lowerSymbolOperand(MO, Printer.GetCPISymbol(MO.getIndex()));
    break;
  case MachineOperand::MO_JumpTableIndex:
    MCOp = lowerSymbolOperand(MO, Printer.GetJTISymbol(MO.getIndex()));
    break;
  case MachineOperand::MO_MCSymbol:
    MCOp = lowerSymbolOperand(MO, MO.getMCSymbol());
    break;
  }
  return true;
}
