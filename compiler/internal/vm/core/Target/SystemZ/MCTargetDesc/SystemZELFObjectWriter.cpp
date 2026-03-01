//===-- SystemZELFObjectWriter.cpp - SystemZ ELF writer -------------------===//
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

#include "MCTargetDesc/SystemZMCAsmInfo.h"
#include "MCTargetDesc/SystemZMCFixups.h"
#include "MCTargetDesc/SystemZMCTargetDesc.h"
#include "vm/core/BinaryFormat/ELF.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCELFObjectWriter.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCFixup.h"
#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/MC/MCValue.h"
#include "vm/core/Support/ErrorHandling.h"
#include <cassert>
#include <cstdint>
#include <memory>

using namespace vm::core;

namespace {

class SystemZELFObjectWriter : public MCELFObjectTargetWriter {
public:
  SystemZELFObjectWriter(uint8_t OSABI);
  ~SystemZELFObjectWriter() override = default;

protected:
  // Override MCELFObjectTargetWriter.
  unsigned getRelocType(const MCFixup &, const MCValue &,
                        bool IsPCRel) const override;
  bool needsRelocateWithSymbol(const MCValue &, unsigned Type) const override;
  unsigned getAbsoluteReloc(SMLoc Loc, unsigned Kind) const;
  unsigned getPCRelReloc(SMLoc Loc, unsigned Kind) const;
};

} // end anonymous namespace

SystemZELFObjectWriter::SystemZELFObjectWriter(uint8_t OSABI)
    : MCELFObjectTargetWriter(/*Is64Bit_=*/true, OSABI, ELF::EM_S390,
                              /*HasRelocationAddend_=*/true) {}

// Return the relocation type for an absolute value of MCFixupKind Kind.
unsigned SystemZELFObjectWriter::getAbsoluteReloc(SMLoc Loc,
                                                  unsigned Kind) const {
  switch (Kind) {
  case FK_Data_1:
  case SystemZ::FK_390_U8Imm:
  case SystemZ::FK_390_S8Imm:
    return ELF::R_390_8;
  case SystemZ::FK_390_U12Imm:
    return ELF::R_390_12;
  case FK_Data_2:
  case SystemZ::FK_390_U16Imm:
  case SystemZ::FK_390_S16Imm:
    return ELF::R_390_16;
  case SystemZ::FK_390_S20Imm:
    return ELF::R_390_20;
  case FK_Data_4:
  case SystemZ::FK_390_U32Imm:
  case SystemZ::FK_390_S32Imm:
    return ELF::R_390_32;
  case FK_Data_8:
    return ELF::R_390_64;
  }
  reportError(Loc, "Unsupported absolute address");
  return 0;
}

// Return the relocation type for a PC-relative value of MCFixupKind Kind.
unsigned SystemZELFObjectWriter::getPCRelReloc(SMLoc Loc, unsigned Kind) const {
  switch (Kind) {
  case FK_Data_2:
  case SystemZ::FK_390_U16Imm:
  case SystemZ::FK_390_S16Imm:
    return ELF::R_390_PC16;
  case FK_Data_4:
  case SystemZ::FK_390_U32Imm:
  case SystemZ::FK_390_S32Imm:
    return ELF::R_390_PC32;
  case FK_Data_8:
    return ELF::R_390_PC64;
  case SystemZ::FK_390_PC12DBL:
    return ELF::R_390_PC12DBL;
  case SystemZ::FK_390_PC16DBL:
    return ELF::R_390_PC16DBL;
  case SystemZ::FK_390_PC24DBL:
    return ELF::R_390_PC24DBL;
  case SystemZ::FK_390_PC32DBL:
    return ELF::R_390_PC32DBL;
  }
  reportError(Loc, "Unsupported PC-relative address");
  return 0;
}

