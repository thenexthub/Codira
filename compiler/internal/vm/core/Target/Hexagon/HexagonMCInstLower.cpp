//===- HexagonMCInstLower.cpp - Convert Hexagon MachineInstr to an MCInst -===//
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
// This file contains code to lower Hexagon MachineInstrs to their corresponding
// MCInst records.
//
//===----------------------------------------------------------------------===//

#include "HexagonAsmPrinter.h"
#include "MCTargetDesc/HexagonMCExpr.h"
#include "MCTargetDesc/HexagonMCInstrInfo.h"
#include "MCTargetDesc/HexagonMCTargetDesc.h"
#include "vm/core/ADT/APFloat.h"
#include "vm/core/ADT/APInt.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineOperand.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/raw_ostream.h"
#include <cassert>

using namespace vm::core;

namespace vm::core {

void HexagonLowerToMC(const MCInstrInfo &MCII, const MachineInstr *MI,
                      MCInst &MCB, HexagonAsmPrinter &AP);

} // end namespace vm::core

static MCOperand GetSymbolRef(const MachineOperand &MO, const MCSymbol *Symbol,
                              HexagonAsmPrinter &Printer, bool MustExtend) {
  MCContext &MC = Printer.OutContext;
  const MCExpr *ME;

  // Populate the relocation type based on Hexagon target flags
  // set on an operand
  HexagonMCExpr::VariantKind RelocationType;
  switch (MO.getTargetFlags() & ~HexagonII::HMOTF_ConstExtended) {
  default:
    RelocationType = HexagonMCExpr::VK_None;
    break;
  case HexagonII::MO_PCREL:
    RelocationType = HexagonMCExpr::VK_PCREL;
    break;
  case HexagonII::MO_GOT:
    RelocationType = HexagonMCExpr::VK_GOT;
    break;
  case HexagonII::MO_LO16:
    RelocationType = HexagonMCExpr::VK_LO16;
    break;
  case HexagonII::MO_HI16:
    RelocationType = HexagonMCExpr::VK_HI16;
    break;
  case HexagonII::MO_GPREL:
    RelocationType = HexagonMCExpr::VK_GPREL;
    break;
  case HexagonII::MO_GDGOT:
    RelocationType = HexagonMCExpr::VK_GD_GOT;
    break;
  case HexagonII::MO_GDPLT:
    RelocationType = HexagonMCExpr::VK_GD_PLT;
    break;
  case HexagonII::MO_IE:
    RelocationType = HexagonMCExpr::VK_IE;
    break;
  case HexagonII::MO_IEGOT:
    RelocationType = HexagonMCExpr::VK_IE_GOT;
    break;
  case HexagonII::MO_TPREL:
    RelocationType = HexagonMCExpr::VK_TPREL;
    break;
  }

  ME = MCSymbolRefExpr::create(Symbol, RelocationType, MC);

  if (!MO.isJTI() && MO.getOffset())
    ME = MCBinaryExpr::createAdd(ME, MCConstantExpr::create(MO.getOffset(), MC),
                                 MC);

  ME = HexagonMCExpr::create(ME, MC);
  HexagonMCInstrInfo::setMustExtend(*ME, MustExtend);
  return MCOperand::createExpr(ME);
}

