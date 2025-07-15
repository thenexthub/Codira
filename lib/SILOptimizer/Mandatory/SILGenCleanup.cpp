//===--- SILGenCleanup.cpp ------------------------------------------------===//
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
/// Perform peephole-style "cleanup" to aid SIL diagnostic passes.
///
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "silgen-cleanup"

#include "language/Basic/Assertions.h"
#include "language/Basic/Defer.h"
#include "language/SIL/BasicBlockBits.h"
#include "language/SIL/BasicBlockDatastructures.h"
#include "language/SIL/BasicBlockUtils.h"
#include "language/SIL/OSSALifetimeCompletion.h"
#include "language/SIL/PrettyStackTrace.h"
#include "language/SIL/PrunedLiveness.h"
#include "language/SIL/SILInstruction.h"
#include "language/SILOptimizer/Analysis/DeadEndBlocksAnalysis.h"
#include "language/SILOptimizer/Analysis/PostOrderAnalysis.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SILOptimizer/Utils/BasicBlockOptUtils.h"
#include "language/SILOptimizer/Utils/CanonicalizeInstruction.h"
#include "language/SILOptimizer/Utils/InstOptUtils.h"
#include "toolchain/ADT/PostOrderIterator.h"

using namespace language;

// Define a CanonicalizeInstruction subclass for use in SILGenCleanup.
struct SILGenCanonicalize final : CanonicalizeInstruction {
  bool changed = false;
  toolchain::SmallPtrSet<SILInstruction *, 16> deadOperands;

  SILGenCanonicalize(DeadEndBlocks &deadEndBlocks)
      : CanonicalizeInstruction(DEBUG_TYPE, deadEndBlocks) {}

  void notifyNewInstruction(SILInstruction *) override { changed = true; }

  // Just delete the given 'inst' and record its operands. The callback isn't
  // allowed to mutate any other instructions.
  void killInstruction(SILInstruction *inst) override {
    deadOperands.erase(inst);
    for (auto &operand : inst->getAllOperands()) {
      if (auto *operInst = operand.get()->getDefiningInstruction())
        deadOperands.insert(operInst);
    }
    inst->eraseFromParent();
    changed = true;
  }

  void notifyHasNewUsers(SILValue) override { changed = true; }

  /// Delete trivially dead instructions in non-deterministic order.
  ///
  /// We either have that nextII is endII or if nextII is not endII then endII
  /// is nextII->getParent()->end().
  SILBasicBlock::iterator deleteDeadOperands(SILBasicBlock::iterator nextII,
                                             SILBasicBlock::iterator endII) {
    auto callbacks = InstModCallbacks().onDelete([&](SILInstruction *deadInst) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "Trivially dead: " << *deadInst);

      // If nextII is the instruction we are going to delete, move nextII past
      // it.
      if (deadInst->getIterator() == nextII)
        ++nextII;

      // Then remove the instruction from the set and delete it.
      deadOperands.erase(deadInst);
      deadInst->eraseFromParent();
    });

    while (!deadOperands.empty()) {
      SILInstruction *deadOperInst = *deadOperands.begin();

      // Make sure at least the first instruction is removed from the set.
      deadOperands.erase(deadOperInst);

      // Then delete this instruction/everything else that we can.
      eliminateDeadInstruction(deadOperInst, callbacks);
    }
    return nextII;
  }
};

//===----------------------------------------------------------------------===//
// SILGenCleanup: Top-Level Module Transform
//===----------------------------------------------------------------------===//

namespace {

// SILGenCleanup must run on all functions that will be seen by any analysis
// used by diagnostics before transforming the function that requires the
// analysis. e.g. Closures need to be cleaned up before the closure's parent can
// be diagnosed.
//
// TODO: This pass can be converted to a function transform if the mandatory
// pipeline runs in bottom-up closure order.
struct SILGenCleanup : SILModuleTransform {
  void run() override;

