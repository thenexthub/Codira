//===--- LoopRotate.cpp - Loop structure simplify -------------------------===//
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

#define DEBUG_TYPE "sil-looprotate"

#include "language/Basic/Assertions.h"
#include "language/SIL/Dominance.h"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILBuilder.h"
#include "language/SIL/SILInstruction.h"
#include "language/SILOptimizer/Analysis/Analysis.h"
#include "language/SILOptimizer/Analysis/DeadEndBlocksAnalysis.h"
#include "language/SILOptimizer/Analysis/DominanceAnalysis.h"
#include "language/SILOptimizer/Analysis/LoopAnalysis.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SILOptimizer/Utils/BasicBlockOptUtils.h"
#include "language/SILOptimizer/Utils/CFGOptUtils.h"
#include "language/SILOptimizer/Utils/LoopUtils.h"
#include "language/SILOptimizer/Utils/OwnershipOptUtils.h"
#include "language/SILOptimizer/Utils/SILSSAUpdater.h"
#include "language/SILOptimizer/Utils/SILInliner.h"

#include "toolchain/Support/Debug.h"
#include "toolchain/Support/CommandLine.h"

using namespace language;

/// The size limit for the loop block to duplicate.
///
/// Larger blocks will not be duplicated to avoid too much code size increase.
/// It's very seldom that the default value of 20 is exceeded (< 0.3% of all
/// loops in the language benchmarks).
static toolchain::cl::opt<int> LoopRotateSizeLimit("looprotate-size-limit",
                                              toolchain::cl::init(20));
static toolchain::cl::opt<bool> RotateSingleBlockLoop("looprotate-single-block-loop",
                                                 toolchain::cl::init(false));
static toolchain::cl::opt<bool>
    LoopRotateInfiniteBudget("looprotate-infinite-budget",
                             toolchain::cl::init(false));

static bool rotateLoop(SILLoop *loop, DominanceInfo *domInfo,
                       SILLoopInfo *loopInfo, bool rotateSingleBlockLoops,
                       SILBasicBlock *upToBB, SILPassManager *pm);

/// Check whether all operands are loop invariant.
static bool
hasLoopInvariantOperands(SILInstruction *inst, SILLoop *loop,
                         toolchain::DenseSet<SILInstruction *> &invariant) {
  auto operands = inst->getAllOperands();

  return toolchain::all_of(operands, [=](Operand &operand) {
    ValueBase *def = operand.get();
    // Operand is outside the loop or marked invariant.
    if (auto *inst = def->getDefiningInstruction())
      return !loop->contains(inst->getParent()) || invariant.count(inst);
    if (auto *arg = dyn_cast<SILArgument>(def))
      return !loop->contains(arg->getParent());

    return false;
  });
}

/// We cannot duplicate blocks with AllocStack instructions (they need to be
/// FIFO). Other instructions can be moved to the preheader.
static bool
canDuplicateOrMoveToPreheader(SILLoop *loop, SILBasicBlock *preheader,
                              SILBasicBlock *bb,
                              SmallVectorImpl<SILInstruction *> &moves,
                              SinkAddressProjections &sinkProj) {
  toolchain::DenseSet<SILInstruction *> invariants;
  int cost = 0;
  for (auto &instRef : *bb) {
    auto *inst = &instRef;
    if (!inst->isTriviallyDuplicatable()) {
      return false;
    }
    // It wouldn't make sense to rotate dealloc_stack without also rotating the
    // alloc_stack, which is covered by isTriviallyDuplicatable.
    if (isa<DeallocStackInst>(inst)) {
      return false;
    }
    if (isa<FunctionRefInst>(inst)) {
      moves.push_back(inst);
      invariants.insert(inst);
      continue;
    }
    if (isa<DynamicFunctionRefInst>(inst)) {
      moves.push_back(inst);
      invariants.insert(inst);
      continue;
    }
    if (isa<PreviousDynamicFunctionRefInst>(inst)) {
      moves.push_back(inst);
      invariants.insert(inst);
      continue;
    }
    if (isa<IntegerLiteralInst>(inst)) {
      moves.push_back(inst);
      invariants.insert(inst);
      continue;
    }
    if (auto *MI = dyn_cast<MethodInst>(inst)) {
      if (MI->getMember().isForeign)
        return false;
      if (!hasLoopInvariantOperands(inst, loop, invariants))
        continue;
      moves.push_back(inst);
      invariants.insert(inst);
      continue;
    }
    if (!inst->mayHaveSideEffects() && !inst->mayReadFromMemory() &&
        !isa<TermInst>(inst) &&
        !isa<AllocationInst>(inst) && /* not marked mayhavesideeffects */
        !hasOwnershipOperandsOrResults(inst) &&
        hasLoopInvariantOperands(inst, loop, invariants)) {
      moves.push_back(inst);
      invariants.insert(inst);
      continue;
    }
    if (!sinkProj.analyzeAddressProjections(inst)) {
      return false;
    }

    cost += (int)instructionInlineCost(instRef);
  }

  return cost < LoopRotateSizeLimit || LoopRotateInfiniteBudget;
}