// Create an MCInst from a MachineInstr
void toolchain::HexagonLowerToMC(const MCInstrInfo &MCII, const MachineInstr *MI,
                            MCInst &MCB, HexagonAsmPrinter &AP) {
  if (MI->getOpcode() == Hexagon::ENDLOOP0) {
    HexagonMCInstrInfo::setInnerLoop(MCB);
    return;
  }
  if (MI->getOpcode() == Hexagon::ENDLOOP1) {
    HexagonMCInstrInfo::setOuterLoop(MCB);
    return;
  }
  if (MI->getOpcode() == Hexagon::PATCHABLE_FUNCTION_ENTER) {
    AP.EmitSled(*MI, HexagonAsmPrinter::SledKind::FUNCTION_ENTER);
    return;
  }
  if (MI->getOpcode() == Hexagon::PATCHABLE_FUNCTION_EXIT) {
    AP.EmitSled(*MI, HexagonAsmPrinter::SledKind::FUNCTION_EXIT);
    return;
  }
  if (MI->getOpcode() == Hexagon::PATCHABLE_TAIL_CALL) {
    AP.EmitSled(*MI, HexagonAsmPrinter::SledKind::TAIL_CALL);
    return;
  }

  MCInst *MCI = AP.OutContext.createMCInst();
  MCI->setOpcode(MI->getOpcode());
  assert(MCI->getOpcode() == static_cast<unsigned>(MI->getOpcode()) &&
         "MCI opcode should have been set on construction");

  for (const MachineOperand &MO : MI->operands()) {
    MCOperand MCO;
    bool MustExtend = MO.getTargetFlags() & HexagonII::HMOTF_ConstExtended;

    switch (MO.getType()) {
    default:
      MI->print(errs());
      llvm_unreachable("unknown operand type");
    case MachineOperand::MO_RegisterMask:
      continue;
    case MachineOperand::MO_Register:
      // Ignore all implicit register operands.
      if (MO.isImplicit())
        continue;
      MCO = MCOperand::createReg(MO.getReg());
      break;
    case MachineOperand::MO_FPImmediate: {
      APFloat Val = MO.getFPImm()->getValueAPF();
      // FP immediates are used only when setting GPRs, so they may be dealt
      // with like regular immediates from this point on.
      auto Expr = HexagonMCExpr::create(
          MCConstantExpr::create(*Val.bitcastToAPInt().getRawData(),
                                 AP.OutContext),
          AP.OutContext);
      HexagonMCInstrInfo::setMustExtend(*Expr, MustExtend);
      MCO = MCOperand::createExpr(Expr);
      break;
    }
    case MachineOperand::MO_Immediate: {
      auto Expr = HexagonMCExpr::create(
          MCConstantExpr::create(MO.getImm(), AP.OutContext), AP.OutContext);
      HexagonMCInstrInfo::setMustExtend(*Expr, MustExtend);
      MCO = MCOperand::createExpr(Expr);
      break;
    }
    case MachineOperand::MO_MachineBasicBlock: {
      MCExpr const *Expr = MCSymbolRefExpr::create(MO.getMBB()->getSymbol(),
                                                   AP.OutContext);
      Expr = HexagonMCExpr::create(Expr, AP.OutContext);
      HexagonMCInstrInfo::setMustExtend(*Expr, MustExtend);
      MCO = MCOperand::createExpr(Expr);
      break;
    }
    case MachineOperand::MO_GlobalAddress:
      MCO = GetSymbolRef(MO, AP.getSymbol(MO.getGlobal()), AP, MustExtend);
      break;
    case MachineOperand::MO_ExternalSymbol:
      MCO = GetSymbolRef(MO, AP.GetExternalSymbolSymbol(MO.getSymbolName()),
                         AP, MustExtend);
      break;
    case MachineOperand::MO_JumpTableIndex:
      MCO = GetSymbolRef(MO, AP.GetJTISymbol(MO.getIndex()), AP, MustExtend);
      break;
    case MachineOperand::MO_ConstantPoolIndex:
      MCO = GetSymbolRef(MO, AP.GetCPISymbol(MO.getIndex()), AP, MustExtend);
      break;
    case MachineOperand::MO_BlockAddress:
      MCO = GetSymbolRef(MO, AP.GetBlockAddressSymbol(MO.getBlockAddress()), AP,
                         MustExtend);
      break;
    }

    MCI->addOperand(MCO);
  }
  AP.HexagonProcessInstruction(*MCI, *MI);
  HexagonMCInstrInfo::extendIfNeeded(AP.OutContext, MCII, MCB, *MCI);
  MCB.addOperand(MCOperand::createInst(MCI));
}
