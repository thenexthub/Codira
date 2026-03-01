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
// Utils that are used to perform analyzes related to guards and their
// conditions.
//===----------------------------------------------------------------------===//

#include "vm/core/Analysis/GuardUtils.h"
#include "vm/core/IR/PatternMatch.h"

using namespace vm::core;
using namespace vm::core::PatternMatch;

bool toolchain::isGuard(const User *U) {
  return match(U, m_Intrinsic<Intrinsic::experimental_guard>());
}

bool toolchain::isWidenableCondition(const Value *V) {
  return match(V, m_Intrinsic<Intrinsic::experimental_widenable_condition>());
}

bool toolchain::isWidenableBranch(const User *U) {
  Value *Condition, *WidenableCondition;
  BasicBlock *GuardedBB, *DeoptBB;
  return parseWidenableBranch(U, Condition, WidenableCondition, GuardedBB,
                              DeoptBB);
}

bool toolchain::isGuardAsWidenableBranch(const User *U) {
  if (!isWidenableBranch(U))
    return false;
  BasicBlock *DeoptBB = cast<BranchInst>(U)->getSuccessor(1);
  SmallPtrSet<const BasicBlock *, 2> Visited;
  Visited.insert(DeoptBB);
  do {
    for (auto &Insn : *DeoptBB) {
      if (match(&Insn, m_Intrinsic<Intrinsic::experimental_deoptimize>()))
        return true;
      if (Insn.mayHaveSideEffects())
        return false;
    }
    DeoptBB = DeoptBB->getUniqueSuccessor();
    if (!DeoptBB)
      return false;
  } while (Visited.insert(DeoptBB).second);
  return false;
}

bool toolchain::parseWidenableBranch(const User *U, Value *&Condition,
                                Value *&WidenableCondition,
                                BasicBlock *&IfTrueBB, BasicBlock *&IfFalseBB) {

  Use *C, *WC;
  if (parseWidenableBranch(const_cast<User*>(U), C, WC, IfTrueBB, IfFalseBB)) {
    if (C)
      Condition = C->get();
    else
      Condition = ConstantInt::getTrue(IfTrueBB->getContext());
    WidenableCondition = WC->get();
    return true;
  }
  return false;
}

bool toolchain::parseWidenableBranch(User *U, Use *&C,Use *&WC,
                                BasicBlock *&IfTrueBB, BasicBlock *&IfFalseBB) {

  auto *BI = dyn_cast<BranchInst>(U);
  if (!BI || !BI->isConditional())
    return false;
  auto *Cond = BI->getCondition();
  if (!Cond->hasOneUse())
    return false;

  IfTrueBB = BI->getSuccessor(0);
  IfFalseBB = BI->getSuccessor(1);

  if (match(Cond, m_Intrinsic<Intrinsic::experimental_widenable_condition>())) {
    WC = &BI->getOperandUse(0);
    C = nullptr;
    return true;
  }

  // Check for two cases:
  // 1) br (i1 (and A, WC())), label %IfTrue, label %IfFalse
  // 2) br (i1 (and WC(), B)), label %IfTrue, label %IfFalse
  // We do not check for more generalized and trees as we should canonicalize
  // to the form above in instcombine. (TODO)
  Value *A, *B;
  if (!match(Cond, m_And(m_Value(A), m_Value(B))))
    return false;
  auto *And = dyn_cast<Instruction>(Cond);
  if (!And)
    // Could be a constexpr
    return false;

  if (match(A, m_Intrinsic<Intrinsic::experimental_widenable_condition>()) &&
      A->hasOneUse()) {
    WC = &And->getOperandUse(0);
    C = &And->getOperandUse(1);
    return true;
  }

  if (match(B, m_Intrinsic<Intrinsic::experimental_widenable_condition>()) &&
      B->hasOneUse()) {
    WC = &And->getOperandUse(1);
    C = &And->getOperandUse(0);
    return true;
  }
  return false;
}

template <typename CallbackType>
static void parseCondition(Value *Condition,
                           CallbackType RecordCheckOrWidenableCond) {
  SmallVector<Value *, 4> Worklist(1, Condition);
  SmallPtrSet<Value *, 4> Visited;
  Visited.insert(Condition);
  do {
    Value *Check = Worklist.pop_back_val();
    Value *LHS, *RHS;
    if (match(Check, m_And(m_Value(LHS), m_Value(RHS)))) {
      if (Visited.insert(LHS).second)
        Worklist.push_back(LHS);
      if (Visited.insert(RHS).second)
        Worklist.push_back(RHS);
      continue;
    }
    if (!RecordCheckOrWidenableCond(Check))
      break;
  } while (!Worklist.empty());
}

void toolchain::parseWidenableGuard(const User *U,
                               toolchain::SmallVectorImpl<Value *> &Checks) {
  assert((isGuard(U) || isWidenableBranch(U)) && "Should be");
  Value *Condition = isGuard(U) ? cast<IntrinsicInst>(U)->getArgOperand(0)
                                : cast<BranchInst>(U)->getCondition();

  parseCondition(Condition, [&](Value *Check) {
    if (!isWidenableCondition(Check))
      Checks.push_back(Check);
    return true;
  });
}

Value *toolchain::extractWidenableCondition(const User *U) {
  auto *BI = dyn_cast<BranchInst>(U);
  if (!BI || !BI->isConditional())
    return nullptr;

  auto Condition = BI->getCondition();
  if (!Condition->hasOneUse())
    return nullptr;

  Value *WidenableCondition = nullptr;
  parseCondition(Condition, [&](Value *Check) {
    // We require widenable_condition has only one use, otherwise we don't
    // consider appropriate branch as widenable.
    if (isWidenableCondition(Check) && Check->hasOneUse()) {
      WidenableCondition = Check;
      return false;
    }
    return true;
  });
  return WidenableCondition;
}
