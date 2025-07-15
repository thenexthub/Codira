//===--- MoveOnlyTempAllocationFromLetTester.cpp --------------------------===//
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
///
/// \file A simple tester for the utility function
/// eliminateTemporaryAllocationsFromLet that allows us to write separate SIL
/// test cases for the utility.
///
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "sil-move-only-checker"

#include "MoveOnlyAddressCheckerUtils.h"
#include "MoveOnlyDiagnostics.h"
#include "MoveOnlyUtils.h"

#include "language/Basic/Assertions.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/PassManager/Transforms.h"

using namespace language;
using namespace language::siloptimizer;

namespace {

struct MoveOnlyTempAllocationFromLetTester : SILFunctionTransform {
  void run() override {
    auto *fn = getFunction();

    // Don't rerun diagnostics on deserialized functions.
    if (getFunction()->wasDeserializedCanonical())
      return;

    assert(fn->getModule().getStage() == SILStage::Raw &&
           "Should only run on Raw SIL");

    TOOLCHAIN_DEBUG(toolchain::dbgs()
               << "===> MoveOnlyTempAllocationFromLetTester. Visiting: "
               << fn->getName() << '\n');

    toolchain::SmallSetVector<MarkUnresolvedNonCopyableValueInst *, 32>
        moveIntroducersToProcess;
    DiagnosticEmitter diagnosticEmitter(getFunction());

    unsigned diagCount = diagnosticEmitter.getDiagnosticCount();
    searchForCandidateAddressMarkUnresolvedNonCopyableValueInsts(
        getFunction(), getAnalysis<PostOrderAnalysis>(),
        moveIntroducersToProcess, diagnosticEmitter);

    // Return early if we emitted a diagnostic.
    if (diagCount != diagnosticEmitter.getDiagnosticCount())
      return;

    bool madeChange = false;
    while (!moveIntroducersToProcess.empty()) {
      auto *next = moveIntroducersToProcess.pop_back_val();
      madeChange |= eliminateTemporaryAllocationsFromLet(next);
    }

    if (madeChange)
      invalidateAnalysis(SILAnalysis::InvalidationKind::Instructions);
  }
};

} // namespace

SILTransform *language::createMoveOnlyTempAllocationFromLetTester() {
  return new MoveOnlyTempAllocationFromLetTester();
}
