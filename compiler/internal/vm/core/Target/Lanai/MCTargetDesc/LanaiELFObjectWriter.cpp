//===-- LanaiELFObjectWriter.cpp - Lanai ELF Writer -----------------------===//
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

#include "MCTargetDesc/LanaiBaseInfo.h"
#include "MCTargetDesc/LanaiFixupKinds.h"
#include "vm/core/BinaryFormat/ELF.h"
#include "vm/core/MC/MCELFObjectWriter.h"
#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/Support/ErrorHandling.h"

using namespace vm::core;

namespace {

class LanaiELFObjectWriter : public MCELFObjectTargetWriter {
public:
  explicit LanaiELFObjectWriter(uint8_t OSABI);

  ~LanaiELFObjectWriter() override = default;

protected:
  unsigned getRelocType(const MCFixup &, const MCValue &,
                        bool IsPCRel) const override;
  bool needsRelocateWithSymbol(const MCValue &, unsigned Type) const override;
};

} // end anonymous namespace

LanaiELFObjectWriter::LanaiELFObjectWriter(uint8_t OSABI)
    : MCELFObjectTargetWriter(/*Is64Bit_=*/false, OSABI, ELF::EM_LANAI,
                              /*HasRelocationAddend_=*/true) {}

unsigned LanaiELFObjectWriter::getRelocType(const MCFixup &Fixup,
                                            const MCValue &, bool) const {
  unsigned Type;
  unsigned Kind = static_cast<unsigned>(Fixup.getKind());
  switch (Kind) {
  case Lanai::FIXUP_LANAI_21:
    Type = ELF::R_LANAI_21;
    break;
  case Lanai::FIXUP_LANAI_21_F:
    Type = ELF::R_LANAI_21_F;
    break;
  case Lanai::FIXUP_LANAI_25:
    Type = ELF::R_LANAI_25;
    break;
  case Lanai::FIXUP_LANAI_32:
  case FK_Data_4:
    Type = ELF::R_LANAI_32;
    break;
  case Lanai::FIXUP_LANAI_HI16:
    Type = ELF::R_LANAI_HI16;
    break;
  case Lanai::FIXUP_LANAI_LO16:
    Type = ELF::R_LANAI_LO16;
    break;
  case Lanai::FIXUP_LANAI_NONE:
    Type = ELF::R_LANAI_NONE;
    break;

  default:
    llvm_unreachable("Invalid fixup kind!");
  }
  return Type;
}

bool LanaiELFObjectWriter::needsRelocateWithSymbol(const MCValue &,
                                                   unsigned Type) const {
  switch (Type) {
  case ELF::R_LANAI_21:
  case ELF::R_LANAI_21_F:
  case ELF::R_LANAI_25:
  case ELF::R_LANAI_32:
  case ELF::R_LANAI_HI16:
    return true;
  default:
    return false;
  }
}

std::unique_ptr<MCObjectTargetWriter>
toolchain::createLanaiELFObjectWriter(uint8_t OSABI) {
  return std::make_unique<LanaiELFObjectWriter>(OSABI);
}
