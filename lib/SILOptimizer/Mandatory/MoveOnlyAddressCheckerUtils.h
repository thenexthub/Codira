//===--- MoveOnlyAddressCheckerUtils.h ------------------------------------===//
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

#ifndef LANGUAGE_SILOPTIMIZER_MANDATORY_MOVEONLYADDRESSCHECKERUTILS_H
#define LANGUAGE_SILOPTIMIZER_MANDATORY_MOVEONLYADDRESSCHECKERUTILS_H

#include "MoveOnlyBorrowToDestructureUtils.h"

namespace language {

class PostOrderAnalysis;
class DeadEndBlocksAnalysis;

namespace siloptimizer {

class DiagnosticEmitter;

/// Searches for candidate mark must checks.
///
/// NOTE: To see if we emitted a diagnostic, use \p
/// diagnosticEmitter.getDiagnosticCount().
void searchForCandidateAddressMarkUnresolvedNonCopyableValueInsts(
    SILFunction *fn, PostOrderAnalysis *poa,
    toolchain::SmallSetVector<MarkUnresolvedNonCopyableValueInst *, 32>
        &moveIntroducersToProcess,
    DiagnosticEmitter &diagnosticEmitter);

struct MoveOnlyAddressChecker {
  SILFunction *fn;
  DiagnosticEmitter &diagnosticEmitter;
  borrowtodestructure::IntervalMapAllocator &allocator;
  DominanceInfo *domTree;
  PostOrderAnalysis *poa;
  DeadEndBlocksAnalysis *deadEndBlocksAnalysis;

  /// \returns true if we changed the IR. To see if we emitted a diagnostic, use
  /// \p diagnosticEmitter.getDiagnosticCount().
  bool check(toolchain::SmallSetVector<MarkUnresolvedNonCopyableValueInst *, 32>
                 &moveIntroducersToProcess);
  bool completeLifetimes();
};

} // namespace siloptimizer

} // namespace language

#endif
