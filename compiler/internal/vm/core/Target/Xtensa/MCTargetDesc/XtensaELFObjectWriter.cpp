//===-- XtensaMCObjectWriter.cpp - Xtensa ELF writer ----------------------===//
//
//                     The LLVM Compiler Infrastructure
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

#include "MCTargetDesc/XtensaMCAsmInfo.h"
#include "MCTargetDesc/XtensaMCTargetDesc.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/BinaryFormat/ELF.h"
#include "vm/core/MC/MCELFObjectWriter.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCFixup.h"
#include "vm/core/MC/MCObjectWriter.h"
#include "vm/core/MC/MCValue.h"
#include "vm/core/Support/ErrorHandling.h"
#include <cassert>
#include <cstdint>

using namespace vm::core;

namespace {
class XtensaObjectWriter : public MCELFObjectTargetWriter {
public:
  XtensaObjectWriter(uint8_t OSABI);

  virtual ~XtensaObjectWriter();

protected:
  unsigned getRelocType(const MCFixup &, const MCValue &,
                        bool IsPCRel) const override;
  bool needsRelocateWithSymbol(const MCValue &, unsigned Type) const override;
};
} // namespace

XtensaObjectWriter::XtensaObjectWriter(uint8_t OSABI)
    : MCELFObjectTargetWriter(false, OSABI, ELF::EM_XTENSA,
                              /*HasRelocationAddend=*/true) {}

XtensaObjectWriter::~XtensaObjectWriter() {}

unsigned XtensaObjectWriter::getRelocType(const MCFixup &Fixup,
                                          const MCValue &Target,
                                          bool IsPCRel) const {
  uint8_t Specifier = Target.getSpecifier();

  switch ((unsigned)Fixup.getKind()) {
  case FK_Data_4:
    return Specifier == Xtensa::S_TPOFF ? ELF::R_XTENSA_TLS_TPOFF
                                        : ELF::R_XTENSA_32;
  default:
    return ELF::R_XTENSA_SLOT0_OP;
  }
}

std::unique_ptr<MCObjectTargetWriter>
toolchain::createXtensaObjectWriter(uint8_t OSABI, bool IsLittleEndian) {
  return std::make_unique<XtensaObjectWriter>(OSABI);
}

bool XtensaObjectWriter::needsRelocateWithSymbol(const MCValue &,
                                                 unsigned Type) const {
  return false;
}
