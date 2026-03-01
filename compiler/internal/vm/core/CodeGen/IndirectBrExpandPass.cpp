//===- IndirectBrExpandPass.cpp - Expand indirectbr to switch -------------===//
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
/// Implements an expansion pass to turn `indirectbr` instructions in the IR
/// into `switch` instructions. This works by enumerating the basic blocks in
/// a dense range of integers, replacing each `blockaddr` constant with the
/// corresponding integer constant, and then building a switch that maps from
/// the integers to the actual blocks. All of the indirectbr instructions in the
/// function are redirected to this common switch.
///
/// While this is generically useful if a target is unable to codegen
/// `indirectbr` natively, it is primarily useful when there is some desire to
/// get the builtin non-jump-table lowering of a switch even when the input
/// source contained an explicit indirect branch construct.
///
/// Note that it doesn't make any sense to enable this pass unless a target also
/// disables jump-table lowering of switches. Doing that is likely to pessimize
/// the code.
///
//===----------------------------------------------------------------------===//

#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/Sequence.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/Analysis/DomTreeUpdater.h"
#include "vm/core/CodeGen/IndirectBrExpand.h"
#include "vm/core/CodeGen/TargetPassConfig.h"
#include "vm/core/CodeGen/TargetSubtargetInfo.h"
#include "vm/core/IR/BasicBlock.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/Dominators.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Target/TargetMachine.h"
#include <optional>

using namespace vm::core;

#define DEBUG_TYPE "indirectbr-expand"

namespace {

class IndirectBrExpandLegacyPass : public FunctionPass {
public:
  static char ID; // Pass identification, replacement for typeid

  IndirectBrExpandLegacyPass() : FunctionPass(ID) {
    initializeIndirectBrExpandLegacyPassPass(*PassRegistry::getPassRegistry());
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<DominatorTreeWrapperPass>();
  }

  bool runOnFunction(Function &F) override;
};

} // end anonymous namespace

static bool runImpl(Function &F, const TargetLowering *TLI,
                    DomTreeUpdater *DTU);

PreservedAnalyses IndirectBrExpandPass::run(Function &F,
                                            FunctionAnalysisManager &FAM) {
  auto *STI = TM->getSubtargetImpl(F);
  if (!STI->enableIndirectBrExpand())
    return PreservedAnalyses::all();

  auto *TLI = STI->getTargetLowering();
  auto *DT = FAM.getCachedResult<DominatorTreeAnalysis>(F);
  DomTreeUpdater DTU(DT, DomTreeUpdater::UpdateStrategy::Lazy);

  bool Changed = runImpl(F, TLI, DT ? &DTU : nullptr);
  if (!Changed)
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserve<DominatorTreeAnalysis>();
  return PA;
}

char IndirectBrExpandLegacyPass::ID = 0;

INITIALIZE_PASS_BEGIN(IndirectBrExpandLegacyPass, DEBUG_TYPE,
                      "Expand indirectbr instructions", false, false)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(IndirectBrExpandLegacyPass, DEBUG_TYPE,
                    "Expand indirectbr instructions", false, false)

FunctionPass *toolchain::createIndirectBrExpandPass() {
  return new IndirectBrExpandLegacyPass();
}

