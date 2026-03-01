//===---------------------- RetireControlUnit.cpp ---------------*- C++ -*-===//
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
/// This file simulates the hardware responsible for retiring instructions.
///
//===----------------------------------------------------------------------===//

#include "vm/core/MCA/HardwareUnits/RetireControlUnit.h"
#include "vm/core/Support/Debug.h"

#define DEBUG_TYPE "toolchain-mca"

namespace vm::core {
namespace mca {

RetireControlUnit::RetireControlUnit(const MCSchedModel &SM)
    : NextAvailableSlotIdx(0), CurrentInstructionSlotIdx(0),
      AvailableEntries(SM.isOutOfOrder() ? SM.MicroOpBufferSize : 0),
      MaxRetirePerCycle(0) {
  assert(SM.isOutOfOrder() &&
         "RetireControlUnit is not available for in-order processors");
  // Check if the scheduling model provides extra information about the machine
  // processor. If so, then use that information to set the reorder buffer size
  // and the maximum number of instructions retired per cycle.
  if (SM.hasExtraProcessorInfo()) {
    const MCExtraProcessorInfo &EPI = SM.getExtraProcessorInfo();
    if (EPI.ReorderBufferSize)
      AvailableEntries = EPI.ReorderBufferSize;
    MaxRetirePerCycle = EPI.MaxRetirePerCycle;
  }
  NumROBEntries = AvailableEntries;
  assert(NumROBEntries && "Invalid reorder buffer size!");
  Queue.resize(2 * NumROBEntries);
}

// Reserves a number of slots, and returns a new token.
unsigned RetireControlUnit::dispatch(const InstRef &IR) {
  const Instruction &Inst = *IR.getInstruction();
  unsigned Entries = normalizeQuantity(Inst.getNumMicroOps());
  assert((AvailableEntries >= Entries) && "Reorder Buffer unavailable!");

  unsigned TokenID = NextAvailableSlotIdx;
  Queue[NextAvailableSlotIdx] = {IR, Entries, false};
  NextAvailableSlotIdx += std::max(1U, Entries);
  NextAvailableSlotIdx %= Queue.size();
  assert(TokenID < UnhandledTokenID && "Invalid token ID");

  AvailableEntries -= Entries;
  return TokenID;
}

const RetireControlUnit::RUToken &RetireControlUnit::getCurrentToken() const {
  const RetireControlUnit::RUToken &Current = Queue[CurrentInstructionSlotIdx];
#ifndef NDEBUG
  const Instruction *Inst = Current.IR.getInstruction();
  assert(Inst && "Invalid RUToken in the RCU queue.");
#endif
  return Current;
}

unsigned RetireControlUnit::computeNextSlotIdx() const {
  const RetireControlUnit::RUToken &Current = getCurrentToken();
  unsigned NextSlotIdx = CurrentInstructionSlotIdx + std::max(1U, Current.NumSlots);
  return NextSlotIdx % Queue.size();
}

const RetireControlUnit::RUToken &RetireControlUnit::peekNextToken() const {
  return Queue[computeNextSlotIdx()];
}

void RetireControlUnit::consumeCurrentToken() {
  RetireControlUnit::RUToken &Current = Queue[CurrentInstructionSlotIdx];
  Current.IR.getInstruction()->retire();

  // Update the slot index to be the next item in the circular queue.
  CurrentInstructionSlotIdx += std::max(1U, Current.NumSlots);
  CurrentInstructionSlotIdx %= Queue.size();
  AvailableEntries += Current.NumSlots;
  Current = { InstRef(), 0U, false };
}

void RetireControlUnit::onInstructionExecuted(unsigned TokenID) {
  assert(Queue.size() > TokenID);
  assert(Queue[TokenID].IR.getInstruction() && "Instruction was not dispatched!");
  assert(Queue[TokenID].Executed == false && "Instruction already executed!");
  Queue[TokenID].Executed = true;
}

#ifndef NDEBUG
void RetireControlUnit::dump() const {
  dbgs() << "Retire Unit: { Total ROB Entries =" << NumROBEntries
         << ", Available ROB entries=" << AvailableEntries << " }\n";
}
#endif

} // namespace mca
} // namespace vm::core
