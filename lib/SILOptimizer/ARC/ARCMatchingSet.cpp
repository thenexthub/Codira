//===--- ARCMatchingSet.cpp - ARC Matching Set Builder --------------------===//
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

#define DEBUG_TYPE "arc-sequence-opts"

#include "ARCMatchingSet.h"
#include "RefCountState.h"
#include "language/Basic/Assertions.h"
#include "language/Basic/BlotMapVector.h"
#include "language/SIL/SILBuilder.h"
#include "language/SIL/SILVisitor.h"
#include "language/SILOptimizer/Analysis/ARCAnalysis.h"
#include "language/SILOptimizer/Analysis/AliasAnalysis.h"
#include "language/SILOptimizer/Analysis/PostOrderAnalysis.h"
#include "language/SILOptimizer/Analysis/RCIdentityAnalysis.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SILOptimizer/Utils/InstOptUtils.h"
#include "toolchain/ADT/MapVector.h"
#include "toolchain/ADT/PointerUnion.h"
#include "toolchain/ADT/SmallPtrSet.h"
#include "toolchain/ADT/Statistic.h"
#include "toolchain/ADT/StringSwitch.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/Debug.h"

using namespace language;

//===----------------------------------------------------------------------===//
//                          ARC Matching Set Builder
//===----------------------------------------------------------------------===//

/// Match retains to releases and return whether or not all of the releases
/// were known safe.
std::optional<MatchingSetFlags>
ARCMatchingSetBuilder::matchIncrementsToDecrements() {
  MatchingSetFlags Flags = {true, true};

  // For each increment in our list of new increments.
  //
  // FIXME: Refactor this into its own function.
  for (SILInstruction *Increment : NewIncrements) {
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Looking up state for increment: "
                            << *Increment);

    auto BURefCountState = BUMap.find(Increment);
    assert(BURefCountState != BUMap.end() && "Catch this on debug builds.");
    if (BURefCountState == BUMap.end()) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "        FAILURE! Could not find state for "
                                 "increment!\n");
      return std::nullopt;
    }

    TOOLCHAIN_DEBUG(toolchain::dbgs() << "        SUCCESS! Found state for increment.\n");

    // If we are not tracking a ref count inst for this increment, there is
    // nothing we can pair it with implying we should skip it.
    if (!(*BURefCountState)->second.isTrackingRefCountInst()) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "    SKIPPING INCREMENT! State not tracking "
                                 "any instruction.\n");
      continue;
    }

    bool BUIsKnownSafe = (*BURefCountState)->second.isKnownSafe();
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "        BOTTOM UP KNOWNSAFE: "
                            << (BUIsKnownSafe ? "true" : "false") << "\n");
    Flags.KnownSafe &= BUIsKnownSafe;

    bool BUCodeMotionSafe = (*BURefCountState)->second.isCodeMotionSafe();
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "        BOTTOM UP CODEMOTIONSAFE: "
                            << (BUCodeMotionSafe ? "true" : "false") << "\n");
    Flags.CodeMotionSafe &= BUCodeMotionSafe;

    // Now that we know we have an inst, grab the decrement.
    for (auto DecIter : (*BURefCountState)->second.getInstructions()) {
      SILInstruction *Decrement = DecIter;
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "        Decrement: " << *Decrement);

      // Now grab the increment matched up with the decrement from the bottom up
      // map.
      // If we can't find it, bail we can't match this increment up with
      // anything.
      auto TDRefCountState = TDMap.find(Decrement);
      if (TDRefCountState == TDMap.end()) {
        TOOLCHAIN_DEBUG(toolchain::dbgs() << "            FAILURE! Could not find state "
                                   "for decrement.\n");
        return std::nullopt;
      }
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "            SUCCESS! Found state for "
                                 "decrement.\n");

      // Make sure the increment we are looking at is also matched to our
      // decrement. Otherwise bail.
      if (!(*TDRefCountState)->second.isTrackingRefCountInst() ||
          !(*TDRefCountState)->second.containsInstruction(Increment)) {
        TOOLCHAIN_DEBUG(
            toolchain::dbgs() << "            FAILURE! Not tracking instruction or "
                            "found increment that did not match.\n");
        return std::nullopt;
      }

      // Add the decrement to the decrement to move set. If we don't insert
      // anything, just continue.
      if (!MatchSet.Decrements.insert(Decrement)) {
        TOOLCHAIN_DEBUG(toolchain::dbgs()
                   << "        SKIPPING! Already processed this decrement\n");
        continue;
      }

      NewDecrements.push_back(Decrement);
    }
  }

  return Flags;
}