static void mapOperands(SILInstruction *inst,
                        const toolchain::DenseMap<ValueBase *, SILValue> &valueMap) {
  for (auto &operand : inst->getAllOperands()) {
    SILValue origVal = operand.get();
    ValueBase *origDef = origVal;
    auto found = valueMap.find(origDef);
    if (found != valueMap.end()) {
      SILValue mappedVal = found->second;
      operand.set(mappedVal);
    }
  }
}

static void updateSSAForUseOfValue(
    SILSSAUpdater &updater, SmallVectorImpl<SILPhiArgument *> &insertedPhis,
    const toolchain::DenseMap<ValueBase *, SILValue> &valueMap,
    SILBasicBlock *Header, SILBasicBlock *EntryCheckBlock, SILValue Res,
    SILPassManager *pm) {
  // Find the mapped instruction.
  assert(valueMap.count(Res) && "Expected to find value in map!");
  SILValue MappedValue = valueMap.find(Res)->second;
  assert(MappedValue);
  assert(Res->getType() == MappedValue->getType() && "The types must match");

  insertedPhis.clear();
  updater.initialize(MappedValue->getFunction(), Res->getType(),
                     Res->getOwnershipKind());
  updater.addAvailableValue(Header, Res);
  updater.addAvailableValue(EntryCheckBlock, MappedValue);

  // Because of the way that phi nodes are represented we have to collect all
  // uses before we update SSA. Modifying one phi node can invalidate another
  // unrelated phi nodes operands through the common branch instruction (that
  // has to be modified). This would invalidate a plain ValueUseIterator.
  // Instead we collect uses wrapping uses in branches specially so that we
  // can reconstruct the use even after the branch has been modified.
  SmallVector<UseWrapper, 8> storedUses;
  for (auto *use : Res->getUses())
    storedUses.push_back(UseWrapper(use));
  for (auto useWrapper : storedUses) {
    Operand *use = useWrapper;
    SILInstruction *user = use->getUser();
    assert(user && "Missing user");

    // Ignore uses in the same basic block.
    if (user->getParent() == Header)
      continue;

    assert(user->getParent() != EntryCheckBlock &&
           "The entry check block should dominate the header");
    updater.rewriteUse(*use);
  }

  replacePhisWithIncomingValues(pm, insertedPhis);
}

static void
updateSSAForUseOfInst(SILSSAUpdater &updater,
                      SmallVectorImpl<SILPhiArgument *> &insertedPhis,
                      const toolchain::DenseMap<ValueBase *, SILValue> &valueMap,
                      SILBasicBlock *header, SILBasicBlock *entryCheckBlock,
                      SILInstruction *inst, SILPassManager *pm) {
  for (auto result : inst->getResults())
    updateSSAForUseOfValue(updater, insertedPhis, valueMap, header,
                           entryCheckBlock, result, pm);
}

/// Rewrite the code we just created in the preheader and update SSA form.
static void rewriteNewLoopEntryCheckBlock(
    SILBasicBlock *header, SILBasicBlock *entryCheckBlock,
    const toolchain::DenseMap<ValueBase *, SILValue> &valueMap,
    SILPassManager *pm) {
  SmallVector<SILPhiArgument *, 8> insertedPhis;
  SILSSAUpdater updater(&insertedPhis);

  // Fix PHIs (incoming arguments). We iterate by index in case we replace the
  // phi argument so we do not invalidate iterators.
  for (unsigned i : range(header->getNumArguments())) {
    auto *arg = header->getArguments()[i];
    updateSSAForUseOfValue(updater, insertedPhis, valueMap, header,
                           entryCheckBlock, arg, pm);
  }

  auto instIter = header->begin();

  // The terminator might change from under us.
  while (instIter != header->end()) {
    auto &inst = *instIter;
    updateSSAForUseOfInst(updater, insertedPhis, valueMap, header,
                          entryCheckBlock, &inst, pm);
    ++instIter;
  }
}

