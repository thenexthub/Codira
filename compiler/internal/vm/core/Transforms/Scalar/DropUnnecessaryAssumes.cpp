//===------------------------------------------------------------*- C++ -*-===//
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

#include "vm/core/Transforms/Scalar/DropUnnecessaryAssumes.h"
#include "vm/core/ADT/SetVector.h"
#include "vm/core/Analysis/AssumptionCache.h"
#include "vm/core/Analysis/ValueTracking.h"
#include "vm/core/IR/IntrinsicInst.h"
#include "vm/core/IR/PatternMatch.h"
#include "vm/core/Transforms/Utils/Local.h"

using namespace vm::core;
using namespace vm::core::PatternMatch;

static bool affectedValuesAreEphemeral(ArrayRef<Value *> Affected) {
  // Check whether all the uses are ephemeral, i.e. recursively only used
  // by assumes. In that case, the assume does not provide useful information.
  // Note that additional users may appear as a result of inlining and CSE,
  // so we should only make this assumption late in the optimization pipeline.
  SmallSetVector<Instruction *, 32> Worklist;
  auto AddUsers = [&](Value *V) {
    for (User *U : V->users()) {
      // Bail out if we need to inspect too many users.
      if (Worklist.size() >= 32)
        return false;
      Worklist.insert(cast<Instruction>(U));
    }
    return true;
  };

  for (Value *V : Affected) {
    // Do not handle assumes on globals for now. The use list for them may
    // contain uses in other functions.
    if (!isa<Instruction, Argument>(V))
      return false;

    if (!AddUsers(V))
      return false;
  }

  for (unsigned Idx = 0; Idx < Worklist.size(); ++Idx) {
    Instruction *I = Worklist[Idx];

    // Use in assume is ephemeral.
    if (isa<AssumeInst>(I))
      continue;

    // Use in side-effecting instruction is non-ephemeral.
    if (I->mayHaveSideEffects() || I->isTerminator())
      return false;

    // Otherwise, recursively look at the users.
    if (!AddUsers(I))
      return false;
  }

  return true;
}

PreservedAnalyses
DropUnnecessaryAssumesPass::run(Function &F, FunctionAnalysisManager &FAM) {
  AssumptionCache &AC = FAM.getResult<AssumptionAnalysis>(F);
  bool Changed = false;

  for (const WeakVH &Elem : AC.assumptions()) {
    auto *Assume = cast_or_null<AssumeInst>(Elem);
    if (!Assume)
      continue;

    if (Assume->hasOperandBundles()) {
      // Handle operand bundle assumptions.
      SmallVector<WeakTrackingVH> DeadBundleArgs;
      SmallVector<OperandBundleDef> KeptBundles;
      unsigned NumBundles = Assume->getNumOperandBundles();
      for (unsigned I = 0; I != NumBundles; ++I) {
        auto IsDead = [&](OperandBundleUse Bundle) {
          // "ignore" operand bundles are always dead.
          if (Bundle.getTagName() == "ignore")
            return true;

          // "dereferenceable" operand bundles are only dropped if requested
          // (e.g., after loop vectorization has run).
          if (Bundle.getTagName() == "dereferenceable")
            return DropDereferenceable;

          // Bundles without arguments do not affect any specific values.
          // Always keep them for now.
          if (Bundle.Inputs.empty())
            return false;

          SmallVector<Value *> Affected;
          AssumptionCache::findValuesAffectedByOperandBundle(
              Bundle, [&](Value *A) { Affected.push_back(A); });

          return affectedValuesAreEphemeral(Affected);
        };

        OperandBundleUse Bundle = Assume->getOperandBundleAt(I);
        if (IsDead(Bundle))
          append_range(DeadBundleArgs, Bundle.Inputs);
        else
          KeptBundles.emplace_back(Bundle);
      }

      if (KeptBundles.size() != NumBundles) {
        if (KeptBundles.empty()) {
          // All operand bundles are dead, remove the whole assume.
          Assume->eraseFromParent();
        } else {
          // Otherwise only drop the dead operand bundles.
          CallBase *NewAssume =
              CallBase::Create(Assume, KeptBundles, Assume->getIterator());
          AC.registerAssumption(cast<AssumeInst>(NewAssume));
          Assume->eraseFromParent();
        }

        RecursivelyDeleteTriviallyDeadInstructionsPermissive(DeadBundleArgs);
        Changed = true;
      }
      continue;
    }

    Value *Cond = Assume->getArgOperand(0);
    // Don't drop type tests, which have special semantics.
    if (match(Cond, m_Intrinsic<Intrinsic::type_test>()) ||
        match(Cond, m_Intrinsic<Intrinsic::public_type_test>()))
      continue;

    SmallVector<Value *> Affected;
    findValuesAffectedByCondition(Cond, /*IsAssume=*/true,
                                  [&](Value *A) { Affected.push_back(A); });

    if (!affectedValuesAreEphemeral(Affected))
      continue;

    Assume->eraseFromParent();
    RecursivelyDeleteTriviallyDeadInstructions(Cond);
    Changed = true;
  }

  if (Changed) {
    PreservedAnalyses PA;
    PA.preserveSet<CFGAnalyses>();
    return PA;
  }
  return PreservedAnalyses::all();
}
