//===---------------------- RetireStage.cpp ---------------------*- C++ -*-===//
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
/// This file defines the retire stage of an instruction pipeline.
/// The RetireStage represents the process logic that interacts with the
/// simulated RetireControlUnit hardware.
///
//===----------------------------------------------------------------------===//

#include "vm/core/MCA/Stages/RetireStage.h"
#include "vm/core/MCA/HWEventListener.h"
#include "vm/core/Support/Debug.h"

#define DEBUG_TYPE "toolchain-mca"

namespace vm::core {
namespace mca {

toolchain::Error RetireStage::cycleStart() {
  PRF.cycleStart();

  const unsigned MaxRetirePerCycle = RCU.getMaxRetirePerCycle();
  unsigned NumRetired = 0;
  while (!RCU.isEmpty()) {
    if (MaxRetirePerCycle != 0 && NumRetired == MaxRetirePerCycle)
      break;
    const RetireControlUnit::RUToken &Current = RCU.getCurrentToken();
    if (!Current.Executed)
      break;
    notifyInstructionRetired(Current.IR);
    RCU.consumeCurrentToken();
    NumRetired++;
  }

  return toolchain::ErrorSuccess();
}

toolchain::Error RetireStage::cycleEnd() {
  PRF.cycleEnd();
  return toolchain::ErrorSuccess();
}

toolchain::Error RetireStage::execute(InstRef &IR) {
  Instruction &IS = *IR.getInstruction();

  PRF.onInstructionExecuted(&IS);
  unsigned TokenID = IS.getRCUTokenID();
  assert(TokenID != RetireControlUnit::UnhandledTokenID);
  RCU.onInstructionExecuted(TokenID);

  return toolchain::ErrorSuccess();
}

void RetireStage::notifyInstructionRetired(const InstRef &IR) const {
  LLVM_DEBUG(toolchain::dbgs() << "[E] Instruction Retired: #" << IR << '\n');
  toolchain::SmallVector<unsigned, 4> FreedRegs(PRF.getNumRegisterFiles());
  const Instruction &Inst = *IR.getInstruction();

  // Release the load/store queue entries.
  if (Inst.isMemOp())
    LSU.onInstructionRetired(IR);

  for (const WriteState &WS : Inst.getDefs())
    PRF.removeRegisterWrite(WS, FreedRegs);
  notifyEvent<HWInstructionEvent>(HWInstructionRetiredEvent(IR, FreedRegs));
}

} // namespace mca
} // namespace vm::core
