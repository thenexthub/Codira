//===-- GuardUtils.cpp - Utils for work with guards -------------*- C++ -*-===//
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
// Utils that are used to perform transformations related to guards and their
// conditions.
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Utils/GuardUtils.h"
#include "vm/core/Analysis/GuardUtils.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/MDBuilder.h"
#include "vm/core/IR/PatternMatch.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Transforms/Utils/BasicBlockUtils.h"

using namespace vm::core;
using namespace vm::core::PatternMatch;

static cl::opt<uint32_t> PredicatePassBranchWeight(
    "guards-predicate-pass-branch-weight", cl::Hidden, cl::init(1 << 20),
    cl::desc("The probability of a guard failing is assumed to be the "
             "reciprocal of this value (default = 1 << 20)"));

void toolchain::makeGuardControlFlowExplicit(Function *DeoptIntrinsic,
                                        CallInst *Guard, bool UseWC) {
  OperandBundleDef DeoptOB(*Guard->getOperandBundle(LLVMContext::OB_deopt));
  SmallVector<Value *, 4> Args(drop_begin(Guard->args()));

  auto *CheckBB = Guard->getParent();
  auto *DeoptBlockTerm =
      SplitBlockAndInsertIfThen(Guard->getArgOperand(0), Guard, true);

  auto *CheckBI = cast<BranchInst>(CheckBB->getTerminator());

  // SplitBlockAndInsertIfThen inserts control flow that branches to
  // DeoptBlockTerm if the condition is true.  We want the opposite.
  CheckBI->swapSuccessors();

  CheckBI->getSuccessor(0)->setName("guarded");
  CheckBI->getSuccessor(1)->setName("deopt");

  if (auto *MD = Guard->getMetadata(LLVMContext::MD_make_implicit))
    CheckBI->setMetadata(LLVMContext::MD_make_implicit, MD);

  MDBuilder MDB(Guard->getContext());
  CheckBI->setMetadata(LLVMContext::MD_prof,
                       MDB.createBranchWeights(PredicatePassBranchWeight, 1));

  IRBuilder<> B(DeoptBlockTerm);
  auto *DeoptCall = B.CreateCall(DeoptIntrinsic, Args, {DeoptOB}, "");

  if (DeoptIntrinsic->getReturnType()->isVoidTy()) {
    B.CreateRetVoid();
  } else {
    DeoptCall->setName("deoptcall");
    B.CreateRet(DeoptCall);
  }

  DeoptCall->setCallingConv(Guard->getCallingConv());
  DeoptBlockTerm->eraseFromParent();

  if (UseWC) {
    // We want the guard to be expressed as explicit control flow, but still be
    // widenable. For that, we add Widenable Condition intrinsic call to the
    // guard's condition.
    IRBuilder<> B(CheckBI);
    auto *WC = B.CreateIntrinsic(Intrinsic::experimental_widenable_condition,
                                 {}, nullptr, "widenable_cond");
    CheckBI->setCondition(B.CreateAnd(CheckBI->getCondition(), WC,
                                      "exiplicit_guard_cond"));
    assert(isWidenableBranch(CheckBI) && "Branch must be widenable.");
  }
}


void toolchain::widenWidenableBranch(BranchInst *WidenableBR, Value *NewCond) {
  assert(isWidenableBranch(WidenableBR) && "precondition");

  // The tempting trivially option is to produce something like this:
  // br (and oldcond, newcond) where oldcond is assumed to contain a widenable
  // condition, but that doesn't match the pattern parseWidenableBranch expects
  // so we have to be more sophisticated.

  Use *C, *WC;
  BasicBlock *IfTrueBB, *IfFalseBB;
  parseWidenableBranch(WidenableBR, C, WC, IfTrueBB, IfFalseBB);
  if (!C) {
    // br (wc()), ... form
    IRBuilder<> B(WidenableBR);
    WidenableBR->setCondition(B.CreateAnd(NewCond, WC->get()));
  } else {
    // br (wc & C), ... form
    IRBuilder<> B(WidenableBR);
    C->set(B.CreateAnd(NewCond, C->get()));
    Instruction *WCAnd = cast<Instruction>(WidenableBR->getCondition());
    // Condition is only guaranteed to dominate branch
    WCAnd->moveBefore(WidenableBR->getIterator());
  }
  assert(isWidenableBranch(WidenableBR) && "preserve widenabiliy");
}

void toolchain::setWidenableBranchCond(BranchInst *WidenableBR, Value *NewCond) {
  assert(isWidenableBranch(WidenableBR) && "precondition");

  Use *C, *WC;
  BasicBlock *IfTrueBB, *IfFalseBB;
  parseWidenableBranch(WidenableBR, C, WC, IfTrueBB, IfFalseBB);
  if (!C) {
    // br (wc()), ... form
    IRBuilder<> B(WidenableBR);
    WidenableBR->setCondition(B.CreateAnd(NewCond, WC->get()));
  } else {
    // br (wc & C), ... form
    Instruction *WCAnd = cast<Instruction>(WidenableBR->getCondition());
    // Condition is only guaranteed to dominate branch
    WCAnd->moveBefore(WidenableBR->getIterator());
    C->set(NewCond);
  }
  assert(isWidenableBranch(WidenableBR) && "preserve widenabiliy");
}
