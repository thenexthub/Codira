//=- LoongArchMCInstLower.cpp - Convert LoongArch MachineInstr to an MCInst -=//
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
// This file contains code to lower LoongArch MachineInstrs to their
// corresponding MCInst records.
//
//===----------------------------------------------------------------------===//

#include "LoongArch.h"
#include "MCTargetDesc/LoongArchBaseInfo.h"
#include "MCTargetDesc/LoongArchMCAsmInfo.h"
#include "vm/core/BinaryFormat/ELF.h"
#include "vm/core/CodeGen/AsmPrinter.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCContext.h"

using namespace vm::core;

static MCOperand lowerSymbolOperand(const MachineOperand &MO, MCSymbol *Sym,
                                    const AsmPrinter &AP) {
  MCContext &Ctx = AP.OutContext;
  uint16_t Kind = 0;

  switch (LoongArchII::getDirectFlags(MO)) {
  default:
    llvm_unreachable("Unknown target flag on GV operand");
  case LoongArchII::MO_None:
    Kind = LoongArchMCExpr::VK_None;
    break;
  case LoongArchII::MO_CALL:
  case LoongArchII::MO_CALL_PLT:
    Kind = ELF::R_LARCH_B26;
    break;
  case LoongArchII::MO_PCREL_HI:
    Kind = ELF::R_LARCH_PCALA_HI20;
    break;
  case LoongArchII::MO_PCREL_LO:
    Kind = ELF::R_LARCH_PCALA_LO12;
    break;
  case LoongArchII::MO_PCREL64_LO:
    Kind = ELF::R_LARCH_PCALA64_LO20;
    break;
  case LoongArchII::MO_PCREL64_HI:
    Kind = ELF::R_LARCH_PCALA64_HI12;
    break;
  case LoongArchII::MO_GOT_PC_HI:
    Kind = ELF::R_LARCH_GOT_PC_HI20;
    break;
  case LoongArchII::MO_GOT_PC_LO:
    Kind = ELF::R_LARCH_GOT_PC_LO12;
    break;
  case LoongArchII::MO_GOT_PC64_LO:
    Kind = ELF::R_LARCH_GOT64_PC_LO20;
    break;
  case LoongArchII::MO_GOT_PC64_HI:
    Kind = ELF::R_LARCH_GOT64_PC_HI12;
    break;
  case LoongArchII::MO_LE_HI:
    Kind = ELF::R_LARCH_TLS_LE_HI20;
    break;
  case LoongArchII::MO_LE_LO:
    Kind = ELF::R_LARCH_TLS_LE_LO12;
    break;
  case LoongArchII::MO_LE64_LO:
    Kind = ELF::R_LARCH_TLS_LE64_LO20;
    break;
  case LoongArchII::MO_LE64_HI:
    Kind = ELF::R_LARCH_TLS_LE64_HI12;
    break;
  case LoongArchII::MO_IE_PC_HI:
    Kind = ELF::R_LARCH_TLS_IE_PC_HI20;
    break;
  case LoongArchII::MO_IE_PC_LO:
    Kind = ELF::R_LARCH_TLS_IE_PC_LO12;
    break;
  case LoongArchII::MO_IE_PC64_LO:
    Kind = ELF::R_LARCH_TLS_IE64_PC_LO20;
    break;
  case LoongArchII::MO_IE_PC64_HI:
    Kind = ELF::R_LARCH_TLS_IE64_PC_HI12;
    break;
  case LoongArchII::MO_LD_PC_HI:
    Kind = ELF::R_LARCH_TLS_LD_PC_HI20;
    break;
  case LoongArchII::MO_GD_PC_HI:
    Kind = ELF::R_LARCH_TLS_GD_PC_HI20;
    break;
  case LoongArchII::MO_CALL30:
    Kind = ELF::R_LARCH_CALL30;
    break;
  case LoongArchII::MO_CALL36:
    Kind = ELF::R_LARCH_CALL36;
    break;
  case LoongArchII::MO_DESC_PC_HI:
    Kind = ELF::R_LARCH_TLS_DESC_PC_HI20;
    break;
  case LoongArchII::MO_DESC_PC_LO:
    Kind = ELF::R_LARCH_TLS_DESC_PC_LO12;
    break;
  case LoongArchII::MO_DESC64_PC_LO:
    Kind = ELF::R_LARCH_TLS_DESC64_PC_LO20;
    break;
  case LoongArchII::MO_DESC64_PC_HI:
    Kind = ELF::R_LARCH_TLS_DESC64_PC_HI12;
    break;
  case LoongArchII::MO_DESC_LD:
    Kind = ELF::R_LARCH_TLS_DESC_LD;
    break;
  case LoongArchII::MO_DESC_CALL:
    Kind = ELF::R_LARCH_TLS_DESC_CALL;
    break;
  case LoongArchII::MO_LE_HI_R:
    Kind = ELF::R_LARCH_TLS_LE_HI20_R;
    break;
  case LoongArchII::MO_LE_ADD_R:
    Kind = ELF::R_LARCH_TLS_LE_ADD_R;
    break;
  case LoongArchII::MO_LE_LO_R:
    Kind = ELF::R_LARCH_TLS_LE_LO12_R;
    break;
  case LoongArchII::MO_PCADD_HI:
    Kind = ELF::R_LARCH_PCADD_HI20;
    break;
  case LoongArchII::MO_PCADD_LO:
    Kind = ELF::R_LARCH_PCADD_LO12;
    break;
  case LoongArchII::MO_GOT_PCADD_HI:
    Kind = ELF::R_LARCH_GOT_PCADD_HI20;
    break;
  case LoongArchII::MO_GOT_PCADD_LO:
    Kind = ELF::R_LARCH_GOT_PCADD_LO12;
    break;
  case LoongArchII::MO_IE_PCADD_HI:
    Kind = ELF::R_LARCH_TLS_IE_PCADD_HI20;
    break;
  case LoongArchII::MO_IE_PCADD_LO:
    Kind = ELF::R_LARCH_TLS_IE_PCADD_LO12;
    break;
  case LoongArchII::MO_LD_PCADD_HI:
    Kind = ELF::R_LARCH_TLS_LD_PCADD_HI20;
    break;
  case LoongArchII::MO_LD_PCADD_LO:
    Kind = ELF::R_LARCH_TLS_LD_PCADD_LO12;
    break;
  case LoongArchII::MO_GD_PCADD_HI:
    Kind = ELF::R_LARCH_TLS_GD_PCADD_HI20;
    break;
  case LoongArchII::MO_GD_PCADD_LO:
    Kind = ELF::R_LARCH_TLS_GD_PCADD_LO12;
    break;
  case LoongArchII::MO_DESC_PCADD_HI:
    Kind = ELF::R_LARCH_TLS_DESC_PCADD_HI20;
    break;
  case LoongArchII::MO_DESC_PCADD_LO:
    Kind = ELF::R_LARCH_TLS_DESC_PCADD_LO12;
    break;
    // TODO: Handle more target-flags.
  }

  const MCExpr *ME = MCSymbolRefExpr::create(Sym, Ctx);

  if (!MO.isJTI() && !MO.isMBB() && MO.getOffset())
    ME = MCBinaryExpr::createAdd(
        ME, MCConstantExpr::create(MO.getOffset(), Ctx), Ctx);

  if (Kind != LoongArchMCExpr::VK_None)
    ME = LoongArchMCExpr::create(ME, Kind, Ctx, LoongArchII::hasRelaxFlag(MO));
  return MCOperand::createExpr(ME);
}

