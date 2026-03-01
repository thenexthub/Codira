//===----------------------- LSUnit.cpp --------------------------*- C++-*-===//
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
/// A Load-Store Unit for the toolchain-mca tool.
///
//===----------------------------------------------------------------------===//

#include "vm/core/MCA/HardwareUnits/LSUnit.h"
#include "vm/core/MCA/Instruction.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"

#define DEBUG_TYPE "toolchain-mca"

namespace vm::core {
namespace mca {

LSUnitBase::LSUnitBase(const MCSchedModel &SM, unsigned LQ, unsigned SQ,
                       bool AssumeNoAlias)
    : LQSize(LQ), SQSize(SQ), UsedLQEntries(0), UsedSQEntries(0),
      NoAlias(AssumeNoAlias) {
  if (SM.hasExtraProcessorInfo()) {
    const MCExtraProcessorInfo &EPI = SM.getExtraProcessorInfo();
    if (!LQSize && EPI.LoadQueueID) {
      const MCProcResourceDesc &LdQDesc = *SM.getProcResource(EPI.LoadQueueID);
      LQSize = std::max(0, LdQDesc.BufferSize);
    }

    if (!SQSize && EPI.StoreQueueID) {
      const MCProcResourceDesc &StQDesc = *SM.getProcResource(EPI.StoreQueueID);
      SQSize = std::max(0, StQDesc.BufferSize);
    }
  }
}

LSUnitBase::~LSUnitBase() = default;

void LSUnit::cycleEvent() {
  for (const std::pair<unsigned, std::unique_ptr<MemoryGroup>> &G : Groups)
    G.second->cycleEvent();
}

#ifndef NDEBUG
void LSUnit::dump() const {
  dbgs() << "[LSUnit] LQ_Size = " << getLoadQueueSize() << '\n';
  dbgs() << "[LSUnit] SQ_Size = " << getStoreQueueSize() << '\n';
  dbgs() << "[LSUnit] NextLQSlotIdx = " << getUsedLQEntries() << '\n';
  dbgs() << "[LSUnit] NextSQSlotIdx = " << getUsedSQEntries() << '\n';
  dbgs() << "\n";
  for (const auto &GroupIt : Groups) {
    const MemoryGroup &Group = *GroupIt.second;
    dbgs() << "[LSUnit] Group (" << GroupIt.first << "): "
           << "[ #Preds = " << Group.getNumPredecessors()
           << ", #GIssued = " << Group.getNumExecutingPredecessors()
           << ", #GExecuted = " << Group.getNumExecutedPredecessors()
           << ", #Inst = " << Group.getNumInstructions()
           << ", #IIssued = " << Group.getNumExecuting()
           << ", #IExecuted = " << Group.getNumExecuted() << '\n';
  }
}
#endif

unsigned LSUnit::dispatch(const InstRef &IR) {
  const Instruction &IS = *IR.getInstruction();
  bool IsStoreBarrier = IS.isAStoreBarrier();
  bool IsLoadBarrier = IS.isALoadBarrier();
  assert((IS.getMayLoad() || IS.getMayStore()) && "Not a memory operation!");

  if (IS.getMayLoad())
    acquireLQSlot();
  if (IS.getMayStore())
    acquireSQSlot();

  if (IS.getMayStore()) {
    unsigned NewGID = createMemoryGroup();
    MemoryGroup &NewGroup = getGroup(NewGID);
    NewGroup.addInstruction();

    // A store may not pass a previous load or load barrier.
    unsigned ImmediateLoadDominator =
        std::max(CurrentLoadGroupID, CurrentLoadBarrierGroupID);
    if (ImmediateLoadDominator) {
      MemoryGroup &IDom = getGroup(ImmediateLoadDominator);
      LLVM_DEBUG(dbgs() << "[LSUnit]: GROUP DEP: (" << ImmediateLoadDominator
                        << ") --> (" << NewGID << ")\n");
      IDom.addSuccessor(&NewGroup, !assumeNoAlias());
    }

    // A store may not pass a previous store barrier.
    if (CurrentStoreBarrierGroupID) {
      MemoryGroup &StoreGroup = getGroup(CurrentStoreBarrierGroupID);
      LLVM_DEBUG(dbgs() << "[LSUnit]: GROUP DEP: ("
                        << CurrentStoreBarrierGroupID << ") --> (" << NewGID
                        << ")\n");
      StoreGroup.addSuccessor(&NewGroup, true);
    }

    // A store may not pass a previous store.
    if (CurrentStoreGroupID &&
        (CurrentStoreGroupID != CurrentStoreBarrierGroupID)) {
      MemoryGroup &StoreGroup = getGroup(CurrentStoreGroupID);
      LLVM_DEBUG(dbgs() << "[LSUnit]: GROUP DEP: (" << CurrentStoreGroupID
                        << ") --> (" << NewGID << ")\n");
      StoreGroup.addSuccessor(&NewGroup, !assumeNoAlias());
    }

    CurrentStoreGroupID = NewGID;
    if (IsStoreBarrier)
      CurrentStoreBarrierGroupID = NewGID;

    if (IS.getMayLoad()) {
      CurrentLoadGroupID = NewGID;
      if (IsLoadBarrier)
        CurrentLoadBarrierGroupID = NewGID;
    }

    return NewGID;
  }

  assert(IS.getMayLoad() && "Expected a load!");

  unsigned ImmediateLoadDominator =
      std::max(CurrentLoadGroupID, CurrentLoadBarrierGroupID);

  // A new load group is created if we are in one of the following situations:
  // 1) This is a load barrier (by construction, a load barrier is always
  //    assigned to a different memory group).
  // 2) There is no load in flight (by construction we always keep loads and
  //    stores into separate memory groups).
  // 3) There is a load barrier in flight. This load depends on it.
  // 4) There is an intervening store between the last load dispatched to the
  //    LSU and this load. We always create a new group even if this load
  //    does not alias the last dispatched store.
  // 5) There is no intervening store and there is an active load group.
  //    However that group has already started execution, so we cannot add
  //    this load to it.
  bool ShouldCreateANewGroup =
      IsLoadBarrier || !ImmediateLoadDominator ||
      CurrentLoadBarrierGroupID == ImmediateLoadDominator ||
      ImmediateLoadDominator <= CurrentStoreGroupID ||
      getGroup(ImmediateLoadDominator).isExecuting();

  if (ShouldCreateANewGroup) {
    unsigned NewGID = createMemoryGroup();
    MemoryGroup &NewGroup = getGroup(NewGID);
    NewGroup.addInstruction();

    // A load may not pass a previous store or store barrier
    // unless flag 'NoAlias' is set.
    if (!assumeNoAlias() && CurrentStoreGroupID) {
      MemoryGroup &StoreGroup = getGroup(CurrentStoreGroupID);
      LLVM_DEBUG(dbgs() << "[LSUnit]: GROUP DEP: (" << CurrentStoreGroupID
                        << ") --> (" << NewGID << ")\n");
      StoreGroup.addSuccessor(&NewGroup, true);
    }

    // A load barrier may not pass a previous load or load barrier.
    if (IsLoadBarrier) {
      if (ImmediateLoadDominator) {
        MemoryGroup &LoadGroup = getGroup(ImmediateLoadDominator);
        LLVM_DEBUG(dbgs() << "[LSUnit]: GROUP DEP: (" << ImmediateLoadDominator
                          << ") --> (" << NewGID << ")\n");
        LoadGroup.addSuccessor(&NewGroup, true);
      }
    } else {
      // A younger load cannot pass a older load barrier.
      if (CurrentLoadBarrierGroupID) {
        MemoryGroup &LoadGroup = getGroup(CurrentLoadBarrierGroupID);
        LLVM_DEBUG(dbgs() << "[LSUnit]: GROUP DEP: ("
                          << CurrentLoadBarrierGroupID << ") --> (" << NewGID
                          << ")\n");
        LoadGroup.addSuccessor(&NewGroup, true);
      }
    }

    CurrentLoadGroupID = NewGID;
    if (IsLoadBarrier)
      CurrentLoadBarrierGroupID = NewGID;
    return NewGID;
  }

  // A load may pass a previous load.
  MemoryGroup &Group = getGroup(CurrentLoadGroupID);
  Group.addInstruction();
  return CurrentLoadGroupID;
}

LSUnit::Status LSUnit::isAvailable(const InstRef &IR) const {
  const Instruction &IS = *IR.getInstruction();
  if (IS.getMayLoad() && isLQFull())
    return LSUnit::LSU_LQUEUE_FULL;
  if (IS.getMayStore() && isSQFull())
    return LSUnit::LSU_SQUEUE_FULL;
  return LSUnit::LSU_AVAILABLE;
}

void LSUnit::onInstructionRetired(const InstRef &IR) {
  const Instruction &IS = *IR.getInstruction();
  bool IsALoad = IS.getMayLoad();
  bool IsAStore = IS.getMayStore();
  assert((IsALoad || IsAStore) && "Expected a memory operation!");

  if (IsALoad) {
    releaseLQSlot();
    LLVM_DEBUG(dbgs() << "[LSUnit]: Instruction idx=" << IR.getSourceIndex()
                      << " has been removed from the load queue.\n");
  }

  if (IsAStore) {
    releaseSQSlot();
    LLVM_DEBUG(dbgs() << "[LSUnit]: Instruction idx=" << IR.getSourceIndex()
                      << " has been removed from the store queue.\n");
  }
}

void LSUnit::onInstructionExecuted(const InstRef &IR) {
  const Instruction &IS = *IR.getInstruction();
  if (!IS.isMemOp())
    return;

  unsigned GroupID = IS.getLSUTokenID();
  auto It = Groups.find(GroupID);
  assert(It != Groups.end() && "Instruction not dispatched to the LS unit");
  It->second->onInstructionExecuted(IR);
  if (It->second->isExecuted())
    Groups.erase(It);

  if (!isValidGroupID(GroupID)) {
    if (GroupID == CurrentLoadGroupID)
      CurrentLoadGroupID = 0;
    if (GroupID == CurrentStoreGroupID)
      CurrentStoreGroupID = 0;
    if (GroupID == CurrentLoadBarrierGroupID)
      CurrentLoadBarrierGroupID = 0;
    if (GroupID == CurrentStoreBarrierGroupID)
      CurrentStoreBarrierGroupID = 0;
  }
}

} // namespace mca
} // namespace vm::core
