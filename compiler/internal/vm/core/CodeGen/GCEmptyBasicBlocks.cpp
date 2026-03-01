//===-- GCEmptyBasicBlocks.cpp ----------------------------------*- C++ -*-===//
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
///
/// \file
/// This file contains the implementation of empty blocks garbage collection
/// pass.
///
//===----------------------------------------------------------------------===//
#include "vm/core/CodeGen/GCEmptyBasicBlocks.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/CodeGen/MachineBasicBlock.h"
#include "vm/core/CodeGen/MachineFunction.h"
#include "vm/core/CodeGen/MachineFunctionPass.h"
#include "vm/core/CodeGen/MachineJumpTableInfo.h"
#include "vm/core/CodeGen/Passes.h"
#include "vm/core/InitializePasses.h"

using namespace vm::core;

#define DEBUG_TYPE "gc-empty-basic-blocks"

STATISTIC(NumEmptyBlocksRemoved, "Number of empty blocks removed");

static bool removeEmptyBlocks(MachineFunction &MF);

PreservedAnalyses
GCEmptyBasicBlocksPass::run(MachineFunction &MF,
                            MachineFunctionAnalysisManager &MFAM) {
  bool Changed = removeEmptyBlocks(MF);
  if (Changed)
    return getMachineFunctionPassPreservedAnalyses();
  return PreservedAnalyses::all();
}

class GCEmptyBasicBlocksLegacy : public MachineFunctionPass {
public:
  static char ID;

  GCEmptyBasicBlocksLegacy() : MachineFunctionPass(ID) {
    initializeGCEmptyBasicBlocksLegacyPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override {
    return "Remove Empty Basic Blocks.";
  }

  bool runOnMachineFunction(MachineFunction &MF) override {
    return removeEmptyBlocks(MF);
  }
};

bool removeEmptyBlocks(MachineFunction &MF) {
  if (MF.size() < 2)
    return false;
  MachineJumpTableInfo *JTI = MF.getJumpTableInfo();
  int NumRemoved = 0;

  // Iterate over all blocks except the last one. We can't remove the last block
  // since it has no fallthrough block to rewire its predecessors to.
  for (MachineFunction::iterator MBB = MF.begin(),
                                 LastMBB = MachineFunction::iterator(MF.back()),
                                 NextMBB;
       MBB != LastMBB; MBB = NextMBB) {
    NextMBB = std::next(MBB);
    // TODO If a block is an eh pad, or it has address taken, we don't remove
    // it. Removing such blocks is possible, but it probably requires a more
    // complex logic.
    if (MBB->isEHPad() || MBB->hasAddressTaken())
      continue;
    // Skip blocks with real code.
    bool HasAnyRealCode = toolchain::any_of(*MBB, [](const MachineInstr &MI) {
      return !MI.isPosition() && !MI.isImplicitDef() && !MI.isKill() &&
             !MI.isDebugInstr();
    });
    if (HasAnyRealCode)
      continue;

    LLVM_DEBUG(dbgs() << "Removing basic block " << MBB->getName()
                      << " in function " << MF.getName() << ":\n"
                      << *MBB << "\n");
    SmallVector<MachineBasicBlock *, 8> Preds(MBB->predecessors());
    // Rewire the predecessors of this block to use the next block.
    for (auto &Pred : Preds)
      Pred->ReplaceUsesOfBlockWith(&*MBB, &*NextMBB);
    // Update the jump tables.
    if (JTI)
      JTI->ReplaceMBBInJumpTables(&*MBB, &*NextMBB);
    // Remove this block from predecessors of all its successors.
    while (!MBB->succ_empty())
      MBB->removeSuccessor(MBB->succ_end() - 1);
    // Finally, remove the block from the function.
    MBB->eraseFromParent();
    ++NumRemoved;
  }
  NumEmptyBlocksRemoved += NumRemoved;
  return NumRemoved != 0;
}

char GCEmptyBasicBlocksLegacy::ID = 0;
INITIALIZE_PASS(GCEmptyBasicBlocksLegacy, "gc-empty-basic-blocks",
                "Removes empty basic blocks and redirects their uses to their "
                "fallthrough blocks.",
                false, false)

MachineFunctionPass *toolchain::createGCEmptyBasicBlocksLegacyPass() {
  return new GCEmptyBasicBlocksLegacy();
}
