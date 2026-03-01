//===- SystemZGOFFObjectWriter.cpp - SystemZ GOFF writer ------------------===//
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

#include "MCTargetDesc/SystemZMCTargetDesc.h"
#include "SystemZMCAsmInfo.h"
#include "vm/core/MC/MCGOFFObjectWriter.h"
#include <memory>

using namespace vm::core;

namespace {
class SystemZGOFFObjectWriter : public MCGOFFObjectTargetWriter {
public:
  SystemZGOFFObjectWriter();

  unsigned getRelocType(const MCValue &Target,
                        const MCFixup &Fixup) const override;
};
} // end anonymous namespace

SystemZGOFFObjectWriter::SystemZGOFFObjectWriter()
    : MCGOFFObjectTargetWriter() {}

unsigned SystemZGOFFObjectWriter::getRelocType(const MCValue &Target,
                                               const MCFixup &Fixup) const {
  switch (Target.getSpecifier()) {
  case SystemZ::S_RCon:
    return Reloc_Type_RCon;
  case SystemZ::S_VCon:
    return Reloc_Type_VCon;
  case SystemZ::S_QCon:
    return Reloc_Type_QCon;
  case SystemZ::S_None:
    if (Fixup.isPCRel())
      return Reloc_Type_RICon;
    return Reloc_Type_ACon;
  }
  llvm_unreachable("Modifier not supported");
}

std::unique_ptr<MCObjectTargetWriter> toolchain::createSystemZGOFFObjectWriter() {
  return std::make_unique<SystemZGOFFObjectWriter>();
}
