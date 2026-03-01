//===-- MSP430MCInstLower.cpp - Convert MSP430 MachineInstr to an MCInst --===//
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
// This file contains code to lower MSP430 MachineInstrs to their corresponding
// MCInst records.
//
//===----------------------------------------------------------------------===//

#include "MSP430MCInstLower.h"
#include "vm/core/ADT/SmallString.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/IR/DataLayout.h"
#include "vm/core/IR/Mangler.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/Target/TargetMachine.h"
using namespace vm::core;

MCSymbol *MSP430MCInstLower::
GetGlobalAddressSymbol(const MachineOperand &MO) const {
  switch (MO.getTargetFlags()) {
  default: llvm_unreachable("Unknown target flag on GV operand");
  case 0: break;
  }

  return Printer.getSymbol(MO.getGlobal());
}

MCSymbol *MSP430MCInstLower::
GetExternalSymbolSymbol(const MachineOperand &MO) const {
  switch (MO.getTargetFlags()) {
  default: llvm_unreachable("Unknown target flag on GV operand");
  case 0: break;
  }

  return Printer.GetExternalSymbolSymbol(MO.getSymbolName());
}

MCSymbol *MSP430MCInstLower::
GetJumpTableSymbol(const MachineOperand &MO) const {
  const DataLayout &DL = Printer.getDataLayout();
  SmallString<256> Name;
  raw_svector_ostream(Name) << DL.getPrivateGlobalPrefix() << "JTI"
                            << Printer.getFunctionNumber() << '_'
                            << MO.getIndex();

  switch (MO.getTargetFlags()) {
  default: llvm_unreachable("Unknown target flag on GV operand");
  case 0: break;
  }

  // Create a symbol for the name.
  return Ctx.getOrCreateSymbol(Name);
}

MCSymbol *MSP430MCInstLower::
GetConstantPoolIndexSymbol(const MachineOperand &MO) const {
  const DataLayout &DL = Printer.getDataLayout();
  SmallString<256> Name;
  raw_svector_ostream(Name) << DL.getPrivateGlobalPrefix() << "CPI"
                            << Printer.getFunctionNumber() << '_'
                            << MO.getIndex();

  switch (MO.getTargetFlags()) {
  default: llvm_unreachable("Unknown target flag on GV operand");
  case 0: break;
  }

  // Create a symbol for the name.
  return Ctx.getOrCreateSymbol(Name);
}

MCSymbol *MSP430MCInstLower::
GetBlockAddressSymbol(const MachineOperand &MO) const {
  switch (MO.getTargetFlags()) {
  default: llvm_unreachable("Unknown target flag on GV operand");
  case 0: break;
  }

  return Printer.GetBlockAddressSymbol(MO.getBlockAddress());
}

MCOperand MSP430MCInstLower::
LowerSymbolOperand(const MachineOperand &MO, MCSymbol *Sym) const {
  // FIXME: We would like an efficient form for this, so we don't have to do a
  // lot of extra uniquing.
  const MCExpr *Expr = MCSymbolRefExpr::create(Sym, Ctx);

  switch (MO.getTargetFlags()) {
  default: llvm_unreachable("Unknown target flag on GV operand");
  case 0: break;
  }

  if (!MO.isJTI() && MO.getOffset())
    Expr = MCBinaryExpr::createAdd(Expr,
                                   MCConstantExpr::create(MO.getOffset(), Ctx),
                                   Ctx);
  return MCOperand::createExpr(Expr);
}

#define GET_REGINFO_ENUM
#include "MSP430GenRegisterInfo.inc"

void MSP430MCInstLower::Lower(const MachineInstr *MI, MCInst &OutMI) const {
  OutMI.setOpcode(MI->getOpcode());

  for (const MachineOperand &MO : MI->operands()) {
    MCOperand MCOp;
    switch (MO.getType()) {
    default:
      MI->print(errs());
      llvm_unreachable("unknown operand type");
    case MachineOperand::MO_Register:
      // Ignore all implicit register operands.
      if (MO.isImplicit()) continue;
      MCOp = MCOperand::createReg(MO.getReg());
      break;
    case MachineOperand::MO_Immediate:
      MCOp = MCOperand::createImm(MO.getImm());
      break;
    case MachineOperand::MO_MachineBasicBlock:
      MCOp = MCOperand::createExpr(MCSymbolRefExpr::create(
                         MO.getMBB()->getSymbol(), Ctx));
      break;
    case MachineOperand::MO_GlobalAddress:
      MCOp = LowerSymbolOperand(MO, GetGlobalAddressSymbol(MO));
      break;
    case MachineOperand::MO_ExternalSymbol:
      MCOp = LowerSymbolOperand(MO, GetExternalSymbolSymbol(MO));
      break;
    case MachineOperand::MO_JumpTableIndex:
      MCOp = LowerSymbolOperand(MO, GetJumpTableSymbol(MO));
      break;
    case MachineOperand::MO_ConstantPoolIndex:
      MCOp = LowerSymbolOperand(MO, GetConstantPoolIndexSymbol(MO));
      break;
    case MachineOperand::MO_BlockAddress:
      MCOp = LowerSymbolOperand(MO, GetBlockAddressSymbol(MO));
      break;
    case MachineOperand::MO_RegisterMask:
      continue;
    }

    OutMI.addOperand(MCOp);
  }
}
