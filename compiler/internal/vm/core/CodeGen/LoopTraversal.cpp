//===- LoopTraversal.cpp - Optimal basic block traversal order --*- C++ -*-===//
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

#include "vm/core/CodeGen/LoopTraversal.h"
#include "vm/core/ADT/PostOrderIterator.h"
#include "vm/core/CodeGen/MachineFunction.h"

using namespace vm::core;

bool LoopTraversal::isBlockDone(MachineBasicBlock *MBB) {
  unsigned MBBNumber = MBB->getNumber();
  assert(MBBNumber < MBBInfos.size() && "Unexpected basic block number.");
  return MBBInfos[MBBNumber].PrimaryCompleted &&
         MBBInfos[MBBNumber].IncomingCompleted ==
             MBBInfos[MBBNumber].PrimaryIncoming &&
         MBBInfos[MBBNumber].IncomingProcessed == MBB->pred_size();
}

LoopTraversal::TraversalOrder LoopTraversal::traverse(MachineFunction &MF) {
  // Initialize the MMBInfos
  MBBInfos.assign(MF.getNumBlockIDs(), MBBInfo());

  MachineBasicBlock *Entry = &*MF.begin();
  ReversePostOrderTraversal<MachineBasicBlock *> RPOT(Entry);
  SmallVector<MachineBasicBlock *, 4> Workqueue;
  SmallVector<TraversedMBBInfo, 4> MBBTraversalOrder;
  for (MachineBasicBlock *MBB : RPOT) {
    // N.B: IncomingProcessed and IncomingCompleted were already updated while
    // processing this block's predecessors.
    unsigned MBBNumber = MBB->getNumber();
    assert(MBBNumber < MBBInfos.size() && "Unexpected basic block number.");
    MBBInfos[MBBNumber].PrimaryCompleted = true;
    MBBInfos[MBBNumber].PrimaryIncoming = MBBInfos[MBBNumber].IncomingProcessed;
    bool Primary = true;
    Workqueue.push_back(MBB);
    while (!Workqueue.empty()) {
      MachineBasicBlock *ActiveMBB = Workqueue.pop_back_val();
      bool Done = isBlockDone(ActiveMBB);
      MBBTraversalOrder.push_back(TraversedMBBInfo(ActiveMBB, Primary, Done));
      for (MachineBasicBlock *Succ : ActiveMBB->successors()) {
        unsigned SuccNumber = Succ->getNumber();
        assert(SuccNumber < MBBInfos.size() &&
               "Unexpected basic block number.");
        if (!isBlockDone(Succ)) {
          if (Primary)
            MBBInfos[SuccNumber].IncomingProcessed++;
          if (Done)
            MBBInfos[SuccNumber].IncomingCompleted++;
          if (isBlockDone(Succ))
            Workqueue.push_back(Succ);
        }
      }
      Primary = false;
    }
  }

  // We need to go through again and finalize any blocks that are not done yet.
  // This is possible if blocks have dead predecessors, so we didn't visit them
  // above.
  for (MachineBasicBlock *MBB : RPOT) {
    if (!isBlockDone(MBB))
      MBBTraversalOrder.push_back(TraversedMBBInfo(MBB, false, true));
    // Don't update successors here. We'll get to them anyway through this
    // loop.
  }

  MBBInfos.clear();

  return MBBTraversalOrder;
}
