//===-- MSP430AsmBackend.cpp - MSP430 Assembler Backend -------------------===//
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

#include "MCTargetDesc/MSP430FixupKinds.h"
#include "MCTargetDesc/MSP430MCTargetDesc.h"
#include "vm/core/ADT/APInt.h"
#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCAssembler.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCELFObjectWriter.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/MCSymbol.h"
#include "vm/core/MC/MCTargetOptions.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

namespace {
class MSP430AsmBackend : public MCAsmBackend {
  uint8_t OSABI;

  uint64_t adjustFixupValue(const MCFixup &Fixup, uint64_t Value,
                            MCContext &Ctx) const;

public:
  MSP430AsmBackend(const MCSubtargetInfo &STI, uint8_t OSABI)
      : MCAsmBackend(toolchain::endianness::little), OSABI(OSABI) {}
  ~MSP430AsmBackend() override = default;

  void applyFixup(const MCFragment &, const MCFixup &, const MCValue &Target,
                  uint8_t *Data, uint64_t Value, bool IsResolved) override;

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override {
    return createMSP430ELFObjectWriter(OSABI);
  }

  MCFixupKindInfo getFixupKindInfo(MCFixupKind Kind) const override {
    // clang-format off
    const static MCFixupKindInfo Infos[MSP430::NumTargetFixupKinds] = {
      // This table must be in the same order of enum in MSP430FixupKinds.h.
      //
      // name            offset bits flags
      {"fixup_32",            0, 32, 0},
      {"fixup_10_pcrel",      0, 10, 0},
      {"fixup_16",            0, 16, 0},
      {"fixup_16_pcrel",      0, 16, 0},
      {"fixup_16_byte",       0, 16, 0},
      {"fixup_16_pcrel_byte", 0, 16, 0},
      {"fixup_2x_pcrel",      0, 10, 0},
      {"fixup_rl_pcrel",      0, 16, 0},
      {"fixup_8",             0,  8, 0},
      {"fixup_sym_diff",      0, 32, 0},
    };
    // clang-format on
    static_assert((std::size(Infos)) == MSP430::NumTargetFixupKinds,
                  "Not all fixup kinds added to Infos array");

    if (Kind < FirstTargetFixupKind)
      return MCAsmBackend::getFixupKindInfo(Kind);
  
    return Infos[Kind - FirstTargetFixupKind];
  }

  bool writeNopData(raw_ostream &OS, uint64_t Count,
                    const MCSubtargetInfo *STI) const override;
};

uint64_t MSP430AsmBackend::adjustFixupValue(const MCFixup &Fixup,
                                            uint64_t Value,
                                            MCContext &Ctx) const {
  unsigned Kind = Fixup.getKind();
  switch (Kind) {
  case MSP430::fixup_10_pcrel: {
    if (Value & 0x1)
      Ctx.reportError(Fixup.getLoc(), "fixup value must be 2-byte aligned");

    // Offset is signed
    int16_t Offset = Value;
    // Jumps are in words
    Offset >>= 1;
    // PC points to the next instruction so decrement by one
    --Offset;

    if (Offset < -512 || Offset > 511)
      Ctx.reportError(Fixup.getLoc(), "fixup value out of range");

    // Mask 10 bits
    Offset &= 0x3ff;

    return Offset;
  }
  default:
    return Value;
  }
}

void MSP430AsmBackend::applyFixup(const MCFragment &F, const MCFixup &Fixup,
                                  const MCValue &Target, uint8_t *Data,
                                  uint64_t Value, bool IsResolved) {
  maybeAddReloc(F, Fixup, Target, Value, IsResolved);
  Value = adjustFixupValue(Fixup, Value, getContext());
  MCFixupKindInfo Info = getFixupKindInfo(Fixup.getKind());
  if (!Value)
    return; // Doesn't change encoding.

  // Shift the value into position.
  Value <<= Info.TargetOffset;

  unsigned NumBytes = alignTo(Info.TargetSize + Info.TargetOffset, 8) / 8;
  assert(Fixup.getOffset() + NumBytes <= F.getSize() &&
         "Invalid fixup offset!");

  // For each byte of the fragment that the fixup touches, mask in the
  // bits from the fixup value.
  for (unsigned i = 0; i != NumBytes; ++i) {
    Data[i] |= uint8_t((Value >> (i * 8)) & 0xff);
  }
}

bool MSP430AsmBackend::writeNopData(raw_ostream &OS, uint64_t Count,
                                    const MCSubtargetInfo *STI) const {
  if ((Count % 2) != 0)
    return false;

  // The canonical nop on MSP430 is mov #0, r3
  uint64_t NopCount = Count / 2;
  while (NopCount--)
    OS.write("\x03\x43", 2);

  return true;
}

} // end anonymous namespace

MCAsmBackend *toolchain::createMSP430MCAsmBackend(const Target &T,
                                             const MCSubtargetInfo &STI,
                                             const MCRegisterInfo &MRI,
                                             const MCTargetOptions &Options) {
  return new MSP430AsmBackend(STI, ELF::ELFOSABI_STANDALONE);
}
