//===---------------------- EntryStage.cpp ----------------------*- C++ -*-===//
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
/// This file defines the Fetch stage of an instruction pipeline.  Its sole
/// purpose in life is to produce instructions for the rest of the pipeline.
///
//===----------------------------------------------------------------------===//

#include "vm/core/MCA/Stages/EntryStage.h"
#include "vm/core/MCA/Instruction.h"

namespace vm::core {
namespace mca {

bool EntryStage::hasWorkToComplete() const {
  return static_cast<bool>(CurrentInstruction) || !SM.isEnd();
}

bool EntryStage::isAvailable(const InstRef & /* unused */) const {
  if (CurrentInstruction)
    return checkNextStage(CurrentInstruction);
  return false;
}

Error EntryStage::getNextInstruction() {
  assert(!CurrentInstruction && "There is already an instruction to process!");
  if (!SM.hasNext()) {
    if (!SM.isEnd())
      return toolchain::make_error<InstStreamPause>();
    else
      return toolchain::ErrorSuccess();
  }
  SourceRef SR = SM.peekNext();
  std::unique_ptr<Instruction> Inst = std::make_unique<Instruction>(SR.second);
  CurrentInstruction = InstRef(SR.first, Inst.get());
  Instructions.emplace_back(std::move(Inst));
  SM.updateNext();
  return toolchain::ErrorSuccess();
}

toolchain::Error EntryStage::execute(InstRef & /*unused */) {
  assert(CurrentInstruction && "There is no instruction to process!");
  if (toolchain::Error Val = moveToTheNextStage(CurrentInstruction))
    return Val;

  // Move the program counter.
  CurrentInstruction.invalidate();
  return getNextInstruction();
}

toolchain::Error EntryStage::cycleStart() {
  if (!CurrentInstruction)
    return getNextInstruction();
  return toolchain::ErrorSuccess();
}

toolchain::Error EntryStage::cycleResume() {
  assert(!CurrentInstruction);
  return getNextInstruction();
}

toolchain::Error EntryStage::cycleEnd() {
  // Find the first instruction which hasn't been retired.
  auto Range = drop_begin(Instructions, NumRetired);
  auto It = find_if(Range, [](const std::unique_ptr<Instruction> &I) {
    return !I->isRetired();
  });

  NumRetired = std::distance(Instructions.begin(), It);
  // Erase instructions up to the first that hasn't been retired.
  if ((NumRetired * 2) >= Instructions.size()) {
    Instructions.erase(Instructions.begin(), It);
    NumRetired = 0;
  }

  return toolchain::ErrorSuccess();
}

} // namespace mca
} // namespace vm::core