std::optional<MatchingSetFlags>
ARCMatchingSetBuilder::matchDecrementsToIncrements() {
  MatchingSetFlags Flags = {true, true};

  // For each increment in our list of new increments.
  //
  // FIXME: Refactor this into its own function.
  for (SILInstruction *Decrement : NewDecrements) {
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "    Looking up state for decrement: "
                            << *Decrement);

    auto TDRefCountState = TDMap.find(Decrement);
    assert(TDRefCountState != TDMap.end() && "Catch this on debug builds.");
    if (TDRefCountState == TDMap.end()) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "        FAILURE! Could not find state for "
                                 "increment!\n");
      return std::nullopt;
    }

    TOOLCHAIN_DEBUG(toolchain::dbgs() << "        SUCCESS! Found state for decrement.\n");

    // If we are not tracking a ref count inst for this increment, there is
    // nothing we can pair it with implying we should skip it.
    if (!(*TDRefCountState)->second.isTrackingRefCountInst()) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "    SKIPPING DECREMENT! State not tracking "
                                 "any instruction.\n");
      continue;
    }

    bool TDIsKnownSafe = (*TDRefCountState)->second.isKnownSafe();
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "        TOP DOWN KNOWNSAFE: "
                            << (TDIsKnownSafe ? "true" : "false") << "\n");
    Flags.KnownSafe &= TDIsKnownSafe;

    bool TDCodeMotionSafe = (*TDRefCountState)->second.isCodeMotionSafe();
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "        TOP DOWN CODEMOTIONSAFE: "
                            << (TDCodeMotionSafe ? "true" : "false") << "\n");
    Flags.CodeMotionSafe &= TDCodeMotionSafe;

    // Now that we know we have an inst, grab the decrement.
    for (auto IncIter : (*TDRefCountState)->second.getInstructions()) {
      SILInstruction *Increment = IncIter;
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "        Increment: " << *Increment);

      // Now grab the increment matched up with the decrement from the bottom up
      // map.
      // If we can't find it, bail we can't match this increment up with
      // anything.
      auto BURefCountState = BUMap.find(Increment);
      if (BURefCountState == BUMap.end()) {
        TOOLCHAIN_DEBUG(toolchain::dbgs() << "            FAILURE! Could not find state "
                                   "for increment.\n");
        return std::nullopt;
      }

      TOOLCHAIN_DEBUG(
          toolchain::dbgs() << "            SUCCESS! Found state for increment.\n");

      // Make sure the increment we are looking at is also matched to our
      // decrement.
      // Otherwise bail.
      if (!(*BURefCountState)->second.isTrackingRefCountInst() ||
          !(*BURefCountState)->second.containsInstruction(Decrement)) {
        TOOLCHAIN_DEBUG(
            toolchain::dbgs() << "            FAILURE! Not tracking instruction or "
                            "found increment that did not match.\n");
        return std::nullopt;
      }

      // Add the decrement to the decrement to move set. If we don't insert
      // anything, just continue.
      if (!MatchSet.Increments.insert(Increment)) {
        TOOLCHAIN_DEBUG(toolchain::dbgs()
                   << "    SKIPPING! Already processed this increment.\n");
        continue;
      }

      NewIncrements.push_back(Increment);
    }
  }

  return Flags;
}

/// Visit each retain/release that is matched up to our operand over and over
/// again until we converge by not adding any more to the set which we can move.
/// If we find a situation that we cannot handle, we bail and return false. If
/// we succeed and it is safe to move increment/releases, we return true.
bool ARCMatchingSetBuilder::matchUpIncDecSetsForPtr() {
  bool KnownSafeTD = true;
  bool KnownSafeBU = true;
  bool CodeMotionSafeTD = true;
  bool CodeMotionSafeBU = true;

  while (true) {
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "Attempting to match up increments -> "
               "decrements:\n");
    // For each increment in our list of new increments, attempt to match them
    // up with decrements.
    auto Result = matchIncrementsToDecrements();
    if (!Result) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "    FAILED TO MATCH INCREMENTS -> "
                 "DECREMENTS!\n");
      return false;
    }
    if (!Result->KnownSafe) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "    NOT KNOWN SAFE!\n");
      KnownSafeTD = false;
    }
    if (!Result->CodeMotionSafe) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "    NOT CODE MOTION SAFE!\n");
      CodeMotionSafeTD = false;
    }
      
    NewIncrements.clear();

    // If we do not have any decrements to attempt to match up with, bail.
    if (NewDecrements.empty())
      break;

    TOOLCHAIN_DEBUG(toolchain::dbgs() << "Attempting to match up decrements -> "
               "increments:\n");
    Result = matchDecrementsToIncrements();
    if (!Result) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "    FAILED TO MATCH DECREMENTS -> "
                 "INCREMENTS!\n");
      return false;
    }
    if (!Result->KnownSafe) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "    NOT KNOWN SAFE!\n");
      KnownSafeBU = false;
    }
    if (!Result->CodeMotionSafe) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "    NOT CODE MOTION SAFE!\n");
      CodeMotionSafeBU = false;
    }
    NewDecrements.clear();

    // If we do not have any increment to attempt to match up with again, bail.
    if (NewIncrements.empty())
      break;
  }

  // There is no way we can get a top-down code motion but not a bottom-up, or vice
  // versa.
  assert(CodeMotionSafeTD == CodeMotionSafeBU && "Asymmetric code motion safety");

  bool UnconditionallySafe = (KnownSafeTD && KnownSafeBU);
  bool CodeMotionSafe = (CodeMotionSafeTD && CodeMotionSafeBU);
  if (UnconditionallySafe || CodeMotionSafe) {
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "UNCONDITIONALLY OR CODE MOTION SAFE! "
               "DELETING INSTS.\n");
  } else {
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "NOT UNCONDITIONALLY SAFE AND CODE MOTION "
               "BLOCKED!\n");
    return false;
  }

  // Make sure we always have increments and decrements in the match set.
  assert(MatchSet.Increments.empty() == MatchSet.Decrements.empty() &&
         "Match set without increments or decrements");

  if (!MatchSet.Increments.empty())
    MatchedPair = true;

  // Success!
  TOOLCHAIN_DEBUG(toolchain::dbgs() << "SUCCESS! We can remove things.\n");
  return true;
}
