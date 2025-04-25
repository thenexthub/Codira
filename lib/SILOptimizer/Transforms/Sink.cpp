//===--- Sink.cpp ----- Code Sinking --------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//
//===----------------------------------------------------------------------===//
///
/// \file
/// Many SIL instructions that don't have side effects at the SIL level are
/// lowered to a sequence of LLVM instructions that does have side effects that
/// LLVM can't sink. This pass sinks instructions close to their users.
///
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "sink-instructions"

#include "language/Basic/Assertions.h"
#include "language/SIL/DebugUtils.h"
#include "language/SIL/Dominance.h"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILDebugScope.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILValue.h"
#include "language/SILOptimizer/Analysis/DominanceAnalysis.h"
#include "language/SILOptimizer/Analysis/LoopAnalysis.h"
#include "language/SILOptimizer/Analysis/PostOrderAnalysis.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SILOptimizer/Utils/InstOptUtils.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/Debug.h"

using namespace language;

STATISTIC(NumInstrSunk, "Number of instructions sunk");

namespace {

class CodeSinkingPass : public SILFunctionTransform {
public:
  CodeSinkingPass() = default;

  DominanceInfo *DT;
  PostOrderFunctionInfo *PO;
  SILLoopInfo *LoopInfo;

  /// returns True if were able to sink the instruction \p II
  /// closer to it's users.
  bool sinkInstruction(SILInstruction *II) {

    // We can't sink instructions that may read or write to memory
    // or side effects because it can change the semantics of the program.
    if (II->mayHaveSideEffects() || II->mayReadOrWriteMemory()) return false;

    // Some instructions don't have direct memory side effects but can't be sunk
    // because of other reasons.
    if (isa<MarkUninitializedInst>(II)  ||
        isa<MarkFunctionEscapeInst>(II) ||
        isa<MarkDependenceInst>(II)) return false;

    // We don't sink stack allocations to not destroy the proper nesting of
    // stack allocations.
    if (II->isAllocatingStack() || II->isDeallocatingStack())
      return false;

    SILBasicBlock *CurrentBlock = II->getParent();
    SILBasicBlock *Dest = nullptr;
    unsigned InitialLoopDepth = LoopInfo->getLoopDepth(CurrentBlock);

    // TODO: We may want to delete debug instructions to allow us to sink more
    // instructions.
    for (auto result : II->getResults()) {
      for (auto *Operand : result->getUses()) {
        SILInstruction *User = Operand->getUser();

        // Check if the instruction is already in the user's block.
        if (User->getParent() == CurrentBlock) return false;

        // Record the block of the first user and move on to
        // other users.
        if (!Dest) {
          Dest = User->getParent();
          continue;
        }

        // Find a location that dominates all users. If we did not find such
        // a block or if it is the current block then bail out.
        Dest = DT->findNearestCommonDominator(Dest, User->getParent());

        if (!Dest || Dest == CurrentBlock)
          return false;
      }
    }

    if (!Dest) return false;

    // We don't want to sink instructions into loops. Walk up the dom tree
    // until we reach the same loop nest level.
    while (LoopInfo->getLoopDepth(Dest) != InitialLoopDepth) {
      auto Node = DT->getNode(Dest);
      assert(Node && "Invalid dom tree");
      auto IDom = Node->getIDom();
      assert(IDom && "Can't find the idom");
      Dest = IDom->getBlock();

      if (!Dest || Dest == CurrentBlock)
        return false;
    }

    II->moveBefore(&*Dest->begin());
    ++NumInstrSunk;
    return true;
  }

  void run() override {
    bool Changed = false;
    auto *F = getFunction();
    DT = PM->getAnalysis<DominanceAnalysis>()->get(F);
    PO = getAnalysis<PostOrderAnalysis>()->get(F);
    SILLoopAnalysis *LA = PM->getAnalysis<SILLoopAnalysis>();
    LoopInfo = LA->get(F);

    auto postOrder = PO->getPostOrder();

    // Scan the blocks in a post-order direction to make sure that we sink the
    // users of each instruction before visiting the instruction itself to allow
    // us to scan the function just once.
    for (auto &BB : postOrder) {
      auto Inst = BB->end();
      auto Begin = BB->begin();

      // Skip empty blocks.
      if (Inst == Begin) continue;
      // Point to the first real instruction.
      --Inst;

      while (true) {
        if (Inst == Begin) {
          // This is the first instruction in the block. Try to sink it and
          // move on to the next block.
          Changed |= sinkInstruction(&*Inst);
          break;
        } else {
          // Move the iterator to the next instruction because we may sink the
          // current instruction.
          SILInstruction *II = &*Inst;
          --Inst;
          Changed |= sinkInstruction(II);
        }
      }

    }

    if (Changed) PM->invalidateAnalysis(F,
                             SILAnalysis::InvalidationKind::Instructions);
  }


};
} // end anonymous namespace

SILTransform *swift::createCodeSinking() {
  return new CodeSinkingPass();
}
