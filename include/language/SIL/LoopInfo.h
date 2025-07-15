//===--- LoopInfo.h - SIL Loop Analysis -------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_SIL_LOOPINFO_H
#define LANGUAGE_SIL_LOOPINFO_H

#include "language/SIL/CFG.h"
#include "language/SIL/SILBasicBlock.h"
#include "toolchain/Analysis/LoopInfo.h"
#include "toolchain/ADT/iterator_range.h"

namespace language {
  class DominanceInfo;
  class SILLoop;
  class SILPassManager;
}

// Implementation in LoopInfoImpl.h
#ifdef __GNUC__
__extension__ extern template
class toolchain::LoopBase<language::SILBasicBlock, language::SILLoop>;
__extension__ extern template
class toolchain::LoopInfoBase<language::SILBasicBlock, language::SILLoop>;
#endif

namespace language {

class SILLoop;

/// Information about a single natural loop.
class SILLoop : public toolchain::LoopBase<SILBasicBlock, SILLoop> {
public:
  SILLoop() {}
  void dump() const;

  iterator_range<iterator> getSubLoopRange() const {
    return make_range(begin(), end());
  }

  void getExitingAndLatchBlocks(
    SmallVectorImpl<SILBasicBlock *> &ExitingAndLatchBlocks) const {
    this->getExitingBlocks(ExitingAndLatchBlocks);
    SILBasicBlock *header = getHeader();
    for (auto *predBB : header->getPredecessorBlocks()) {
      if (contains(predBB) && !this->isLoopExiting(predBB))
        ExitingAndLatchBlocks.push_back(predBB);
    }
  }

  SILFunction *getFunction() const { return getHeader()->getParent(); }

private:
  friend class toolchain::LoopInfoBase<SILBasicBlock, SILLoop>;

  explicit SILLoop(SILBasicBlock *BB)
    : toolchain::LoopBase<SILBasicBlock, SILLoop>(BB) {}
};

/// Information about loops in a function.
class SILLoopInfo {
  friend class toolchain::LoopBase<SILBasicBlock, SILLoop>;
  using SILLoopInfoBase = toolchain::LoopInfoBase<SILBasicBlock, SILLoop>;

  SILLoopInfoBase LI;
  DominanceInfo *Dominance;

  void operator=(const SILLoopInfo &) = delete;
  SILLoopInfo(const SILLoopInfo &) = delete;

public:
  SILLoopInfo(SILFunction *F, DominanceInfo *DT);

  SILLoopInfoBase &getBase() { return LI; }

  /// Verify loop information. This is very expensive.
  void verify() const;

  /// The iterator interface to the top-level loops in the current
  /// function.
  using iterator = SILLoopInfoBase::iterator;
  iterator begin() const { return LI.begin(); }
  iterator end() const { return LI.end(); }
  bool empty() const { return LI.empty(); }
  iterator_range<iterator> getTopLevelLoops() const {
    return make_range(begin(), end());
  }

  /// Return the inner most loop that BB lives in.  If a basic block is in no
  /// loop (for example the entry node), null is returned.
  SILLoop *getLoopFor(const SILBasicBlock *BB) const {
    return LI.getLoopFor(BB);
  }

  /// Return the inner most loop that BB lives in.  If a basic block is in no
  /// loop (for example the entry node), null is returned.
  const SILLoop *operator[](const SILBasicBlock *BB) const {
    return LI.getLoopFor(BB);
  }

  /// Return the loop nesting level of the specified block.
  unsigned getLoopDepth(const SILBasicBlock *BB) const {
    return LI.getLoopDepth(BB);
  }

  /// True if the block is a loop header node.
  bool isLoopHeader(SILBasicBlock *BB) const {
    return LI.isLoopHeader(BB);
  }

  /// This removes the specified top-level loop from this loop info object. The
  /// loop is not deleted, as it will presumably be inserted into another loop.
  SILLoop *removeLoop(iterator I) { return LI.removeLoop(I); }

  /// Change the top-level loop that contains BB to the specified loop.  This
  /// should be used by transformations that restructure the loop hierarchy
  /// tree.
  void changeLoopFor(SILBasicBlock *BB, SILLoop *L) { LI.changeLoopFor(BB, L); }

  /// Replace the specified loop in the top-level loops list with the indicated
  /// loop.
  void changeTopLevelLoop(SILLoop *OldLoop, SILLoop *NewLoop) {
    LI.changeTopLevelLoop(OldLoop, NewLoop);
  }

  ///  This adds the specified loop to the collection of top-level loops.
  void addTopLevelLoop(SILLoop *New) {
    LI.addTopLevelLoop(New);
  }

  /// This method completely removes BB from all data structures, including all
  /// of the Loop objects it is nested in and our mapping from SILBasicBlocks to
  /// loops.
  void removeBlock(SILBasicBlock *BB) {
    LI.removeBlock(BB);
  }
};

} // end namespace language

#endif
