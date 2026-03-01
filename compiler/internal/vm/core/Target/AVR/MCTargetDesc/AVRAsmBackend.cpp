//===-- AVRAsmBackend.cpp - AVR Asm Backend  ------------------------------===//
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
// This file implements the AVRAsmBackend class.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/AVRAsmBackend.h"
#include "MCTargetDesc/AVRFixupKinds.h"
#include "MCTargetDesc/AVRMCTargetDesc.h"
#include "vm/core/ADT/StringSwitch.h"
#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCAssembler.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCELFObjectWriter.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/MCValue.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/MathExtras.h"
#include "vm/core/Support/raw_ostream.h"

namespace adjust {

using namespace vm::core;

static void unsigned_width(unsigned Width, uint64_t Value,
                           std::string Description, const MCFixup &Fixup,
                           MCContext *Ctx) {
  if (!isUIntN(Width, Value)) {
    std::string Diagnostic = "out of range " + Description;

    int64_t Max = maxUIntN(Width);

    Diagnostic +=
        " (expected an integer in the range 0 to " + std::to_string(Max) + ")";

    Ctx->reportError(Fixup.getLoc(), Diagnostic);
  }
}

/// Adjusts the value of a branch target before fixup application.
static void adjustBranch(unsigned Size, const MCFixup &Fixup, uint64_t &Value,
                         MCContext *Ctx) {
  // We have one extra bit of precision because the value is rightshifted by
  // one.
  unsigned_width(Size + 1, Value, std::string("branch target"), Fixup, Ctx);

  // Rightshifts the value by one.
  AVR::fixups::adjustBranchTarget(Value);
}

/// Adjusts the value of a relative branch target before fixup application.
static bool adjustRelativeBranch(unsigned Size, const MCFixup &Fixup,
                                 uint64_t &Value, const MCSubtargetInfo *STI) {
  // Jumps are relative to the current instruction.
  Value -= 2;

  // We have one extra bit of precision because the value is rightshifted by
  // one.
  Size += 1;

  assert(STI && "STI can not be NULL");

  if (!isIntN(Size, Value) && STI->hasFeature(AVR::FeatureWrappingRjmp)) {
    const int32_t FlashSize = 0x2000;
    int32_t SignedValue = Value;

    uint64_t WrappedValue = SignedValue > 0 ? (uint64_t)(Value - FlashSize)
                                            : (uint64_t)(FlashSize + Value);

    if (isIntN(Size, WrappedValue)) {
      Value = WrappedValue;
    }
  }

  if (!isIntN(Size, Value)) {
    return false;
  }

  // Rightshifts the value by one.
  AVR::fixups::adjustBranchTarget(Value);

  return true;
}

/// 22-bit absolute fixup.
///
/// Resolves to:
/// 1001 kkkk 010k kkkk kkkk kkkk 111k kkkk
///
/// Offset of 0 (so the result is left shifted by 3 bits before application).
static void fixup_call(unsigned Size, const MCFixup &Fixup, uint64_t &Value,
                       MCContext *Ctx) {
  adjustBranch(Size, Fixup, Value, Ctx);

  auto top = Value & (0xf00000 << 6);   // the top four bits
  auto middle = Value & (0x1ffff << 5); // the middle 13 bits
  auto bottom = Value & 0x1f;           // end bottom 5 bits

  Value = (top << 6) | (middle << 3) | (bottom << 0);
}

/// 7-bit PC-relative fixup.
///
/// Resolves to:
/// 0000 00kk kkkk k000
/// Offset of 0 (so the result is left shifted by 3 bits before application).
static void fixup_7_pcrel(unsigned Size, const MCFixup &Fixup, uint64_t &Value,
                          MCContext *Ctx) {
  if (!adjustRelativeBranch(Size, Fixup, Value, Ctx->getSubtargetInfo())) {
    llvm_unreachable("should've been emitted as a relocation");
  }

  // Because the value may be negative, we must mask out the sign bits
  Value &= 0x7f;
}

/// 12-bit PC-relative fixup.
/// Yes, the fixup is 12 bits even though the name says otherwise.
///
/// Resolves to:
/// 0000 kkkk kkkk kkkk
/// Offset of 0 (so the result isn't left-shifted before application).
static void fixup_13_pcrel(unsigned Size, const MCFixup &Fixup, uint64_t &Value,
                           MCContext *Ctx) {
  if (!adjustRelativeBranch(Size, Fixup, Value, Ctx->getSubtargetInfo())) {
    llvm_unreachable("should've been emitted as a relocation");
  }

  // Because the value may be negative, we must mask out the sign bits
  Value &= 0xfff;
}

/// 6-bit fixup for the immediate operand of the STD/LDD family of
/// instructions.
///
/// Resolves to:
/// 10q0 qq10 0000 1qqq
static void fixup_6(const MCFixup &Fixup, uint64_t &Value, MCContext *Ctx) {
  unsigned_width(6, Value, std::string("immediate"), Fixup, Ctx);

  Value = ((Value & 0x20) << 8) | ((Value & 0x18) << 7) | (Value & 0x07);
}

/// 6-bit fixup for the immediate operand of the ADIW family of
/// instructions.
///
/// Resolves to:
/// 0000 0000 kk00 kkkk
static void fixup_6_adiw(const MCFixup &Fixup, uint64_t &Value,
                         MCContext *Ctx) {
  unsigned_width(6, Value, std::string("immediate"), Fixup, Ctx);

  Value = ((Value & 0x30) << 2) | (Value & 0x0f);
}

/// 5-bit port number fixup on the SBIC family of instructions.
///
/// Resolves to:
/// 0000 0000 AAAA A000
static void fixup_port5(const MCFixup &Fixup, uint64_t &Value, MCContext *Ctx) {
  unsigned_width(5, Value, std::string("port number"), Fixup, Ctx);

  Value &= 0x1f;

  Value <<= 3;
}

/// 6-bit port number fixup on the IN family of instructions.
///
/// Resolves to:
/// 1011 0AAd dddd AAAA
static void fixup_port6(const MCFixup &Fixup, uint64_t &Value, MCContext *Ctx) {
  unsigned_width(6, Value, std::string("port number"), Fixup, Ctx);

  Value = ((Value & 0x30) << 5) | (Value & 0x0f);
}

/// 7-bit data space address fixup for the LDS/STS instructions on AVRTiny.
///
/// Resolves to:
/// 1010 ikkk dddd kkkk
static void fixup_lds_sts_16(const MCFixup &Fixup, uint64_t &Value,
                             MCContext *Ctx) {
  unsigned_width(7, Value, std::string("immediate"), Fixup, Ctx);
  Value = ((Value & 0x70) << 8) | (Value & 0x0f);
}

/// Adjusts a program memory address.
/// This is a simple right-shift.
static void pm(uint64_t &Value) { Value >>= 1; }

/// Fixups relating to the LDI instruction.
namespace ldi {

/// Adjusts a value to fix up the immediate of an `LDI Rd, K` instruction.
///
/// Resolves to:
/// 0000 KKKK 0000 KKKK
/// Offset of 0 (so the result isn't left-shifted before application).
static void fixup(unsigned Size, const MCFixup &Fixup, uint64_t &Value,
                  MCContext *Ctx) {
  uint64_t upper = Value & 0xf0;
  uint64_t lower = Value & 0x0f;

  Value = (upper << 4) | lower;
}

static void neg(uint64_t &Value) { Value *= -1; }

static void lo8(unsigned Size, const MCFixup &Fixup, uint64_t &Value,
                MCContext *Ctx) {
  Value &= 0xff;
  ldi::fixup(Size, Fixup, Value, Ctx);
}

static void hi8(unsigned Size, const MCFixup &Fixup, uint64_t &Value,
                MCContext *Ctx) {
  Value = (Value & 0xff00) >> 8;
  ldi::fixup(Size, Fixup, Value, Ctx);
}

static void hh8(unsigned Size, const MCFixup &Fixup, uint64_t &Value,
                MCContext *Ctx) {
  Value = (Value & 0xff0000) >> 16;
  ldi::fixup(Size, Fixup, Value, Ctx);
}

static void ms8(unsigned Size, const MCFixup &Fixup, uint64_t &Value,
                MCContext *Ctx) {
  Value = (Value & 0xff000000) >> 24;
  ldi::fixup(Size, Fixup, Value, Ctx);
}

} // namespace ldi
} // namespace adjust

