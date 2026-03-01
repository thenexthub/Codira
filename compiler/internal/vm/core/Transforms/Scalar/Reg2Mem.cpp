//===- Reg2Mem.cpp - Convert registers to allocas -------------------------===//
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
// This file demotes all registers to memory references.  It is intended to be
// the inverse of PromoteMemoryToRegister.  By converting to loads, the only
// values live across basic blocks are allocas and loads before phi nodes.
// It is intended that this should make CFG hacking much easier.
// To make later hacking easier, the entry block is split into two, such that
// all introduced allocas and nothing else are in the entry block.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Scalar/Reg2Mem.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/Analysis/LoopInfo.h"
#include "vm/core/IR/BasicBlock.h"
#include "vm/core/IR/CFG.h"
#include "vm/core/IR/Dominators.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/InstIterator.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/PassManager.h"
#include "vm/core/InitializePasses.h"
#include "vm/core/Transforms/Scalar.h"
#include "vm/core/Transforms/Utils.h"
#include "vm/core/Transforms/Utils/BasicBlockUtils.h"
#include "vm/core/Transforms/Utils/Local.h"
#include <list>
using namespace vm::core;

#define DEBUG_TYPE "reg2mem"

STATISTIC(NumRegsDemoted, "Number of registers demoted");
STATISTIC(NumPhisDemoted, "Number of phi-nodes demoted");

static bool valueEscapes(const Instruction &Inst) {
  if (!Inst.getType()->isSized())
    return false;

  const BasicBlock *BB = Inst.getParent();
  for (const User *U : Inst.users()) {
    const Instruction *UI = cast<Instruction>(U);
    if (UI->getParent() != BB || isa<PHINode>(UI))
      return true;
  }
  return false;
}

static bool runPass(Function &F) {
  // Insert all new allocas into entry block.
  BasicBlock *BBEntry = &F.getEntryBlock();
  assert(pred_empty(BBEntry) &&
         "Entry block to function must not have predecessors!");

  // Find first non-alloca instruction and create insertion point. This is
  // safe if block is well-formed: it always have terminator, otherwise
  // we'll get and assertion.
  BasicBlock::iterator I = BBEntry->begin();
  while (isa<AllocaInst>(I)) ++I;

  CastInst *AllocaInsertionPoint = new BitCastInst(
      Constant::getNullValue(Type::getInt32Ty(F.getContext())),
      Type::getInt32Ty(F.getContext()), "reg2mem alloca point", I);

  // Find the escaped instructions. But don't create stack slots for
  // allocas in entry block.
  std::list<Instruction*> WorkList;
  for (Instruction &I : instructions(F))
    if (!(isa<AllocaInst>(I) && I.getParent() == BBEntry) && valueEscapes(I))
      WorkList.push_front(&I);

  // Demote escaped instructions
  NumRegsDemoted += WorkList.size();
  for (Instruction *I : WorkList)
    DemoteRegToStack(*I, false, AllocaInsertionPoint->getIterator());

  WorkList.clear();

  // Find all phi's
  for (BasicBlock &BB : F)
    for (auto &Phi : BB.phis())
      WorkList.push_front(&Phi);

  // Demote phi nodes
  NumPhisDemoted += WorkList.size();
  for (Instruction *I : WorkList)
    DemotePHIToStack(cast<PHINode>(I), AllocaInsertionPoint->getIterator());

  return true;
}

PreservedAnalyses RegToMemPass::run(Function &F, FunctionAnalysisManager &AM) {
  auto *DT = &AM.getResult<DominatorTreeAnalysis>(F);
  auto *LI = &AM.getResult<LoopAnalysis>(F);
  unsigned N = SplitAllCriticalEdges(F, CriticalEdgeSplittingOptions(DT, LI));
  bool Changed = runPass(F);
  if (N == 0 && !Changed)
    return PreservedAnalyses::all();
  PreservedAnalyses PA;
  PA.preserve<DominatorTreeAnalysis>();
  PA.preserve<LoopAnalysis>();
  return PA;
}

namespace {

class RegToMemWrapperPass : public FunctionPass {
public:
  static char ID;

  RegToMemWrapperPass() : FunctionPass(ID) {}

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();

    AU.addPreserved<DominatorTreeWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();

    AU.addPreserved<LoopInfoWrapperPass>();
    AU.addRequired<LoopInfoWrapperPass>();
  }

  bool runOnFunction(Function &F) override {
    DominatorTree *DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
    LoopInfo *LI = &getAnalysis<LoopInfoWrapperPass>().getLoopInfo();

    unsigned N = SplitAllCriticalEdges(F, CriticalEdgeSplittingOptions(DT, LI));
    bool Changed = runPass(F);
    return N != 0 || Changed;
  }
};
} // namespace

INITIALIZE_PASS_BEGIN(RegToMemWrapperPass, "reg2mem", "", true, true)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass);
INITIALIZE_PASS_DEPENDENCY(LoopInfoWrapperPass);
INITIALIZE_PASS_END(RegToMemWrapperPass, "reg2mem", "", true, true)

char RegToMemWrapperPass::ID = 0;

FunctionPass *toolchain::createRegToMemWrapperPass() {
  return new RegToMemWrapperPass();
}
