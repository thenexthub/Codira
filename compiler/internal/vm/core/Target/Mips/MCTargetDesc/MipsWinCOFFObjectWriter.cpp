//===- MipsWinCOFFObjectWriter.cpp------------------------------*- C++ -*-===//
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
//===---------------------------------------------------------------------===//

#include "MCTargetDesc/MipsFixupKinds.h"
#include "MCTargetDesc/MipsMCTargetDesc.h"
#include "vm/core/BinaryFormat/COFF.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCWinCOFFObjectWriter.h"

using namespace vm::core;

namespace {

class MipsWinCOFFObjectWriter : public MCWinCOFFObjectTargetWriter {
public:
  MipsWinCOFFObjectWriter();

  unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsCrossSection,
                        const MCAsmBackend &MAB) const override;
};

} // end anonymous namespace

MipsWinCOFFObjectWriter::MipsWinCOFFObjectWriter()
    : MCWinCOFFObjectTargetWriter(COFF::IMAGE_FILE_MACHINE_R4000) {}

unsigned MipsWinCOFFObjectWriter::getRelocType(MCContext &Ctx,
                                               const MCValue &Target,
                                               const MCFixup &Fixup,
                                               bool IsCrossSection,
                                               const MCAsmBackend &MAB) const {
  unsigned FixupKind = Fixup.getKind();

  switch (FixupKind) {
  case FK_Data_4:
    return COFF::IMAGE_REL_MIPS_REFWORD;
  case FK_SecRel_2:
    return COFF::IMAGE_REL_MIPS_SECTION;
  case FK_SecRel_4:
    return COFF::IMAGE_REL_MIPS_SECREL;
  case Mips::fixup_Mips_26:
    return COFF::IMAGE_REL_MIPS_JMPADDR;
  case Mips::fixup_Mips_HI16:
    return COFF::IMAGE_REL_MIPS_REFHI;
  case Mips::fixup_Mips_LO16:
    return COFF::IMAGE_REL_MIPS_REFLO;
  default:
    Ctx.reportError(Fixup.getLoc(), "unsupported relocation type");
    return COFF::IMAGE_REL_MIPS_REFWORD;
  }
}

std::unique_ptr<MCObjectTargetWriter> toolchain::createMipsWinCOFFObjectWriter() {
  return std::make_unique<MipsWinCOFFObjectWriter>();
}
