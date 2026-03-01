//===-- LoongArchDisassembler.cpp - Disassembler for LoongArch ------------===//
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
// This file implements the LoongArchDisassembler class.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/LoongArchMCTargetDesc.h"
#include "TargetInfo/LoongArchTargetInfo.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCDecoder.h"
#include "vm/core/MC/MCDecoderOps.h"
#include "vm/core/MC/MCDisassembler/MCDisassembler.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/Endian.h"

using namespace vm::core;
using namespace vm::core::MCD;

#define DEBUG_TYPE "loongarch-disassembler"

typedef MCDisassembler::DecodeStatus DecodeStatus;

namespace {
class LoongArchDisassembler : public MCDisassembler {
public:
  LoongArchDisassembler(const MCSubtargetInfo &STI, MCContext &Ctx)
      : MCDisassembler(STI, Ctx) {}

  DecodeStatus getInstruction(MCInst &Instr, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &CStream) const override;
};
} // end namespace

static MCDisassembler *createLoongArchDisassembler(const Target &T,
                                                   const MCSubtargetInfo &STI,
                                                   MCContext &Ctx) {
  return new LoongArchDisassembler(STI, Ctx);
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeLoongArchDisassembler() {
  // Register the disassembler for each target.
  TargetRegistry::RegisterMCDisassembler(getTheLoongArch32Target(),
                                         createLoongArchDisassembler);
  TargetRegistry::RegisterMCDisassembler(getTheLoongArch64Target(),
                                         createLoongArchDisassembler);
}

static DecodeStatus DecodeGPRRegisterClass(MCInst &Inst, uint64_t RegNo,
                                           uint64_t Address,
                                           const MCDisassembler *Decoder) {
  if (RegNo >= 32)
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(LoongArch::R0 + RegNo));
  return MCDisassembler::Success;
}

static DecodeStatus
DecodeGPRNoR0R1RegisterClass(MCInst &Inst, uint64_t RegNo, uint64_t Address,
                             const MCDisassembler *Decoder) {
  if (RegNo <= 1)
    return MCDisassembler::Fail;
  return DecodeGPRRegisterClass(Inst, RegNo, Address, Decoder);
}

static DecodeStatus DecodeFPR32RegisterClass(MCInst &Inst, uint64_t RegNo,
                                             uint64_t Address,
                                             const MCDisassembler *Decoder) {
  if (RegNo >= 32)
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(LoongArch::F0 + RegNo));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeFPR64RegisterClass(MCInst &Inst, uint64_t RegNo,
                                             uint64_t Address,
                                             const MCDisassembler *Decoder) {
  if (RegNo >= 32)
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(LoongArch::F0_64 + RegNo));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeCFRRegisterClass(MCInst &Inst, uint64_t RegNo,
                                           uint64_t Address,
                                           const MCDisassembler *Decoder) {
  if (RegNo >= 8)
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(LoongArch::FCC0 + RegNo));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeFCSRRegisterClass(MCInst &Inst, uint64_t RegNo,
                                            uint64_t Address,
                                            const MCDisassembler *Decoder) {
  if (RegNo >= 4)
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(LoongArch::FCSR0 + RegNo));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeLSX128RegisterClass(MCInst &Inst, uint64_t RegNo,
                                              uint64_t Address,
                                              const MCDisassembler *Decoder) {
  if (RegNo >= 32)
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(LoongArch::VR0 + RegNo));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeLASX256RegisterClass(MCInst &Inst, uint64_t RegNo,
                                               uint64_t Address,
                                               const MCDisassembler *Decoder) {
  if (RegNo >= 32)
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(LoongArch::XR0 + RegNo));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeSCRRegisterClass(MCInst &Inst, uint64_t RegNo,
                                           uint64_t Address,
                                           const MCDisassembler *Decoder) {
  if (RegNo >= 4)
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(LoongArch::SCR0 + RegNo));
  return MCDisassembler::Success;
}

template <unsigned N, int P = 0>
static DecodeStatus decodeUImmOperand(MCInst &Inst, uint64_t Imm,
                                      int64_t Address,
                                      const MCDisassembler *Decoder) {
  assert(isUInt<N>(Imm) && "Invalid immediate");
  Inst.addOperand(MCOperand::createImm(Imm + P));
  return MCDisassembler::Success;
}

template <unsigned N, unsigned S = 0>
static DecodeStatus decodeSImmOperand(MCInst &Inst, uint64_t Imm,
                                      int64_t Address,
                                      const MCDisassembler *Decoder) {
  assert(isUInt<N>(Imm) && "Invalid immediate");
  // Shift left Imm <S> bits, then sign-extend the number in the bottom <N+S>
  // bits.
  Inst.addOperand(MCOperand::createImm(SignExtend64<N + S>(Imm << S)));
  return MCDisassembler::Success;
}

// Decode AMSWAP.W and UD, which share the same base encoding.
// If rk == 1 and rd == rj, interpret the instruction as UD;
// otherwise decode as AMSWAP.W.
static DecodeStatus DecodeAMOrUDInstruction(MCInst &Inst, unsigned Insn,
                                            uint64_t Address,
                                            const MCDisassembler *Decoder) {
  unsigned Rd = fieldFromInstruction(Insn, 0, 5);
  unsigned Rj = fieldFromInstruction(Insn, 5, 5);
  unsigned Rk = fieldFromInstruction(Insn, 10, 5);

  if (Rk == 1 && Rd == Rj) {
    Inst.setOpcode(LoongArch::UD);
    Inst.addOperand(MCOperand::createImm(Rd));
  } else {
    Inst.setOpcode(LoongArch::AMSWAP_W);
    Inst.addOperand(MCOperand::createReg(LoongArch::R0 + Rd));
    Inst.addOperand(MCOperand::createReg(LoongArch::R0 + Rk));
    Inst.addOperand(MCOperand::createReg(LoongArch::R0 + Rj));
  }

  return MCDisassembler::Success;
}

#include "LoongArchGenDisassemblerTables.inc"

DecodeStatus LoongArchDisassembler::getInstruction(MCInst &MI, uint64_t &Size,
                                                   ArrayRef<uint8_t> Bytes,
                                                   uint64_t Address,
                                                   raw_ostream &CS) const {
  uint32_t Insn;
  DecodeStatus Result;

  // We want to read exactly 4 bytes of data because all LoongArch instructions
  // are fixed 32 bits.
  if (Bytes.size() < 4) {
    Size = 0;
    return MCDisassembler::Fail;
  }

  Insn = support::endian::read32le(Bytes.data());
  // Calling the auto-generated decoder function.
  Result = decodeInstruction(DecoderTable32, MI, Insn, Address, this, STI);
  Size = 4;

  return Result;
}
