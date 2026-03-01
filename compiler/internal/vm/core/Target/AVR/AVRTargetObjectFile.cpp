//===-- AVRTargetObjectFile.cpp - AVR Object Files ------------------------===//
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

#include "AVRTargetObjectFile.h"
#include "AVRTargetMachine.h"

#include "vm/core/BinaryFormat/ELF.h"
#include "vm/core/IR/GlobalValue.h"
#include "vm/core/IR/Mangler.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCSectionELF.h"

#include "AVR.h"

namespace vm::core {
void AVRTargetObjectFile::Initialize(MCContext &Ctx, const TargetMachine &TM) {
  Base::Initialize(Ctx, TM);
  ProgmemDataSection =
      Ctx.getELFSection(".progmem.data", ELF::SHT_PROGBITS, ELF::SHF_ALLOC);
  Progmem1DataSection =
      Ctx.getELFSection(".progmem1.data", ELF::SHT_PROGBITS, ELF::SHF_ALLOC);
  Progmem2DataSection =
      Ctx.getELFSection(".progmem2.data", ELF::SHT_PROGBITS, ELF::SHF_ALLOC);
  Progmem3DataSection =
      Ctx.getELFSection(".progmem3.data", ELF::SHT_PROGBITS, ELF::SHF_ALLOC);
  Progmem4DataSection =
      Ctx.getELFSection(".progmem4.data", ELF::SHT_PROGBITS, ELF::SHF_ALLOC);
  Progmem5DataSection =
      Ctx.getELFSection(".progmem5.data", ELF::SHT_PROGBITS, ELF::SHF_ALLOC);
}

MCSection *AVRTargetObjectFile::SelectSectionForGlobal(
    const GlobalObject *GO, SectionKind Kind, const TargetMachine &TM) const {
  // Global values in flash memory are placed in the progmem*.data section
  // unless they already have a user assigned section.
  const auto &AVRTM = static_cast<const AVRTargetMachine &>(TM);
  if (AVR::isProgramMemoryAddress(GO) && !GO->hasSection() &&
      Kind.isReadOnly()) {
    // The AVR subtarget should support LPM to access section '.progmem*.data'.
    if (!AVRTM.getSubtargetImpl()->hasLPM()) {
      // TODO: Get the global object's location in source file.
      getContext().reportError(
          SMLoc(),
          "Current AVR subtarget does not support accessing program memory");
      return Base::SelectSectionForGlobal(GO, Kind, TM);
    }
    // The AVR subtarget should support ELPM to access section
    // '.progmem[1|2|3|4|5].data'.
    if (!AVRTM.getSubtargetImpl()->hasELPM() &&
        AVR::getAddressSpace(GO) != AVR::ProgramMemory) {
      // TODO: Get the global object's location in source file.
      getContext().reportError(SMLoc(),
                               "Current AVR subtarget does not support "
                               "accessing extended program memory");
      return ProgmemDataSection;
    }
    switch (AVR::getAddressSpace(GO)) {
    case AVR::ProgramMemory: // address space 1
      return ProgmemDataSection;
    case AVR::ProgramMemory1: // address space 2
      return Progmem1DataSection;
    case AVR::ProgramMemory2: // address space 3
      return Progmem2DataSection;
    case AVR::ProgramMemory3: // address space 4
      return Progmem3DataSection;
    case AVR::ProgramMemory4: // address space 5
      return Progmem4DataSection;
    case AVR::ProgramMemory5: // address space 6
      return Progmem5DataSection;
    default:
      llvm_unreachable("unexpected program memory index");
    }
  }

  // Otherwise, we work the same way as ELF.
  return Base::SelectSectionForGlobal(GO, Kind, TM);
}
} // end of namespace vm::core