bool toolchain::lowerLoongArchMachineOperandToMCOperand(const MachineOperand &MO,
                                                   MCOperand &MCOp,
                                                   const AsmPrinter &AP) {
  switch (MO.getType()) {
  default:
    report_fatal_error(
        "lowerLoongArchMachineOperandToMCOperand: unknown operand type");
  case MachineOperand::MO_Register:
    // Ignore all implicit register operands.
    if (MO.isImplicit())
      return false;
    MCOp = MCOperand::createReg(MO.getReg());
    break;
  case MachineOperand::MO_RegisterMask:
    // Regmasks are like implicit defs.
    return false;
  case MachineOperand::MO_Immediate:
    MCOp = MCOperand::createImm(MO.getImm());
    break;
  case MachineOperand::MO_ConstantPoolIndex:
    MCOp = lowerSymbolOperand(MO, AP.GetCPISymbol(MO.getIndex()), AP);
    break;
  case MachineOperand::MO_GlobalAddress:
    MCOp = lowerSymbolOperand(MO, AP.getSymbolPreferLocal(*MO.getGlobal()), AP);
    break;
  case MachineOperand::MO_MachineBasicBlock:
    MCOp = lowerSymbolOperand(MO, MO.getMBB()->getSymbol(), AP);
    break;
  case MachineOperand::MO_ExternalSymbol:
    MCOp = lowerSymbolOperand(
        MO, AP.GetExternalSymbolSymbol(MO.getSymbolName()), AP);
    break;
  case MachineOperand::MO_BlockAddress:
    MCOp = lowerSymbolOperand(
        MO, AP.GetBlockAddressSymbol(MO.getBlockAddress()), AP);
    break;
  case MachineOperand::MO_JumpTableIndex:
    MCOp = lowerSymbolOperand(MO, AP.GetJTISymbol(MO.getIndex()), AP);
    break;
  case MachineOperand::MO_MCSymbol:
    MCOp = lowerSymbolOperand(MO, MO.getMCSymbol(), AP);
    break;
  }
  return true;
}

bool toolchain::lowerLoongArchMachineInstrToMCInst(const MachineInstr *MI,
                                              MCInst &OutMI, AsmPrinter &AP) {
  OutMI.setOpcode(MI->getOpcode());

  for (const MachineOperand &MO : MI->operands()) {
    MCOperand MCOp;
    if (lowerLoongArchMachineOperandToMCOperand(MO, MCOp, AP))
      OutMI.addOperand(MCOp);
  }
  return false;
}
