//===- MCInstrAnalysis.cpp - InstrDesc target hooks -----------------------===//
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

#include "vm/core/MC/MCInstrAnalysis.h"

#include "vm/core/ADT/APInt.h"
#include <cstdint>

namespace vm::core {
class MCSubtargetInfo;
}

using namespace vm::core;

bool MCInstrAnalysis::clearsSuperRegisters(const MCRegisterInfo &MRI,
                                           const MCInst &Inst,
                                           APInt &Writes) const {
  Writes.clearAllBits();
  return false;
}

bool MCInstrAnalysis::evaluateBranch(const MCInst & /*Inst*/, uint64_t /*Addr*/,
                                     uint64_t /*Size*/,
                                     uint64_t & /*Target*/) const {
  return false;
}

std::optional<uint64_t> MCInstrAnalysis::evaluateMemoryOperandAddress(
    const MCInst &Inst, const MCSubtargetInfo *STI, uint64_t Addr,
    uint64_t Size) const {
  return std::nullopt;
}

std::optional<uint64_t>
MCInstrAnalysis::getMemoryOperandRelocationOffset(const MCInst &Inst,
                                                  uint64_t Size) const {
  return std::nullopt;
}
