//===--- SemanticARCOptVisitor.cpp ----------------------------------------===//
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
/// \file
///
/// Implementation of the main optimize loop of the ARC visitor peephole
/// optimizer.
///
//===----------------------------------------------------------------------===//

#include "SemanticARCOptVisitor.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/Defer.h"
#include "language/SIL/DebugUtils.h"

using namespace language;
using namespace language::semanticarc;

bool SemanticARCOptVisitor::optimizeWithoutFixedPoint() {
  bool madeChange = false;

  // First process the worklist until we reach a fixed point.
  madeChange |= processWorklist();

  return madeChange;
}

bool SemanticARCOptVisitor::optimize() {
  bool madeChange = false;

  // First process the worklist until we reach a fixed point.
  madeChange |= processWorklist();

  {
    // If we made a change, set that we assume we are at fixed point and then
    // re-run the worklist so that we can
    // properly seeded the ARC peephole map.
    ctx.assumingAtFixedPoint = true;
    LANGUAGE_DEFER { ctx.assumingAtFixedPoint = false; };

    // Add everything in visitedSinceLastMutation to the worklist so we
    // recompute our fixed point.
    drainVisitedSinceLastMutationIntoWorklist();

    // Then re-run the worklist. We shouldn't modify anything since we are at a
    // fixed point and are just using this to seed the
    // joinedOwnedIntroducerToConsumedOperands after we have finished changing
    // things. If we did change something, we did something weird, so assert!
    bool madeAdditionalChanges = processWorklist();
    (void)madeAdditionalChanges;
    assert(!madeAdditionalChanges && "Should be at the fixed point");
  }

  return madeChange;
}

bool SemanticARCOptVisitor::processWorklist() {
  // NOTE: The madeChange here is not strictly necessary since we only have
  // items added to the worklist today if we have already made /some/ sort of
  // change. That being said, I think there is a low cost to including this here
  // and makes the algorithm more correct, visually and in the face of potential
  // refactoring.
  bool madeChange = false;

  while (!worklist.empty()) {
    // Pop the last element off the list. If we were returned None, we blotted
    // this element, so skip it.
    SILValue next = worklist.pop_back_val().value_or(SILValue());
    if (!next)
      continue;

    // First check if this is a value that we have visited since the last time
    // we erased an instruction. If we have visited it, skip it. Every time we
    // modify something, we should be deleting an instruction, so we have not
    // found any further information.
    if (!visitedSinceLastMutation.insert(next).second) {
      continue;
    }

    // First check if this is an instruction that is trivially dead. This can
    // occur if we eliminate rr traffic resulting in dead projections and the
    // like.
    //
    // If we delete, we first add all of our deleted instructions operands to
    // the worklist and then remove all results (since we are going to delete
    // the instruction).
    if (auto *defInst = next->getDefiningInstruction()) {
      if (isInstructionTriviallyDead(defInst)) {
        assert(!ctx.assumingAtFixedPoint &&
               "Assumed was at fixed point and recomputing state?!");
        deleteAllDebugUses(defInst);
        eraseInstruction(defInst);
        madeChange = true;
        ctx.verify();
        continue;
      }
    }

    // Otherwise, if we have a single value instruction (to be expanded later
    // perhaps), try to visit that value recursively.
    if (auto *svi = dyn_cast<SingleValueInstruction>(next)) {
      bool madeSingleChange = visit(svi);
      assert((!madeSingleChange || !ctx.assumingAtFixedPoint) &&
             "Assumed was at fixed point and modified state?!");
      madeChange |= madeSingleChange;
      if (madeSingleChange) {
        ctx.verify();
      }
      continue;
    }
  }

  return madeChange;
}