unsigned SystemZELFObjectWriter::getRelocType(const MCFixup &Fixup,
                                              const MCValue &Target,
                                              bool IsPCRel) const {
  SMLoc Loc = Fixup.getLoc();
  unsigned Kind = Fixup.getKind();
  auto Specifier = SystemZ::Specifier(Target.getSpecifier());
  switch (Specifier) {
  case SystemZ::S_INDNTPOFF:
  case SystemZ::S_NTPOFF:
  case SystemZ::S_TLSGD:
  case SystemZ::S_TLSLD:
  case SystemZ::S_TLSLDM:
  case SystemZ::S_DTPOFF:
    if (auto *SA = const_cast<MCSymbol *>(Target.getAddSym()))
      static_cast<MCSymbolELF *>(SA)->setType(ELF::STT_TLS);
    break;
  default:
    break;
  }

  switch (Specifier) {
  case SystemZ::S_None:
    if (IsPCRel)
      return getPCRelReloc(Loc, Kind);
    return getAbsoluteReloc(Loc, Kind);

  case SystemZ::S_NTPOFF:
    assert(!IsPCRel && "NTPOFF shouldn't be PC-relative");
    switch (Kind) {
    case FK_Data_4:
      return ELF::R_390_TLS_LE32;
    case FK_Data_8:
      return ELF::R_390_TLS_LE64;
    }
    reportError(Loc, "Unsupported thread-local address (local-exec)");
    return 0;

  case SystemZ::S_INDNTPOFF:
    if (IsPCRel && Kind == SystemZ::FK_390_PC32DBL)
      return ELF::R_390_TLS_IEENT;
    reportError(Loc,
                "Only PC-relative INDNTPOFF accesses are supported for now");
    return 0;

  case SystemZ::S_DTPOFF:
    assert(!IsPCRel && "DTPOFF shouldn't be PC-relative");
    switch (Kind) {
    case FK_Data_4:
      return ELF::R_390_TLS_LDO32;
    case FK_Data_8:
      return ELF::R_390_TLS_LDO64;
    }
    reportError(Loc, "Unsupported thread-local address (local-dynamic)");
    return 0;

  case SystemZ::S_TLSLDM:
    assert(!IsPCRel && "TLSLDM shouldn't be PC-relative");
    switch (Kind) {
    case FK_Data_4:
      return ELF::R_390_TLS_LDM32;
    case FK_Data_8:
      return ELF::R_390_TLS_LDM64;
    case SystemZ::FK_390_TLS_CALL:
      return ELF::R_390_TLS_LDCALL;
    }
    reportError(Loc, "Unsupported thread-local address (local-dynamic)");
    return 0;

  case SystemZ::S_TLSGD:
    assert(!IsPCRel && "TLSGD shouldn't be PC-relative");
    switch (Kind) {
    case FK_Data_4:
      return ELF::R_390_TLS_GD32;
    case FK_Data_8:
      return ELF::R_390_TLS_GD64;
    case SystemZ::FK_390_TLS_CALL:
      return ELF::R_390_TLS_GDCALL;
    }
    reportError(Loc, "Unsupported thread-local address (general-dynamic)");
    return 0;

  case SystemZ::S_GOT:
  case SystemZ::S_GOTENT:
    if (IsPCRel && Kind == SystemZ::FK_390_PC32DBL)
      return ELF::R_390_GOTENT;
    reportError(Loc, "Only PC-relative GOT accesses are supported for now");
    return 0;

  case SystemZ::S_PLT:
    assert(IsPCRel && "@PLT shouldn't be PC-relative");
    switch (Kind) {
    case SystemZ::FK_390_PC12DBL:
      return ELF::R_390_PLT12DBL;
    case SystemZ::FK_390_PC16DBL:
      return ELF::R_390_PLT16DBL;
    case SystemZ::FK_390_PC24DBL:
      return ELF::R_390_PLT24DBL;
    case SystemZ::FK_390_PC32DBL:
      return ELF::R_390_PLT32DBL;
    }
    reportError(Loc, "Unsupported PC-relative PLT address");
    return 0;

  default:
    llvm_unreachable("Modifier not supported");
  }
}

bool SystemZELFObjectWriter::needsRelocateWithSymbol(const MCValue &V,
                                                     unsigned Type) const {
  switch (V.getSpecifier()) {
  case SystemZ::S_GOT:
  case SystemZ::S_PLT:
    return true;
  default:
    return false;
  }
}

std::unique_ptr<MCObjectTargetWriter>
toolchain::createSystemZELFObjectWriter(uint8_t OSABI) {
  return std::make_unique<SystemZELFObjectWriter>(OSABI);
}
