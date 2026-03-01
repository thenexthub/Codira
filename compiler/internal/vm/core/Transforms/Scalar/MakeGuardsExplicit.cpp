//===- MakeGuardsExplicit.cpp - Turn guard intrinsics into guard branches -===//
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
// This pass lowers the @toolchain.experimental.guard intrinsic to the new form of
// guard represented as widenable explicit branch to the deopt block. The
// difference between this pass and LowerGuardIntrinsic is that after this pass
// the guard represented as intrinsic:
//
//   call void(i1, ...) @toolchain.experimental.guard(i1 %old_cond) [ "deopt"() ]
//
// transforms to a guard represented as widenable explicit branch:
//
//   %widenable_cond = call i1 @toolchain.experimental.widenable.condition()
//   br i1 (%old_cond & %widenable_cond), label %guarded, label %deopt
//
// Here:
//   - The semantics of @toolchain.experimental.widenable.condition allows to replace
//     %widenable_cond with the construction (%widenable_cond & %any_other_cond)
//     without loss of correctness;
//   - %guarded is the lower part of old guard intrinsic's parent block split by
//     the intrinsic call;
//   - %deopt is a block containing a sole call to @toolchain.experimental.deoptimize
//     intrinsic.
//
// Therefore, this branch preserves the property of widenability.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Scalar/MakeGuardsExplicit.h"
#include "vm/core/Analysis/GuardUtils.h"
#include "vm/core/IR/InstIterator.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/Intrinsics.h"
#include "vm/core/Pass.h"
#include "vm/core/Transforms/Utils/GuardUtils.h"

using namespace vm::core;

static void turnToExplicitForm(CallInst *Guard, Function *DeoptIntrinsic) {
  // Replace the guard with an explicit branch (just like in GuardWidening).
  BasicBlock *OriginalBB = Guard->getParent();
  (void)OriginalBB;
  makeGuardControlFlowExplicit(DeoptIntrinsic, Guard, true);
  assert(isWidenableBranch(OriginalBB->getTerminator()) && "should hold");

  Guard->eraseFromParent();
}

static bool explicifyGuards(Function &F) {
  // Check if we can cheaply rule out the possibility of not having any work to
  // do.
  auto *GuardDecl = Intrinsic::getDeclarationIfExists(
      F.getParent(), Intrinsic::experimental_guard);
  if (!GuardDecl || GuardDecl->use_empty())
    return false;

  SmallVector<CallInst *, 8> GuardIntrinsics;
  for (auto &I : instructions(F))
    if (isGuard(&I))
      GuardIntrinsics.push_back(cast<CallInst>(&I));

  if (GuardIntrinsics.empty())
    return false;

  auto *DeoptIntrinsic = Intrinsic::getOrInsertDeclaration(
      F.getParent(), Intrinsic::experimental_deoptimize, {F.getReturnType()});
  DeoptIntrinsic->setCallingConv(GuardDecl->getCallingConv());

  for (auto *Guard : GuardIntrinsics)
    turnToExplicitForm(Guard, DeoptIntrinsic);

  return true;
}

PreservedAnalyses MakeGuardsExplicitPass::run(Function &F,
                                           FunctionAnalysisManager &) {
  if (explicifyGuards(F))
    return PreservedAnalyses::none();
  return PreservedAnalyses::all();
}
