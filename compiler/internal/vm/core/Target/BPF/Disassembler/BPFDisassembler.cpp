//===- BPFDisassembler.cpp - Disassembler for BPF ---------------*- C++ -*-===//
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
// This file is part of the BPF Disassembler.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/BPFMCTargetDesc.h"
#include "TargetInfo/BPFTargetInfo.h"
#include "vm/core/ADT/ArrayRef.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCDecoder.h"
#include "vm/core/MC/MCDecoderOps.h"
#include "vm/core/MC/MCDisassembler/MCDisassembler.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/MathExtras.h"
#include "vm/core/TargetParser/SubtargetFeature.h"
#include <cstdint>

using namespace vm::core;
using namespace vm::core::MCD;

#define DEBUG_TYPE "bpf-disassembler"

typedef MCDisassembler::DecodeStatus DecodeStatus;

namespace {

/// A disassembler class for BPF.
class BPFDisassembler : public MCDisassembler {
public:
  enum BPF_CLASS {
    BPF_LD = 0x0,
    BPF_LDX = 0x1,
    BPF_ST = 0x2,
    BPF_STX = 0x3,
    BPF_ALU = 0x4,
    BPF_JMP = 0x5,
    BPF_JMP32 = 0x6,
    BPF_ALU64 = 0x7
  };

  enum BPF_SIZE {
    BPF_W = 0x0,
    BPF_H = 0x1,
    BPF_B = 0x2,
    BPF_DW = 0x3
  };

  enum BPF_MODE {
    BPF_IMM = 0x0,
    BPF_ABS = 0x1,
    BPF_IND = 0x2,
    BPF_MEM = 0x3,
    BPF_MEMSX = 0x4,
    BPF_ATOMIC = 0x6
  };

  BPFDisassembler(const MCSubtargetInfo &STI, MCContext &Ctx)
      : MCDisassembler(STI, Ctx) {}
  ~BPFDisassembler() override = default;

  DecodeStatus getInstruction(MCInst &Instr, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &CStream) const override;

  uint8_t getInstClass(uint64_t Inst) const { return (Inst >> 56) & 0x7; };
  uint8_t getInstSize(uint64_t Inst) const { return (Inst >> 59) & 0x3; };
  uint8_t getInstMode(uint64_t Inst) const { return (Inst >> 61) & 0x7; };
};

} // end anonymous namespace

