//===-- SystemZMCAsmBackend.cpp - SystemZ assembler backend ---------------===//
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

#include "MCTargetDesc/SystemZMCFixups.h"
#include "MCTargetDesc/SystemZMCTargetDesc.h"
#include "vm/core/ADT/StringSwitch.h"
#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCAssembler.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCELFObjectWriter.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/MCValue.h"

using namespace vm::core;

// Value is a fully-resolved relocation value: Symbol + Addend [- Pivot].
// Return the bits that should be installed in a relocation field for
// fixup kind Kind.
static uint64_t extractBitsForFixup(MCFixupKind Kind, uint64_t Value,
                                    const MCFixup &Fixup, MCContext &Ctx) {
  if (Kind < FirstTargetFixupKind)
    return Value;

  auto checkFixupInRange = [&](int64_t Min, int64_t Max) -> bool {
    int64_t SVal = int64_t(Value);
    if (SVal < Min || SVal > Max) {
      Ctx.reportError(Fixup.getLoc(), "operand out of range (" + Twine(SVal) +
                                          " not between " + Twine(Min) +
                                          " and " + Twine(Max) + ")");
      return false;
    }
    return true;
  };

  auto handlePCRelFixupValue = [&](unsigned W) -> uint64_t {
    if (Value % 2 != 0)
      Ctx.reportError(Fixup.getLoc(), "Non-even PC relative offset.");
    if (!checkFixupInRange(minIntN(W) * 2, maxIntN(W) * 2))
      return 0;
    return (int64_t)Value / 2;
  };

  auto handleImmValue = [&](bool IsSigned, unsigned W) -> uint64_t {
    if (!(IsSigned ? checkFixupInRange(minIntN(W), maxIntN(W))
                   : checkFixupInRange(0, maxUIntN(W))))
      return 0;
    return Value;
  };

  switch (unsigned(Kind)) {
  case SystemZ::FK_390_PC12DBL:
    return handlePCRelFixupValue(12);
  case SystemZ::FK_390_PC16DBL:
    return handlePCRelFixupValue(16);
  case SystemZ::FK_390_PC24DBL:
    return handlePCRelFixupValue(24);
  case SystemZ::FK_390_PC32DBL:
    return handlePCRelFixupValue(32);

  case SystemZ::FK_390_TLS_CALL:
    return 0;

  case SystemZ::FK_390_S8Imm:
    return handleImmValue(true, 8);
  case SystemZ::FK_390_S16Imm:
    return handleImmValue(true, 16);
  case SystemZ::FK_390_S20Imm: {
    Value = handleImmValue(true, 20);
    // S20Imm is used only for signed 20-bit displacements.
    // The high byte of a 20 bit displacement value comes first.
    uint64_t DLo = Value & 0xfff;
    uint64_t DHi = (Value >> 12) & 0xff;
    return (DLo << 8) | DHi;
  }
  case SystemZ::FK_390_S32Imm:
    return handleImmValue(true, 32);
  case SystemZ::FK_390_U1Imm:
    return handleImmValue(false, 1);
  case SystemZ::FK_390_U2Imm:
    return handleImmValue(false, 2);
  case SystemZ::FK_390_U3Imm:
    return handleImmValue(false, 3);
  case SystemZ::FK_390_U4Imm:
    return handleImmValue(false, 4);
  case SystemZ::FK_390_U8Imm:
    return handleImmValue(false, 8);
  case SystemZ::FK_390_U12Imm:
    return handleImmValue(false, 12);
  case SystemZ::FK_390_U16Imm:
    return handleImmValue(false, 16);
  case SystemZ::FK_390_U32Imm:
    return handleImmValue(false, 32);
  case SystemZ::FK_390_U48Imm:
    return handleImmValue(false, 48);
  }

  llvm_unreachable("Unknown fixup kind!");
}

namespace {
class SystemZMCAsmBackend : public MCAsmBackend {
public:
  SystemZMCAsmBackend() : MCAsmBackend(toolchain::endianness::big) {}

