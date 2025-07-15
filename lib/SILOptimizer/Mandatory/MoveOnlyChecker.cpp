//===--- MoveOnlyChecker.cpp ----------------------------------------------===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "sil-move-only-checker"

#include "language/AST/AccessScope.h"
#include "language/AST/DiagnosticEngine.h"
#include "language/AST/DiagnosticsSIL.h"
#include "language/AST/SemanticAttrs.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Debug.h"
#include "language/Basic/Defer.h"
#include "language/Basic/FrozenMultiMap.h"
#include "language/Basic/SmallBitVector.h"
#include "language/SIL/ApplySite.h"
#include "language/SIL/BasicBlockBits.h"
#include "language/SIL/BasicBlockData.h"
#include "language/SIL/BasicBlockDatastructures.h"
#include "language/SIL/BasicBlockUtils.h"
#include "language/SIL/Consumption.h"
#include "language/SIL/DebugUtils.h"
#include "language/SIL/FieldSensitivePrunedLiveness.h"
#include "language/SIL/InstructionUtils.h"
#include "language/SIL/MemAccessUtils.h"
#include "language/SIL/OSSALifetimeCompletion.h"
#include "language/SIL/OwnershipUtils.h"
#include "language/SIL/PrunedLiveness.h"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILArgumentConvention.h"
#include "language/SIL/SILBasicBlock.h"
#include "language/SIL/SILBuilder.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILUndef.h"
#include "language/SIL/SILValue.h"
#include "language/SILOptimizer/Analysis/ClosureScope.h"
#include "language/SILOptimizer/Analysis/DeadEndBlocksAnalysis.h"
#include "language/SILOptimizer/Analysis/DominanceAnalysis.h"
#include "language/SILOptimizer/Analysis/NonLocalAccessBlockAnalysis.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SILOptimizer/Utils/CanonicalizeOSSALifetime.h"
#include "language/SILOptimizer/Utils/InstructionDeleter.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/ADT/MapVector.h"
#include "toolchain/ADT/PointerIntPair.h"
#include "toolchain/ADT/PointerUnion.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/SmallBitVector.h"
#include "toolchain/ADT/SmallPtrSet.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/Support/Debug.h"
#include "toolchain/Support/ErrorHandling.h"

#include "MoveOnlyAddressCheckerUtils.h"
#include "MoveOnlyDiagnostics.h"
#include "MoveOnlyObjectCheckerUtils.h"
#include "MoveOnlyUtils.h"

using namespace language;
using namespace language::siloptimizer;

//===----------------------------------------------------------------------===//
//                     MARK: Top Level Object Entrypoint
//===----------------------------------------------------------------------===//

namespace {

struct MoveOnlyChecker {
  DiagnosticEmitter diagnosticEmitter;
  SILFunction *fn;
  DominanceInfo *domTree;
  PostOrderAnalysis *poa;
  DeadEndBlocksAnalysis *deba;
  bool madeChange = false;
  borrowtodestructure::IntervalMapAllocator allocator;

  MoveOnlyChecker(SILFunction *fn, DominanceInfo *domTree,
                  PostOrderAnalysis *poa, DeadEndBlocksAnalysis *deba)
      : diagnosticEmitter(fn), fn(fn), domTree(domTree), poa(poa), deba(deba) {}

  void checkObjects();
  void completeObjectLifetimes(ArrayRef<MarkUnresolvedNonCopyableValueInst *>);
  void checkAddresses();
};

} // namespace

void MoveOnlyChecker::checkObjects() {
  toolchain::SmallSetVector<MarkUnresolvedNonCopyableValueInst *, 32>
      moveIntroducersToProcess;
  unsigned diagCount = diagnosticEmitter.getDiagnosticCount();
  madeChange |= searchForCandidateObjectMarkUnresolvedNonCopyableValueInsts(
      fn, moveIntroducersToProcess, diagnosticEmitter);

  TOOLCHAIN_DEBUG(
      toolchain::dbgs()
      << "Emitting diagnostic when checking for mark must check inst: "
      << (diagCount != diagnosticEmitter.getDiagnosticCount() ? "yes" : "no")
      << '\n');

  if (moveIntroducersToProcess.empty()) {
    TOOLCHAIN_DEBUG(toolchain::dbgs()
               << "No move introducers found?! Returning early?!\n");
    return;
  }

  completeObjectLifetimes(moveIntroducersToProcess.getArrayRef());

  MoveOnlyObjectChecker checker{diagnosticEmitter, domTree, deba, poa,
                                allocator};
  madeChange |= checker.check(moveIntroducersToProcess);
}

