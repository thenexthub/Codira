//===- MipsOptionRecord.h - Abstraction for storing information -*- C++ -*-===//
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
//
// MipsOptionRecord - Abstraction for storing arbitrary information in
// ELF files. Arbitrary information (e.g. register usage) can be stored in Mips
// specific ELF sections like .Mips.options. Specific records should subclass
// MipsOptionRecord and provide an implementation to EmitMipsOptionRecord which
// basically just dumps the information into an ELF section. More information
// about .Mips.option can be found in the SysV ABI and the 64-bit ELF Object
// specification.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MIPS_MIPSOPTIONRECORD_H
#define LLVM_LIB_TARGET_MIPS_MIPSOPTIONRECORD_H

#include "MCTargetDesc/MipsMCTargetDesc.h"
#include "vm/core/MC/MCContext.h"
#include "vm/core/MC/MCRegisterInfo.h"
#include <cstdint>

namespace vm::core {

class MipsELFStreamer;

class MipsOptionRecord {
public:
  virtual ~MipsOptionRecord() = default;

  virtual void EmitMipsOptionRecord() = 0;
};

class MipsRegInfoRecord : public MipsOptionRecord {
public:
  MipsRegInfoRecord(MipsELFStreamer *S, MCContext &Context)
      : Streamer(S), Context(Context) {
    ri_gprmask = 0;
    ri_cprmask[0] = ri_cprmask[1] = ri_cprmask[2] = ri_cprmask[3] = 0;
    ri_gp_value = 0;

    const MCRegisterInfo *TRI = Context.getRegisterInfo();
    GPR32RegClass = &(TRI->getRegClass(Mips::GPR32RegClassID));
    GPR64RegClass = &(TRI->getRegClass(Mips::GPR64RegClassID));
    FGR32RegClass = &(TRI->getRegClass(Mips::FGR32RegClassID));
    FGR64RegClass = &(TRI->getRegClass(Mips::FGR64RegClassID));
    AFGR64RegClass = &(TRI->getRegClass(Mips::AFGR64RegClassID));
    MSA128BRegClass = &(TRI->getRegClass(Mips::MSA128BRegClassID));
    COP0RegClass = &(TRI->getRegClass(Mips::COP0RegClassID));
    COP2RegClass = &(TRI->getRegClass(Mips::COP2RegClassID));
    COP3RegClass = &(TRI->getRegClass(Mips::COP3RegClassID));
  }

  ~MipsRegInfoRecord() override = default;

  void EmitMipsOptionRecord() override;
  void SetPhysRegUsed(MCRegister Reg, const MCRegisterInfo *MCRegInfo);

private:
  MipsELFStreamer *Streamer;
  MCContext &Context;
  const MCRegisterClass *GPR32RegClass;
  const MCRegisterClass *GPR64RegClass;
  const MCRegisterClass *FGR32RegClass;
  const MCRegisterClass *FGR64RegClass;
  const MCRegisterClass *AFGR64RegClass;
  const MCRegisterClass *MSA128BRegClass;
  const MCRegisterClass *COP0RegClass;
  const MCRegisterClass *COP2RegClass;
  const MCRegisterClass *COP3RegClass;
  uint32_t ri_gprmask;
  uint32_t ri_cprmask[4];
  int64_t ri_gp_value;
};

} // end namespace vm::core

#endif // LLVM_LIB_TARGET_MIPS_MIPSOPTIONRECORD_H
