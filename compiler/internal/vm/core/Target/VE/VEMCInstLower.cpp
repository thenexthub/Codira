//===-- VEMCInstLower.cpp - Convert VE MachineInstr to MCInst -------------===//
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
// This file contains code to lower VE MachineInstrs to their corresponding
// MCInst records.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/VEMCAsmInfo.h"
#include "VE.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineOperand.h"
#include "vm/core/IR/Mangler.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCInst.h"

using namespace vm::core;

static MCOperand LowerSymbolOperand(const MachineInstr *MI,
                                    const MachineOperand &MO,
                                    const MCSymbol *Symbol, AsmPrinter &AP) {
  auto Kind = (VE::Specifier)MO.getTargetFlags();

  const MCExpr *Expr = MCSymbolRefExpr::create(Symbol, AP.OutContext);
  // Add offset iff MO is not jump table info or machine basic block.
  if (!MO.isJTI() && !MO.isMBB() && MO.getOffset())
    Expr = MCBinaryExpr::createAdd(
        Expr, MCConstantExpr::create(MO.getOffset(), AP.OutContext),
        AP.OutContext);
  Expr = MCSpecifierExpr::create(Expr, Kind, AP.OutContext);
  return MCOperand::createExpr(Expr);
}

static MCOperand LowerOperand(const MachineInstr *MI, const MachineOperand &MO,
                              AsmPrinter &AP) {
  switch (MO.getType()) {
  default:
    report_fatal_error("unsupported operand type");

  case MachineOperand::MO_Register:
    if (MO.isImplicit())
      break;
    return MCOperand::createReg(MO.getReg());

  case MachineOperand::MO_BlockAddress:
    return LowerSymbolOperand(
        MI, MO, AP.GetBlockAddressSymbol(MO.getBlockAddress()), AP);
  case MachineOperand::MO_ConstantPoolIndex:
    return LowerSymbolOperand(MI, MO, AP.GetCPISymbol(MO.getIndex()), AP);
  case MachineOperand::MO_ExternalSymbol:
    return LowerSymbolOperand(
        MI, MO, AP.GetExternalSymbolSymbol(MO.getSymbolName()), AP);
  case MachineOperand::MO_GlobalAddress:
    return LowerSymbolOperand(MI, MO, AP.getSymbol(MO.getGlobal()), AP);
  case MachineOperand::MO_Immediate:
    return MCOperand::createImm(MO.getImm());
  case MachineOperand::MO_JumpTableIndex:
    return LowerSymbolOperand(MI, MO, AP.GetJTISymbol(MO.getIndex()), AP);
  case MachineOperand::MO_MachineBasicBlock:
    return LowerSymbolOperand(MI, MO, MO.getMBB()->getSymbol(), AP);

  case MachineOperand::MO_RegisterMask:
    break;
  }
  return MCOperand();
}

void toolchain::LowerVEMachineInstrToMCInst(const MachineInstr *MI, MCInst &OutMI,
                                       AsmPrinter &AP) {
  OutMI.setOpcode(MI->getOpcode());

  for (const MachineOperand &MO : MI->operands()) {
    MCOperand MCOp = LowerOperand(MI, MO, AP);

    if (MCOp.isValid())
      OutMI.addOperand(MCOp);
  }
}
