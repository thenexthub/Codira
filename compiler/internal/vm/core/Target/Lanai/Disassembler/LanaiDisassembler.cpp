//===- LanaiDisassembler.cpp - Disassembler for Lanai -----------*- C++ -*-===//
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
// This file is part of the Lanai Disassembler.
//
//===----------------------------------------------------------------------===//

#include "LanaiDisassembler.h"

#include "LanaiAluCode.h"
#include "LanaiCondCode.h"
#include "LanaiInstrInfo.h"
#include "TargetInfo/LanaiTargetInfo.h"
#include "vm/core/MC/MCDecoder.h"
#include "vm/core/MC/MCDecoderOps.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/MathExtras.h"

#define DEBUG_TYPE "lanai-disassembler"

using namespace vm::core;
using namespace vm::core::MCD;

typedef MCDisassembler::DecodeStatus DecodeStatus;

static MCDisassembler *createLanaiDisassembler(const Target & /*T*/,
                                               const MCSubtargetInfo &STI,
                                               MCContext &Ctx) {
  return new LanaiDisassembler(STI, Ctx);
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeLanaiDisassembler() {
  // Register the disassembler
  TargetRegistry::RegisterMCDisassembler(getTheLanaiTarget(),
                                         createLanaiDisassembler);
}

LanaiDisassembler::LanaiDisassembler(const MCSubtargetInfo &STI, MCContext &Ctx)
    : MCDisassembler(STI, Ctx) {}

// clang-format off
static const unsigned GPRDecoderTable[] = {
  Lanai::R0,  Lanai::R1,  Lanai::PC,  Lanai::R3,  Lanai::SP,  Lanai::FP,
  Lanai::R6,  Lanai::R7,  Lanai::RV,  Lanai::R9,  Lanai::RR1, Lanai::RR2,
  Lanai::R12, Lanai::R13, Lanai::R14, Lanai::RCA, Lanai::R16, Lanai::R17,
  Lanai::R18, Lanai::R19, Lanai::R20, Lanai::R21, Lanai::R22, Lanai::R23,
  Lanai::R24, Lanai::R25, Lanai::R26, Lanai::R27, Lanai::R28, Lanai::R29,
  Lanai::R30, Lanai::R31
};
// clang-format on

DecodeStatus DecodeGPRRegisterClass(MCInst &Inst, unsigned RegNo,
                                    uint64_t /*Address*/,
                                    const MCDisassembler * /*Decoder*/) {
  if (RegNo > 31)
    return MCDisassembler::Fail;

  unsigned Reg = GPRDecoderTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static DecodeStatus decodeRiMemoryValue(MCInst &Inst, unsigned Insn,
                                        uint64_t Address,
                                        const MCDisassembler *Decoder) {
  // RI memory values encoded using 23 bits:
  //   5 bit register, 16 bit constant
  unsigned Register = (Insn >> 18) & 0x1f;
  Inst.addOperand(MCOperand::createReg(GPRDecoderTable[Register]));
  unsigned Offset = (Insn & 0xffff);
  Inst.addOperand(MCOperand::createImm(SignExtend32<16>(Offset)));

  return MCDisassembler::Success;
}

static DecodeStatus decodeRrMemoryValue(MCInst &Inst, unsigned Insn,
                                        uint64_t Address,
                                        const MCDisassembler *Decoder) {
  // RR memory values encoded using 20 bits:
  //   5 bit register, 5 bit register, 2 bit PQ, 3 bit ALU operator, 5 bit JJJJJ
  unsigned Register = (Insn >> 15) & 0x1f;
  Inst.addOperand(MCOperand::createReg(GPRDecoderTable[Register]));
  Register = (Insn >> 10) & 0x1f;
  Inst.addOperand(MCOperand::createReg(GPRDecoderTable[Register]));

  return MCDisassembler::Success;
}

static DecodeStatus decodeSplsValue(MCInst &Inst, unsigned Insn,
                                    uint64_t Address,
                                    const MCDisassembler *Decoder) {
  // RI memory values encoded using 17 bits:
  //   5 bit register, 10 bit constant
  unsigned Register = (Insn >> 12) & 0x1f;
  Inst.addOperand(MCOperand::createReg(GPRDecoderTable[Register]));
  unsigned Offset = (Insn & 0x3ff);
  Inst.addOperand(MCOperand::createImm(SignExtend32<10>(Offset)));

  return MCDisassembler::Success;
}

static bool tryAddingSymbolicOperand(int64_t Value, bool IsBranch,
                                     uint64_t Address, uint64_t Offset,
                                     uint64_t Width, MCInst &MI,
                                     const MCDisassembler *Decoder) {
  return Decoder->tryAddingSymbolicOperand(MI, Value, Address, IsBranch, Offset,
                                           Width, /*InstSize=*/0);
}

static DecodeStatus decodeBranch(MCInst &MI, unsigned Insn, uint64_t Address,
                                 const MCDisassembler *Decoder) {
  if (!tryAddingSymbolicOperand(Insn + Address, false, Address, 2, 23, MI,
                                Decoder))
    MI.addOperand(MCOperand::createImm(Insn));
  return MCDisassembler::Success;
}

static DecodeStatus decodeShiftImm(MCInst &Inst, unsigned Insn,
                                   uint64_t Address,
                                   const MCDisassembler *Decoder) {
  unsigned Offset = (Insn & 0xffff);
  Inst.addOperand(MCOperand::createImm(SignExtend32<16>(Offset)));

  return MCDisassembler::Success;
}

static DecodeStatus decodePredicateOperand(MCInst &Inst, unsigned Val,
                                           uint64_t Address,
                                           const MCDisassembler *Decoder) {
  if (Val >= LPCC::UNKNOWN)
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createImm(Val));
  return MCDisassembler::Success;
}

#include "LanaiGenDisassemblerTables.inc"

static DecodeStatus readInstruction32(ArrayRef<uint8_t> Bytes, uint64_t &Size,
                                      uint32_t &Insn) {
  // We want to read exactly 4 bytes of data.
  if (Bytes.size() < 4) {
    Size = 0;
    return MCDisassembler::Fail;
  }

  // Encoded as big-endian 32-bit word in the stream.
  Insn =
      (Bytes[0] << 24) | (Bytes[1] << 16) | (Bytes[2] << 8) | (Bytes[3] << 0);

  return MCDisassembler::Success;
}

static void PostOperandDecodeAdjust(MCInst &Instr, uint32_t Insn) {
  unsigned AluOp = LPAC::ADD;
  // Fix up for pre and post operations.
  int PqShift = -1;
  if (isRMOpcode(Instr.getOpcode()))
    PqShift = 16;
  else if (isSPLSOpcode(Instr.getOpcode()))
    PqShift = 10;
  else if (isRRMOpcode(Instr.getOpcode())) {
    PqShift = 16;
    // Determine RRM ALU op.
    AluOp = (Insn >> 8) & 0x7;
    if (AluOp == 7)
      // Handle JJJJJ
      // 0b10000 or 0b11000
      AluOp |= 0x20 | (((Insn >> 3) & 0xf) << 1);
  }

  if (PqShift != -1) {
    unsigned PQ = (Insn >> PqShift) & 0x3;
    switch (PQ) {
    case 0x0:
      if (Instr.getOperand(2).isReg()) {
        Instr.getOperand(2).setReg(Lanai::R0);
      }
      if (Instr.getOperand(2).isImm())
        Instr.getOperand(2).setImm(0);
      break;
    case 0x1:
      AluOp = LPAC::makePostOp(AluOp);
      break;
    case 0x2:
      break;
    case 0x3:
      AluOp = LPAC::makePreOp(AluOp);
      break;
    }
    Instr.addOperand(MCOperand::createImm(AluOp));
  }
}

DecodeStatus
LanaiDisassembler::getInstruction(MCInst &Instr, uint64_t &Size,
                                  ArrayRef<uint8_t> Bytes, uint64_t Address,
                                  raw_ostream & /*CStream*/) const {
  uint32_t Insn;

  DecodeStatus Result = readInstruction32(Bytes, Size, Insn);

  if (Result == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  // Call auto-generated decoder function
  Result =
      decodeInstruction(DecoderTableLanai32, Instr, Insn, Address, this, STI);

  if (Result != MCDisassembler::Fail) {
    PostOperandDecodeAdjust(Instr, Insn);
    Size = 4;
    return Result;
  }

  return MCDisassembler::Fail;
}
