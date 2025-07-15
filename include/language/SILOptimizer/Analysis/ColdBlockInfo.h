//===--- ColdBlockInfo.h - Fast/slow path analysis for SIL CFG --*- C++ -*-===//
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

#ifndef LANGUAGE_SILOPTIMIZER_ANALYSIS_COLDBLOCKS_H
#define LANGUAGE_SILOPTIMIZER_ANALYSIS_COLDBLOCKS_H

#include "toolchain/ADT/DenseMap.h"
#include "language/SIL/SILValue.h"
#include "language/Basic/FixedBitSet.h"

namespace language {
class DominanceAnalysis;
class PostDominanceAnalysis;
class SILBasicBlock;
class CondBranchInst;

/// Cache a set of basic blocks that have been determined to be cold or hot.
///
/// This does not inherit from SILAnalysis because it is not worth preserving
/// across passes.
class ColdBlockInfo {
public:
  // Represents the temperatures of edges flowing into a block.
  //
  //         T = "top" -- both warm and cold edges
  //        /  \
  //     Warm  Cold
  //        \  /
  //         B = "bottom" -- neither warm or cold edges
  struct State {
    // Each state kind, as an integer, is its position in any bit vectors.
    enum Temperature {
      Warm = 0,
      Cold = 1
    };

    // Number of states, excluding Top or Bottom, in this flow problem.
    static constexpr unsigned NumStates = 2;
  };

  using Energy = FixedBitSet<State::NumStates, State::Temperature>;

private:
  DominanceAnalysis *DA;
  PostDominanceAnalysis *PDA;

  /// Each block in this map has been determined to be cold and/or warm.
  /// Make sure to always use the set/resetToCold methods to update this!
  toolchain::DenseMap<const SILBasicBlock*, Energy> EnergyMap;

  // This is a cache and shouldn't be copied around.
  ColdBlockInfo(const ColdBlockInfo &) = delete;
  ColdBlockInfo &operator=(const ColdBlockInfo &) = delete;
  TOOLCHAIN_DUMP_METHOD void dump() const;

  /// Tracks whether any information was changed in the energy map.
  bool changedMap = false;
  void resetToCold(const SILBasicBlock *BB);
  void set(const SILBasicBlock *BB, State::Temperature temp);

  using ExpectedValue = std::optional<bool>;

  void setExpectedCondition(CondBranchInst *CondBranch, ExpectedValue value);

  ExpectedValue searchForExpectedValue(SILValue Cond,
                                       unsigned recursionDepth = 0);
  void searchForExpectedValue(CondBranchInst *CondBranch);

  bool inferFromEdgeProfile(SILBasicBlock *BB);

public:
  ColdBlockInfo(DominanceAnalysis *DA, PostDominanceAnalysis *PDA);

  void analyze(SILFunction *F);

  bool isCold(const SILBasicBlock *BB) const;
};
} // end namespace language

#endif
