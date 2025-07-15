//===--- MoveOnlyAddressCheckerTester.cpp ---------------------------------===//
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
#include "MoveOnlyBorrowToDestructureUtils.h"
#include "MoveOnlyDiagnostics.h"
#include "MoveOnlyObjectCheckerUtils.h"
#include "MoveOnlyUtils.h"

#include <utility>

using namespace language;
using namespace language::siloptimizer;

//===----------------------------------------------------------------------===//
//                         MARK: Top Level Entrypoint
//===----------------------------------------------------------------------===//

namespace {

class MoveOnlyAddressCheckerTesterPass : public SILFunctionTransform {
  void run() override {
    auto *fn = getFunction();

    // Don't rerun diagnostics on deserialized functions.
    if (getFunction()->wasDeserializedCanonical())
      return;

    assert(fn->getModule().getStage() == SILStage::Raw &&
           "Should only run on Raw SIL");
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "===> MoveOnly Addr Checker. Visiting: "
                            << fn->getName() << '\n');
    auto *dominanceAnalysis = getAnalysis<DominanceAnalysis>();
    DominanceInfo *domTree = dominanceAnalysis->get(fn);
    auto *poa = getAnalysis<PostOrderAnalysis>();
    auto *deba = getAnalysis<DeadEndBlocksAnalysis>();

    DiagnosticEmitter diagnosticEmitter(fn);
    toolchain::SmallSetVector<MarkUnresolvedNonCopyableValueInst *, 32>
        moveIntroducersToProcess;
    searchForCandidateAddressMarkUnresolvedNonCopyableValueInsts(
        fn, getAnalysis<PostOrderAnalysis>(), moveIntroducersToProcess,
        diagnosticEmitter);

    TOOLCHAIN_DEBUG(toolchain::dbgs()
               << "Emitting diagnostic when checking for mark must check inst: "
               << (diagnosticEmitter.getDiagnosticCount() ? "yes" : "no")
               << '\n');

    bool madeChange = false;
    if (moveIntroducersToProcess.empty()) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "No move introducers found?!\n");
    } else {
      borrowtodestructure::IntervalMapAllocator allocator;
      MoveOnlyAddressChecker checker{
          getFunction(), diagnosticEmitter, allocator, domTree, poa, deba};
      madeChange = checker.completeLifetimes();
      madeChange |= checker.check(moveIntroducersToProcess);
    }

    // If we did not emit any diagnostics, emit a diagnostic if we missed any
    // copies.
    if (!diagnosticEmitter.emittedDiagnostic()) {
      emitCheckerMissedCopyOfNonCopyableTypeErrors(getFunction(),
                                                   diagnosticEmitter);
    }

    // Then cleanup any copies we left behind for either reason and emit an
    // error.
    madeChange |=
        cleanupNonCopyableCopiesAfterEmittingDiagnostic(getFunction());

    if (madeChange) {
      invalidateAnalysis(SILAnalysis::InvalidationKind::Instructions);
    }
  }
};

} // anonymous namespace

SILTransform *language::createMoveOnlyAddressChecker() {
  return new MoveOnlyAddressCheckerTesterPass();
}
