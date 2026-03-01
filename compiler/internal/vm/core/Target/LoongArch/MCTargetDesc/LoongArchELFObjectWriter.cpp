//===-- LoongArchELFObjectWriter.cpp - LoongArch ELF Writer ---*- C++ -*---===//
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

#include "MCTargetDesc/LoongArchFixupKinds.h"
#include "MCTargetDesc/LoongArchMCTargetDesc.h"
#include "vm/core/BinaryFormat/ELF.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCELFObjectWriter.h"
#include "vm/core/MC/MCFixup.h"
#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/MC/MCValue.h"
#include "vm/core/Support/ErrorHandling.h"

using namespace vm::core;

namespace {
class LoongArchELFObjectWriter : public MCELFObjectTargetWriter {
public:
  LoongArchELFObjectWriter(uint8_t OSABI, bool Is64Bit);

  ~LoongArchELFObjectWriter() override;

  bool needsRelocateWithSymbol(const MCValue &, unsigned Type) const override {
    return true;
  }

protected:
  unsigned getRelocType(const MCFixup &, const MCValue &,
                        bool IsPCRel) const override;
};
} // end namespace

LoongArchELFObjectWriter::LoongArchELFObjectWriter(uint8_t OSABI, bool Is64Bit)
    : MCELFObjectTargetWriter(Is64Bit, OSABI, ELF::EM_LOONGARCH,
                              /*HasRelocationAddend=*/true) {}

LoongArchELFObjectWriter::~LoongArchELFObjectWriter() = default;

unsigned LoongArchELFObjectWriter::getRelocType(const MCFixup &Fixup,
                                                const MCValue &Target,
                                                bool IsPCRel) const {
  switch (Target.getSpecifier()) {
  case ELF::R_LARCH_TLS_LE_HI20:
  case ELF::R_LARCH_TLS_IE_PC_HI20:
  case ELF::R_LARCH_TLS_IE_HI20:
  case ELF::R_LARCH_TLS_LD_PC_HI20:
  case ELF::R_LARCH_TLS_LD_HI20:
  case ELF::R_LARCH_TLS_GD_PC_HI20:
  case ELF::R_LARCH_TLS_GD_HI20:
  case ELF::R_LARCH_TLS_DESC_PC_HI20:
  case ELF::R_LARCH_TLS_DESC_HI20:
  case ELF::R_LARCH_TLS_LE_HI20_R:
  case ELF::R_LARCH_TLS_LD_PCREL20_S2:
  case ELF::R_LARCH_TLS_GD_PCREL20_S2:
  case ELF::R_LARCH_TLS_DESC_PCREL20_S2:
    if (auto *SA = const_cast<MCSymbol *>(Target.getAddSym()))
      static_cast<MCSymbolELF *>(SA)->setType(ELF::STT_TLS);
    break;
  default:
    break;
  }

  auto Kind = Fixup.getKind();
  if (mc::isRelocation(Fixup.getKind()))
    return Kind;
  switch (Kind) {
  default:
    reportError(Fixup.getLoc(), "Unsupported relocation type");
    return ELF::R_LARCH_NONE;
  case FK_Data_1:
    reportError(Fixup.getLoc(), "1-byte data relocations not supported");
    return ELF::R_LARCH_NONE;
  case FK_Data_2:
    reportError(Fixup.getLoc(), "2-byte data relocations not supported");
    return ELF::R_LARCH_NONE;
  case FK_Data_4:
    return IsPCRel ? ELF::R_LARCH_32_PCREL : ELF::R_LARCH_32;
  case FK_Data_8:
    return IsPCRel ? ELF::R_LARCH_64_PCREL : ELF::R_LARCH_64;
  case LoongArch::fixup_loongarch_b16:
    return ELF::R_LARCH_B16;
  case LoongArch::fixup_loongarch_b21:
    return ELF::R_LARCH_B21;
  case LoongArch::fixup_loongarch_b26:
    return ELF::R_LARCH_B26;
  case LoongArch::fixup_loongarch_abs_hi20:
    return ELF::R_LARCH_ABS_HI20;
  case LoongArch::fixup_loongarch_abs_lo12:
    return ELF::R_LARCH_ABS_LO12;
  case LoongArch::fixup_loongarch_abs64_lo20:
    return ELF::R_LARCH_ABS64_LO20;
  case LoongArch::fixup_loongarch_abs64_hi12:
    return ELF::R_LARCH_ABS64_HI12;
  }
}

std::unique_ptr<MCObjectTargetWriter>
toolchain::createLoongArchELFObjectWriter(uint8_t OSABI, bool Is64Bit) {
  return std::make_unique<LoongArchELFObjectWriter>(OSABI, Is64Bit);
}