static MCDisassembler *createBPFDisassembler(const Target &T,
                                             const MCSubtargetInfo &STI,
                                             MCContext &Ctx) {
  return new BPFDisassembler(STI, Ctx);
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeBPFDisassembler() {
  // Register the disassembler.
  TargetRegistry::RegisterMCDisassembler(getTheBPFTarget(),
                                         createBPFDisassembler);
  TargetRegistry::RegisterMCDisassembler(getTheBPFleTarget(),
                                         createBPFDisassembler);
  TargetRegistry::RegisterMCDisassembler(getTheBPFbeTarget(),
                                         createBPFDisassembler);
}

static const unsigned GPRDecoderTable[] = {
    BPF::R0,  BPF::R1,  BPF::R2,  BPF::R3,  BPF::R4,  BPF::R5,
    BPF::R6,  BPF::R7,  BPF::R8,  BPF::R9,  BPF::R10, BPF::R11};

static DecodeStatus DecodeGPRRegisterClass(MCInst &Inst, unsigned RegNo,
                                           uint64_t /*Address*/,
                                           const MCDisassembler * /*Decoder*/) {
  if (RegNo > 11)
    return MCDisassembler::Fail;

  unsigned Reg = GPRDecoderTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static const unsigned GPR32DecoderTable[] = {
    BPF::W0,  BPF::W1,  BPF::W2,  BPF::W3,  BPF::W4,  BPF::W5,
    BPF::W6,  BPF::W7,  BPF::W8,  BPF::W9,  BPF::W10, BPF::W11};

static DecodeStatus
DecodeGPR32RegisterClass(MCInst &Inst, unsigned RegNo, uint64_t /*Address*/,
                         const MCDisassembler * /*Decoder*/) {
  if (RegNo > 11)
    return MCDisassembler::Fail;

  unsigned Reg = GPR32DecoderTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static DecodeStatus decodeMemoryOpValue(MCInst &Inst, unsigned Insn,
                                        uint64_t Address,
                                        const MCDisassembler *Decoder) {
  unsigned Register = (Insn >> 16) & 0xf;
  if (Register > 11)
    return MCDisassembler::Fail;

  Inst.addOperand(MCOperand::createReg(GPRDecoderTable[Register]));
  unsigned Offset = (Insn & 0xffff);
  Inst.addOperand(MCOperand::createImm(SignExtend32<16>(Offset)));

  return MCDisassembler::Success;
}

#include "BPFGenDisassemblerTables.inc"
static DecodeStatus readInstruction64(ArrayRef<uint8_t> Bytes, uint64_t Address,
                                      uint64_t &Size, uint64_t &Insn,
                                      bool IsLittleEndian) {
  uint64_t Lo, Hi;

  if (Bytes.size() < 8) {
    Size = 0;
    return MCDisassembler::Fail;
  }

  Size = 8;
  if (IsLittleEndian) {
    Hi = (Bytes[0] << 24) | (Bytes[1] << 16) | (Bytes[2] << 0) | (Bytes[3] << 8);
    Lo = (Bytes[4] << 0) | (Bytes[5] << 8) | (Bytes[6] << 16) | (Bytes[7] << 24);
  } else {
    Hi = (Bytes[0] << 24) | ((Bytes[1] & 0x0F) << 20) | ((Bytes[1] & 0xF0) << 12) |
         (Bytes[2] << 8) | (Bytes[3] << 0);
    Lo = (Bytes[4] << 24) | (Bytes[5] << 16) | (Bytes[6] << 8) | (Bytes[7] << 0);
  }
  Insn = Make_64(Hi, Lo);

  return MCDisassembler::Success;
}

DecodeStatus BPFDisassembler::getInstruction(MCInst &Instr, uint64_t &Size,
                                             ArrayRef<uint8_t> Bytes,
                                             uint64_t Address,
                                             raw_ostream &CStream) const {
  bool IsLittleEndian = getContext().getAsmInfo()->isLittleEndian();
  uint64_t Insn, Hi;
  DecodeStatus Result;

  Result = readInstruction64(Bytes, Address, Size, Insn, IsLittleEndian);
  if (Result == MCDisassembler::Fail) return MCDisassembler::Fail;

  uint8_t InstClass = getInstClass(Insn);
  uint8_t InstMode = getInstMode(Insn);
  if ((InstClass == BPF_LDX || InstClass == BPF_STX) &&
      getInstSize(Insn) != BPF_DW &&
      (InstMode == BPF_MEM || InstMode == BPF_ATOMIC) &&
      STI.hasFeature(BPF::ALU32))
    Result = decodeInstruction(DecoderTableBPFALU3264, Instr, Insn, Address,
                               this, STI);
  else
    Result = decodeInstruction(DecoderTableBPF64, Instr, Insn, Address, this,
                               STI);

  if (Result == MCDisassembler::Fail) return MCDisassembler::Fail;

  switch (Instr.getOpcode()) {
  case BPF::LD_imm64:
  case BPF::LD_pseudo: {
    if (Bytes.size() < 16) {
      Size = 0;
      return MCDisassembler::Fail;
    }
    Size = 16;
    if (IsLittleEndian)
      Hi = (Bytes[12] << 0) | (Bytes[13] << 8) | (Bytes[14] << 16) | (Bytes[15] << 24);
    else
      Hi = (Bytes[12] << 24) | (Bytes[13] << 16) | (Bytes[14] << 8) | (Bytes[15] << 0);
    auto& Op = Instr.getOperand(1);
    Op.setImm(Make_64(Hi, Op.getImm()));
    break;
  }
  }

  return Result;
}

typedef DecodeStatus (*DecodeFunc)(MCInst &MI, unsigned insn, uint64_t Address,
                                   const MCDisassembler *Decoder);
