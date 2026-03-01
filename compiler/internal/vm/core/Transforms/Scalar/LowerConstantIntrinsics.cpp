//===- LowerConstantIntrinsics.cpp - Lower constant intrinsic calls -------===//
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
// This pass lowers all remaining 'objectsize' 'is.constant' intrinsic calls
// and provides constant propagation and basic CFG cleanup on the result.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Scalar/LowerConstantIntrinsics.h"
#include "vm/core/ADT/PostOrderIterator.h"
#include "vm/core/ADT/SetVector.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/Analysis/DomTreeUpdater.h"
#include "vm/core/Analysis/GlobalsModRef.h"
#include "vm/core/Analysis/InstructionSimplify.h"
#include "vm/core/Analysis/MemoryBuiltins.h"
#include "vm/core/Analysis/TargetLibraryInfo.h"
#include "vm/core/IR/BasicBlock.h"
#include "vm/core/IR/Constants.h"
#include "vm/core/IR/Dominators.h"
#include "vm/core/IR/Function.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/IntrinsicInst.h"
#include "vm/core/IR/PatternMatch.h"
#include "vm/core/Pass.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Transforms/Scalar.h"
#include "vm/core/Transforms/Utils/Local.h"
#include <optional>

using namespace vm::core;
using namespace vm::core::PatternMatch;

#define DEBUG_TYPE "lower-is-constant-intrinsic"

STATISTIC(IsConstantIntrinsicsHandled,
          "Number of 'is.constant' intrinsic calls handled");
STATISTIC(ObjectSizeIntrinsicsHandled,
          "Number of 'objectsize' intrinsic calls handled");

static Value *lowerIsConstantIntrinsic(IntrinsicInst *II) {
  if (auto *C = dyn_cast<Constant>(II->getOperand(0)))
    if (C->isManifestConstant())
      return ConstantInt::getTrue(II->getType());
  return ConstantInt::getFalse(II->getType());
}

static bool replaceConditionalBranchesOnConstant(Instruction *II,
                                                 Value *NewValue,
                                                 DomTreeUpdater *DTU) {
  bool HasDeadBlocks = false;
  SmallSetVector<Instruction *, 8> UnsimplifiedUsers;
  replaceAndRecursivelySimplify(II, NewValue, nullptr, nullptr, nullptr,
                                &UnsimplifiedUsers);
  // UnsimplifiedUsers can contain PHI nodes that may be removed when
  // replacing the branch instructions, so use a value handle worklist
  // to handle those possibly removed instructions.
  SmallVector<WeakVH, 8> Worklist(UnsimplifiedUsers.begin(),
                                  UnsimplifiedUsers.end());

  for (auto &VH : Worklist) {
    BranchInst *BI = dyn_cast_or_null<BranchInst>(VH);
    if (!BI)
      continue;
    if (BI->isUnconditional())
      continue;

    BasicBlock *Target, *Other;
    if (match(BI->getOperand(0), m_Zero())) {
      Target = BI->getSuccessor(1);
      Other = BI->getSuccessor(0);
    } else if (match(BI->getOperand(0), m_One())) {
      Target = BI->getSuccessor(0);
      Other = BI->getSuccessor(1);
    } else {
      Target = nullptr;
      Other = nullptr;
    }
    if (Target && Target != Other) {
      BasicBlock *Source = BI->getParent();
      Other->removePredecessor(Source);

      Instruction *NewBI = BranchInst::Create(Target, Source);
      NewBI->setDebugLoc(BI->getDebugLoc());
      BI->eraseFromParent();

      if (DTU)
        DTU->applyUpdates({{DominatorTree::Delete, Source, Other}});
      if (pred_empty(Other))
        HasDeadBlocks = true;
    }
  }
  return HasDeadBlocks;
}

bool toolchain::lowerConstantIntrinsics(Function &F, const TargetLibraryInfo &TLI,
                                   DominatorTree *DT) {
  std::optional<DomTreeUpdater> DTU;
  if (DT)
    DTU.emplace(DT, DomTreeUpdater::UpdateStrategy::Lazy);

  bool HasDeadBlocks = false;
  const auto &DL = F.getDataLayout();
  SmallVector<WeakTrackingVH, 8> Worklist;

  ReversePostOrderTraversal<Function *> RPOT(&F);
  for (BasicBlock *BB : RPOT) {
    for (Instruction &I: *BB) {
      IntrinsicInst *II = dyn_cast<IntrinsicInst>(&I);
      if (!II)
        continue;
      switch (II->getIntrinsicID()) {
      default:
        break;
      case Intrinsic::is_constant:
      case Intrinsic::objectsize:
        Worklist.push_back(WeakTrackingVH(&I));
        break;
      }
    }
  }
  for (WeakTrackingVH &VH: Worklist) {
    // Items on the worklist can be mutated by earlier recursive replaces.
    // This can remove the intrinsic as dead (VH == null), but also replace
    // the intrinsic in place.
    if (!VH)
      continue;
    IntrinsicInst *II = dyn_cast<IntrinsicInst>(&*VH);
    if (!II)
      continue;
    Value *NewValue;
    switch (II->getIntrinsicID()) {
    default:
      continue;
    case Intrinsic::is_constant:
      NewValue = lowerIsConstantIntrinsic(II);
      LLVM_DEBUG(dbgs() << "Folding " << *II << " to " << *NewValue << "\n");
      IsConstantIntrinsicsHandled++;
      break;
    case Intrinsic::objectsize:
      NewValue = lowerObjectSizeCall(II, DL, &TLI, true);
      LLVM_DEBUG(dbgs() << "Folding " << *II << " to " << *NewValue << "\n");
      ObjectSizeIntrinsicsHandled++;
      break;
    }
    HasDeadBlocks |= replaceConditionalBranchesOnConstant(
        II, NewValue, DTU ? &*DTU : nullptr);
  }
  if (HasDeadBlocks)
    removeUnreachableBlocks(F, DTU ? &*DTU : nullptr);
  return !Worklist.empty();
}

PreservedAnalyses
LowerConstantIntrinsicsPass::run(Function &F, FunctionAnalysisManager &AM) {
  if (lowerConstantIntrinsics(F, AM.getResult<TargetLibraryAnalysis>(F),
                              AM.getCachedResult<DominatorTreeAnalysis>(F))) {
    PreservedAnalyses PA;
    PA.preserve<DominatorTreeAnalysis>();
    return PA;
  }

  return PreservedAnalyses::all();
}
