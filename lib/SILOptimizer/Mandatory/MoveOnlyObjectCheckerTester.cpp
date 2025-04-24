//===--- MoveOnlyObjectCheckerTester.cpp ----------------------------------===//
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
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/IntervalMap.h"
#include "llvm/ADT/PointerIntPair.h"
#include "llvm/ADT/PointerUnion.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallBitVector.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/RecyclingAllocator.h"

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

    LLVM_DEBUG(llvm::dbgs() << "===> MoveOnly Object Checker. Visiting: "
                            << fn->getName() << '\n');

    auto *dominanceAnalysis = getAnalysis<DominanceAnalysis>();
    DominanceInfo *domTree = dominanceAnalysis->get(fn);
    auto *poa = getAnalysis<PostOrderAnalysis>();
    auto *deba = getAnalysis<DeadEndBlocksAnalysis>();

    DiagnosticEmitter diagnosticEmitter(fn);
    borrowtodestructure::IntervalMapAllocator allocator;

    unsigned diagCount = diagnosticEmitter.getDiagnosticCount();
    llvm::SmallSetVector<MarkUnresolvedNonCopyableValueInst *, 32>
        moveIntroducersToProcess;
    bool madeChange =
        searchForCandidateObjectMarkUnresolvedNonCopyableValueInsts(
            fn, moveIntroducersToProcess, diagnosticEmitter);

    LLVM_DEBUG(llvm::dbgs()
               << "Emitting diagnostic when checking for mark must check inst: "
               << (diagCount != diagnosticEmitter.getDiagnosticCount() ? "yes"
                                                                       : "no")
               << '\n');

    if (moveIntroducersToProcess.empty()) {
      LLVM_DEBUG(llvm::dbgs()
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

SILTransform *swift::createMoveOnlyObjectChecker() {
  return new MoveOnlyObjectCheckerTesterPass();
}
