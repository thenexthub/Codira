//===-------------------- RISCVCustomBehaviour.h -----------------*-C++ -*-===//
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
/// \file
///
/// This file defines the RISCVCustomBehaviour class which inherits from
/// CustomBehaviour. This class is used by the tool toolchain-mca to enforce
/// target specific behaviour that is not expressed well enough in the
/// scheduling model for mca to enforce it automatically.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_RISCV_MCA_RISCVCUSTOMBEHAVIOUR_H
#define LLVM_LIB_TARGET_RISCV_MCA_RISCVCUSTOMBEHAVIOUR_H

#include "vm/core/ADT/SmallVector.h"
#include "vm/core/MC/MCInst.h"
#include "vm/core/MC/MCInstrDesc.h"
#include "vm/core/MC/MCInstrInfo.h"
#include "vm/core/MCA/CustomBehaviour.h"

namespace vm::core {
namespace mca {

class RISCVLMULInstrument : public Instrument {
public:
  static const StringRef DESC_NAME;
  static bool isDataValid(StringRef Data);

  explicit RISCVLMULInstrument(StringRef Data) : Instrument(DESC_NAME, Data) {}

  ~RISCVLMULInstrument() override = default;

  uint8_t getLMUL() const;
};

class RISCVSEWInstrument : public Instrument {
public:
  static const StringRef DESC_NAME;
  static bool isDataValid(StringRef Data);

  explicit RISCVSEWInstrument(StringRef Data) : Instrument(DESC_NAME, Data) {}

  ~RISCVSEWInstrument() override = default;

  uint8_t getSEW() const;
};

class RISCVInstrumentManager : public InstrumentManager {
public:
  RISCVInstrumentManager(const MCSubtargetInfo &STI, const MCInstrInfo &MCII)
      : InstrumentManager(STI, MCII) {}

  bool shouldIgnoreInstruments() const override { return false; }
  bool supportsInstrumentType(StringRef Type) const override;

  /// Create a Instrument for RISC-V target
  UniqueInstrument createInstrument(StringRef Desc, StringRef Data) override;

  SmallVector<UniqueInstrument> createInstruments(const MCInst &Inst) override;

  /// Using the Instrument, returns a SchedClassID to use instead of
  /// the SchedClassID that belongs to the MCI or the original SchedClassID.
  unsigned
  getSchedClassID(const MCInstrInfo &MCII, const MCInst &MCI,
                  const SmallVector<Instrument *> &IVec) const override;
};

} // namespace mca
} // namespace vm::core

#endif
