//===-- X86WinCOFFObjectWriter.cpp - X86 Win COFF Writer ------------------===//
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

#include "MCTargetDesc/X86FixupKinds.h"
#include "MCTargetDesc/X86MCAsmInfo.h"
#include "MCTargetDesc/X86MCTargetDesc.h"
#include "vm/core/BinaryFormat/COFF.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCFixup.h"
#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/MC/MCValue.h"
#include "vm/core/MC/MCWinCOFFObjectWriter.h"
#include "vm/core/Support/ErrorHandling.h"

using namespace vm::core;

namespace {

class X86WinCOFFObjectWriter : public MCWinCOFFObjectTargetWriter {
public:
  X86WinCOFFObjectWriter(bool Is64Bit);
  ~X86WinCOFFObjectWriter() override = default;

  unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsCrossSection,
                        const MCAsmBackend &MAB) const override;
};

} // end anonymous namespace

X86WinCOFFObjectWriter::X86WinCOFFObjectWriter(bool Is64Bit)
    : MCWinCOFFObjectTargetWriter(Is64Bit ? COFF::IMAGE_FILE_MACHINE_AMD64
                                          : COFF::IMAGE_FILE_MACHINE_I386) {}

unsigned X86WinCOFFObjectWriter::getRelocType(MCContext &Ctx,
                                              const MCValue &Target,
                                              const MCFixup &Fixup,
                                              bool IsCrossSection,
                                              const MCAsmBackend &MAB) const {
  const bool Is64Bit = getMachine() == COFF::IMAGE_FILE_MACHINE_AMD64;
  unsigned FixupKind = Fixup.getKind();
  bool PCRel = Fixup.isPCRel();
  if (IsCrossSection) {
    // IMAGE_REL_AMD64_REL64 does not exist. We treat FK_Data_8 as FK_PCRel_4 so
    // that .quad a-b can lower to IMAGE_REL_AMD64_REL32. This allows generic
    // instrumentation to not bother with the COFF limitation. A negative value
    // needs attention.
    if (!PCRel &&
        (FixupKind == FK_Data_4 || FixupKind == toolchain::X86::reloc_signed_4byte ||
         (FixupKind == FK_Data_8 && Is64Bit))) {
      FixupKind = FK_Data_4;
      PCRel = true;
    } else {
      Ctx.reportError(Fixup.getLoc(), "Cannot represent this expression");
      return COFF::IMAGE_REL_AMD64_ADDR32;
    }
  }

  auto Spec = Target.getSpecifier();
  if (Is64Bit) {
    switch (FixupKind) {
    case X86::reloc_riprel_4byte:
    case X86::reloc_riprel_4byte_movq_load:
    case X86::reloc_riprel_4byte_movq_load_rex2:
    case X86::reloc_riprel_4byte_relax:
    case X86::reloc_riprel_4byte_relax_rex:
    case X86::reloc_riprel_4byte_relax_rex2:
    case X86::reloc_riprel_4byte_relax_evex:
    case X86::reloc_branch_4byte_pcrel:
      return COFF::IMAGE_REL_AMD64_REL32;
    case FK_Data_4:
      if (PCRel)
        return COFF::IMAGE_REL_AMD64_REL32;
      [[fallthrough]];
    case X86::reloc_signed_4byte:
    case X86::reloc_signed_4byte_relax:
      if (Spec == MCSymbolRefExpr::VK_COFF_IMGREL32)
        return COFF::IMAGE_REL_AMD64_ADDR32NB;
      if (Spec == X86::S_COFF_SECREL)
        return COFF::IMAGE_REL_AMD64_SECREL;
      return COFF::IMAGE_REL_AMD64_ADDR32;
    case FK_Data_8:
      return COFF::IMAGE_REL_AMD64_ADDR64;
    case FK_SecRel_2:
      return COFF::IMAGE_REL_AMD64_SECTION;
    case FK_SecRel_4:
      return COFF::IMAGE_REL_AMD64_SECREL;
    default:
      Ctx.reportError(Fixup.getLoc(), "unsupported relocation type");
      return COFF::IMAGE_REL_AMD64_ADDR32;
    }
  } else if (getMachine() == COFF::IMAGE_FILE_MACHINE_I386) {
    switch (FixupKind) {
    case X86::reloc_riprel_4byte:
    case X86::reloc_riprel_4byte_movq_load:
      return COFF::IMAGE_REL_I386_REL32;
    case FK_Data_4:
      if (PCRel)
        return COFF::IMAGE_REL_I386_REL32;
      [[fallthrough]];
    case X86::reloc_signed_4byte:
    case X86::reloc_signed_4byte_relax:
      if (Spec == MCSymbolRefExpr::VK_COFF_IMGREL32)
        return COFF::IMAGE_REL_I386_DIR32NB;
      if (Spec == X86::S_COFF_SECREL)
        return COFF::IMAGE_REL_I386_SECREL;
      return COFF::IMAGE_REL_I386_DIR32;
    case FK_SecRel_2:
      return COFF::IMAGE_REL_I386_SECTION;
    case FK_SecRel_4:
      return COFF::IMAGE_REL_I386_SECREL;
    default:
      Ctx.reportError(Fixup.getLoc(), "unsupported relocation type");
      return COFF::IMAGE_REL_I386_DIR32;
    }
  } else
    llvm_unreachable("Unsupported COFF machine type.");
}

std::unique_ptr<MCObjectTargetWriter>
toolchain::createX86WinCOFFObjectWriter(bool Is64Bit) {
  return std::make_unique<X86WinCOFFObjectWriter>(Is64Bit);
}
