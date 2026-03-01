//===-- RISCVMachObjectWriter.cpp - RISC-V Mach Object Writer -------------===//
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

#include "MCTargetDesc/RISCVFixupKinds.h"
#include "MCTargetDesc/RISCVMCTargetDesc.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/ADT/Twine.h"
#include "vm/core/BinaryFormat/MachO.h"
#include "vm/core/MC/MCAsmInfo.h"
#include "vm/core/MC/MCAssembler.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCExpr.h"
#include "vm/core/MC/MCFixup.h"
#include "vm/core/MC/MCMachObjectWriter.h"
#include "vm/core/MC/MCSection.h"
#include "vm/core/MC/MCSectionMachO.h"
#include "vm/core/MC/MCSymbol.h"
#include "vm/core/MC/MCValue.h"
#include "vm/core/Support/Casting.h"
#include "vm/core/Support/MathExtras.h"
#include <cassert>
#include <cstdint>

using namespace vm::core;

namespace {

class RISCVMachObjectWriter : public MCMachObjectTargetWriter {
public:
  RISCVMachObjectWriter(uint32_t CPUType, uint32_t CPUSubtype)
      : MCMachObjectTargetWriter(false, CPUType, CPUSubtype) {}

  void recordRelocation(MachObjectWriter *Writer, MCAssembler &Asm,
                        const MCFragment *Fragment, const MCFixup &Fixup,
                        MCValue Target, uint64_t &FixedValue) override;
};

} // end anonymous namespace

void RISCVMachObjectWriter::recordRelocation(
    MachObjectWriter *Writer, MCAssembler &Asm, const MCFragment *Fragment,
    const MCFixup &Fixup, MCValue Target, uint64_t &FixedValue) {
  llvm_unreachable("unimplemented");
}

std::unique_ptr<MCObjectTargetWriter>
toolchain::createRISCVMachObjectWriter(uint32_t CPUType, uint32_t CPUSubtype) {
  return std::make_unique<RISCVMachObjectWriter>(CPUType, CPUSubtype);
}
