//===- MCAsmBackend.cpp - Target MC Assembly Backend ----------------------===//
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

#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCAssembler.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCDXContainerWriter.h"
#include "vm/core/MC/MCELFObjectWriter.h"
#include "vm/core/MC/MCGOFFObjectWriter.h"
#include "vm/core/MC/MCMachObjectWriter.h"
#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/MC/MCSPIRVObjectWriter.h"
#include "vm/core/MC/MCWasmObjectWriter.h"
#include "vm/core/MC/MCWinCOFFObjectWriter.h"
#include "vm/core/MC/MCXCOFFObjectWriter.h"
#include <cassert>
#include <cstddef>
#include <cstdint>

using namespace vm::core;

MCAsmBackend::~MCAsmBackend() = default;

MCContext &MCAsmBackend::getContext() const { return Asm->getContext(); }

std::unique_ptr<MCObjectWriter>
MCAsmBackend::createObjectWriter(raw_pwrite_stream &OS) const {
  auto TW = createObjectTargetWriter();
  bool IsLE = Endian == toolchain::endianness::little;
  switch (TW->getFormat()) {
  case Triple::MachO:
    return std::make_unique<MachObjectWriter>(
        cast<MCMachObjectTargetWriter>(std::move(TW)), OS, IsLE);
  case Triple::COFF:
    return createWinCOFFObjectWriter(
        cast<MCWinCOFFObjectTargetWriter>(std::move(TW)), OS);
  case Triple::ELF:
    return std::make_unique<ELFObjectWriter>(
        cast<MCELFObjectTargetWriter>(std::move(TW)), OS, IsLE);
  case Triple::SPIRV:
    return createSPIRVObjectWriter(
        cast<MCSPIRVObjectTargetWriter>(std::move(TW)), OS);
  case Triple::Wasm:
    return createWasmObjectWriter(cast<MCWasmObjectTargetWriter>(std::move(TW)),
                                  OS);
  case Triple::GOFF:
    return createGOFFObjectWriter(cast<MCGOFFObjectTargetWriter>(std::move(TW)),
                                  OS);
  case Triple::XCOFF:
    return createXCOFFObjectWriter(
        cast<MCXCOFFObjectTargetWriter>(std::move(TW)), OS);
  case Triple::DXContainer:
    return std::make_unique<DXContainerObjectWriter>(
        cast<MCDXContainerTargetWriter>(std::move(TW)), OS);
  default:
    llvm_unreachable("unexpected object format");
  }
}

std::unique_ptr<MCObjectWriter>
MCAsmBackend::createDwoObjectWriter(raw_pwrite_stream &OS,
                                    raw_pwrite_stream &DwoOS) const {
  auto TW = createObjectTargetWriter();
  switch (TW->getFormat()) {
  case Triple::COFF:
    return createWinCOFFDwoObjectWriter(
        cast<MCWinCOFFObjectTargetWriter>(std::move(TW)), OS, DwoOS);
  case Triple::ELF:
    return std::make_unique<ELFObjectWriter>(
        cast<MCELFObjectTargetWriter>(std::move(TW)), OS, DwoOS,
        Endian == toolchain::endianness::little);
  case Triple::Wasm:
    return createWasmDwoObjectWriter(
        cast<MCWasmObjectTargetWriter>(std::move(TW)), OS, DwoOS);
  default:
    report_fatal_error("dwo only supported with COFF, ELF, and Wasm");
  }
}

std::optional<MCFixupKind> MCAsmBackend::getFixupKind(StringRef Name) const {
  return std::nullopt;
}

MCFixupKindInfo MCAsmBackend::getFixupKindInfo(MCFixupKind Kind) const {
  // clang-format off
  static const MCFixupKindInfo Builtins[] = {
      {"FK_NONE", 0, 0, 0},
      {"FK_Data_1", 0, 8, 0},
      {"FK_Data_2", 0, 16, 0},
      {"FK_Data_4", 0, 32, 0},
      {"FK_Data_8", 0, 64, 0},
      {"FK_Data_leb128", 0, 0, 0},
      {"FK_SecRel_1", 0, 8, 0},
      {"FK_SecRel_2", 0, 16, 0},
      {"FK_SecRel_4", 0, 32, 0},
      {"FK_SecRel_8", 0, 64, 0},
  };
  // clang-format on

  assert(size_t(Kind - FK_NONE) < std::size(Builtins) && "Unknown fixup kind");
  return Builtins[Kind - FK_NONE];
}

bool MCAsmBackend::fixupNeedsRelaxationAdvanced(const MCFragment &,
                                                const MCFixup &Fixup,
                                                const MCValue &, uint64_t Value,
                                                bool Resolved) const {
  if (!Resolved)
    return true;
  return fixupNeedsRelaxation(Fixup, Value);
}

void MCAsmBackend::maybeAddReloc(const MCFragment &F, const MCFixup &Fixup,
                                 const MCValue &Target, uint64_t &Value,
                                 bool IsResolved) {
  if (!IsResolved)
    Asm->getWriter().recordRelocation(F, Fixup, Target, Value);
}

bool MCAsmBackend::isDarwinCanonicalPersonality(const MCSymbol *Sym) const {
  assert(getContext().isMachO());
  // Consider a NULL personality (ie., no personality encoding) to be canonical
  // because it's always at 0.
  if (!Sym)
    return true;

  StringRef name = Sym->getName();
  // XXX: We intentionally leave out "___gcc_personality_v0" because, despite
  // being system-defined like these two, it is not very commonly-used.
  // Reserving an empty slot for it seems silly.
  return name == "___gxx_personality_v0" || name == "___objc_personality_v0";
}

const MCSubtargetInfo *MCAsmBackend::getSubtargetInfo(const MCFragment &F) {
  const MCSubtargetInfo *STI = nullptr;
  STI = F.getSubtargetInfo();
  assert(!F.hasInstructions() || STI != nullptr);
  return STI;
}