/// Update the dominator tree after rotating the loop.
/// The former preheader now dominates all of the former headers children. The
/// former latch now dominates the former header.
static void updateDomTree(DominanceInfo *domInfo, SILBasicBlock *preheader,
                          SILBasicBlock *latch, SILBasicBlock *header) {
  auto *headerN = domInfo->getNode(header);
  SmallVector<DominanceInfoNode *, 4> Children(headerN->begin(),
                                               headerN->end());
  auto *preheaderN = domInfo->getNode(preheader);
  for (auto *Child : Children)
    domInfo->changeImmediateDominator(Child, preheaderN);

  if (header != latch)
    domInfo->changeImmediateDominator(headerN, domInfo->getNode(latch));
}

static bool rotateLoopAtMostUpToLatch(SILLoop *loop, DominanceInfo *domInfo,
                                      SILLoopInfo *loopInfo, SILPassManager *pm) {
  auto *latch = loop->getLoopLatch();
  if (!latch) {
    TOOLCHAIN_DEBUG(toolchain::dbgs()
               << *loop << " does not have a single latch block\n");
    return false;
  }

  bool didRotate = rotateLoop(
      loop, domInfo, loopInfo,
      RotateSingleBlockLoop /* rotateSingleBlockLoops */, latch, pm);

  // Keep rotating at most until we hit the original latch.
  if (didRotate)
    while (rotateLoop(loop, domInfo, loopInfo, false, latch, pm)) {
    }

  return didRotate;
}

/// Check whether this a single basic block loop - ignoring split back edges.
static bool isSingleBlockLoop(SILLoop *L) {
  auto Blocks = L->getBlocks();
  auto NumBlocks = Blocks.size();
  if (NumBlocks > 2)
    return false;

  if (NumBlocks == 1)
    return true;

  auto *header = L->getHeader();
  auto *BackEdge = Blocks[1];
  if (BackEdge == header)
    BackEdge = Blocks[0];

  if (!BackEdge->getSingleSuccessorBlock())
    return false;

  assert(BackEdge->getSingleSuccessorBlock() == header
         && "Loop not well formed");

  // Check whether the back-edge block is just a split-edge.
  for (SILInstruction &inst : make_range(BackEdge->begin(), --BackEdge->end())) {
    if (instructionInlineCost(inst) != InlineCost::Free)
      return false;
  }
  return true;
}

