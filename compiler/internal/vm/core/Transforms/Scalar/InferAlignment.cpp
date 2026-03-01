//===- InferAlignment.cpp -------------------------------------------------===//
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
// Infer alignment for load, stores and other memory operations based on
// trailing zero known bits information.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Scalar/InferAlignment.h"
#include "vm/core/ADT/APInt.h"
#include "vm/core/ADT/STLFunctionalExtras.h"
#include "vm/core/Analysis/AssumptionCache.h"
#include "vm/core/Analysis/ValueTracking.h"
#include "vm/core/IR/Instruction.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/IntrinsicInst.h"
#include "vm/core/IR/PatternMatch.h"
#include "vm/core/Support/KnownBits.h"
#include "vm/core/Transforms/Scalar.h"
#include "vm/core/Transforms/Utils/Local.h"

using namespace vm::core;
using namespace vm::core::PatternMatch;

static bool tryToImproveAlign(
    const DataLayout &DL, Instruction *I,
    function_ref<Align(Value *PtrOp, Align OldAlign, Align PrefAlign)> Fn) {

  if (auto *PtrOp = getLoadStorePointerOperand(I)) {
    Align OldAlign = getLoadStoreAlignment(I);
    Align PrefAlign = DL.getPrefTypeAlign(getLoadStoreType(I));

    Align NewAlign = Fn(PtrOp, OldAlign, PrefAlign);
    if (NewAlign > OldAlign) {
      setLoadStoreAlignment(I, NewAlign);
      return true;
    }
  }

  Value *PtrOp;
  const APInt *Const;
  if (match(I, m_And(m_PtrToIntOrAddr(m_Value(PtrOp)), m_APInt(Const)))) {
    Align ActualAlign = Fn(PtrOp, Align(1), Align(1));
    if (Const->ult(ActualAlign.value())) {
      I->replaceAllUsesWith(Constant::getNullValue(I->getType()));
      return true;
    }
    if (Const->uge(
            APInt::getBitsSetFrom(Const->getBitWidth(), Log2(ActualAlign)))) {
      I->replaceAllUsesWith(I->getOperand(0));
      return true;
    }
  }

  IntrinsicInst *II = dyn_cast<IntrinsicInst>(I);
  if (!II)
    return false;

  // TODO: Handle more memory intrinsics.
  switch (II->getIntrinsicID()) {
  case Intrinsic::masked_load:
  case Intrinsic::masked_store: {
    unsigned PtrOpIdx = II->getIntrinsicID() == Intrinsic::masked_load ? 0 : 1;
    Value *PtrOp = II->getArgOperand(PtrOpIdx);
    Type *Type = II->getIntrinsicID() == Intrinsic::masked_load
                     ? II->getType()
                     : II->getArgOperand(0)->getType();

    Align OldAlign = II->getParamAlign(PtrOpIdx).valueOrOne();
    Align PrefAlign = DL.getPrefTypeAlign(Type);
    Align NewAlign = Fn(PtrOp, OldAlign, PrefAlign);
    if (NewAlign <= OldAlign)
      return false;

    II->addParamAttr(PtrOpIdx,
                     Attribute::getWithAlignment(II->getContext(), NewAlign));
    return true;
  }
  default:
    return false;
  }
}

bool inferAlignment(Function &F, AssumptionCache &AC, DominatorTree &DT) {
  const DataLayout &DL = F.getDataLayout();
  bool Changed = false;

  // Enforce preferred type alignment if possible. We do this as a separate
  // pass first, because it may improve the alignments we infer below.
  for (BasicBlock &BB : F) {
    for (Instruction &I : BB) {
      Changed |= tryToImproveAlign(
          DL, &I, [&](Value *PtrOp, Align OldAlign, Align PrefAlign) {
            if (PrefAlign > OldAlign)
              return std::max(OldAlign,
                              tryEnforceAlignment(PtrOp, PrefAlign, DL));
            return OldAlign;
          });
    }
  }

  // Compute alignment from known bits.
  auto InferFromKnownBits = [&](Instruction &I, Value *PtrOp) {
    KnownBits Known = computeKnownBits(PtrOp, DL, &AC, &I, &DT);
    unsigned TrailZ =
        std::min(Known.countMinTrailingZeros(), +Value::MaxAlignmentExponent);
    return Align(1ull << std::min(Known.getBitWidth() - 1, TrailZ));
  };

  // Propagate alignment between loads and stores that originate from the
  // same base pointer.
  DenseMap<Value *, Align> BestBasePointerAligns;
  auto InferFromBasePointer = [&](Value *PtrOp, Align LoadStoreAlign) {
    APInt OffsetFromBase(DL.getIndexTypeSizeInBits(PtrOp->getType()), 0);
    PtrOp = PtrOp->stripAndAccumulateConstantOffsets(DL, OffsetFromBase, true);
    // Derive the base pointer alignment from the load/store alignment
    // and the offset from the base pointer.
    Align BasePointerAlign =
        commonAlignment(LoadStoreAlign, OffsetFromBase.getLimitedValue());

    auto [It, Inserted] =
        BestBasePointerAligns.try_emplace(PtrOp, BasePointerAlign);
    if (!Inserted) {
      // If the stored base pointer alignment is better than the
      // base pointer alignment we derived, we may be able to use it
      // to improve the load/store alignment. If not, store the
      // improved base pointer alignment for future iterations.
      if (It->second > BasePointerAlign) {
        Align BetterLoadStoreAlign =
            commonAlignment(It->second, OffsetFromBase.getLimitedValue());
        return BetterLoadStoreAlign;
      }
      It->second = BasePointerAlign;
    }
    return LoadStoreAlign;
  };

  for (BasicBlock &BB : F) {
    // We need to reset the map for each block because alignment information
    // can only be propagated from instruction A to B if A dominates B.
    // This is because control flow (and exception throwing) could be dependent
    // on the address (and its alignment) at runtime. Some sort of dominator
    // tree approach could be better, but doing a simple forward pass through a
    // single basic block is correct too.
    BestBasePointerAligns.clear();

    for (Instruction &I : BB) {
      Changed |= tryToImproveAlign(
          DL, &I, [&](Value *PtrOp, Align OldAlign, Align PrefAlign) {
            return std::max(InferFromKnownBits(I, PtrOp),
                            InferFromBasePointer(PtrOp, OldAlign));
          });
    }
  }

  return Changed;
}

PreservedAnalyses InferAlignmentPass::run(Function &F,
                                          FunctionAnalysisManager &AM) {
  AssumptionCache &AC = AM.getResult<AssumptionAnalysis>(F);
  DominatorTree &DT = AM.getResult<DominatorTreeAnalysis>(F);
  inferAlignment(F, AC, DT);
  // Changes to alignment shouldn't invalidated analyses.
  return PreservedAnalyses::all();
}
