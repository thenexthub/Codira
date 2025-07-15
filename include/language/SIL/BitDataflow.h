//===--- BitDataflow.h ------------------------------------------*- C++ -*-===//
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
/// \file Contains the BitDataflow utility for performing bit-wise dataflow
/// analysis on a SILFunction.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SIL_BIT_DATAFLOW_H
#define LANGUAGE_SIL_BIT_DATAFLOW_H

#include "language/SIL/BasicBlockData.h"
#include "toolchain/ADT/SmallBitVector.h"

namespace language {

class SILFunction;

/// A utility to calculate forward or backward dataflow of bit sets on a
/// SILFunction.
class BitDataflow {

  /// What kind of terminators can be reached from a block.
  enum class ExitReachability : uint8_t {
    /// Worst case: the block is part of a cycle which neither reaches a
    /// function-exit nor an unreachable-instruction.
    InInfiniteLoop,

    /// An unreachable-instruction can be reached from the block, but not a
    /// function-exit (like "return" or "throw").
    ReachesUnreachable,

    /// A function-exit can be reached from the block.
    /// This is the case for most basic blocks.
    ReachesExit
  };

public:
  using Bits = toolchain::SmallBitVector;

  /// Basic-block specific information used for dataflow analysis.
  struct BlockState {
    /// The bits valid at the entry (i.e. the first instruction) of the block.
    Bits entrySet;

    /// The bits valid at the exit (i.e. after the terminator) of the block.
    Bits exitSet;

    /// Generated bits of the block.
    Bits genSet;

    /// Killed bits of the block.
    Bits killSet;

    /// True, if this block is reachable from the entry block, i.e. is not an
    /// unreachable block.
    ///
    /// This flag is only computed if entryReachabilityAnalysis is called.
    bool reachableFromEntry = false;

    /// What kind of terminators can be reached from this block.
    ///
    /// This is only computed if exitReachableAnalysis is called.
    ExitReachability exitReachability = ExitReachability::InInfiniteLoop;

    BlockState(unsigned numLocations) :
      entrySet(numLocations), exitSet(numLocations),
      genSet(numLocations), killSet(numLocations) {}

    bool exitReachable() const {
      return exitReachability == ExitReachability::ReachesExit;
    }

    bool isInInfiniteLoop() const {
      return exitReachability == ExitReachability::InInfiniteLoop;
    }
  };

private:
  BasicBlockData<BlockState> blockStates;

public:

  using iterator = BasicBlockData<BlockState>::iterator;

  /// Sets up the BlockState datastructures and associates all basic blocks with
  /// a state.
  BitDataflow(SILFunction *function, unsigned numLocations);

  BitDataflow(const BitDataflow &) = delete;
  BitDataflow &operator=(const BitDataflow &) = delete;

  iterator begin() { return blockStates.begin(); }
  iterator end() { return blockStates.end(); }

  /// Returns the state of a block.
  BlockState &operator[] (SILBasicBlock *block) {
    return blockStates[block];
  }

  /// Calculates the BlockState::reachableFromEntry flags.
  void entryReachabilityAnalysis();

  /// Calculates the BlockState::exitReachable flags.
  void exitReachableAnalysis();

  using JoinOperation = std::function<void (Bits &dest, const Bits &src)>;

  /// Derives the block exit sets from the entry sets by applying the gen and
  /// kill sets.
  /// At control flow joins, the \p join operation is applied.
  void solveForward(JoinOperation join);

  /// Calls solveForward() with a bit-intersection as join operation.
  void solveForwardWithIntersect();

  /// Calls solveForward() with a bit-union as join operation.
  void solveForwardWithUnion();

  /// Derives the block entry sets from the exit sets by applying the gen and
  /// kill sets.
  /// At control flow joins, the \p join operation is applied.
  void solveBackward(JoinOperation join);

  /// Calls solveBackward() with a bit-intersection as join operation.
  void solveBackwardWithIntersect();

  /// Calls solveBackward() with a bit-union as join operation.
  void solveBackwardWithUnion();

  /// Debug dump the BitDataflow state.
  void dump() const;
};

} // end language namespace

#endif
