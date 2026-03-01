//===-- M68kMCInstLower.cpp - M68k MachineInstr to MCInst -------*- C++ -*-===//
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
/// This file contains code to lower M68k MachineInstrs to their
/// corresponding MCInst records.
///
//===----------------------------------------------------------------------===//

#include "M68kMCInstLower.h"

#include "M68kAsmPrinter.h"
#include "M68kInstrInfo.h"

#include "MCTargetDesc/M68kBaseInfo.h"
#include "MCTargetDesc/M68kMCAsmInfo.h"

#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineInstr.h"
#include "vm/core/CodeGen/MachineOperand.h"
#include "vm/core/IR/Mangler.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCInst.h"

using namespace vm::core;

#define DEBUG_TYPE "m68k-mc-inst-lower"

M68kMCInstLower::M68kMCInstLower(MachineFunction &MF, M68kAsmPrinter &AP)
    : Ctx(AP.OutContext), MF(MF), TM(MF.getTarget()), MAI(*TM.getMCAsmInfo()),
      AsmPrinter(AP) {}

MCSymbol *
M68kMCInstLower::GetSymbolFromOperand(const MachineOperand &MO) const {
  assert((MO.isGlobal() || MO.isSymbol() || MO.isMBB()) &&
         "Isn't a symbol reference");

  const auto &TT = TM.getTargetTriple();
  if (MO.isGlobal() && TT.isOSBinFormatELF())
    return AsmPrinter.getSymbolPreferLocal(*MO.getGlobal());

  const DataLayout &DL = MF.getDataLayout();

  MCSymbol *Sym = nullptr;
  SmallString<128> Name;
  StringRef Suffix;

  if (!Suffix.empty())
    Name += DL.getPrivateGlobalPrefix();

  if (MO.isGlobal()) {
    const GlobalValue *GV = MO.getGlobal();
    AsmPrinter.getNameWithPrefix(Name, GV);
  } else if (MO.isSymbol()) {
    Mangler::getNameWithPrefix(Name, MO.getSymbolName(), DL);
  } else if (MO.isMBB()) {
    assert(Suffix.empty());
    Sym = MO.getMBB()->getSymbol();
  }

  Name += Suffix;
  if (!Sym)
    Sym = Ctx.getOrCreateSymbol(Name);

  return Sym;
}

MCOperand M68kMCInstLower::LowerSymbolOperand(const MachineOperand &MO,
                                              MCSymbol *Sym) const {
  // FIXME We would like an efficient form for this, so we don't have to do a
  // lot of extra uniquing. This fixme is originally from X86
  const MCExpr *Expr = nullptr;
  M68k::Specifier RefKind = M68k::S_None;

  switch (MO.getTargetFlags()) {
  default:
    llvm_unreachable("Unknown target flag on GV operand");
  case M68kII::MO_NO_FLAG:
  case M68kII::MO_ABSOLUTE_ADDRESS:
  case M68kII::MO_PC_RELATIVE_ADDRESS:
    break;
  case M68kII::MO_GOTPCREL:
    RefKind = M68k::S_GOTPCREL;
    break;
  case M68kII::MO_GOT:
    RefKind = M68k::S_GOT;
    break;
  case M68kII::MO_GOTOFF:
    RefKind = M68k::S_GOTOFF;
    break;
  case M68kII::MO_PLT:
    RefKind = M68k::S_PLT;
    break;
  case M68kII::MO_TLSGD:
    RefKind = M68k::S_TLSGD;
    break;
  case M68kII::MO_TLSLD:
    RefKind = M68k::S_TLSLD;
    break;
  case M68kII::MO_TLSLDM:
    RefKind = M68k::S_TLSLDM;
    break;
  case M68kII::MO_TLSIE:
    RefKind = M68k::S_GOTTPOFF;
    break;
  case M68kII::MO_TLSLE:
    RefKind = M68k::S_TPOFF;
    break;
  }

  if (!Expr) {
    Expr = MCSymbolRefExpr::create(Sym, RefKind, Ctx);
  }

  if (!MO.isJTI() && !MO.isMBB() && MO.getOffset()) {
    Expr = MCBinaryExpr::createAdd(
        Expr, MCConstantExpr::create(MO.getOffset(), Ctx), Ctx);
  }

  return MCOperand::createExpr(Expr);
}

std::optional<MCOperand>
M68kMCInstLower::LowerOperand(const MachineInstr *MI,
                              const MachineOperand &MO) const {
  switch (MO.getType()) {
  default:
    llvm_unreachable("unknown operand type");
  case MachineOperand::MO_Register:
    // Ignore all implicit register operands.
    if (MO.isImplicit())
      return std::nullopt;
    return MCOperand::createReg(MO.getReg());
  case MachineOperand::MO_Immediate:
    return MCOperand::createImm(MO.getImm());
  case MachineOperand::MO_MachineBasicBlock:
  case MachineOperand::MO_GlobalAddress:
  case MachineOperand::MO_ExternalSymbol:
    return LowerSymbolOperand(MO, GetSymbolFromOperand(MO));
  case MachineOperand::MO_MCSymbol:
    return LowerSymbolOperand(MO, MO.getMCSymbol());
  case MachineOperand::MO_JumpTableIndex:
    return LowerSymbolOperand(MO, AsmPrinter.GetJTISymbol(MO.getIndex()));
  case MachineOperand::MO_ConstantPoolIndex:
    return LowerSymbolOperand(MO, AsmPrinter.GetCPISymbol(MO.getIndex()));
  case MachineOperand::MO_BlockAddress:
    return LowerSymbolOperand(
        MO, AsmPrinter.GetBlockAddressSymbol(MO.getBlockAddress()));
  case MachineOperand::MO_RegisterMask:
    // Ignore call clobbers.
    return std::nullopt;
  }
}

void M68kMCInstLower::Lower(const MachineInstr *MI, MCInst &OutMI) const {
  unsigned Opcode = MI->getOpcode();
  OutMI.setOpcode(Opcode);

  for (unsigned i = 0, e = MI->getNumOperands(); i != e; ++i) {
    const MachineOperand &MO = MI->getOperand(i);
    std::optional<MCOperand> MCOp = LowerOperand(MI, MO);

    if (MCOp.has_value() && MCOp.value().isValid())
      OutMI.addOperand(MCOp.value());
  }

  // TAILJMPj, TAILJMPq - Lower to the correct jump instructions.
  if (Opcode == M68k::TAILJMPj || Opcode == M68k::TAILJMPq) {
    assert(OutMI.getNumOperands() == 1 && "Unexpected number of operands");
    switch (Opcode) {
    case M68k::TAILJMPj:
      Opcode = M68k::JMP32j;
      break;
    case M68k::TAILJMPq:
      Opcode = M68k::BRA8;
      break;
    }
    OutMI.setOpcode(Opcode);
  }
}
