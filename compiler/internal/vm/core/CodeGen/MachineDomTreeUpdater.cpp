//===- MachineDomTreeUpdater.cpp -----------------------------------------===//
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
// This file implements the MachineDomTreeUpdater class, which provides a
// uniform way to update dominator tree related data structures.
//
//===----------------------------------------------------------------------===//

#include "vm/core/CodeGen/MachineDomTreeUpdater.h"
#include "vm/core/Analysis/GenericDomTreeUpdaterImpl.h"
#include "vm/core/CodeGen/MachinePostDominators.h"
#include "vm/core/Support/Compiler.h"

namespace vm::core {

template class LLVM_EXPORT_TEMPLATE GenericDomTreeUpdater<
    MachineDomTreeUpdater, MachineDominatorTree, MachinePostDominatorTree>;

template LLVM_EXPORT_TEMPLATE void
GenericDomTreeUpdater<MachineDomTreeUpdater, MachineDominatorTree,
                      MachinePostDominatorTree>::recalculate(MachineFunction
                                                                 &MF);

template LLVM_EXPORT_TEMPLATE void GenericDomTreeUpdater<
    MachineDomTreeUpdater, MachineDominatorTree,
    MachinePostDominatorTree>::applyUpdatesImpl</*IsForward=*/true>();
template LLVM_EXPORT_TEMPLATE void GenericDomTreeUpdater<
    MachineDomTreeUpdater, MachineDominatorTree,
    MachinePostDominatorTree>::applyUpdatesImpl</*IsForward=*/false>();

bool MachineDomTreeUpdater::forceFlushDeletedBB() {
  if (DeletedBBs.empty())
    return false;

  for (auto *BB : DeletedBBs) {
    eraseDelBBNode(BB);
    BB->eraseFromParent();
  }
  DeletedBBs.clear();
  return true;
}

// The DT and PDT require the nodes related to updates
// are not deleted when update functions are called.
// So MachineBasicBlock deletions must be pended when the
// UpdateStrategy is Lazy. When the UpdateStrategy is
// Eager, the MachineBasicBlock will be deleted immediately.
void MachineDomTreeUpdater::deleteBB(MachineBasicBlock *DelBB) {
  validateDeleteBB(DelBB);
  if (Strategy == UpdateStrategy::Lazy) {
    DeletedBBs.insert(DelBB);
    return;
  }

  eraseDelBBNode(DelBB);
  DelBB->eraseFromParent();
}

void MachineDomTreeUpdater::validateDeleteBB(MachineBasicBlock *DelBB) {
  assert(DelBB && "Invalid push_back of nullptr DelBB.");
  assert(DelBB->pred_empty() && "DelBB has one or more predecessors.");
}

} // namespace vm::core
