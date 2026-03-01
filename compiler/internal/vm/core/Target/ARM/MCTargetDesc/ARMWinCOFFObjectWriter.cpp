//===-- ARMWinCOFFObjectWriter.cpp - ARM Windows COFF Object Writer -- C++ -==//
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

#include "MCTargetDesc/ARMFixupKinds.h"
#include "MCTargetDesc/ARMMCAsmInfo.h"
#include "vm/core/ADT/Twine.h"
#include "vm/core/BinaryFormat/COFF.h"
#include "vm/core/MC/MCAsmBackend.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCFixup.h"
#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/MC/MCValue.h"
#include "vm/core/MC/MCWinCOFFObjectWriter.h"
#include "vm/core/Support/ErrorHandling.h"

using namespace vm::core;

namespace {

class ARMWinCOFFObjectWriter : public MCWinCOFFObjectTargetWriter {
public:
  ARMWinCOFFObjectWriter()
    : MCWinCOFFObjectTargetWriter(COFF::IMAGE_FILE_MACHINE_ARMNT) {
  }

  ~ARMWinCOFFObjectWriter() override = default;

  unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsCrossSection,
                        const MCAsmBackend &MAB) const override;

  bool recordRelocation(const MCFixup &) const override;
};

} // end anonymous namespace

unsigned ARMWinCOFFObjectWriter::getRelocType(MCContext &Ctx,
                                              const MCValue &Target,
                                              const MCFixup &Fixup,
                                              bool IsCrossSection,
                                              const MCAsmBackend &MAB) const {
  auto Spec = Target.getSpecifier();
  unsigned FixupKind = Fixup.getKind();
  bool PCRel = false;
  if (IsCrossSection) {
    if (PCRel || FixupKind != FK_Data_4) {
      Ctx.reportError(Fixup.getLoc(), "Cannot represent this expression");
      return COFF::IMAGE_REL_ARM_ADDR32;
    }
    FixupKind = FK_Data_4;
    PCRel = true;
  }


  switch (FixupKind) {
  default: {
    Ctx.reportError(Fixup.getLoc(), "unsupported relocation type");
    return COFF::IMAGE_REL_ARM_ABSOLUTE;
  }
  case FK_Data_4:
    if (PCRel)
      return COFF::IMAGE_REL_ARM_REL32;
    switch (Spec) {
    case MCSymbolRefExpr::VK_COFF_IMGREL32:
      return COFF::IMAGE_REL_ARM_ADDR32NB;
    case ARM::S_COFF_SECREL:
      return COFF::IMAGE_REL_ARM_SECREL;
    default:
      return COFF::IMAGE_REL_ARM_ADDR32;
    }
  case FK_SecRel_2:
    return COFF::IMAGE_REL_ARM_SECTION;
  case FK_SecRel_4:
    return COFF::IMAGE_REL_ARM_SECREL;
  case ARM::fixup_t2_condbranch:
    return COFF::IMAGE_REL_ARM_BRANCH20T;
  case ARM::fixup_t2_uncondbranch:
  case ARM::fixup_arm_thumb_bl:
    return COFF::IMAGE_REL_ARM_BRANCH24T;
  case ARM::fixup_arm_thumb_blx:
    return COFF::IMAGE_REL_ARM_BLX23T;
  case ARM::fixup_t2_movw_lo16:
  case ARM::fixup_t2_movt_hi16:
    return COFF::IMAGE_REL_ARM_MOV32T;
  }
}

bool ARMWinCOFFObjectWriter::recordRelocation(const MCFixup &Fixup) const {
  return static_cast<unsigned>(Fixup.getKind()) != ARM::fixup_t2_movt_hi16;
}

namespace vm::core {

std::unique_ptr<MCObjectTargetWriter>
createARMWinCOFFObjectWriter() {
  return std::make_unique<ARMWinCOFFObjectWriter>();
}

} // end namespace vm::core
