//===-- VEMCCodeEmitter.cpp - Convert VE code to machine code -------------===//
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
// This file implements the VEMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/VEFixupKinds.h"
#include "VE.h"
#include "VEMCAsmInfo.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/MC/MCCodeEmitter.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCFixup.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/MC/MCRegisterInfo.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/MCSymbol.h"
#include "vm/core/Support/EndianStream.h"
#include <cassert>
#include <cstdint>

using namespace vm::core;

#define DEBUG_TYPE "mccodeemitter"

STATISTIC(MCNumEmitted, "Number of MC instructions emitted");

namespace {

class VEMCCodeEmitter : public MCCodeEmitter {
  MCContext &Ctx;

public:
  VEMCCodeEmitter(const MCInstrInfo &, MCContext &ctx)
      : Ctx(ctx) {}
  VEMCCodeEmitter(const VEMCCodeEmitter &) = delete;
  VEMCCodeEmitter &operator=(const VEMCCodeEmitter &) = delete;
  ~VEMCCodeEmitter() override = default;

  void encodeInstruction(const MCInst &MI, SmallVectorImpl<char> &CB,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

  // getBinaryCodeForInstr - TableGen'erated function for getting the
  // binary encoding for an instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  /// getMachineOpValue - Return binary encoding of operand. If the machine
  /// operand requires relocation, record the relocation and return zero.
  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  uint64_t getBranchTargetOpValue(const MCInst &MI, unsigned OpNo,
                                  SmallVectorImpl<MCFixup> &Fixups,
                                  const MCSubtargetInfo &STI) const;
  uint64_t getCCOpValue(const MCInst &MI, unsigned OpNo,
                        SmallVectorImpl<MCFixup> &Fixups,
                        const MCSubtargetInfo &STI) const;
  uint64_t getRDOpValue(const MCInst &MI, unsigned OpNo,
                        SmallVectorImpl<MCFixup> &Fixups,
                        const MCSubtargetInfo &STI) const;
};

} // end anonymous namespace

static void addFixup(SmallVectorImpl<MCFixup> &Fixups, uint32_t Offset,
                     const MCExpr *Value, uint16_t Kind) {
  bool PCRel = false;
  switch (Kind) {
  case VE::fixup_ve_srel32:
  case VE::fixup_ve_pc_hi32:
  case VE::fixup_ve_pc_lo32:
    PCRel = true;
  }
  Fixups.push_back(MCFixup::create(Offset, Value, Kind, PCRel));
}

void VEMCCodeEmitter::encodeInstruction(const MCInst &MI,
                                        SmallVectorImpl<char> &CB,
                                        SmallVectorImpl<MCFixup> &Fixups,
                                        const MCSubtargetInfo &STI) const {
  uint64_t Bits = getBinaryCodeForInstr(MI, Fixups, STI);
  support::endian::write<uint64_t>(CB, Bits, toolchain::endianness::little);

  ++MCNumEmitted; // Keep track of the # of mi's emitted.
}

unsigned VEMCCodeEmitter::getMachineOpValue(const MCInst &MI,
                                            const MCOperand &MO,
                                            SmallVectorImpl<MCFixup> &Fixups,
                                            const MCSubtargetInfo &STI) const {
  if (MO.isReg())
    return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());
  if (MO.isImm())
    return static_cast<unsigned>(MO.getImm());

  assert(MO.isExpr());

  const MCExpr *Expr = MO.getExpr();
  if (const auto *SExpr = dyn_cast<MCSpecifierExpr>(Expr)) {
    auto Kind = VE::getFixupKind(SExpr->getSpecifier());
    addFixup(Fixups, 0, Expr, Kind);
    return 0;
  }

  int64_t Res;
  if (Expr->evaluateAsAbsolute(Res))
    return Res;

  llvm_unreachable("Unhandled expression!");
  return 0;
}

uint64_t
VEMCCodeEmitter::getBranchTargetOpValue(const MCInst &MI, unsigned OpNo,
                                        SmallVectorImpl<MCFixup> &Fixups,
                                        const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  if (MO.isReg() || MO.isImm())
    return getMachineOpValue(MI, MO, Fixups, STI);

  addFixup(Fixups, 0, MO.getExpr(), VE::fixup_ve_srel32);
  return 0;
}

uint64_t VEMCCodeEmitter::getCCOpValue(const MCInst &MI, unsigned OpNo,
                                       SmallVectorImpl<MCFixup> &Fixups,
                                       const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  if (MO.isImm())
    return VECondCodeToVal(
        static_cast<VECC::CondCode>(getMachineOpValue(MI, MO, Fixups, STI)));
  return 0;
}

uint64_t VEMCCodeEmitter::getRDOpValue(const MCInst &MI, unsigned OpNo,
                                       SmallVectorImpl<MCFixup> &Fixups,
                                       const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  if (MO.isImm())
    return VERDToVal(static_cast<VERD::RoundingMode>(
        getMachineOpValue(MI, MO, Fixups, STI)));
  return 0;
}

#include "VEGenMCCodeEmitter.inc"

MCCodeEmitter *toolchain::createVEMCCodeEmitter(const MCInstrInfo &MCII,
                                           MCContext &Ctx) {
  return new VEMCCodeEmitter(MCII, Ctx);
}
