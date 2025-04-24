//===--- MoveOnlyAddressCheckerTester.cpp ---------------------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2022 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
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
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/PointerIntPair.h"
#include "llvm/ADT/PointerUnion.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallBitVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"

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
    LLVM_DEBUG(llvm::dbgs() << "===> MoveOnly Addr Checker. Visiting: "
                            << fn->getName() << '\n');
    auto *dominanceAnalysis = getAnalysis<DominanceAnalysis>();
    DominanceInfo *domTree = dominanceAnalysis->get(fn);
    auto *poa = getAnalysis<PostOrderAnalysis>();
    auto *deba = getAnalysis<DeadEndBlocksAnalysis>();

    DiagnosticEmitter diagnosticEmitter(fn);
    llvm::SmallSetVector<MarkUnresolvedNonCopyableValueInst *, 32>
        moveIntroducersToProcess;
    searchForCandidateAddressMarkUnresolvedNonCopyableValueInsts(
        fn, getAnalysis<PostOrderAnalysis>(), moveIntroducersToProcess,
        diagnosticEmitter);

    LLVM_DEBUG(llvm::dbgs()
               << "Emitting diagnostic when checking for mark must check inst: "
               << (diagnosticEmitter.getDiagnosticCount() ? "yes" : "no")
               << '\n');

    bool madeChange = false;
    if (moveIntroducersToProcess.empty()) {
      LLVM_DEBUG(llvm::dbgs() << "No move introducers found?!\n");
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

SILTransform *swift::createMoveOnlyAddressChecker() {
  return new MoveOnlyAddressCheckerTesterPass();
}
