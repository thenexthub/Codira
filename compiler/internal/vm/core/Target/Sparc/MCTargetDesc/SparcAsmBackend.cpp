//===-- SparcAsmBackend.cpp - Sparc Assembler Backend ---------------------===//
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

#include "MCTargetDesc/SparcFixupKinds.h"
#include "MCTargetDesc/SparcMCTargetDesc.h"
#include "vm/core/ADT/StringSwitch.h"
#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCELFObjectWriter.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/MCValue.h"
#include "vm/core/MC/TargetRegistry.h"
#include "vm/core/Support/EndianStream.h"

using namespace vm::core;

static unsigned adjustFixupValue(unsigned Kind, uint64_t Value) {
  switch (Kind) {
  default:
    assert(uint16_t(Kind) < FirstTargetFixupKind && "Unknown fixup kind!");
    return Value;
  case FK_Data_1:
  case FK_Data_2:
  case FK_Data_4:
  case FK_Data_8:
    return Value;

  case Sparc::fixup_sparc_call30:
    return (Value >> 2) & 0x3fffffff;

  case ELF::R_SPARC_WDISP22:
    return (Value >> 2) & 0x3fffff;

  case ELF::R_SPARC_WDISP19:
    return (Value >> 2) & 0x7ffff;

  case ELF::R_SPARC_WDISP16: {
    // A.3 Branch on Integer Register with Prediction (BPr)
    // Inst{21-20} = d16hi;
    // Inst{13-0}  = d16lo;
    unsigned d16hi = (Value >> 16) & 0x3;
    unsigned d16lo = (Value >> 2) & 0x3fff;
    return (d16hi << 20) | d16lo;
  }

  case ELF::R_SPARC_WDISP10: {
    // FIXME this really should be an error reporting check.
    assert((Value & 0x3) == 0);

    // 7.17 Compare and Branch
    // Inst{20-19} = d10hi;
    // Inst{12-5}  = d10lo;
    unsigned d10hi = (Value >> 10) & 0x3;
    unsigned d10lo = (Value >> 2) & 0xff;
    return (d10hi << 19) | (d10lo << 5);
  }

  case ELF::R_SPARC_HIX22:
    return (~Value >> 10) & 0x3fffff;

  case ELF::R_SPARC_PC22:
  case ELF::R_SPARC_HI22:
  case ELF::R_SPARC_LM22:
    return (Value >> 10) & 0x3fffff;

  case Sparc::fixup_sparc_13:
    return Value & 0x1fff;

  case ELF::R_SPARC_5:
    return Value & 0x1f;

  case ELF::R_SPARC_LOX10:
    return (Value & 0x3ff) | 0x1c00;

  case ELF::R_SPARC_PC10:
  case ELF::R_SPARC_LO10:
    return Value & 0x3ff;

  case ELF::R_SPARC_H44:
    return (Value >> 22) & 0x3fffff;
  case ELF::R_SPARC_M44:
    return (Value >> 12) & 0x3ff;
  case ELF::R_SPARC_L44:
    return Value & 0xfff;

  case ELF::R_SPARC_HH22:
    return (Value >> 42) & 0x3fffff;
  case ELF::R_SPARC_HM10:
    return (Value >> 32) & 0x3ff;
  }
}

/// getFixupKindNumBytes - The number of bytes the fixup may change.
static unsigned getFixupKindNumBytes(unsigned Kind) {
    switch (Kind) {
  default:
    return 4;
  case FK_Data_1:
    return 1;
  case FK_Data_2:
    return 2;
  case FK_Data_8:
    return 8;
  }
}

namespace {
class SparcAsmBackend : public MCAsmBackend {
protected:
  bool Is64Bit;
  bool IsV8Plus;

public:
  SparcAsmBackend(const MCSubtargetInfo &STI)
      : MCAsmBackend(STI.getTargetTriple().isLittleEndian()
                         ? toolchain::endianness::little
                         : toolchain::endianness::big),
        Is64Bit(STI.getTargetTriple().isArch64Bit()),
        IsV8Plus(STI.hasFeature(Sparc::FeatureV8Plus)) {}

  std::optional<MCFixupKind> getFixupKind(StringRef Name) const override;
  MCFixupKindInfo getFixupKindInfo(MCFixupKind Kind) const override;
  void applyFixup(const MCFragment &, const MCFixup &, const MCValue &Target,
                  uint8_t *Data, uint64_t Value, bool IsResolved) override;

  bool writeNopData(raw_ostream &OS, uint64_t Count,
                    const MCSubtargetInfo *STI) const override {

    // If the count is not 4-byte aligned, we must be writing data into the
    // text section (otherwise we have unaligned instructions, and thus have
    // far bigger problems), so just write zeros instead.
    OS.write_zeros(Count % 4);

    uint64_t NumNops = Count / 4;
    for (uint64_t i = 0; i != NumNops; ++i)
      support::endian::write<uint32_t>(OS, 0x01000000, Endian);

    return true;
  }
};

class ELFSparcAsmBackend : public SparcAsmBackend {
  Triple::OSType OSType;

public:
  ELFSparcAsmBackend(const MCSubtargetInfo &STI, Triple::OSType OSType)
      : SparcAsmBackend(STI), OSType(OSType) {}

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override {
    uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(OSType);
    return createSparcELFObjectWriter(Is64Bit, IsV8Plus, OSABI);
  }
};
} // end anonymous namespace

