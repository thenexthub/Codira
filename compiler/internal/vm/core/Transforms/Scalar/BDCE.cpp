//===---- BDCE.cpp - Bit-tracking dead code elimination -------------------===//
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
// This file implements the Bit-Tracking Dead Code Elimination pass. Some
// instructions (shifts, some ands, ors, etc.) kill some of their input bits.
// We track these dead bits and remove instructions that compute only these
// dead bits. We also simplify sext that generates unused extension bits,
// converting it to a zext.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Transforms/Scalar/BDCE.h"
#include "vm/core/ADT/SmallPtrSet.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/Statistic.h"
#include "vm/core/Analysis/DemandedBits.h"
#include "vm/core/Analysis/GlobalsModRef.h"
#include "vm/core/IR/IRBuilder.h"
#include "vm/core/IR/InstIterator.h"
#include "vm/core/IR/Instructions.h"
#include "vm/core/IR/PatternMatch.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/raw_ostream.h"
#include "vm/core/Transforms/Utils/Local.h"

using namespace vm::core;
using namespace PatternMatch;

#define DEBUG_TYPE "bdce"

STATISTIC(NumRemoved, "Number of instructions removed (unused)");
STATISTIC(NumSimplified, "Number of instructions trivialized (dead bits)");
STATISTIC(NumSExt2ZExt,
          "Number of sign extension instructions converted to zero extension");

/// If an instruction is trivialized (dead), then the chain of users of that
/// instruction may need to be cleared of assumptions that can no longer be
/// guaranteed correct.
static void clearAssumptionsOfUsers(Instruction *I, DemandedBits &DB) {
  assert(I->getType()->isIntOrIntVectorTy() &&
         "Trivializing a non-integer value?");

  // If all bits of a user are demanded, then we know that nothing below that
  // in the def-use chain needs to be changed.
  if (DB.getDemandedBits(I).isAllOnes())
    return;

  // Initialize the worklist with eligible direct users.
  SmallPtrSet<Instruction *, 16> Visited;
  SmallVector<Instruction *, 16> WorkList;
  for (User *JU : I->users()) {
    auto *J = cast<Instruction>(JU);
    if (J->getType()->isIntOrIntVectorTy()) {
      Visited.insert(J);
      WorkList.push_back(J);
    }

    // Note that we need to check for non-int types above before asking for
    // demanded bits. Normally, the only way to reach an instruction with an
    // non-int type is via an instruction that has side effects (or otherwise
    // will demand its input bits). However, if we have a readnone function
    // that returns an unsized type (e.g., void), we must avoid asking for the
    // demanded bits of the function call's return value. A void-returning
    // readnone function is always dead (and so we can stop walking the use/def
    // chain here), but the check is necessary to avoid asserting.
  }

  // DFS through subsequent users while tracking visits to avoid cycles.
  while (!WorkList.empty()) {
    Instruction *J = WorkList.pop_back_val();

    // NSW, NUW, and exact are based on operands that might have changed.
    J->dropPoisonGeneratingAnnotations();

    // We do not have to worry about toolchain.assume, because it demands its
    // operand, so trivializing can't change it.

    // If all bits of a user are demanded, then we know that nothing below
    // that in the def-use chain needs to be changed.
    if (DB.getDemandedBits(J).isAllOnes())
      continue;

    for (User *KU : J->users()) {
      auto *K = cast<Instruction>(KU);
      if (Visited.insert(K).second && K->getType()->isIntOrIntVectorTy())
        WorkList.push_back(K);
    }
  }
}

static bool bitTrackingDCE(Function &F, DemandedBits &DB) {
  SmallVector<Instruction*, 128> Worklist;
  bool Changed = false;
  for (Instruction &I : instructions(F)) {
    // If the instruction has side effects and no non-dbg uses,
    // skip it. This way we avoid computing known bits on an instruction
    // that will not help us.
    if (I.mayHaveSideEffects() && I.use_empty())
      continue;

    // Remove instructions that are dead, either because they were not reached
    // during analysis or have no demanded bits.
    if (DB.isInstructionDead(&I) ||
        (I.getType()->isIntOrIntVectorTy() && DB.getDemandedBits(&I).isZero() &&
         wouldInstructionBeTriviallyDead(&I))) {
      Worklist.push_back(&I);
      Changed = true;
      continue;
    }

    // Convert SExt into ZExt if none of the extension bits is required
    if (SExtInst *SE = dyn_cast<SExtInst>(&I)) {
      APInt Demanded = DB.getDemandedBits(SE);
      const uint32_t SrcBitSize = SE->getSrcTy()->getScalarSizeInBits();
      auto *const DstTy = SE->getDestTy();
      const uint32_t DestBitSize = DstTy->getScalarSizeInBits();
      if (Demanded.countl_zero() >= (DestBitSize - SrcBitSize)) {
        clearAssumptionsOfUsers(SE, DB);
        IRBuilder<> Builder(SE);
        I.replaceAllUsesWith(
            Builder.CreateZExt(SE->getOperand(0), DstTy, SE->getName()));
        Worklist.push_back(SE);
        Changed = true;
        NumSExt2ZExt++;
        continue;
      }
    }

    // Simplify and, or, xor when their mask does not affect the demanded bits.
    if (auto *BO = dyn_cast<BinaryOperator>(&I)) {
      APInt Demanded = DB.getDemandedBits(BO);
      if (!Demanded.isAllOnes()) {
        const APInt *Mask;
        if (match(BO->getOperand(1), m_APInt(Mask))) {
          bool CanBeSimplified = false;
          switch (BO->getOpcode()) {
          case Instruction::Or:
          case Instruction::Xor:
            CanBeSimplified = !Demanded.intersects(*Mask);
            break;
          case Instruction::And:
            CanBeSimplified = Demanded.isSubsetOf(*Mask);
            break;
          default:
            // TODO: Handle more cases here.
            break;
          }

          if (CanBeSimplified) {
            clearAssumptionsOfUsers(BO, DB);
            BO->replaceAllUsesWith(BO->getOperand(0));
            Worklist.push_back(BO);
            ++NumSimplified;
            Changed = true;
            continue;
          }
        }
      }
    }

    for (Use &U : I.operands()) {
      // DemandedBits only detects dead integer uses.
      if (!U->getType()->isIntOrIntVectorTy())
        continue;

      if (!isa<Instruction>(U) && !isa<Argument>(U))
        continue;

      if (!DB.isUseDead(&U))
        continue;

      LLVM_DEBUG(dbgs() << "BDCE: Trivializing: " << U << " (all bits dead)\n");

      clearAssumptionsOfUsers(&I, DB);

      // Substitute all uses with zero. In theory we could use `freeze poison`
      // instead, but that seems unlikely to be profitable.
      U.set(ConstantInt::get(U->getType(), 0));
      ++NumSimplified;
      Changed = true;
    }
  }

  for (Instruction *&I : toolchain::reverse(Worklist)) {
    salvageDebugInfo(*I);
    I->dropAllReferences();
  }

  for (Instruction *&I : Worklist) {
    ++NumRemoved;
    I->eraseFromParent();
  }

  return Changed;
}

PreservedAnalyses BDCEPass::run(Function &F, FunctionAnalysisManager &AM) {
  auto &DB = AM.getResult<DemandedBitsAnalysis>(F);
  if (!bitTrackingDCE(F, DB))
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserveSet<CFGAnalyses>();
  return PA;
}