  bool completeOSSALifetimes(SILFunction *function);
  template <typename Range>
  bool completeLifetimesInRange(Range const &range,
                                OSSALifetimeCompletion &completion,
                                BasicBlockSet &completed);
};

// Iterate over `iterator` until finding a block in `include` and not in
// `exclude`.
SILBasicBlock *
findFirstBlock(SILFunction *function, SILFunction::iterator &iterator,
               toolchain::function_ref<bool(SILBasicBlock *)> include,
               toolchain::function_ref<bool(SILBasicBlock *)> exclude) {
  while (iterator != function->end()) {
    auto *block = &*iterator;
    iterator = std::next(iterator);
    if (!include(block))
      continue;
    if (exclude(block))
      continue;
    return block;
  }
  return nullptr;
}

// Walk backward from `from` following first predecessors until finding the
// first already-reached block.
SILBasicBlock *findFirstLoop(SILFunction *function, SILBasicBlock *from) {
  BasicBlockSet path(function);
  auto *current = from;
  while (auto *block = current) {
    current = nullptr;
    if (!path.insert(block)) {
      return block;
    }
    if (block->pred_empty()) {
      return nullptr;
    }
    current = *block->getPredecessorBlocks().begin();
  }
  toolchain_unreachable("finished function-exiting loop!?");
}

/// Populate `roots` with the last blocks that are discovered via backwards
/// walks along any non-repeating paths starting at the ends in `backward`.
void collectReachableRoots(SILFunction *function, BasicBlockWorklist &backward,
                           StackList<SILBasicBlock *> &roots) {
  assert(!backward.empty());
  assert(roots.empty());

  // Always include the entry block as a root.  Currently SILGen will emit
  // consumes in unreachable blocks of values defined in reachable blocks (e.g.
  // test/SILGen/unreachable_code.code:testUnreachableCatchClause).
  // TODO: [fix_silgen_destroy_unreachable] Fix SILGen not to emit such
  //                                        destroys and don't add the entry
  //                                        block to roots here.
  roots.push_back(function->getEntryBlock());

  // First, find all blocks backwards-reachable from dead-end blocks.
  while (auto *block = backward.pop()) {
    for (auto *predecessor : block->getPredecessorBlocks()) {
      backward.pushIfNotVisited(predecessor);
    }
  }

  // Simple case: unpredecessored blocks.
  //
  // Every unpredecessored block reachable from some dead-end block is a root.
  for (auto &block : *function) {
    if (&block == function->getEntryBlock()) {
      // TODO: [fix_silgen_destroy_unreachable] Remove this condition.
      continue;
    }
    if (!block.pred_empty())
      continue;
    if (!backward.isVisited(&block))
      continue;
    roots.push_back(&block);
  }

  // Complex case: unreachable loops.
  //
  // Iteratively (the first time, these are the roots discovered in "Simple
  // case" above), determine which blocks are forward-reachable from roots.
  // Then, look for a block that is backwards-reachable from dead-end blocks
  // but not forwards-reachable from roots so far discovered.  If one is found,
  // it is forwards-reachable from an unreachable loop.  Walk backwards from
  // that block to find a representative block in the loop.  Add that
  // representative block to roots and iterate.  If no such block is found, all
  // roots have been found.
  BasicBlockWorklist forward(function);
  for (auto *root : roots) {
    forward.push(root);
  }
  bool changed = false;
  auto iterator = function->begin();
  do {
    changed = false;

    // Propagate forward-reachability from roots discovered so far.
    while (auto *block = forward.pop()) {
      for (auto *successor : block->getSuccessorBlocks()) {
        forward.pushIfNotVisited(successor);
      }
    }
    // Any block in `backward` but not in `forward` is forward-reachable from an
    // unreachable loop.
    if (auto *target = findFirstBlock(
            function, iterator, /*include=*/
            [&backward](auto *block) { return backward.isVisited(block); },
            /*exclude=*/
            [&forward](auto *block) { return forward.isVisited(block); })) {
      // Find the first unreachable loop it's forwards-reachable from.
      auto *loop = findFirstLoop(function, target);
      ASSERT(loop);
      forward.push(loop);
      roots.push_back(loop);
      changed = true;
    }
  } while (changed);
}

bool SILGenCleanup::completeOSSALifetimes(SILFunction *function) {
  if (!getModule()->getOptions().OSSACompleteLifetimes)
    return false;

  TOOLCHAIN_DEBUG(toolchain::dbgs() << "Completing lifetimes in " << function->getName()
                          << "\n");

  BasicBlockWorklist deadends(function);
  DeadEndBlocks *deba = getAnalysis<DeadEndBlocksAnalysis>()->get(function);
  for (auto &block : *function) {
    if (deba->isDeadEnd(&block))
      deadends.push(&block);
  }

  if (deadends.empty()) {
    // There are no dead-end blocks, so there are no lifetimes to complete.
    // (SILGen may emit incomplete lifetimes, but not underconsumed lifetimes.)
    return false;
  }

  // Lifetimes must be completed in unreachable blocks that are reachable via
  // backwards walk from dead-end blocks.  First, check whether there are any
  // unreachable blocks.
  ReachableBlocks reachableBlocks(function);
  reachableBlocks.compute();
  StackList<SILBasicBlock *> roots(function);
  if (!reachableBlocks.hasUnreachableBlocks()) {
    // There are no blocks that are unreachable from the entry block.  Thus
    // every block will be completed when completing the post-order of the
    // entry block.
    roots.push_back(function->getEntryBlock());
  } else {
    // There are unreachable blocks.  Determine the roots that can be reached
    // when walking from the unreachable blocks.
    collectReachableRoots(function, deadends, roots);
  }

  bool changed = false;
  OSSALifetimeCompletion completion(
    function, /*DomInfo*/ nullptr, *deba,
    OSSALifetimeCompletion::ExtendTrivialVariable);
  BasicBlockSet completed(function);
  for (auto *root : roots) {
    if (root == function->getEntryBlock()) {
      assert(!completed.contains(root));
      // When completing from the entry block, prefer the PostOrderAnalysis so
      // the result is cached.
      PostOrderFunctionInfo *postOrder =
          getAnalysis<PostOrderAnalysis>()->get(function);
      changed |= completeLifetimesInRange(postOrder->getPostOrder(), completion,
                                          completed);
    }
    if (completed.contains(root)) {
      // This block has already been completed in some other post-order
      // traversal.  Thus the entire post-order rooted at it has already been
      // completed.
      continue;
    }
    changed |= completeLifetimesInRange(
        make_range(po_begin(root), po_end(root)), completion, completed);
  }
  function->verifyOwnership(/*deadEndBlocks=*/nullptr);
  return changed;
}

template <typename Range>
bool SILGenCleanup::completeLifetimesInRange(Range const &range,
                                             OSSALifetimeCompletion &completion,
                                             BasicBlockSet &completed) {
  bool changed = false;
  for (auto *block : range) {
    if (!completed.insert(block))
      continue;
    TOOLCHAIN_DEBUG(toolchain::dbgs()
               << "Completing lifetimes in bb" << block->getDebugID() << "\n");
    for (SILInstruction &inst : reverse(*block)) {
      for (auto result : inst.getResults()) {
        TOOLCHAIN_DEBUG(toolchain::dbgs() << "completing " << result << "\n");
        if (completion.completeOSSALifetime(
                result, OSSALifetimeCompletion::Boundary::Availability) ==
            LifetimeCompletion::WasCompleted) {
          TOOLCHAIN_DEBUG(toolchain::dbgs() << "\tcompleted!\n");
          changed = true;
        }
      }
    }
    for (SILArgument *arg : block->getArguments()) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "completing " << *arg << "\n");
      assert(!arg->isReborrow() && "reborrows not legal at this SIL stage");
      if (completion.completeOSSALifetime(
              arg, OSSALifetimeCompletion::Boundary::Availability) ==
          LifetimeCompletion::WasCompleted) {
        TOOLCHAIN_DEBUG(toolchain::dbgs() << "\tcompleted!\n");
        changed = true;
      }
    }
  }
  return changed;
}