/// We rotated a loop if it has the following properties.
///
/// * It has an exiting header with a conditional branch.
/// * It has a preheader (the function will try to create one for critical edges
///   from cond_br).
///
/// We will rotate at most up to the basic block passed as an argument.
/// We will not rotate a loop where the header is equal to the latch except is
/// rotateSingleBlockLoops is true.
///
/// Note: The code relies on the 'UpTo' basic block to stay within the rotate
/// loop for termination.
static bool rotateLoop(SILLoop *loop, DominanceInfo *domInfo,
                       SILLoopInfo *loopInfo, bool rotateSingleBlockLoops,
                       SILBasicBlock *upToBB, SILPassManager *pm) {
  assert(loop != nullptr && domInfo != nullptr && loopInfo != nullptr
         && "Missing loop information");

  auto *header = loop->getHeader();
  if (!header)
    return false;

  // We need a preheader - this is also a canonicalization for follow-up
  // passes.
  auto *preheader = loop->getLoopPreheader();
  if (!preheader) {
    TOOLCHAIN_DEBUG(toolchain::dbgs() << *loop << " no preheader\n");
    TOOLCHAIN_DEBUG(loop->getHeader()->getParent()->dump());
    return false;
  }

  if (!rotateSingleBlockLoops && (header == upToBB || isSingleBlockLoop(loop)))
    return false;

  assert(rotateSingleBlockLoops || loop->getBlocks().size() != 1);

  // Need a conditional branch that guards the entry into the loop.
  auto *loopEntryBranch = dyn_cast<CondBranchInst>(header->getTerminator());
  if (!loopEntryBranch)
    return false;

  // The header needs to exit the loop.
  if (!loop->isLoopExiting(header)) {
    TOOLCHAIN_DEBUG(toolchain::dbgs() << *loop << " not an exiting header\n");
    TOOLCHAIN_DEBUG(loop->getHeader()->getParent()->dump());
    return false;
  }

  // We need a single backedge and the latch must not exit the loop if it is
  // also the header.
  auto *latch = loop->getLoopLatch();
  if (!latch) {
    TOOLCHAIN_DEBUG(toolchain::dbgs() << *loop << " no single latch\n");
    return false;
  }

  // Make sure we can duplicate the header.
  SmallVector<SILInstruction *, 8> moveToPreheader;
  SinkAddressProjections sinkProj;
  if (!canDuplicateOrMoveToPreheader(loop, preheader, header, moveToPreheader,
                                     sinkProj)) {
    TOOLCHAIN_DEBUG(toolchain::dbgs()
               << *loop << " instructions in header preventing rotating\n");
    return false;
  }

  auto *newHeader = loopEntryBranch->getTrueBB();
  auto *exit = loopEntryBranch->getFalseBB();
  if (loop->contains(exit))
    std::swap(newHeader, exit);
  assert(loop->contains(newHeader) && !loop->contains(exit)
         && "Could not find loop header and exit block");

  // It does not make sense to rotate the loop if the new header is loop
  // exiting as well.
  if (loop->isLoopExiting(newHeader)) {
    return false;
  }

  // Incomplete liveranges in the dead-end exit block can cause a missing adjacent
  // phi-argument for a re-borrow if there is a borrow-scope is in the loop.
  // But even when we have complete lifetimes, it's probably not worth rotating
  // a loop where the header block branches to a dead-end block.
  auto *deBlocks = pm->getAnalysis<DeadEndBlocksAnalysis>()->get(exit->getParent());
  if (deBlocks->isDeadEnd(exit)) {
    return false;
  }

  // We don't want to rotate such that we merge two headers of separate loops
  // into one. This can be turned into an assert again once we have guaranteed
  // preheader insertions.
  if (!newHeader->getSinglePredecessorBlock() && header != latch)
    return false;

  // Now that we know we can perform the rotation - move the instructions that
  // need moving.
  for (auto *inst : moveToPreheader)
    inst->moveBefore(preheader->getTerminator());

  TOOLCHAIN_DEBUG(toolchain::dbgs() << " Rotating " << *loop);

  // Map the values for the duplicated header block. We are duplicating the
  // header instructions into the end of the preheader.
  toolchain::DenseMap<ValueBase *, SILValue> valueMap;

  // The original 'phi' argument values are just the values coming from the
  // preheader edge.
  ArrayRef<SILArgument *> phis = header->getArguments();
  OperandValueArrayRef preheaderArgs =
      cast<BranchInst>(preheader->getTerminator())->getArgs();
  assert(phis.size() == preheaderArgs.size()
         && "Basic block arguments and incoming edge mismatch");

  // Here we also store the value index to use into the value map (versus
  // non-argument values where the operand use decides which value index to
  // use).
  for (unsigned Idx = 0, E = phis.size(); Idx != E; ++Idx)
    valueMap[phis[Idx]] = preheaderArgs[Idx];

  // The other instructions are just cloned to the preheader.
  TermInst *preheaderBranch = preheader->getTerminator();

  // sink address projections to avoid address phis.
  for (auto &inst : *header) {
    bool success = sinkProj.analyzeAddressProjections(&inst);
    assert(success);
    sinkProj.cloneProjections();
  }

  for (auto &inst : *header) {
    if (auto *bfi = dyn_cast<BorrowedFromInst>(&inst)) {
      // Don't do valueMap[bfi] = valueMap[bfi->getBorrowedValue()]
      // The subscript operator returns a reference into the map. Due to the
      // assignment the map might get "reallocated" from under us.
      auto mappedValue = valueMap[bfi->getBorrowedValue()];
      valueMap[bfi] = mappedValue;
    } else if (SILInstruction *cloned = inst.clone(preheaderBranch)) {
      mapOperands(cloned, valueMap);

      // The actual operand will sort out which result idx to use.
      auto instResults = inst.getResults();
      auto clonedResults = cloned->getResults();
      assert(instResults.size() == clonedResults.size());
      for (auto i : indices(instResults))
        valueMap[instResults[i]] = clonedResults[i];
    }
  }

  preheaderBranch->dropAllReferences();
  preheaderBranch->eraseFromParent();

  // If there were any uses of instructions in the duplicated loop entry check
  // block rewrite them using the ssa updater.
  rewriteNewLoopEntryCheckBlock(header, preheader, valueMap, pm);

  loop->moveToHeader(newHeader);

  // Now the original preheader dominates all of headers children and the
  // original latch dominates the header.
  updateDomTree(domInfo, preheader, latch, header);

  assert(domInfo->getNode(newHeader)->getIDom() == domInfo->getNode(preheader));
  assert(!domInfo->dominates(header, exit)
         || domInfo->getNode(exit)->getIDom() == domInfo->getNode(preheader));
  assert(domInfo->getNode(header)->getIDom() == domInfo->getNode(latch)
         || ((header == latch)
             && domInfo->getNode(header)->getIDom()
                    == domInfo->getNode(preheader)));

  // Beautify the IR. Move the old header to after the old latch as it is now
  // the latch.
  header->getParent()->moveBlockAfter(header, latch);

  // Merge the old latch with the old header if possible.
  if (mergeBasicBlockWithSuccessor(latch, domInfo, loopInfo))
    newHeader = latch; // The old Header is gone. Latch is now the Header.

  // Cloning the header into the preheader created critical edges from the
  // preheader and original header to both the new header and loop exit.
  splitCriticalEdgesFrom(preheader, domInfo, loopInfo);
  splitCriticalEdgesFrom(newHeader, domInfo, loopInfo);

  TOOLCHAIN_DEBUG(toolchain::dbgs() << "  to " << *loop);
  TOOLCHAIN_DEBUG(loop->getHeader()->getParent()->dump());
  return true;
}

