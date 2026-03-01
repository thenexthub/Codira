//===- AMDGPUELFObjectWriter.cpp - AMDGPU ELF Writer ----------------------===//
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

#include "AMDGPUFixupKinds.h"
#include "AMDGPUMCTargetDesc.h"
#include "MCTargetDesc/AMDGPUMCExpr.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCELFObjectWriter.h"
#include "vm/core/MC/MCValue.h"

using namespace vm::core;

namespace {

class AMDGPUELFObjectWriter : public MCELFObjectTargetWriter {
public:
  AMDGPUELFObjectWriter(bool Is64Bit, uint8_t OSABI, bool HasRelocationAddend);

protected:
  unsigned getRelocType(const MCFixup &, const MCValue &,
                        bool IsPCRel) const override;
};


} // end anonymous namespace

AMDGPUELFObjectWriter::AMDGPUELFObjectWriter(bool Is64Bit, uint8_t OSABI,
                                             bool HasRelocationAddend)
    : MCELFObjectTargetWriter(Is64Bit, OSABI, ELF::EM_AMDGPU,
                              HasRelocationAddend) {}

unsigned AMDGPUELFObjectWriter::getRelocType(const MCFixup &Fixup,
                                             const MCValue &Target,
                                             bool IsPCRel) const {
  if (const auto *SymA = Target.getAddSym()) {
    // SCRATCH_RSRC_DWORD[01] is a special global variable that represents
    // the scratch buffer.
    if (SymA->getName() == "SCRATCH_RSRC_DWORD0" ||
        SymA->getName() == "SCRATCH_RSRC_DWORD1")
      return ELF::R_AMDGPU_ABS32_LO;
  }

  switch (AMDGPUMCExpr::Specifier(Target.getSpecifier())) {
  default:
    break;
  case AMDGPUMCExpr::S_GOTPCREL:
    return ELF::R_AMDGPU_GOTPCREL;
  case AMDGPUMCExpr::S_GOTPCREL32_LO:
    return ELF::R_AMDGPU_GOTPCREL32_LO;
  case AMDGPUMCExpr::S_GOTPCREL32_HI:
    return ELF::R_AMDGPU_GOTPCREL32_HI;
  case AMDGPUMCExpr::S_REL32_LO:
    return ELF::R_AMDGPU_REL32_LO;
  case AMDGPUMCExpr::S_REL32_HI:
    return ELF::R_AMDGPU_REL32_HI;
  case AMDGPUMCExpr::S_REL64:
    return ELF::R_AMDGPU_REL64;
  case AMDGPUMCExpr::S_ABS32_LO:
    return ELF::R_AMDGPU_ABS32_LO;
  case AMDGPUMCExpr::S_ABS32_HI:
    return ELF::R_AMDGPU_ABS32_HI;
  case AMDGPUMCExpr::S_ABS64:
    return ELF::R_AMDGPU_ABS64;
  }

  MCFixupKind Kind = Fixup.getKind();
  switch (Kind) {
  default: break;
  case FK_Data_4:
  case FK_SecRel_4:
    return IsPCRel ? ELF::R_AMDGPU_REL32 : ELF::R_AMDGPU_ABS32;
  case FK_Data_8:
    return IsPCRel ? ELF::R_AMDGPU_REL64 : ELF::R_AMDGPU_ABS64;
  }

  if (Fixup.getKind() == AMDGPU::fixup_si_sopp_br) {
    const auto *SymA = Target.getAddSym();
    assert(SymA);

    if (SymA->isUndefined()) {
      reportError(Fixup.getLoc(),
                  Twine("undefined label '") + SymA->getName() + "'");
      return ELF::R_AMDGPU_NONE;
    }
    return ELF::R_AMDGPU_REL16;
  }

  llvm_unreachable("unhandled relocation type");
}

std::unique_ptr<MCObjectTargetWriter>
toolchain::createAMDGPUELFObjectWriter(bool Is64Bit, uint8_t OSABI,
                                  bool HasRelocationAddend) {
  return std::make_unique<AMDGPUELFObjectWriter>(Is64Bit, OSABI,
                                                 HasRelocationAddend);
}