std::optional<MCFixupKind> SparcAsmBackend::getFixupKind(StringRef Name) const {
  unsigned Type;
  Type = toolchain::StringSwitch<unsigned>(Name)
#define ELF_RELOC(X, Y) .Case(#X, Y)
#include "vm/core/BinaryFormat/ELFRelocs/Sparc.def"
#undef ELF_RELOC
             .Case("BFD_RELOC_NONE", ELF::R_SPARC_NONE)
             .Case("BFD_RELOC_8", ELF::R_SPARC_8)
             .Case("BFD_RELOC_16", ELF::R_SPARC_16)
             .Case("BFD_RELOC_32", ELF::R_SPARC_32)
             .Case("BFD_RELOC_64", ELF::R_SPARC_64)
             .Default(-1u);
  if (Type == -1u)
    return std::nullopt;
  return static_cast<MCFixupKind>(FirstLiteralRelocationKind + Type);
}

MCFixupKindInfo SparcAsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
  // clang-format off
  const static MCFixupKindInfo InfosBE[Sparc::NumTargetFixupKinds] = {
      // name                    offset bits flags
      {"fixup_sparc_call30",     2,     30,  0},
      {"fixup_sparc_13",        19,     13,  0},
  };

  const static MCFixupKindInfo InfosLE[Sparc::NumTargetFixupKinds] = {
      // name                    offset bits flags
      {"fixup_sparc_call30",     0,     30,  0},
      {"fixup_sparc_13",         0,     13,  0},
  };
  // clang-format on

  if (!mc::isRelocation(Kind)) {
    if (Kind < FirstTargetFixupKind)
      return MCAsmBackend::getFixupKindInfo(Kind);
    assert(unsigned(Kind - FirstTargetFixupKind) < Sparc::NumTargetFixupKinds &&
           "Invalid kind!");
    if (Endian == toolchain::endianness::little)
      return InfosLE[Kind - FirstTargetFixupKind];

    return InfosBE[Kind - FirstTargetFixupKind];
  }

  MCFixupKindInfo Info{};
  switch (uint16_t(Kind)) {
  case ELF::R_SPARC_PC10:
    Info = {"", 22, 10, 0};
    break;
  case ELF::R_SPARC_PC22:
    Info = {"", 10, 22, 0};
    break;
  case ELF::R_SPARC_WDISP10:
    Info = {"", 0, 32, 0};
    break;
  case ELF::R_SPARC_WDISP16:
    Info = {"", 0, 32, 0};
    break;
  case ELF::R_SPARC_WDISP19:
    Info = {"", 13, 19, 0};
    break;
  case ELF::R_SPARC_WDISP22:
    Info = {"", 10, 22, 0};
    break;

  case ELF::R_SPARC_HI22:
    Info = {"", 10, 22, 0};
    break;
  case ELF::R_SPARC_LO10:
    Info = {"", 22, 10, 0};
    break;
  case ELF::R_SPARC_HH22:
    Info = {"", 10, 22, 0};
    break;
  case ELF::R_SPARC_HM10:
    Info = {"", 22, 10, 0};
    break;
  case ELF::R_SPARC_LM22:
    Info = {"", 10, 22, 0};
    break;
  case ELF::R_SPARC_HIX22:
    Info = {"", 10, 22, 0};
    break;
  case ELF::R_SPARC_LOX10:
    Info = {"", 19, 13, 0};
    break;
  }
  if (Endian == toolchain::endianness::little)
    Info.TargetOffset = 32 - Info.TargetOffset - Info.TargetSize;
  return Info;
}

void SparcAsmBackend::applyFixup(const MCFragment &F, const MCFixup &Fixup,
                                 const MCValue &Target, uint8_t *Data,
                                 uint64_t Value, bool IsResolved) {
  maybeAddReloc(F, Fixup, Target, Value, IsResolved);
  if (!IsResolved)
    return;
  Value = adjustFixupValue(Fixup.getKind(), Value);

  unsigned NumBytes = getFixupKindNumBytes(Fixup.getKind());
  // For each byte of the fragment that the fixup touches, mask in the
  // bits from the fixup value.
  for (unsigned i = 0; i != NumBytes; ++i) {
    unsigned Idx = Endian == toolchain::endianness::little ? i : (NumBytes - 1) - i;
    Data[Idx] |= uint8_t((Value >> (i * 8)) & 0xff);
  }
}

MCAsmBackend *toolchain::createSparcAsmBackend(const Target &T,
                                          const MCSubtargetInfo &STI,
                                          const MCRegisterInfo &MRI,
                                          const MCTargetOptions &Options) {
  return new ELFSparcAsmBackend(STI, STI.getTargetTriple().getOS());
}
