//===-- AVRELFObjectWriter.cpp - AVR ELF Writer ---------------------------===//
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

#include "MCTargetDesc/AVRFixupKinds.h"
#include "MCTargetDesc/AVRMCAsmInfo.h"
#include "MCTargetDesc/AVRMCTargetDesc.h"

#include "vm/core/MC/MCAssembler.h"
#include "vm/core/MC/MCELFObjectWriter.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/MC/MCValue.h"
#include "vm/core/Support/ErrorHandling.h"

namespace vm::core {

/// Writes AVR machine code into an ELF32 object file.
class AVRELFObjectWriter : public MCELFObjectTargetWriter {
public:
  AVRELFObjectWriter(uint8_t OSABI);

  ~AVRELFObjectWriter() override = default;

  unsigned getRelocType(const MCFixup &, const MCValue &,
                        bool IsPCRel) const override;
};

AVRELFObjectWriter::AVRELFObjectWriter(uint8_t OSABI)
    : MCELFObjectTargetWriter(false, OSABI, ELF::EM_AVR, true) {}

unsigned AVRELFObjectWriter::getRelocType(const MCFixup &Fixup,
                                          const MCValue &Target,
                                          bool IsPCRel) const {
  auto Spec = Target.getSpecifier();
  switch ((unsigned)Fixup.getKind()) {
  case FK_Data_1:
    switch (Spec) {
    default:
      llvm_unreachable("Unsupported Modifier");
    case AVR::S_None:
      return ELF::R_AVR_8;
    case AVR::S_DIFF8:
      return ELF::R_AVR_DIFF8;
    case AVR::S_LO8:
      return ELF::R_AVR_8_LO8;
    case AVR::S_HI8:
      return ELF::R_AVR_8_HI8;
    case AVR::S_HH8:
      return ELF::R_AVR_8_HLO8;
    }
  case FK_Data_4:
    switch (Spec) {
    default:
      llvm_unreachable("Unsupported Modifier");
    case AVR::S_None:
      return ELF::R_AVR_32;
    case AVR::S_DIFF32:
      return ELF::R_AVR_DIFF32;
    }
  case FK_Data_2:
    switch (Spec) {
    default:
      llvm_unreachable("Unsupported Modifier");
    case AVR::S_None:
      return ELF::R_AVR_16;
    case AVR::S_AVR_NONE:
    case AVR::S_PM:
      return ELF::R_AVR_16_PM;
    case AVR::S_DIFF16:
      return ELF::R_AVR_DIFF16;
    }
  case AVR::fixup_32:
    return ELF::R_AVR_32;
  case AVR::fixup_7_pcrel:
    return ELF::R_AVR_7_PCREL;
  case AVR::fixup_13_pcrel:
    return ELF::R_AVR_13_PCREL;
  case AVR::fixup_16:
    return ELF::R_AVR_16;
  case AVR::fixup_16_pm:
    return ELF::R_AVR_16_PM;
  case AVR::fixup_lo8_ldi:
    return ELF::R_AVR_LO8_LDI;
  case AVR::fixup_hi8_ldi:
    return ELF::R_AVR_HI8_LDI;
  case AVR::fixup_hh8_ldi:
    return ELF::R_AVR_HH8_LDI;
  case AVR::fixup_lo8_ldi_neg:
    return ELF::R_AVR_LO8_LDI_NEG;
  case AVR::fixup_hi8_ldi_neg:
    return ELF::R_AVR_HI8_LDI_NEG;
  case AVR::fixup_hh8_ldi_neg:
    return ELF::R_AVR_HH8_LDI_NEG;
  case AVR::fixup_lo8_ldi_pm:
    return ELF::R_AVR_LO8_LDI_PM;
  case AVR::fixup_hi8_ldi_pm:
    return ELF::R_AVR_HI8_LDI_PM;
  case AVR::fixup_hh8_ldi_pm:
    return ELF::R_AVR_HH8_LDI_PM;
  case AVR::fixup_lo8_ldi_pm_neg:
    return ELF::R_AVR_LO8_LDI_PM_NEG;
  case AVR::fixup_hi8_ldi_pm_neg:
    return ELF::R_AVR_HI8_LDI_PM_NEG;
  case AVR::fixup_hh8_ldi_pm_neg:
    return ELF::R_AVR_HH8_LDI_PM_NEG;
  case AVR::fixup_call:
    return ELF::R_AVR_CALL;
  case AVR::fixup_ldi:
    return ELF::R_AVR_LDI;
  case AVR::fixup_6:
    return ELF::R_AVR_6;
  case AVR::fixup_6_adiw:
    return ELF::R_AVR_6_ADIW;
  case AVR::fixup_ms8_ldi:
    return ELF::R_AVR_MS8_LDI;
  case AVR::fixup_ms8_ldi_neg:
    return ELF::R_AVR_MS8_LDI_NEG;
  case AVR::fixup_lo8_ldi_gs:
    return ELF::R_AVR_LO8_LDI_GS;
  case AVR::fixup_hi8_ldi_gs:
    return ELF::R_AVR_HI8_LDI_GS;
  case AVR::fixup_8:
    return ELF::R_AVR_8;
  case AVR::fixup_8_lo8:
    return ELF::R_AVR_8_LO8;
  case AVR::fixup_8_hi8:
    return ELF::R_AVR_8_HI8;
  case AVR::fixup_8_hlo8:
    return ELF::R_AVR_8_HLO8;
  case AVR::fixup_diff8:
    return ELF::R_AVR_DIFF8;
  case AVR::fixup_diff16:
    return ELF::R_AVR_DIFF16;
  case AVR::fixup_diff32:
    return ELF::R_AVR_DIFF32;
  case AVR::fixup_lds_sts_16:
    return ELF::R_AVR_LDS_STS_16;
  case AVR::fixup_port6:
    return ELF::R_AVR_PORT6;
  case AVR::fixup_port5:
    return ELF::R_AVR_PORT5;
  default:
    llvm_unreachable("invalid fixup kind!");
  }
}

std::unique_ptr<MCObjectTargetWriter> createAVRELFObjectWriter(uint8_t OSABI) {
  return std::make_unique<AVRELFObjectWriter>(OSABI);
}

} // end of namespace vm::core
