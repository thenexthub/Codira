//===-- WebAssemblyAsmBackend.cpp - WebAssembly Assembler Backend ---------===//
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
///
/// \file
/// This file implements the WebAssemblyAsmBackend class.
///
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/WebAssemblyFixupKinds.h"
#include "MCTargetDesc/WebAssemblyMCTargetDesc.h"
#include "vm/core/ADT/StringSwitch.h"
#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCAssembler.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/MC/MCSubtargetInfo.h"
#include "vm/core/MC/MCSymbol.h"
#include "vm/core/MC/MCValue.h"
#include "vm/core/MC/MCWasmObjectWriter.h"
#include "vm/core/Support/raw_ostream.h"

using namespace vm::core;

namespace {

class WebAssemblyAsmBackend final : public MCAsmBackend {
  bool Is64Bit;
  bool IsEmscripten;

public:
  explicit WebAssemblyAsmBackend(bool Is64Bit, bool IsEmscripten)
      : MCAsmBackend(toolchain::endianness::little), Is64Bit(Is64Bit),
        IsEmscripten(IsEmscripten) {}

  std::optional<MCFixupKind> getFixupKind(StringRef Name) const override;
  MCFixupKindInfo getFixupKindInfo(MCFixupKind Kind) const override;

  void applyFixup(const MCFragment &, const MCFixup &, const MCValue &Target,
                  uint8_t *Data, uint64_t Value, bool) override;

  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override;

  bool writeNopData(raw_ostream &OS, uint64_t Count,
                    const MCSubtargetInfo *STI) const override;
};

std::optional<MCFixupKind>
WebAssemblyAsmBackend::getFixupKind(StringRef Name) const {
  unsigned Type = toolchain::StringSwitch<unsigned>(Name)
#define WASM_RELOC(NAME, ID) .Case(#NAME, ID)
#include "vm/core/BinaryFormat/WasmRelocs.def"
#undef WASM_RELOC
                      .Default(-1u);
  if (Type != -1u)
    return static_cast<MCFixupKind>(FirstLiteralRelocationKind + Type);
  return std::nullopt;
}

MCFixupKindInfo
WebAssemblyAsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
  const static MCFixupKindInfo Infos[WebAssembly::NumTargetFixupKinds] = {
      // This table *must* be in the order that the fixup_* kinds are defined in
      // WebAssemblyFixupKinds.h.
      //
      // Name                     Offset (bits) Size (bits)     Flags
      {"fixup_sleb128_i32", 0, 5 * 8, 0},
      {"fixup_sleb128_i64", 0, 10 * 8, 0},
      {"fixup_uleb128_i32", 0, 5 * 8, 0},
      {"fixup_uleb128_i64", 0, 10 * 8, 0},
  };

  // Fixup kinds from raw relocation types and .reloc directives force
  // relocations and do not use these fields.
  if (mc::isRelocation(Kind))
    return {};

  if (Kind < FirstTargetFixupKind)
    return MCAsmBackend::getFixupKindInfo(Kind);

  assert(unsigned(Kind - FirstTargetFixupKind) <
             WebAssembly::NumTargetFixupKinds &&
         "Invalid kind!");
  return Infos[Kind - FirstTargetFixupKind];
}

bool WebAssemblyAsmBackend::writeNopData(raw_ostream &OS, uint64_t Count,
                                         const MCSubtargetInfo *STI) const {
  for (uint64_t I = 0; I < Count; ++I)
    OS << char(WebAssembly::Nop);

  return true;
}

void WebAssemblyAsmBackend::applyFixup(const MCFragment &F,
                                       const MCFixup &Fixup,
                                       const MCValue &Target, uint8_t *Data,
                                       uint64_t Value, bool IsResolved) {
  if (!IsResolved)
    Asm->getWriter().recordRelocation(F, Fixup, Target, Value);

  MCFixupKindInfo Info = getFixupKindInfo(Fixup.getKind());
  assert(Info.Flags == 0 && "WebAssembly does not use MCFixupKindInfo flags");

  unsigned NumBytes = alignTo(Info.TargetSize, 8) / 8;
  if (Value == 0)
    return; // Doesn't change encoding.

  // Shift the value into position.
  Value <<= Info.TargetOffset;

  assert(Fixup.getOffset() + NumBytes <= F.getSize() &&
         "Invalid fixup offset!");

  // For each byte of the fragment that the fixup touches, mask in the
  // bits from the fixup value.
  for (unsigned I = 0; I != NumBytes; ++I)
    Data[I] |= uint8_t((Value >> (I * 8)) & 0xff);
}

std::unique_ptr<MCObjectTargetWriter>
WebAssemblyAsmBackend::createObjectTargetWriter() const {
  return createWebAssemblyWasmObjectWriter(Is64Bit, IsEmscripten);
}

} // end anonymous namespace

MCAsmBackend *toolchain::createWebAssemblyAsmBackend(const Triple &TT) {
  return new WebAssemblyAsmBackend(TT.isArch64Bit(), TT.isOSEmscripten());
}
