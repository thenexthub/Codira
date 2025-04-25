//===--- ARCMatchingSet.h ---------------------------------------*- C++ -*-===//
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

#ifndef SWIFT_SILOPTIMIZER_PASSMANAGER_GLOBALARCPAIRINGANALYSIS_H
#define SWIFT_SILOPTIMIZER_PASSMANAGER_GLOBALARCPAIRINGANALYSIS_H

#include "GlobalARCSequenceDataflow.h"
#include "GlobalLoopARCSequenceDataflow.h"
#include "language/SIL/SILValue.h"
#include "language/SILOptimizer/Utils/LoopUtils.h"
#include "language/SILOptimizer/Analysis/RCIdentityAnalysis.h"
#include "llvm/ADT/SetVector.h"

namespace language {

class SILInstruction;
class SILFunction;
class AliasAnalysis;
class PostOrderAnalysis;
class LoopRegionFunctionInfo;
class SILLoopInfo;
class RCIdentityFunctionInfo;

/// A set of matching reference count increments, decrements, increment
/// insertion pts, and decrement insertion pts.
struct ARCMatchingSet {

  /// The pointer that this ARCMatchingSet is providing matching increment and
  /// decrement sets for.
  ///
  /// TODO: This should really be called RCIdentity.
  SILValue Ptr;

  /// The set of reference count increments that were paired.
  llvm::SetVector<SILInstruction *> Increments;

  /// The set of reference count decrements that were paired.
  llvm::SetVector<SILInstruction *> Decrements;

  // This is a data structure that cannot be moved or copied.
  ARCMatchingSet() = default;
  ARCMatchingSet(const ARCMatchingSet &) = delete;
  ARCMatchingSet(ARCMatchingSet &&) = delete;
  ARCMatchingSet &operator=(const ARCMatchingSet &) = delete;
  ARCMatchingSet &operator=(ARCMatchingSet &&) = delete;

  void clear() {
    Ptr = SILValue();
    Increments.clear();
    Decrements.clear();
  }
};

struct MatchingSetFlags {
  bool KnownSafe;
  bool CodeMotionSafe;
};
static_assert(std::is_pod<MatchingSetFlags>::value,
              "MatchingSetFlags should be a pod.");

struct ARCMatchingSetBuilder {
  using TDMapTy = BlotMapVector<SILInstruction *, TopDownRefCountState>;
  using BUMapTy = BlotMapVector<SILInstruction *, BottomUpRefCountState>;

  TDMapTy &TDMap;
  BUMapTy &BUMap;

  llvm::SmallVector<SILInstruction *, 8> NewIncrements;
  llvm::SmallVector<SILInstruction *, 8> NewDecrements;
  bool MatchedPair;
  ARCMatchingSet MatchSet;
  bool PtrIsGuaranteedArg;

  RCIdentityFunctionInfo *RCIA;

public:
  ARCMatchingSetBuilder(TDMapTy &TDMap, BUMapTy &BUMap,
                        RCIdentityFunctionInfo *RCIA)
      : TDMap(TDMap), BUMap(BUMap), MatchedPair(false),
        PtrIsGuaranteedArg(false), RCIA(RCIA) {}

  void init(SILInstruction *Inst) {
    clear();
    MatchSet.Ptr = RCIA->getRCIdentityRoot(Inst->getOperand(0));

    // If we have a function argument that is guaranteed, set the guaranteed
    // flag so we know that it is always known safe.
    if (auto *A = dyn_cast<SILFunctionArgument>(MatchSet.Ptr)) {
      auto C = A->getArgumentConvention();
      PtrIsGuaranteedArg = C == SILArgumentConvention::Direct_Guaranteed;
    }
    NewIncrements.push_back(Inst);
  }

  void clear() {
    MatchSet.clear();
    MatchedPair = false;
    NewIncrements.clear();
    NewDecrements.clear();
  }

  bool matchUpIncDecSetsForPtr();

  // We only allow for get result when this object is invalidated via a move.
  ARCMatchingSet &getResult() { return MatchSet; }

  bool matchedPair() const { return MatchedPair; }

private:
  /// Returns .Some(MatchingSetFlags) on success and .None on failure.
  std::optional<MatchingSetFlags> matchIncrementsToDecrements();
  std::optional<MatchingSetFlags> matchDecrementsToIncrements();
};

} // end swift namespace

#endif
