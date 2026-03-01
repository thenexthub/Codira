//===- MipsOptionRecord.cpp - Abstraction for storing information ---------===//
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

#include "MipsOptionRecord.h"
#include "MipsABIInfo.h"
#include "MipsELFStreamer.h"
#include "MipsTargetStreamer.h"
#include "vm/core/BinaryFormat/ELF.h"
#include "vm/core/MC/MCAssembler.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCRegisterInfo.h"
#include "vm/core/MC/MCSectionELF.h"
#include <cassert>

using namespace vm::core;

void MipsRegInfoRecord::EmitMipsOptionRecord() {
  MipsTargetStreamer *MTS =
      static_cast<MipsTargetStreamer *>(Streamer->getTargetStreamer());

  Streamer->pushSection();

  // We need to distinguish between N64 and the rest because at the moment
  // we don't emit .Mips.options for other ELFs other than N64.
  // Since .reginfo has the same information as .Mips.options (ODK_REGINFO),
  // we can use the same abstraction (MipsRegInfoRecord class) to handle both.
  if (MTS->getABI().IsN64()) {
    // The EntrySize value of 1 seems strange since the records are neither
    // 1-byte long nor fixed length but it matches the value GAS emits.
    MCSectionELF *Sec =
        Context.getELFSection(".MIPS.options", ELF::SHT_MIPS_OPTIONS,
                              ELF::SHF_ALLOC | ELF::SHF_MIPS_NOSTRIP, 1);
    Sec->setAlignment(Align(8));
    Streamer->switchSection(Sec);

    Streamer->emitInt8(ELF::ODK_REGINFO); // kind
    Streamer->emitInt8(40);               // size
    Streamer->emitInt16(0);               // section
    Streamer->emitInt32(0);               // info
    Streamer->emitInt32(ri_gprmask);
    Streamer->emitInt32(0); // pad
    Streamer->emitInt32(ri_cprmask[0]);
    Streamer->emitInt32(ri_cprmask[1]);
    Streamer->emitInt32(ri_cprmask[2]);
    Streamer->emitInt32(ri_cprmask[3]);
    Streamer->emitIntValue(ri_gp_value, 8);
  } else {
    MCSectionELF *Sec = Context.getELFSection(".reginfo", ELF::SHT_MIPS_REGINFO,
                                              ELF::SHF_ALLOC, 24);
    Sec->setAlignment(MTS->getABI().IsN32() ? Align(8) : Align(4));
    Streamer->switchSection(Sec);

    Streamer->emitInt32(ri_gprmask);
    Streamer->emitInt32(ri_cprmask[0]);
    Streamer->emitInt32(ri_cprmask[1]);
    Streamer->emitInt32(ri_cprmask[2]);
    Streamer->emitInt32(ri_cprmask[3]);
    assert((ri_gp_value & 0xffffffff) == ri_gp_value);
    Streamer->emitInt32(ri_gp_value);
  }

  Streamer->popSection();
}

void MipsRegInfoRecord::SetPhysRegUsed(MCRegister Reg,
                                       const MCRegisterInfo *MCRegInfo) {
  unsigned Value = 0;

  for (const MCPhysReg &SubReg : MCRegInfo->subregs_inclusive(Reg)) {
    unsigned EncVal = MCRegInfo->getEncodingValue(SubReg);
    Value |= 1 << EncVal;

    if (GPR32RegClass->contains(SubReg) || GPR64RegClass->contains(SubReg))
      ri_gprmask |= Value;
    else if (COP0RegClass->contains(SubReg))
      ri_cprmask[0] |= Value;
    // MIPS COP1 is the FPU.
    else if (FGR32RegClass->contains(SubReg) ||
             FGR64RegClass->contains(SubReg) ||
             AFGR64RegClass->contains(SubReg) ||
             MSA128BRegClass->contains(SubReg))
      ri_cprmask[1] |= Value;
    else if (COP2RegClass->contains(SubReg))
      ri_cprmask[2] |= Value;
    else if (COP3RegClass->contains(SubReg))
      ri_cprmask[3] |= Value;
  }
}
