//===--- MoveOnlyObjectCheckerTester.cpp ----------------------------------===//
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

#include "language/AST/DiagnosticsSIL.h"
#include "language/AST/TypeCheckRequests.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Defer.h"
#include "language/Basic/FrozenMultiMap.h"
#include "language/Basic/STLExtras.h"
#include "language/SIL/BasicBlockBits.h"
#include "language/SIL/BasicBlockUtils.h"
#include "language/SIL/DebugUtils.h"
#include "language/SIL/FieldSensitivePrunedLiveness.h"
#include "language/SIL/InstructionUtils.h"
#include "language/SIL/NodeBits.h"
#include "language/SIL/OwnershipUtils.h"
#include "language/SIL/PostOrder.h"
#include "language/SIL/PrunedLiveness.h"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILBuilder.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILLocation.h"
#include "language/SIL/SILUndef.h"
#include "language/SIL/SILValue.h"
#include "language/SIL/StackList.h"
#include "language/SILOptimizer/Analysis/ClosureScope.h"
#include "language/SILOptimizer/Analysis/DeadEndBlocksAnalysis.h"
#include "language/SILOptimizer/Analysis/DominanceAnalysis.h"
#include "language/SILOptimizer/Analysis/NonLocalAccessBlockAnalysis.h"
#include "language/SILOptimizer/Analysis/PostOrderAnalysis.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SILOptimizer/Utils/CFGOptUtils.h"
#include "language/SILOptimizer/Utils/CanonicalizeOSSALifetime.h"
#include "language/SILOptimizer/Utils/InstructionDeleter.h"
#include "language/SILOptimizer/Utils/SILSSAUpdater.h"
#include "clang/AST/DeclTemplate.h"
#include "toolchain/ADT/ArrayRef.h"
#include "toolchain/ADT/IntervalMap.h"
#include "toolchain/ADT/PointerIntPair.h"
#include "toolchain/ADT/PointerUnion.h"
#include "toolchain/ADT/STLExtras.h"
#include "toolchain/ADT/SmallBitVector.h"
#include "toolchain/ADT/SmallVector.h"
#include "toolchain/Support/Allocator.h"
#include "toolchain/Support/Debug.h"
#include "toolchain/Support/ErrorHandling.h"
#include "toolchain/Support/RecyclingAllocator.h"

#include "MoveOnlyBorrowToDestructureUtils.h"
#include "MoveOnlyDiagnostics.h"
#include "MoveOnlyObjectCheckerUtils.h"
#include "MoveOnlyUtils.h"

using namespace language;
using namespace language::siloptimizer;

//===----------------------------------------------------------------------===//
//                         MARK: Top Level Entrypoint
//===----------------------------------------------------------------------===//

namespace {

class MoveOnlyObjectCheckerTesterPass : public SILFunctionTransform {
  void run() override {
    auto *fn = getFunction();

    // Don't rerun diagnostics on deserialized functions.
    if (getFunction()->wasDeserializedCanonical())
      return;

    assert(fn->getModule().getStage() == SILStage::Raw &&
           "Should only run on Raw SIL");

    TOOLCHAIN_DEBUG(toolchain::dbgs() << "===> MoveOnly Object Checker. Visiting: "
                            << fn->getName() << '\n');

    auto *dominanceAnalysis = getAnalysis<DominanceAnalysis>();
    DominanceInfo *domTree = dominanceAnalysis->get(fn);
    auto *poa = getAnalysis<PostOrderAnalysis>();
    auto *deba = getAnalysis<DeadEndBlocksAnalysis>();

    DiagnosticEmitter diagnosticEmitter(fn);
    borrowtodestructure::IntervalMapAllocator allocator;

    unsigned diagCount = diagnosticEmitter.getDiagnosticCount();
    toolchain::SmallSetVector<MarkUnresolvedNonCopyableValueInst *, 32>
        moveIntroducersToProcess;
    bool madeChange =
        searchForCandidateObjectMarkUnresolvedNonCopyableValueInsts(
            fn, moveIntroducersToProcess, diagnosticEmitter);

    TOOLCHAIN_DEBUG(toolchain::dbgs()
               << "Emitting diagnostic when checking for mark must check inst: "
               << (diagCount != diagnosticEmitter.getDiagnosticCount() ? "yes"
                                                                       : "no")
               << '\n');

    if (moveIntroducersToProcess.empty()) {
      TOOLCHAIN_DEBUG(toolchain::dbgs()
                 << "No move introducers found?! Returning early?!\n");
    } else {
      diagCount = diagnosticEmitter.getDiagnosticCount();
      MoveOnlyObjectChecker checker{diagnosticEmitter, domTree, deba, poa,
                                    allocator};
      madeChange |= checker.check(moveIntroducersToProcess);
    }

    if (diagCount != diagnosticEmitter.getDiagnosticCount()) {
      emitCheckerMissedCopyOfNonCopyableTypeErrors(getFunction(),
                                                   diagnosticEmitter);
    }

    madeChange |=
        cleanupNonCopyableCopiesAfterEmittingDiagnostic(getFunction());

    if (madeChange) {
      invalidateAnalysis(SILAnalysis::InvalidationKind::Instructions);
    }
  }
};

} // anonymous namespace

SILTransform *language::createMoveOnlyObjectChecker() {
  return new MoveOnlyObjectCheckerTesterPass();
}
