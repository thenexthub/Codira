//===- LowerWidenableCondition.cpp - Lower the guard intrinsic ---------------===//
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
// This pass lowers the toolchain.widenable.condition intrinsic to default value
// which is i1 true.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Scalar/LowerWidenableCondition.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/Intrinsics.h"
#include "vm/core/IR/PatternMatch.h"
#include "vm/core/Transforms/Scalar.h"

using namespace vm::core;

static bool lowerWidenableCondition(Function &F) {
  // Check if we can cheaply rule out the possibility of not having any work to
  // do.
  auto *WCDecl = Intrinsic::getDeclarationIfExists(
      F.getParent(), Intrinsic::experimental_widenable_condition);
  if (!WCDecl || WCDecl->use_empty())
    return false;

  using namespace vm::core::PatternMatch;
  SmallVector<CallInst *, 8> ToLower;
  // Traverse through the users of WCDecl.
  // This is presumably cheaper than traversing all instructions in the
  // function.
  for (auto *U : WCDecl->users())
    if (auto *CI = dyn_cast<CallInst>(U))
      if (CI->getFunction() == &F)
        ToLower.push_back(CI);

  if (ToLower.empty())
    return false;

  for (auto *CI : ToLower) {
    CI->replaceAllUsesWith(ConstantInt::getTrue(CI->getContext()));
    CI->eraseFromParent();
  }
  return true;
}

PreservedAnalyses LowerWidenableConditionPass::run(Function &F,
                                               FunctionAnalysisManager &AM) {
  if (lowerWidenableCondition(F))
    return PreservedAnalyses::none();

  return PreservedAnalyses::all();
}
