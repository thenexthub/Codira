//===-- BPFMCCodeEmitter.cpp - Convert BPF code to machine code -----------===//
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
// This file implements the BPFMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/BPFMCFixups.h"
#include "MCTargetDesc/BPFMCTargetDesc.h"
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
#include <cassert>
#include <cstdint>

using namespace vm::core;

#define DEBUG_TYPE "mccodeemitter"

namespace {

class BPFMCCodeEmitter : public MCCodeEmitter {
  const MCRegisterInfo &MRI;
  bool IsLittleEndian;
  MCContext &Ctx;

public:
  BPFMCCodeEmitter(const MCInstrInfo &, const MCRegisterInfo &mri,
                   bool IsLittleEndian, MCContext &ctx)
      : MRI(mri), IsLittleEndian(IsLittleEndian), Ctx(ctx) {}
  BPFMCCodeEmitter(const BPFMCCodeEmitter &) = delete;
  void operator=(const BPFMCCodeEmitter &) = delete;
  ~BPFMCCodeEmitter() override = default;

  // getBinaryCodeForInstr - TableGen'erated function for getting the
  // binary encoding for an instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  // getMachineOpValue - Return binary encoding of operand. If the machin
  // operand requires relocation, record the relocation and return zero.
  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  uint64_t getMemoryOpValue(const MCInst &MI, unsigned Op,
                            SmallVectorImpl<MCFixup> &Fixups,
                            const MCSubtargetInfo &STI) const;

  void encodeInstruction(const MCInst &MI, SmallVectorImpl<char> &CB,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;
};

} // end anonymous namespace

MCCodeEmitter *toolchain::createBPFMCCodeEmitter(const MCInstrInfo &MCII,
                                            MCContext &Ctx) {
  return new BPFMCCodeEmitter(MCII, *Ctx.getRegisterInfo(), true, Ctx);
}

MCCodeEmitter *toolchain::createBPFbeMCCodeEmitter(const MCInstrInfo &MCII,
                                              MCContext &Ctx) {
  return new BPFMCCodeEmitter(MCII, *Ctx.getRegisterInfo(), false, Ctx);
}

static void addFixup(SmallVectorImpl<MCFixup> &Fixups, uint32_t Offset,
                     const MCExpr *Value, uint16_t Kind, bool PCRel = false) {
  Fixups.push_back(MCFixup::create(Offset, Value, Kind, PCRel));
}

unsigned BPFMCCodeEmitter::getMachineOpValue(const MCInst &MI,
                                             const MCOperand &MO,
                                             SmallVectorImpl<MCFixup> &Fixups,
                                             const MCSubtargetInfo &STI) const {
  if (MO.isReg())
    return MRI.getEncodingValue(MO.getReg());
  if (MO.isImm()) {
    uint64_t Imm = MO.getImm();
    uint64_t High32Bits = Imm >> 32, High33Bits = Imm >> 31;
    if (MI.getOpcode() != BPF::LD_imm64 && High32Bits != 0 &&
        High33Bits != 0x1FFFFFFFFULL) {
      Ctx.reportWarning(MI.getLoc(),
                        "immediate out of range, shall fit in 32 bits");
    }
    return static_cast<unsigned>(Imm);
  }

  assert(MO.isExpr());

  const MCExpr *Expr = MO.getExpr();

  assert(Expr->getKind() == MCExpr::SymbolRef);

  if (MI.getOpcode() == BPF::JAL)
    // func call name
    addFixup(Fixups, 0, Expr, FK_Data_4, true);
  else if (MI.getOpcode() == BPF::LD_imm64)
    addFixup(Fixups, 0, Expr, FK_SecRel_8);
  else if (MI.getOpcode() == BPF::JMPL)
    addFixup(Fixups, 0, Expr, BPF::FK_BPF_PCRel_4, true);
  else
    // bb label
    addFixup(Fixups, 0, Expr, FK_Data_2, true);

  return 0;
}

static uint8_t SwapBits(uint8_t Val)
{
  return (Val & 0x0F) << 4 | (Val & 0xF0) >> 4;
}

void BPFMCCodeEmitter::encodeInstruction(const MCInst &MI,
                                         SmallVectorImpl<char> &CB,
                                         SmallVectorImpl<MCFixup> &Fixups,
                                         const MCSubtargetInfo &STI) const {
  unsigned Opcode = MI.getOpcode();
  raw_svector_ostream OS(CB);
  support::endian::Writer OSE(OS, IsLittleEndian ? toolchain::endianness::little
                                                 : toolchain::endianness::big);

  if (Opcode == BPF::LD_imm64 || Opcode == BPF::LD_pseudo) {
    uint64_t Value = getBinaryCodeForInstr(MI, Fixups, STI);
    CB.push_back(Value >> 56);
    if (IsLittleEndian)
      CB.push_back((Value >> 48) & 0xff);
    else
      CB.push_back(SwapBits((Value >> 48) & 0xff));
    OSE.write<uint16_t>(0);
    OSE.write<uint32_t>(Value & 0xffffFFFF);

    const MCOperand &MO = MI.getOperand(1);
    uint64_t Imm = MO.isImm() ? MO.getImm() : 0;
    OSE.write<uint8_t>(0);
    OSE.write<uint8_t>(0);
    OSE.write<uint16_t>(0);
    OSE.write<uint32_t>(Imm >> 32);
  } else {
    // Get instruction encoding and emit it
    uint64_t Value = getBinaryCodeForInstr(MI, Fixups, STI);
    CB.push_back(Value >> 56);
    if (IsLittleEndian)
      CB.push_back(char((Value >> 48) & 0xff));
    else
      CB.push_back(SwapBits((Value >> 48) & 0xff));
    OSE.write<uint16_t>((Value >> 32) & 0xffff);
    OSE.write<uint32_t>(Value & 0xffffFFFF);
  }
}

// Encode BPF Memory Operand
uint64_t BPFMCCodeEmitter::getMemoryOpValue(const MCInst &MI, unsigned Op,
                                            SmallVectorImpl<MCFixup> &Fixups,
                                            const MCSubtargetInfo &STI) const {
  // For CMPXCHG instructions, output is implicitly in R0/W0,
  // so memory operand starts from operand 0.
  int MemOpStartIndex = 1, Opcode = MI.getOpcode();
  if (Opcode == BPF::CMPXCHGW32 || Opcode == BPF::CMPXCHGD)
    MemOpStartIndex = 0;

  uint64_t Encoding;
  const MCOperand Op1 = MI.getOperand(MemOpStartIndex);
  assert(Op1.isReg() && "First operand is not register.");
  Encoding = MRI.getEncodingValue(Op1.getReg());
  Encoding <<= 16;
  MCOperand Op2 = MI.getOperand(MemOpStartIndex + 1);
  assert(Op2.isImm() && "Second operand is not immediate.");
  Encoding |= Op2.getImm() & 0xffff;
  return Encoding;
}

#include "BPFGenMCCodeEmitter.inc"