bool runImpl(Function &F, const TargetLowering *TLI, DomTreeUpdater *DTU) {
  auto &DL = F.getDataLayout();

  SmallVector<IndirectBrInst *, 1> IndirectBrs;

  // Set of all potential successors for indirectbr instructions.
  SmallPtrSet<BasicBlock *, 4> IndirectBrSuccs;

  // Build a list of indirectbrs that we want to rewrite.
  for (BasicBlock &BB : F)
    if (auto *IBr = dyn_cast<IndirectBrInst>(BB.getTerminator())) {
      // Handle the degenerate case of no successors by replacing the indirectbr
      // with unreachable as there is no successor available.
      if (IBr->getNumSuccessors() == 0) {
        (void)new UnreachableInst(F.getContext(), IBr->getIterator());
        IBr->eraseFromParent();
        continue;
      }

      IndirectBrs.push_back(IBr);
      IndirectBrSuccs.insert_range(IBr->successors());
    }

  if (IndirectBrs.empty())
    return false;

  // If we need to replace any indirectbrs we need to establish integer
  // constants that will correspond to each of the basic blocks in the function
  // whose address escapes. We do that here and rewrite all the blockaddress
  // constants to just be those integer constants cast to a pointer type.
  SmallVector<BasicBlock *, 4> BBs;

  for (BasicBlock &BB : F) {
    // Skip blocks that aren't successors to an indirectbr we're going to
    // rewrite.
    if (!IndirectBrSuccs.count(&BB))
      continue;

    auto IsBlockAddressUse = [&](const Use &U) {
      return isa<BlockAddress>(U.getUser());
    };
    auto BlockAddressUseIt = toolchain::find_if(BB.uses(), IsBlockAddressUse);
    if (BlockAddressUseIt == BB.use_end())
      continue;

    assert(std::none_of(std::next(BlockAddressUseIt), BB.use_end(),
                        IsBlockAddressUse) &&
           "There should only ever be a single blockaddress use because it is "
           "a constant and should be uniqued.");

    auto *BA = cast<BlockAddress>(BlockAddressUseIt->getUser());

    // Skip if the constant was formed but ended up not being used (due to DCE
    // or whatever).
    if (!BA->isConstantUsed())
      continue;

    // Compute the index we want to use for this basic block. We can't use zero
    // because null can be compared with block addresses.
    int BBIndex = BBs.size() + 1;
    BBs.push_back(&BB);

    auto *ITy = cast<IntegerType>(DL.getIntPtrType(BA->getType()));
    ConstantInt *BBIndexC = ConstantInt::get(ITy, BBIndex);

    // Now rewrite the blockaddress to an integer constant based on the index.
    // FIXME: This part doesn't properly recognize other uses of blockaddress
    // expressions, for instance, where they are used to pass labels to
    // asm-goto. This part of the pass needs a rework.
    BA->replaceAllUsesWith(ConstantExpr::getIntToPtr(BBIndexC, BA->getType()));
  }

  if (BBs.empty()) {
    // There are no blocks whose address is taken, so any indirectbr instruction
    // cannot get a valid input and we can replace all of them with unreachable.
    SmallVector<DominatorTree::UpdateType, 8> Updates;
    if (DTU)
      Updates.reserve(IndirectBrSuccs.size());
    for (auto *IBr : IndirectBrs) {
      if (DTU) {
        for (BasicBlock *SuccBB : IBr->successors())
          Updates.push_back({DominatorTree::Delete, IBr->getParent(), SuccBB});
      }
      (void)new UnreachableInst(F.getContext(), IBr->getIterator());
      IBr->eraseFromParent();
    }
    if (DTU) {
      assert(Updates.size() == IndirectBrSuccs.size() &&
             "Got unexpected update count.");
      DTU->applyUpdates(Updates);
    }
    return true;
  }

  BasicBlock *SwitchBB;
  Value *SwitchValue;

  // Compute a common integer type across all the indirectbr instructions.
  IntegerType *CommonITy = nullptr;
  for (auto *IBr : IndirectBrs) {
    auto *ITy =
        cast<IntegerType>(DL.getIntPtrType(IBr->getAddress()->getType()));
    if (!CommonITy || ITy->getBitWidth() > CommonITy->getBitWidth())
      CommonITy = ITy;
  }

  auto GetSwitchValue = [CommonITy](IndirectBrInst *IBr) {
    return CastInst::CreatePointerCast(IBr->getAddress(), CommonITy,
                                       Twine(IBr->getAddress()->getName()) +
                                           ".switch_cast",
                                       IBr->getIterator());
  };

  SmallVector<DominatorTree::UpdateType, 8> Updates;

  if (IndirectBrs.size() == 1) {
    // If we only have one indirectbr, we can just directly replace it within
    // its block.
    IndirectBrInst *IBr = IndirectBrs[0];
    SwitchBB = IBr->getParent();
    SwitchValue = GetSwitchValue(IBr);
    if (DTU) {
      Updates.reserve(IndirectBrSuccs.size());
      for (BasicBlock *SuccBB : IBr->successors())
        Updates.push_back({DominatorTree::Delete, IBr->getParent(), SuccBB});
      assert(Updates.size() == IndirectBrSuccs.size() &&
             "Got unexpected update count.");
    }
    IBr->eraseFromParent();
  } else {
    // Otherwise we need to create a new block to hold the switch across BBs,
    // jump to that block instead of each indirectbr, and phi together the
    // values for the switch.
    SwitchBB = BasicBlock::Create(F.getContext(), "switch_bb", &F);
    auto *SwitchPN = PHINode::Create(CommonITy, IndirectBrs.size(),
                                     "switch_value_phi", SwitchBB);
    SwitchValue = SwitchPN;

    // Now replace the indirectbr instructions with direct branches to the
    // switch block and fill out the PHI operands.
    if (DTU)
      Updates.reserve(IndirectBrs.size() + 2 * IndirectBrSuccs.size());
    for (auto *IBr : IndirectBrs) {
      SwitchPN->addIncoming(GetSwitchValue(IBr), IBr->getParent());
      BranchInst::Create(SwitchBB, IBr->getIterator());
      if (DTU) {
        Updates.push_back({DominatorTree::Insert, IBr->getParent(), SwitchBB});
        for (BasicBlock *SuccBB : IBr->successors())
          Updates.push_back({DominatorTree::Delete, IBr->getParent(), SuccBB});
      }
      IBr->eraseFromParent();
    }
  }

  // Now build the switch in the block. The block will have no terminator
  // already.
  auto *SI = SwitchInst::Create(SwitchValue, BBs[0], BBs.size(), SwitchBB);

  // Add a case for each block.
  for (int i : toolchain::seq<int>(1, BBs.size()))
    SI->addCase(ConstantInt::get(CommonITy, i + 1), BBs[i]);

  if (DTU) {
    // If there were multiple indirectbr's, they may have common successors,
    // but in the dominator tree, we only track unique edges.
    SmallPtrSet<BasicBlock *, 8> UniqueSuccessors;
    Updates.reserve(Updates.size() + BBs.size());
    for (BasicBlock *BB : BBs) {
      if (UniqueSuccessors.insert(BB).second)
        Updates.push_back({DominatorTree::Insert, SwitchBB, BB});
    }
    DTU->applyUpdates(Updates);
  }

  return true;
}

bool IndirectBrExpandLegacyPass::runOnFunction(Function &F) {
  auto *TPC = getAnalysisIfAvailable<TargetPassConfig>();
  if (!TPC)
    return false;

  auto &TM = TPC->getTM<TargetMachine>();
  auto &STI = *TM.getSubtargetImpl(F);
  if (!STI.enableIndirectBrExpand())
    return false;
  auto *TLI = STI.getTargetLowering();

  std::optional<DomTreeUpdater> DTU;
  if (auto *DTWP = getAnalysisIfAvailable<DominatorTreeWrapperPass>())
    DTU.emplace(DTWP->getDomTree(), DomTreeUpdater::UpdateStrategy::Lazy);

  return runImpl(F, TLI, DTU ? &*DTU : nullptr);
}
