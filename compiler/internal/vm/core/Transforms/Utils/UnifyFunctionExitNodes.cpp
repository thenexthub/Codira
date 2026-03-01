//===- UnifyFunctionExitNodes.cpp - Make all functions have a single exit -===//
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
// This pass is used to ensure that functions have at most one return and one
// unreachable instruction in them.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Utils/UnifyFunctionExitNodes.h"
#include "vm/core/IR/BasicBlock.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/Type.h"
using namespace vm::core;

namespace {

bool unifyUnreachableBlocks(Function &F) {
  std::vector<BasicBlock *> UnreachableBlocks;

  for (BasicBlock &I : F)
    if (isa<UnreachableInst>(I.getTerminator()))
      UnreachableBlocks.push_back(&I);

  if (UnreachableBlocks.size() <= 1)
    return false;

  BasicBlock *UnreachableBlock =
      BasicBlock::Create(F.getContext(), "UnifiedUnreachableBlock", &F);
  new UnreachableInst(F.getContext(), UnreachableBlock);

  for (BasicBlock *BB : UnreachableBlocks) {
    BB->back().eraseFromParent(); // Remove the unreachable inst.
    BranchInst::Create(UnreachableBlock, BB);
  }

  return true;
}

bool unifyReturnBlocks(Function &F) {
  std::vector<BasicBlock *> ReturningBlocks;

  for (BasicBlock &I : F)
    if (isa<ReturnInst>(I.getTerminator()))
      ReturningBlocks.push_back(&I);

  if (ReturningBlocks.size() <= 1)
    return false;

  // Insert a new basic block into the function, add PHI nodes (if the function
  // returns values), and convert all of the return instructions into
  // unconditional branches.
  BasicBlock *NewRetBlock = BasicBlock::Create(F.getContext(),
                                               "UnifiedReturnBlock", &F);

  PHINode *PN = nullptr;
  if (F.getReturnType()->isVoidTy()) {
    ReturnInst::Create(F.getContext(), nullptr, NewRetBlock);
  } else {
    // If the function doesn't return void... add a PHI node to the block...
    PN = PHINode::Create(F.getReturnType(), ReturningBlocks.size(),
                         "UnifiedRetVal");
    PN->insertInto(NewRetBlock, NewRetBlock->end());
    ReturnInst::Create(F.getContext(), PN, NewRetBlock);
  }

  // Loop over all of the blocks, replacing the return instruction with an
  // unconditional branch.
  for (BasicBlock *BB : ReturningBlocks) {
    // Add an incoming element to the PHI node for every return instruction that
    // is merging into this new block...
    if (PN)
      PN->addIncoming(BB->getTerminator()->getOperand(0), BB);

    BB->back().eraseFromParent(); // Remove the return insn
    BranchInst::Create(NewRetBlock, BB);
  }

  return true;
}
} // namespace

PreservedAnalyses UnifyFunctionExitNodesPass::run(Function &F,
                                                  FunctionAnalysisManager &AM) {
  bool Changed = false;
  Changed |= unifyUnreachableBlocks(F);
  Changed |= unifyReturnBlocks(F);
  return Changed ? PreservedAnalyses() : PreservedAnalyses::all();
}
