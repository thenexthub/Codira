//===--- LoopUtils.h --------------------------------------------*- C++ -*-===//
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
/// This header file declares utility functions for simplifying and
/// canonicalizing loops.
///
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_SILOPTIMIZER_UTILS_LOOPUTILS_H
#define LANGUAGE_SILOPTIMIZER_UTILS_LOOPUTILS_H

#include "toolchain/ADT/SmallVector.h"

namespace language {

class SILFunction;
class SILBasicBlock;
class SILInstruction;
class SILLoop;
class DominanceInfo;
class SILLoopInfo;

/// Canonicalize the loop for rotation and downstream passes.
///
/// Create a single preheader and single latch block.
bool canonicalizeLoop(SILLoop *L, DominanceInfo *DT, SILLoopInfo *LI);

/// Canonicalize all loops in the function F for which \p LI contains loop
/// information. We update loop info and dominance info while we do this.
bool canonicalizeAllLoops(DominanceInfo *DT, SILLoopInfo *LI);

/// Check whether it is safe to duplicate this instruction when duplicating
/// this loop by unrolling or versioning.
bool canDuplicateLoopInstruction(SILLoop *L, SILInstruction *Inst);

/// A visitor that visits loops in a function in a bottom up order. It only
/// performs the visit.
class SILLoopVisitor {
  SILFunction *F;
  SILLoopInfo *LI;

public:
  SILLoopVisitor(SILFunction *Func, SILLoopInfo *LInfo) : F(Func), LI(LInfo) {}
  virtual ~SILLoopVisitor() {}

  void run();

  SILFunction *getFunction() const { return F; }

  virtual void runOnLoop(SILLoop *L) = 0;
  virtual void runOnFunction(SILFunction *F) = 0;
};

/// A group of sil loop visitors, run in sequence on a function.
class SILLoopVisitorGroup : public SILLoopVisitor {
  /// The list of visitors to run.
  ///
  /// This is set to 3, since currently the only place this is used will have at
  /// most 3 such visitors.
  toolchain::SmallVector<SILLoopVisitor *, 3> Visitors;

public:
  SILLoopVisitorGroup(SILFunction *Func, SILLoopInfo *LInfo)
      : SILLoopVisitor(Func, LInfo) {}
  virtual ~SILLoopVisitorGroup() {}

  void addVisitor(SILLoopVisitor *V) {
    Visitors.push_back(V);
  }

  void runOnLoop(SILLoop *L) override {
    for (auto *V : Visitors) {
      V->runOnLoop(L);
    }
  }

  void runOnFunction(SILFunction *F) override {
    for (auto *V : Visitors) {
      V->runOnFunction(F);
    }
  }
};

} // end language namespace

#endif