namespace {

class LoopRotation : public SILFunctionTransform {

  void run() override {
#ifndef LANGUAGE_ENABLE_LANGUAGE_IN_LANGUAGE
    // This pass results in verification failures when Codira sources are not
    // enabled.
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "Loop Rotate disabled in C++-only Codira compiler\n");
    return;
#endif // !LANGUAGE_ENABLE_LANGUAGE_IN_LANGUAGE
    SILFunction *f = getFunction();
    SILLoopAnalysis *loopAnalysis = PM->getAnalysis<SILLoopAnalysis>();
    DominanceAnalysis *domAnalysis = PM->getAnalysis<DominanceAnalysis>();
    SILLoopInfo *loopInfo = loopAnalysis->get(f);
    DominanceInfo *domInfo = domAnalysis->get(f);

    if (loopInfo->empty()) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "No loops in " << f->getName() << "\n");
      return;
    }
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "Rotating loops in " << f->getName() << "\n");

    bool changed = false;
    for (auto *LoopIt : *loopInfo) {
      // Rotate loops recursively bottom-up in the loop tree.
      SmallVector<SILLoop *, 8> worklist;
      worklist.push_back(LoopIt);
      for (unsigned i = 0; i < worklist.size(); ++i) {
        auto *L = worklist[i];
        for (auto *SubLoop : *L)
          worklist.push_back(SubLoop);
      }

      while (!worklist.empty()) {
        SILLoop *loop = worklist.pop_back_val();
        changed |= canonicalizeLoop(loop, domInfo, loopInfo);
        changed |=
            rotateLoopAtMostUpToLatch(loop, domInfo, loopInfo, getPassManager());
      }
    }

    if (changed) {
      updateAllGuaranteedPhis(PM, f);
      // We preserve loop info and the dominator tree.
      domAnalysis->lockInvalidation();
      loopAnalysis->lockInvalidation();
      PM->invalidateAnalysis(f, SILAnalysis::InvalidationKind::FunctionBody);
      domAnalysis->unlockInvalidation();
      loopAnalysis->unlockInvalidation();
    }
  }
};

} // end anonymous namespace

SILTransform *language::createLoopRotate() {
  return new LoopRotation();
}