  // Override MCAsmBackend
  std::optional<MCFixupKind> getFixupKind(StringRef Name) const override;
  MCFixupKindInfo getFixupKindInfo(MCFixupKind Kind) const override;
  void applyFixup(const MCFragment &, const MCFixup &, const MCValue &Target,
                  uint8_t *Data, uint64_t Value, bool IsResolved) override;
  bool writeNopData(raw_ostream &OS, uint64_t Count,
                    const MCSubtargetInfo *STI) const override;
};
} // end anonymous namespace

std::optional<MCFixupKind>
SystemZMCAsmBackend::getFixupKind(StringRef Name) const {
  unsigned Type = toolchain::StringSwitch<unsigned>(Name)
#define ELF_RELOC(X, Y) .Case(#X, Y)
#include "vm/core/BinaryFormat/ELFRelocs/SystemZ.def"
#undef ELF_RELOC
			.Case("BFD_RELOC_NONE", ELF::R_390_NONE)
			.Case("BFD_RELOC_8", ELF::R_390_8)
			.Case("BFD_RELOC_16", ELF::R_390_16)
			.Case("BFD_RELOC_32", ELF::R_390_32)
			.Case("BFD_RELOC_64", ELF::R_390_64)
			.Default(-1u);
  if (Type != -1u)
    return static_cast<MCFixupKind>(FirstLiteralRelocationKind + Type);
  return std::nullopt;
}

MCFixupKindInfo SystemZMCAsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
  // Fixup kinds from .reloc directive are like R_390_NONE. They
  // do not require any extra processing.
  if (mc::isRelocation(Kind))
    return {};

  if (Kind < FirstTargetFixupKind)
    return MCAsmBackend::getFixupKindInfo(Kind);

  assert(unsigned(Kind - FirstTargetFixupKind) < SystemZ::NumTargetFixupKinds &&
         "Invalid kind!");
  return SystemZ::MCFixupKindInfos[Kind - FirstTargetFixupKind];
}

void SystemZMCAsmBackend::applyFixup(const MCFragment &F, const MCFixup &Fixup,
                                     const MCValue &Target, uint8_t *Data,
                                     uint64_t Value, bool IsResolved) {
  if (Target.getSpecifier())
    IsResolved = false;
  maybeAddReloc(F, Fixup, Target, Value, IsResolved);
  MCFixupKind Kind = Fixup.getKind();
  if (mc::isRelocation(Kind))
    return;
  unsigned BitSize = getFixupKindInfo(Kind).TargetSize;
  unsigned Size = (BitSize + 7) / 8;

  assert(Fixup.getOffset() + Size <= F.getSize() && "Invalid fixup offset!");

  // Big-endian insertion of Size bytes.
  Value = extractBitsForFixup(Kind, Value, Fixup, getContext());
  if (BitSize < 64)
    Value &= ((uint64_t)1 << BitSize) - 1;
  unsigned ShiftValue = (Size * 8) - 8;
  for (unsigned I = 0; I != Size; ++I) {
    Data[I] |= uint8_t(Value >> ShiftValue);
    ShiftValue -= 8;
  }
}

bool SystemZMCAsmBackend::writeNopData(raw_ostream &OS, uint64_t Count,
                                       const MCSubtargetInfo *STI) const {
  for (uint64_t I = 0; I != Count; ++I)
    OS << '\x7';
  return true;
}

namespace {
class ELFSystemZAsmBackend : public SystemZMCAsmBackend {
  uint8_t OSABI;

public:
  ELFSystemZAsmBackend(uint8_t OsABI) : SystemZMCAsmBackend(), OSABI(OsABI){};

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override {
    return createSystemZELFObjectWriter(OSABI);
  }
};

class GOFFSystemZAsmBackend : public SystemZMCAsmBackend {
public:
  GOFFSystemZAsmBackend() : SystemZMCAsmBackend(){};

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override {
    return createSystemZGOFFObjectWriter();
  }
};
} // namespace

MCAsmBackend *toolchain::createSystemZMCAsmBackend(const Target &T,
                                              const MCSubtargetInfo &STI,
                                              const MCRegisterInfo &MRI,
                                              const MCTargetOptions &Options) {
  if (STI.getTargetTriple().isOSzOS()) {
    return new GOFFSystemZAsmBackend();
  }

  uint8_t OSABI =
      MCELFObjectTargetWriter::getOSABI(STI.getTargetTriple().getOS());
  return new ELFSystemZAsmBackend(OSABI);
}
