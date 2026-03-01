//===--------------------- CustomBehaviour.cpp ------------------*- C++ -*-===//
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
/// This file implements methods from the CustomBehaviour interface.
///
//===----------------------------------------------------------------------===//

#include "vm/core/MCA/CustomBehaviour.h"
#include "vm/core/MCA/Instruction.h"

namespace vm::core {
namespace mca {

CustomBehaviour::~CustomBehaviour() = default;

unsigned CustomBehaviour::checkCustomHazard(ArrayRef<InstRef> IssuedInst,
                                            const InstRef &IR) {
  // 0 signifies that there are no hazards that need to be waited on
  return 0;
}

std::vector<std::unique_ptr<View>>
CustomBehaviour::getStartViews(toolchain::MCInstPrinter &IP,
                               toolchain::ArrayRef<toolchain::MCInst> Insts) {
  return std::vector<std::unique_ptr<View>>();
}

std::vector<std::unique_ptr<View>>
CustomBehaviour::getPostInstrInfoViews(toolchain::MCInstPrinter &IP,
                                       toolchain::ArrayRef<toolchain::MCInst> Insts) {
  return std::vector<std::unique_ptr<View>>();
}

std::vector<std::unique_ptr<View>>
CustomBehaviour::getEndViews(toolchain::MCInstPrinter &IP,
                             toolchain::ArrayRef<toolchain::MCInst> Insts) {
  return std::vector<std::unique_ptr<View>>();
}

const toolchain::StringRef LatencyInstrument::DESC_NAME = "LATENCY";

bool InstrumentManager::supportsInstrumentType(StringRef Type) const {
  return EnableInstruments && Type == LatencyInstrument::DESC_NAME;
}

bool InstrumentManager::canCustomize(const ArrayRef<Instrument *> IVec) const {
  for (const auto I : IVec) {
    if (I->getDesc() == LatencyInstrument::DESC_NAME) {
      auto LatInst = static_cast<LatencyInstrument *>(I);
      return LatInst->hasValue();
    }
  }
  return false;
}

void InstrumentManager::customize(const ArrayRef<Instrument *> IVec,
                                  InstrDesc &ID) const {
  for (const auto I : IVec) {
    if (I->getDesc() == LatencyInstrument::DESC_NAME) {
      auto LatInst = static_cast<LatencyInstrument *>(I);
      if (LatInst->hasValue()) {
        unsigned Latency = LatInst->getLatency();
        // TODO Allow to customize a subset of ID.Writes
        for (auto &W : ID.Writes)
          W.Latency = Latency;
        ID.MaxLatency = Latency;
      }
    }
  }
}

UniqueInstrument InstrumentManager::createInstrument(StringRef Desc,
                                                     StringRef Data) {
  if (EnableInstruments) {
    if (Desc == LatencyInstrument::DESC_NAME)
      return std::make_unique<LatencyInstrument>(Data);
  }
  return std::make_unique<Instrument>(Desc, Data);
}

SmallVector<UniqueInstrument>
InstrumentManager::createInstruments(const MCInst &Inst) {
  return SmallVector<UniqueInstrument>();
}

unsigned InstrumentManager::getSchedClassID(
    const MCInstrInfo &MCII, const MCInst &MCI,
    const toolchain::SmallVector<Instrument *> &IVec) const {
  return MCII.get(MCI.getOpcode()).getSchedClass();
}

} // namespace mca
} // namespace vm::core
