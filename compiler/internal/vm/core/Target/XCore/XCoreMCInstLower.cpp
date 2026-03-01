//===-- XCoreMCInstLower.cpp - Convert XCore MachineInstr to MCInst -------===//
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
/// This file contains code to lower XCore MachineInstrs to their
/// corresponding MCInst records.
///
//===----------------------------------------------------------------------===//
#include "XCoreMCInstLower.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineOperand.h"
#include "vm/core/IR/Mangler.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCInst.h"

using namespace vm::core;

XCoreMCInstLower::XCoreMCInstLower(class AsmPrinter &asmprinter)
    : Printer(asmprinter) {}

void XCoreMCInstLower::Initialize(MCContext *C) { Ctx = C; }

MCOperand XCoreMCInstLower::LowerSymbolOperand(const MachineOperand &MO,
                                               MachineOperandType MOTy,
                                               unsigned Offset) const {
  const MCSymbol *Symbol;

  switch (MOTy) {
    case MachineOperand::MO_MachineBasicBlock:
      Symbol = MO.getMBB()->getSymbol();
      break;
    case MachineOperand::MO_GlobalAddress:
      Symbol = Printer.getSymbol(MO.getGlobal());
      Offset += MO.getOffset();
      break;
    case MachineOperand::MO_BlockAddress:
      Symbol = Printer.GetBlockAddressSymbol(MO.getBlockAddress());
      Offset += MO.getOffset();
      break;
    case MachineOperand::MO_ExternalSymbol:
      Symbol = Printer.GetExternalSymbolSymbol(MO.getSymbolName());
      Offset += MO.getOffset();
      break;
    case MachineOperand::MO_JumpTableIndex:
      Symbol = Printer.GetJTISymbol(MO.getIndex());
      break;
    case MachineOperand::MO_ConstantPoolIndex:
      Symbol = Printer.GetCPISymbol(MO.getIndex());
      Offset += MO.getOffset();
      break;
    default:
      llvm_unreachable("<unknown operand type>");
  }

  const MCSymbolRefExpr *MCSym = MCSymbolRefExpr::create(Symbol, *Ctx);
  if (!Offset)
    return MCOperand::createExpr(MCSym);

  // Assume offset is never negative.
  assert(Offset > 0);

  const MCConstantExpr *OffsetExpr =  MCConstantExpr::create(Offset, *Ctx);
  const MCBinaryExpr *Add = MCBinaryExpr::createAdd(MCSym, OffsetExpr, *Ctx);
  return MCOperand::createExpr(Add);
}

MCOperand XCoreMCInstLower::LowerOperand(const MachineOperand &MO,
                                         unsigned offset) const {
  MachineOperandType MOTy = MO.getType();

  switch (MOTy) {
    default: llvm_unreachable("unknown operand type");
    case MachineOperand::MO_Register:
      // Ignore all implicit register operands.
      if (MO.isImplicit()) break;
      return MCOperand::createReg(MO.getReg());
    case MachineOperand::MO_Immediate:
      return MCOperand::createImm(MO.getImm() + offset);
    case MachineOperand::MO_MachineBasicBlock:
    case MachineOperand::MO_GlobalAddress:
    case MachineOperand::MO_ExternalSymbol:
    case MachineOperand::MO_JumpTableIndex:
    case MachineOperand::MO_ConstantPoolIndex:
    case MachineOperand::MO_BlockAddress:
      return LowerSymbolOperand(MO, MOTy, offset);
    case MachineOperand::MO_RegisterMask:
      break;
  }

  return MCOperand();
}

void XCoreMCInstLower::Lower(const MachineInstr *MI, MCInst &OutMI) const {
  OutMI.setOpcode(MI->getOpcode());

  for (const MachineOperand &MO : MI->operands()) {
    MCOperand MCOp = LowerOperand(MO);

    if (MCOp.isValid())
      OutMI.addOperand(MCOp);
  }
}
