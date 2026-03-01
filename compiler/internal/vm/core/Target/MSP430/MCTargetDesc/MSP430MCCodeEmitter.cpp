//===-- MSP430MCCodeEmitter.cpp - Convert MSP430 code to machine code -----===//
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
// This file implements the MSP430MCCodeEmitter class.
//
//===----------------------------------------------------------------------===//

#include "MSP430.h"
#include "MCTargetDesc/MSP430MCTargetDesc.h"
#include "MCTargetDesc/MSP430FixupKinds.h"

#include "vm/core/ADT/APFloat.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/MC/MCCodeEmitter.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCFixup.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/MC/MCRegisterInfo.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/Support/EndianStream.h"

#define DEBUG_TYPE "mccodeemitter"

namespace vm::core {

class MSP430MCCodeEmitter : public MCCodeEmitter {
  MCContext &Ctx;
  MCInstrInfo const &MCII;

  // Offset keeps track of current word number being emitted
  // inside a particular instruction.
  mutable unsigned Offset;

  /// TableGen'erated function for getting the binary encoding for an
  /// instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  /// Returns the binary encoding of operands.
  ///
  /// If an operand requires relocation, the relocation is recorded
  /// and zero is returned.
  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  unsigned getMemOpValue(const MCInst &MI, unsigned Op,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const;

  unsigned getPCRelImmOpValue(const MCInst &MI, unsigned Op,
                              SmallVectorImpl<MCFixup> &Fixups,
                              const MCSubtargetInfo &STI) const;

  unsigned getCGImmOpValue(const MCInst &MI, unsigned Op,
                           SmallVectorImpl<MCFixup> &Fixups,
                           const MCSubtargetInfo &STI) const;

  unsigned getCCOpValue(const MCInst &MI, unsigned Op,
                        SmallVectorImpl<MCFixup> &Fixups,
                        const MCSubtargetInfo &STI) const;

public:
  MSP430MCCodeEmitter(MCContext &ctx, MCInstrInfo const &MCII)
      : Ctx(ctx), MCII(MCII) {}

  void encodeInstruction(const MCInst &MI, SmallVectorImpl<char> &CB,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;
};

static void addFixup(SmallVectorImpl<MCFixup> &Fixups, uint32_t Offset,
                     const MCExpr *Value, uint16_t Kind) {
  bool PCRel = false;
  switch (Kind) {
  case MSP430::fixup_10_pcrel:
  case MSP430::fixup_16_pcrel:
  case MSP430::fixup_16_pcrel_byte:
  case MSP430::fixup_2x_pcrel:
  case MSP430::fixup_rl_pcrel:
    PCRel = true;
  }
  Fixups.push_back(MCFixup::create(Offset, Value, Kind, PCRel));
}

void MSP430MCCodeEmitter::encodeInstruction(const MCInst &MI,
                                            SmallVectorImpl<char> &CB,
                                            SmallVectorImpl<MCFixup> &Fixups,
                                            const MCSubtargetInfo &STI) const {
  const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
  // Get byte count of instruction.
  unsigned Size = Desc.getSize();

  // Initialize fixup offset
  Offset = 2;

  uint64_t BinaryOpCode = getBinaryCodeForInstr(MI, Fixups, STI);
  size_t WordCount = Size / 2;

  while (WordCount--) {
    support::endian::write(CB, (uint16_t)BinaryOpCode,
                           toolchain::endianness::little);
    BinaryOpCode >>= 16;
  }
}

unsigned MSP430MCCodeEmitter::getMachineOpValue(const MCInst &MI,
                                                const MCOperand &MO,
                                                SmallVectorImpl<MCFixup> &Fixups,
                                                const MCSubtargetInfo &STI) const {
  if (MO.isReg())
    return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());

  if (MO.isImm()) {
    Offset += 2;
    return MO.getImm();
  }

  assert(MO.isExpr() && "Expected expr operand");
  addFixup(Fixups, Offset, MO.getExpr(), MSP430::fixup_16_byte);
  Offset += 2;
  return 0;
}

unsigned MSP430MCCodeEmitter::getMemOpValue(const MCInst &MI, unsigned Op,
                                            SmallVectorImpl<MCFixup> &Fixups,
                                            const MCSubtargetInfo &STI) const {
  const MCOperand &MO1 = MI.getOperand(Op);
  assert(MO1.isReg() && "Register operand expected");
  unsigned Reg = Ctx.getRegisterInfo()->getEncodingValue(MO1.getReg());

  const MCOperand &MO2 = MI.getOperand(Op + 1);
  if (MO2.isImm()) {
    Offset += 2;
    return ((unsigned)MO2.getImm() << 4) | Reg;
  }

  assert(MO2.isExpr() && "Expr operand expected");
  MSP430::Fixups FixupKind;
  switch (Reg) {
  case 0:
    FixupKind = MSP430::fixup_16_pcrel_byte;
    break;
  case 2:
    FixupKind = MSP430::fixup_16_byte;
    break;
  default:
    FixupKind = MSP430::fixup_16_byte;
    break;
  }
  addFixup(Fixups, Offset, MO2.getExpr(), FixupKind);
  Offset += 2;
  return Reg;
}

unsigned MSP430MCCodeEmitter::getPCRelImmOpValue(const MCInst &MI, unsigned Op,
                                                 SmallVectorImpl<MCFixup> &Fixups,
                                                 const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(Op);
  if (MO.isImm())
    return MO.getImm();

  assert(MO.isExpr() && "Expr operand expected");
  addFixup(Fixups, 0, MO.getExpr(), MSP430::fixup_10_pcrel);
  return 0;
}

unsigned MSP430MCCodeEmitter::getCGImmOpValue(const MCInst &MI, unsigned Op,
                                              SmallVectorImpl<MCFixup> &Fixups,
                                              const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(Op);
  assert(MO.isImm() && "Expr operand expected");

  int64_t Imm = MO.getImm();
  switch (Imm) {
  default:
    llvm_unreachable("Invalid immediate value");
  case 4:  return 0x22;
  case 8:  return 0x32;
  case 0:  return 0x03;
  case 1:  return 0x13;
  case 2:  return 0x23;
  case -1: return 0x33;
  }
}

unsigned MSP430MCCodeEmitter::getCCOpValue(const MCInst &MI, unsigned Op,
                                           SmallVectorImpl<MCFixup> &Fixups,
                                           const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(Op);
  assert(MO.isImm() && "Immediate operand expected");
  switch (MO.getImm()) {
  case MSP430CC::COND_NE: return 0;
  case MSP430CC::COND_E:  return 1;
  case MSP430CC::COND_LO: return 2;
  case MSP430CC::COND_HS: return 3;
  case MSP430CC::COND_N:  return 4;
  case MSP430CC::COND_GE: return 5;
  case MSP430CC::COND_L:  return 6;
  default:
    llvm_unreachable("Unknown condition code");
  }
}

MCCodeEmitter *createMSP430MCCodeEmitter(const MCInstrInfo &MCII,
                                         MCContext &Ctx) {
  return new MSP430MCCodeEmitter(Ctx, MCII);
}

#include "MSP430GenMCCodeEmitter.inc"

} // end of namespace vm::core