void SILGenCleanup::run() {
  auto &module = *getModule();
  for (auto &function : module) {
    if (!function.isDefinition())
      continue;

    PrettyStackTraceSILFunction stackTrace("silgen cleanup", &function);

    TOOLCHAIN_DEBUG(toolchain::dbgs()
               << "\nRunning SILGenCleanup on " << function.getName() << "\n");

    bool changed = completeOSSALifetimes(&function);
    DeadEndBlocks deadEndBlocks(&function);
    SILGenCanonicalize sgCanonicalize(deadEndBlocks);

    // Iterate over all blocks even if they aren't reachable. No phi-less
    // dataflow cycles should have been created yet, and these transformations
    // are simple enough they shouldn't be affected by cycles.
    for (auto &bb : function) {
      for (auto ii = bb.begin(), ie = bb.end(); ii != ie;) {
        ii = sgCanonicalize.canonicalize(&*ii);
        ii = sgCanonicalize.deleteDeadOperands(ii, ie);
      }
    }
    changed |= sgCanonicalize.changed;
    if (changed) {
      auto invalidKind = SILAnalysis::InvalidationKind::Instructions;
      invalidateAnalysis(&function, invalidKind);
    }
  }
}

} // end anonymous namespace

SILTransform *language::createSILGenCleanup() { return new SILGenCleanup(); }