void MoveOnlyChecker::completeObjectLifetimes(
    ArrayRef<MarkUnresolvedNonCopyableValueInst *> insts) {
  // TODO: Delete once OSSALifetimeCompletion is run as part of SILGenCleanup.
  OSSALifetimeCompletion completion(fn, domTree, *deba->get(fn));

  // Collect all values derived from each mark_unresolved_non_copyable_value
  // instruction via ownership instructions and phis.
  ValueWorklist transitiveValues(fn);
  for (auto *inst : insts) {
    transitiveValues.push(inst);
  }
  while (auto value = transitiveValues.pop()) {
    for (auto *use : value->getUses()) {
      auto *user = use->getUser();
      switch (user->getKind()) {
      case SILInstructionKind::BeginBorrowInst:
      case SILInstructionKind::CopyValueInst:
      case SILInstructionKind::MoveValueInst:
        transitiveValues.pushIfNotVisited(cast<SingleValueInstruction>(user));
        break;
      case SILInstructionKind::BranchInst: {
        PhiOperand po(use);
        transitiveValues.pushIfNotVisited(po.getValue());
        break;
      }
      default: {
        auto forward = ForwardingOperation(user);
        if (!forward)
          continue;
        forward.visitForwardedValues([&transitiveValues](auto forwarded) {
          transitiveValues.pushIfNotVisited(forwarded);
          return true;
        });
        break;
      }
      }
    }
  }
  // Complete the lifetime of each collected value.  This is a subset of the
  // work that SILGenCleanup will do.
  for (auto *block : poa->get(fn)->getPostOrder()) {
    for (SILInstruction &inst : reverse(*block)) {
      for (auto result : inst.getResults()) {
        if (toolchain::any_of(result->getUsers(),
                         [](auto *user) { return isa<BranchInst>(user); })) {
          continue;
        }
        if (!transitiveValues.isVisited(result))
          continue;
        if (completion.completeOSSALifetime(
                result, OSSALifetimeCompletion::Boundary::Availability) ==
            LifetimeCompletion::WasCompleted) {
          madeChange = true;
        }
      }
    }
    for (SILArgument *arg : block->getArguments()) {
      if (arg->isReborrow()) {
        continue;
      }
      if (!transitiveValues.isVisited(arg))
        continue;
      if (completion.completeOSSALifetime(
              arg, OSSALifetimeCompletion::Boundary::Availability) ==
          LifetimeCompletion::WasCompleted) {
        madeChange = true;
      }
    }
  }
}

void MoveOnlyChecker::checkAddresses() {
  unsigned diagCount = diagnosticEmitter.getDiagnosticCount();
  toolchain::SmallSetVector<MarkUnresolvedNonCopyableValueInst *, 32>
      moveIntroducersToProcess;
  searchForCandidateAddressMarkUnresolvedNonCopyableValueInsts(
      fn, poa, moveIntroducersToProcess, diagnosticEmitter);

  TOOLCHAIN_DEBUG(
      toolchain::dbgs()
      << "Emitting diagnostic when checking for mark must check inst: "
      << (diagCount != diagnosticEmitter.getDiagnosticCount() ? "yes" : "no")
      << '\n');

  if (moveIntroducersToProcess.empty()) {
    TOOLCHAIN_DEBUG(toolchain::dbgs()
               << "No move introducers found?! Returning early?!\n");
    return;
  }

  MoveOnlyAddressChecker checker{
      fn, diagnosticEmitter, allocator, domTree, poa, deba};
  madeChange |= checker.completeLifetimes();
  madeChange |= checker.check(moveIntroducersToProcess);
}

//===----------------------------------------------------------------------===//
//                         MARK: Top Level Entrypoint
//===----------------------------------------------------------------------===//

namespace {

static bool canonicalizeLoadBorrows(SILFunction *F) {
  bool changed = false;
  for (auto &block : *F) {
    for (auto &inst : block) {
      if (auto *lbi = dyn_cast<LoadBorrowInst>(&inst)) {
        if (lbi->isUnchecked()) {
          changed = true;
          lbi->setUnchecked(false);
        }
      }
    }
  }
  
  return changed;
}

class MoveOnlyCheckerPass : public SILFunctionTransform {
  void run() override {
    auto *fn = getFunction();

    // Don't rerun diagnostics on deserialized functions.
    if (getFunction()->wasDeserializedCanonical())
      return;

    assert(fn->getModule().getStage() == SILStage::Raw &&
           "Should only run on Raw SIL");

    // If an earlier pass told use to not emit diagnostics for this function,
    // clean up any copies, invalidate the analysis, and return early.
    if (fn->hasSemanticsAttr(semantics::NO_MOVEONLY_DIAGNOSTICS)) {
      bool didChange = canonicalizeLoadBorrows(fn);
      didChange |= cleanupNonCopyableCopiesAfterEmittingDiagnostic(getFunction());
      if (didChange) {
        invalidateAnalysis(SILAnalysis::InvalidationKind::Instructions);
      }
      return;
    }

    TOOLCHAIN_DEBUG(toolchain::dbgs()
               << "===> MoveOnly Checker. Visiting: " << fn->getName() << '\n');

    MoveOnlyChecker checker(fn, getAnalysis<DominanceAnalysis>()->get(fn),
                            getAnalysis<PostOrderAnalysis>(),
                            getAnalysis<DeadEndBlocksAnalysis>());

    checker.checkObjects();
    checker.checkAddresses();

    // If we did not emit any diagnostics, emit an error on any copies that
    // remain. If we emitted a diagnostic, we just want to rewrite all of the
    // non-copyable copies into explicit variants below and let the user
    // recompile.
    if (!checker.diagnosticEmitter.emittedDiagnostic()) {
      emitCheckerMissedCopyOfNonCopyableTypeErrors(fn,
                                                   checker.diagnosticEmitter);
    }

    // Remaining borrows
    // should be correctly immutable. We can canonicalize any remaining
    // `load_borrow [unchecked]` instructions.
    checker.madeChange |= canonicalizeLoadBorrows(fn);

    checker.madeChange |=
        cleanupNonCopyableCopiesAfterEmittingDiagnostic(fn);

    if (checker.madeChange)
      invalidateAnalysis(SILAnalysis::InvalidationKind::Instructions);
  }
};

} // namespace

SILTransform *language::createMoveOnlyChecker() {
  return new MoveOnlyCheckerPass();
}