namespace vm::core {

// Prepare value for the target space for it
void AVRAsmBackend::adjustFixupValue(const MCFixup &Fixup,
                                     const MCValue &Target, uint64_t &Value,
                                     MCContext *Ctx) const {
  // The size of the fixup in bits.
  uint64_t Size = AVRAsmBackend::getFixupKindInfo(Fixup.getKind()).TargetSize;

  unsigned Kind = Fixup.getKind();
  switch (Kind) {
  default:
    llvm_unreachable("unhandled fixup");
  case AVR::fixup_7_pcrel:
    adjust::fixup_7_pcrel(Size, Fixup, Value, Ctx);
    break;
  case AVR::fixup_13_pcrel:
    adjust::fixup_13_pcrel(Size, Fixup, Value, Ctx);
    break;
  case AVR::fixup_call:
    adjust::fixup_call(Size, Fixup, Value, Ctx);
    break;
  case AVR::fixup_ldi:
    adjust::ldi::fixup(Size, Fixup, Value, Ctx);
    break;
  case AVR::fixup_lo8_ldi:
    adjust::ldi::lo8(Size, Fixup, Value, Ctx);
    break;
  case AVR::fixup_lo8_ldi_pm:
  case AVR::fixup_lo8_ldi_gs:
    adjust::pm(Value);
    adjust::ldi::lo8(Size, Fixup, Value, Ctx);
    break;
  case AVR::fixup_hi8_ldi:
    adjust::ldi::hi8(Size, Fixup, Value, Ctx);
    break;
  case AVR::fixup_hi8_ldi_pm:
  case AVR::fixup_hi8_ldi_gs:
    adjust::pm(Value);
    adjust::ldi::hi8(Size, Fixup, Value, Ctx);
    break;
  case AVR::fixup_hh8_ldi:
  case AVR::fixup_hh8_ldi_pm:
    if (Kind == AVR::fixup_hh8_ldi_pm)
      adjust::pm(Value);

    adjust::ldi::hh8(Size, Fixup, Value, Ctx);
    break;
  case AVR::fixup_ms8_ldi:
    adjust::ldi::ms8(Size, Fixup, Value, Ctx);
    break;

  case AVR::fixup_lo8_ldi_neg:
  case AVR::fixup_lo8_ldi_pm_neg:
    if (Kind == AVR::fixup_lo8_ldi_pm_neg)
      adjust::pm(Value);

    adjust::ldi::neg(Value);
    adjust::ldi::lo8(Size, Fixup, Value, Ctx);
    break;
  case AVR::fixup_hi8_ldi_neg:
  case AVR::fixup_hi8_ldi_pm_neg:
    if (Kind == AVR::fixup_hi8_ldi_pm_neg)
      adjust::pm(Value);

    adjust::ldi::neg(Value);
    adjust::ldi::hi8(Size, Fixup, Value, Ctx);
    break;
  case AVR::fixup_hh8_ldi_neg:
  case AVR::fixup_hh8_ldi_pm_neg:
    if (Kind == AVR::fixup_hh8_ldi_pm_neg)
      adjust::pm(Value);

    adjust::ldi::neg(Value);
    adjust::ldi::hh8(Size, Fixup, Value, Ctx);
    break;
  case AVR::fixup_ms8_ldi_neg:
    adjust::ldi::neg(Value);
    adjust::ldi::ms8(Size, Fixup, Value, Ctx);
    break;
  case AVR::fixup_16:
    adjust::unsigned_width(16, Value, std::string("port number"), Fixup, Ctx);

    Value &= 0xffff;
    break;
  case AVR::fixup_16_pm:
    Value >>= 1; // Flash addresses are always shifted.
    adjust::unsigned_width(16, Value, std::string("port number"), Fixup, Ctx);

    Value &= 0xffff;
    break;

  case AVR::fixup_6:
    adjust::fixup_6(Fixup, Value, Ctx);
    break;
  case AVR::fixup_6_adiw:
    adjust::fixup_6_adiw(Fixup, Value, Ctx);
    break;

  case AVR::fixup_port5:
    adjust::fixup_port5(Fixup, Value, Ctx);
    break;

  case AVR::fixup_port6:
    adjust::fixup_port6(Fixup, Value, Ctx);
    break;

  case AVR::fixup_lds_sts_16:
    adjust::fixup_lds_sts_16(Fixup, Value, Ctx);
    break;

  // Fixups which do not require adjustments.
  case FK_Data_1:
  case FK_Data_2:
  case FK_Data_4:
  case FK_Data_8:
    break;
  }
}

std::unique_ptr<MCObjectTargetWriter>
AVRAsmBackend::createObjectTargetWriter() const {
  return createAVRELFObjectWriter(MCELFObjectTargetWriter::getOSABI(OSType));
}

void AVRAsmBackend::applyFixup(const MCFragment &F, const MCFixup &Fixup,
                               const MCValue &Target, uint8_t *Data,
                               uint64_t Value, bool IsResolved) {
  // AVR sets the fixup value to bypass the assembly time overflow with a
  // relocation.
  if (IsResolved) {
    auto TargetVal = MCValue::get(Target.getAddSym(), Target.getSubSym(), Value,
                                  Target.getSpecifier());
    if (forceRelocation(F, Fixup, TargetVal))
      IsResolved = false;
  }
  if (!IsResolved)
    Asm->getWriter().recordRelocation(F, Fixup, Target, Value);

  if (mc::isRelocation(Fixup.getKind()))
    return;
  adjustFixupValue(Fixup, Target, Value, &getContext());
  if (Value == 0)
    return; // Doesn't change encoding.

  MCFixupKindInfo Info = getFixupKindInfo(Fixup.getKind());

  // The number of bits in the fixup mask
  unsigned NumBits = Info.TargetSize + Info.TargetOffset;
  auto NumBytes = (NumBits / 8) + ((NumBits % 8) == 0 ? 0 : 1);

  // Shift the value into position.
  Value <<= Info.TargetOffset;

  assert(Fixup.getOffset() + NumBytes <= F.getSize() &&
         "Invalid fixup offset!");

  // For each byte of the fragment that the fixup touches, mask in the
  // bits from the fixup value.
  for (unsigned i = 0; i < NumBytes; ++i) {
    uint8_t mask = (((Value >> (i * 8)) & 0xff));
    Data[i] |= mask;
  }
}

std::optional<MCFixupKind> AVRAsmBackend::getFixupKind(StringRef Name) const {
  unsigned Type;
  Type = toolchain::StringSwitch<unsigned>(Name)
#define ELF_RELOC(X, Y) .Case(#X, Y)
#include "vm/core/BinaryFormat/ELFRelocs/AVR.def"
#undef ELF_RELOC
             .Case("BFD_RELOC_NONE", ELF::R_AVR_NONE)
             .Case("BFD_RELOC_16", ELF::R_AVR_16)
             .Case("BFD_RELOC_32", ELF::R_AVR_32)
             .Default(-1u);
  if (Type != -1u)
    return static_cast<MCFixupKind>(FirstLiteralRelocationKind + Type);
  return std::nullopt;
}

MCFixupKindInfo AVRAsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
  // NOTE: Many AVR fixups work on sets of non-contignous bits. We work around
  // this by saying that the fixup is the size of the entire instruction.
  const static MCFixupKindInfo Infos[AVR::NumTargetFixupKinds] = {
      // This table *must* be in same the order of fixup_* kinds in
      // AVRFixupKinds.h.
      //
      // name                    offset  bits  flags
      {"fixup_32", 0, 32, 0},

      {"fixup_7_pcrel", 3, 7, 0},
      {"fixup_13_pcrel", 0, 12, 0},

      {"fixup_16", 0, 16, 0},
      {"fixup_16_pm", 0, 16, 0},

      {"fixup_ldi", 0, 8, 0},

      {"fixup_lo8_ldi", 0, 8, 0},
      {"fixup_hi8_ldi", 0, 8, 0},
      {"fixup_hh8_ldi", 0, 8, 0},
      {"fixup_ms8_ldi", 0, 8, 0},

      {"fixup_lo8_ldi_neg", 0, 8, 0},
      {"fixup_hi8_ldi_neg", 0, 8, 0},
      {"fixup_hh8_ldi_neg", 0, 8, 0},
      {"fixup_ms8_ldi_neg", 0, 8, 0},

      {"fixup_lo8_ldi_pm", 0, 8, 0},
      {"fixup_hi8_ldi_pm", 0, 8, 0},
      {"fixup_hh8_ldi_pm", 0, 8, 0},

      {"fixup_lo8_ldi_pm_neg", 0, 8, 0},
      {"fixup_hi8_ldi_pm_neg", 0, 8, 0},
      {"fixup_hh8_ldi_pm_neg", 0, 8, 0},

      {"fixup_call", 0, 22, 0},

      {"fixup_6", 0, 16, 0}, // non-contiguous
      {"fixup_6_adiw", 0, 6, 0},

      {"fixup_lo8_ldi_gs", 0, 8, 0},
      {"fixup_hi8_ldi_gs", 0, 8, 0},

      {"fixup_8", 0, 8, 0},
      {"fixup_8_lo8", 0, 8, 0},
      {"fixup_8_hi8", 0, 8, 0},
      {"fixup_8_hlo8", 0, 8, 0},

      {"fixup_diff8", 0, 8, 0},
      {"fixup_diff16", 0, 16, 0},
      {"fixup_diff32", 0, 32, 0},

      {"fixup_lds_sts_16", 0, 16, 0},

      {"fixup_port6", 0, 16, 0}, // non-contiguous
      {"fixup_port5", 3, 5, 0},
  };

