//===- DomTreeUpdater.cpp - DomTree/Post DomTree Updater --------*- C++ -*-===//
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
// This file implements the DomTreeUpdater class, which provides a uniform way
// to update dominator tree related data structures.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/DomTreeUpdater.h"
#include "vm/core/Analysis/GenericDomTreeUpdaterImpl.h"
#include "vm/core/Analysis/PostDominators.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/Support/Compiler.h"
#include "vm/core/Support/GenericDomTree.h"
#include <functional>

namespace vm::core {

template class LLVM_EXPORT_TEMPLATE
    GenericDomTreeUpdater<DomTreeUpdater, DominatorTree, PostDominatorTree>;

template LLVM_EXPORT_TEMPLATE void
GenericDomTreeUpdater<DomTreeUpdater, DominatorTree,
                      PostDominatorTree>::recalculate(Function &F);

template LLVM_EXPORT_TEMPLATE void
GenericDomTreeUpdater<DomTreeUpdater, DominatorTree, PostDominatorTree>::
    applyUpdatesImpl</*IsForward=*/true>();
template LLVM_EXPORT_TEMPLATE void
GenericDomTreeUpdater<DomTreeUpdater, DominatorTree, PostDominatorTree>::
    applyUpdatesImpl</*IsForward=*/false>();

bool DomTreeUpdater::forceFlushDeletedBB() {
  if (DeletedBBs.empty())
    return false;

  for (auto *BB : DeletedBBs) {
    // After calling deleteBB or callbackDeleteBB under Lazy UpdateStrategy,
    // validateDeleteBB() removes all instructions of DelBB and adds an
    // UnreachableInst as its terminator. So we check whether the BasicBlock to
    // delete only has an UnreachableInst inside.
    assert(BB->size() == 1 && isa<UnreachableInst>(BB->getTerminator()) &&
           "DelBB has been modified while awaiting deletion.");
    eraseDelBBNode(BB);
    BB->eraseFromParent();
  }
  DeletedBBs.clear();
  Callbacks.clear();
  return true;
}

// The DT and PDT require the nodes related to updates
// are not deleted when update functions are called.
// So BasicBlock deletions must be pended when the
// UpdateStrategy is Lazy. When the UpdateStrategy is
// Eager, the BasicBlock will be deleted immediately.
void DomTreeUpdater::deleteBB(BasicBlock *DelBB) {
  validateDeleteBB(DelBB);
  if (Strategy == UpdateStrategy::Lazy) {
    DeletedBBs.insert(DelBB);
    return;
  }

  eraseDelBBNode(DelBB);
  DelBB->eraseFromParent();
}

void DomTreeUpdater::callbackDeleteBB(
    BasicBlock *DelBB, std::function<void(BasicBlock *)> Callback) {
  validateDeleteBB(DelBB);
  if (Strategy == UpdateStrategy::Lazy) {
    Callbacks.push_back(CallBackOnDeletion(DelBB, Callback));
    DeletedBBs.insert(DelBB);
    return;
  }

  eraseDelBBNode(DelBB);
  DelBB->removeFromParent();
  Callback(DelBB);
  delete DelBB;
}

void DomTreeUpdater::validateDeleteBB(BasicBlock *DelBB) {
  assert(DelBB && "Invalid push_back of nullptr DelBB.");
  assert(pred_empty(DelBB) && "DelBB has one or more predecessors.");
  // DelBB is unreachable and all its instructions are dead.
  while (!DelBB->empty()) {
    Instruction &I = DelBB->back();
    // Replace used instructions with an arbitrary value (poison).
    if (!I.use_empty())
      I.replaceAllUsesWith(PoisonValue::get(I.getType()));
    DelBB->back().eraseFromParent();
  }
  // Make sure DelBB has a valid terminator instruction. As long as DelBB is a
  // Child of Function F it must contain valid IR.
  new UnreachableInst(DelBB->getContext(), DelBB);
}

LLVM_DUMP_METHOD
void DomTreeUpdater::dump() const {
  Base::dump();
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  raw_ostream &OS = dbgs();
  OS << "Pending Callbacks:\n";
  int Index = 0;
  for (const auto &BB : Callbacks) {
    OS << "  " << Index << " : ";
    ++Index;
    if (BB->hasName())
      OS << BB->getName() << "(";
    else
      OS << "(no_name)(";
    OS << BB << ")\n";
  }
#endif
}

} // namespace vm::core
