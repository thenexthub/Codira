//===-- BPFELFObjectWriter.cpp - BPF ELF Writer ---------------------------===//
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

#include "MCTargetDesc/BPFMCTargetDesc.h"
#include "vm/core/BinaryFormat/ELF.h"
#include "vm/core/MC/MCELFObjectWriter.h"
#include "vm/core/MC/MCFixup.h"
#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/MC/MCValue.h"
#include "vm/core/Support/ErrorHandling.h"
#include <cstdint>

using namespace vm::core;

namespace {

class BPFELFObjectWriter : public MCELFObjectTargetWriter {
public:
  BPFELFObjectWriter(uint8_t OSABI);
  ~BPFELFObjectWriter() override = default;

protected:
  unsigned getRelocType(const MCFixup &, const MCValue &,
                        bool IsPCRel) const override;
};

} // end anonymous namespace

BPFELFObjectWriter::BPFELFObjectWriter(uint8_t OSABI)
    : MCELFObjectTargetWriter(/*Is64Bit*/ true, OSABI, ELF::EM_BPF,
                              /*HasRelocationAddend*/ false) {}

unsigned BPFELFObjectWriter::getRelocType(const MCFixup &Fixup,
                                          const MCValue &Target,
                                          bool IsPCRel) const {
  // determine the type of the relocation
  switch (Fixup.getKind()) {
  default:
    llvm_unreachable("invalid fixup kind!");
  case FK_SecRel_8:
    // LD_imm64 instruction.
    return ELF::R_BPF_64_64;
  case FK_Data_8:
    return ELF::R_BPF_64_ABS64;
  case FK_Data_4:
    if (Fixup.isPCRel()) // CALL instruction
      return ELF::R_BPF_64_32;
    if (const auto *A = Target.getAddSym()) {
      const MCSymbol &Sym = *A;

      if (Sym.isDefined()) {
        auto &Section = static_cast<const MCSectionELF &>(Sym.getSection());
        unsigned Flags = Section.getFlags();

        if (Sym.isTemporary()) {
          // .BTF.ext generates FK_Data_4 relocations for
          // insn offset by creating temporary labels.
          // The reloc symbol should be in text section.
          // Use a different relocation to instruct ExecutionEngine
          // RuntimeDyld not to do relocation for it, yet still to
          // allow lld to do proper adjustment when merging sections.
          if ((Flags & ELF::SHF_ALLOC) && (Flags & ELF::SHF_EXECINSTR))
            return ELF::R_BPF_64_NODYLD32;
        } else {
          // .BTF generates FK_Data_4 relocations for variable
          // offset in DataSec kind.
          // The reloc symbol should be in data section.
          if ((Flags & ELF::SHF_ALLOC) && (Flags & ELF::SHF_WRITE))
            return ELF::R_BPF_64_NODYLD32;
        }
      }
    }
    return ELF::R_BPF_64_ABS32;
  }
}

std::unique_ptr<MCObjectTargetWriter>
toolchain::createBPFELFObjectWriter(uint8_t OSABI) {
  return std::make_unique<BPFELFObjectWriter>(OSABI);
}
