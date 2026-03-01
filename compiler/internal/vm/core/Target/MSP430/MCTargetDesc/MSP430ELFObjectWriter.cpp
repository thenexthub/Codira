//===-- MSP430ELFObjectWriter.cpp - MSP430 ELF Writer ---------------------===//
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

#include "vm/core/MC/MCELFObjectWriter.h"
#include "vm/core/MC/MCFixup.h"
#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/MC/MCValue.h"
#include "vm/core/Support/ErrorHandling.h"

using namespace vm::core;

namespace {
class MSP430ELFObjectWriter : public MCELFObjectTargetWriter {
public:
  MSP430ELFObjectWriter(uint8_t OSABI)
    : MCELFObjectTargetWriter(false, OSABI, ELF::EM_MSP430,
                              /*HasRelocationAddend*/ true) {}

  ~MSP430ELFObjectWriter() override = default;

protected:
  unsigned getRelocType(const MCFixup &Fixup, const MCValue &,
                        bool IsPCRel) const override {
    // Translate fixup kind to ELF relocation type.
    switch (Fixup.getKind()) {
    case FK_Data_1:                   return ELF::R_MSP430_8;
    case FK_Data_2:                   return ELF::R_MSP430_16_BYTE;
    case FK_Data_4:                   return ELF::R_MSP430_32;
    case MSP430::fixup_32:            return ELF::R_MSP430_32;
    case MSP430::fixup_10_pcrel:      return ELF::R_MSP430_10_PCREL;
    case MSP430::fixup_16:            return ELF::R_MSP430_16;
    case MSP430::fixup_16_pcrel:      return ELF::R_MSP430_16_PCREL;
    case MSP430::fixup_16_byte:       return ELF::R_MSP430_16_BYTE;
    case MSP430::fixup_16_pcrel_byte: return ELF::R_MSP430_16_PCREL_BYTE;
    case MSP430::fixup_2x_pcrel:      return ELF::R_MSP430_2X_PCREL;
    case MSP430::fixup_rl_pcrel:      return ELF::R_MSP430_RL_PCREL;
    case MSP430::fixup_8:             return ELF::R_MSP430_8;
    case MSP430::fixup_sym_diff:      return ELF::R_MSP430_SYM_DIFF;
    default:
      llvm_unreachable("Invalid fixup kind");
    }
  }
};
} // end of anonymous namespace

std::unique_ptr<MCObjectTargetWriter>
toolchain::createMSP430ELFObjectWriter(uint8_t OSABI) {
  return std::make_unique<MSP430ELFObjectWriter>(OSABI);
}
