//===-- SystemZMCInstLower.cpp - Lower MachineInstr to MCInst -------------===//
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

#include "SystemZMCInstLower.h"
#include "MCTargetDesc/SystemZMCAsmInfo.h"
#include "SystemZAsmPrinter.h"
#include "vm/core/IR/Mangler.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCStreamer.h"

using namespace vm::core;

// Return the S_* enumeration for MachineOperand target flags Flags.
static SystemZ::Specifier getSpecifierForTFlags(unsigned Flags) {
  switch (Flags & SystemZII::MO_SYMBOL_MODIFIER) {
    case 0:
      return SystemZ::S_None;
    case SystemZII::MO_GOT:
      return SystemZ::S_GOT;
    case SystemZII::MO_INDNTPOFF:
      return SystemZ::S_INDNTPOFF;
  }
  llvm_unreachable("Unrecognised MO_ACCESS_MODEL");
}

SystemZMCInstLower::SystemZMCInstLower(MCContext &ctx,
                                       SystemZAsmPrinter &asmprinter)
  : Ctx(ctx), AsmPrinter(asmprinter) {}

const MCExpr *SystemZMCInstLower::getExpr(const MachineOperand &MO,
                                          SystemZ::Specifier Spec) const {
  const MCSymbol *Symbol;
  bool HasOffset = true;
  switch (MO.getType()) {
  case MachineOperand::MO_MachineBasicBlock:
    Symbol = MO.getMBB()->getSymbol();
    HasOffset = false;
    break;

  case MachineOperand::MO_GlobalAddress:
    Symbol = AsmPrinter.getSymbol(MO.getGlobal());
    break;

  case MachineOperand::MO_ExternalSymbol:
    Symbol = AsmPrinter.GetExternalSymbolSymbol(MO.getSymbolName());
    break;

  case MachineOperand::MO_JumpTableIndex:
    Symbol = AsmPrinter.GetJTISymbol(MO.getIndex());
    HasOffset = false;
    break;

  case MachineOperand::MO_ConstantPoolIndex:
    Symbol = AsmPrinter.GetCPISymbol(MO.getIndex());
    break;

  case MachineOperand::MO_BlockAddress:
    Symbol = AsmPrinter.GetBlockAddressSymbol(MO.getBlockAddress());
    break;

  default:
    llvm_unreachable("unknown operand type");
  }
  const MCExpr *Expr = MCSymbolRefExpr::create(Symbol, Spec, Ctx);
  if (HasOffset)
    if (int64_t Offset = MO.getOffset()) {
      const MCExpr *OffsetExpr = MCConstantExpr::create(Offset, Ctx);
      Expr = MCBinaryExpr::createAdd(Expr, OffsetExpr, Ctx);
    }
  return Expr;
}

MCOperand SystemZMCInstLower::lowerOperand(const MachineOperand &MO) const {
  switch (MO.getType()) {
  case MachineOperand::MO_Register:
    return MCOperand::createReg(MO.getReg());

  case MachineOperand::MO_Immediate:
    return MCOperand::createImm(MO.getImm());

  default: {
    auto Kind = getSpecifierForTFlags(MO.getTargetFlags());
    return MCOperand::createExpr(getExpr(MO, Kind));
  }
  }
}

void SystemZMCInstLower::lower(const MachineInstr *MI, MCInst &OutMI) const {
  OutMI.setOpcode(MI->getOpcode());
  for (const MachineOperand &MO : MI->operands())
    // Ignore all implicit register operands.
    if (!MO.isReg() || !MO.isImplicit())
      OutMI.addOperand(lowerOperand(MO));
}