  // Fixup kinds from .reloc directive are like R_AVR_NONE. They do not require
  // any extra processing.
  if (mc::isRelocation(Kind))
    return {};

  if (Kind < FirstTargetFixupKind)
    return MCAsmBackend::getFixupKindInfo(Kind);

  assert(unsigned(Kind - FirstTargetFixupKind) < AVR::NumTargetFixupKinds &&
         "Invalid kind!");

  return Infos[Kind - FirstTargetFixupKind];
}

bool AVRAsmBackend::writeNopData(raw_ostream &OS, uint64_t Count,
                                 const MCSubtargetInfo *STI) const {
  // If the count is not 2-byte aligned, we must be writing data into the text
  // section (otherwise we have unaligned instructions, and thus have far
  // bigger problems), so just write zeros instead.
  assert((Count % 2) == 0 && "NOP instructions must be 2 bytes");

  OS.write_zeros(Count);
  return true;
}

bool AVRAsmBackend::forceRelocation(const MCFragment &F, const MCFixup &Fixup,
                                    const MCValue &Target) {
  switch ((unsigned)Fixup.getKind()) {
  default:
    return false;

  case AVR::fixup_7_pcrel:
  case AVR::fixup_13_pcrel:
  case AVR::fixup_call:
    return true;
  }
}

MCAsmBackend *createAVRAsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                  const MCRegisterInfo &MRI,
                                  const toolchain::MCTargetOptions &TO) {
  return new AVRAsmBackend(STI.getTargetTriple().getOS());
}

} // end of namespace vm::core
