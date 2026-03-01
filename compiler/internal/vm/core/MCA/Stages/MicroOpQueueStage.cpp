//===---------------------- MicroOpQueueStage.cpp ---------------*- C++ -*-===//
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
/// This file defines the MicroOpQueueStage.
///
//===----------------------------------------------------------------------===//

#include "vm/core/MCA/Stages/MicroOpQueueStage.h"

namespace vm::core {
namespace mca {

#define DEBUG_TYPE "toolchain-mca"

Error MicroOpQueueStage::moveInstructions() {
  InstRef IR = Buffer[CurrentInstructionSlotIdx];
  while (IR && checkNextStage(IR)) {
    if (toolchain::Error Val = moveToTheNextStage(IR))
      return Val;

    Buffer[CurrentInstructionSlotIdx].invalidate();
    unsigned NormalizedOpcodes = getNormalizedOpcodes(IR);
    CurrentInstructionSlotIdx += NormalizedOpcodes;
    CurrentInstructionSlotIdx %= Buffer.size();
    AvailableEntries += NormalizedOpcodes;
    IR = Buffer[CurrentInstructionSlotIdx];
  }

  return toolchain::ErrorSuccess();
}

MicroOpQueueStage::MicroOpQueueStage(unsigned Size, unsigned IPC,
                                     bool ZeroLatencyStage)
    : NextAvailableSlotIdx(0), CurrentInstructionSlotIdx(0), MaxIPC(IPC),
      CurrentIPC(0), IsZeroLatencyStage(ZeroLatencyStage) {
  Buffer.resize(Size ? Size : 1);
  AvailableEntries = Buffer.size();
}

Error MicroOpQueueStage::execute(InstRef &IR) {
  Buffer[NextAvailableSlotIdx] = IR;
  unsigned NormalizedOpcodes = getNormalizedOpcodes(IR);
  NextAvailableSlotIdx += NormalizedOpcodes;
  NextAvailableSlotIdx %= Buffer.size();
  AvailableEntries -= NormalizedOpcodes;
  ++CurrentIPC;
  return toolchain::ErrorSuccess();
}

Error MicroOpQueueStage::cycleStart() {
  CurrentIPC = 0;
  if (!IsZeroLatencyStage)
    return moveInstructions();
  return toolchain::ErrorSuccess();
}

Error MicroOpQueueStage::cycleEnd() {
  if (IsZeroLatencyStage)
    return moveInstructions();
  return toolchain::ErrorSuccess();
}

} // namespace mca
} // namespace vm::core
