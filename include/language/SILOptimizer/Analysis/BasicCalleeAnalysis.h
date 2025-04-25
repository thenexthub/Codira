//===- BasicCalleeAnalysis.h - Determine callees per call site --*- C++ -*-===//
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

#ifndef SWIFT_SILOPTIMIZER_ANALYSIS_BASICCALLEEANALYSIS_H
#define SWIFT_SILOPTIMIZER_ANALYSIS_BASICCALLEEANALYSIS_H

#include "language/SILOptimizer/Analysis/Analysis.h"
#include "language/SIL/CalleeCache.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/ApplySite.h"
#include "language/SIL/SILModule.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/PointerIntPair.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/TinyPtrVector.h"
#include "llvm/Support/Allocator.h"

namespace language {

class BasicCalleeAnalysis : public SILAnalysis {
  SILModule &M;
  std::unique_ptr<CalleeCache> Cache;

public:

  BasicCalleeAnalysis(SILModule *M)
      : SILAnalysis(SILAnalysisKind::BasicCallee), M(*M), Cache(nullptr) {}

  ~BasicCalleeAnalysis() {}

  static bool classof(const SILAnalysis *S) {
    return S->getKind() == SILAnalysisKind::BasicCallee;
  }

  /// Invalidate all information in this analysis.
  virtual void invalidate() override {
    Cache.reset();
  }

  /// Invalidate all of the information for a specific function.
  virtual void invalidate(SILFunction *F, InvalidationKind K) override {
    // No invalidation needed because the analysis does not cache anything
    // per call-site in functions.
  }

  /// Notify the analysis about a newly created function.
  virtual void notifyAddedOrModifiedFunction(SILFunction *F) override {
    // Nothing to be done because the analysis does not cache anything
    // per call-site in functions.
  }

  /// Notify the analysis about a function which will be deleted from the
  /// module.
  virtual void notifyWillDeleteFunction(SILFunction *F) override {
    // No invalidation needed because the analysis does not cache anything per
    // call-site in functions.
  }

  /// Notify the analysis about changed witness or vtables.
  virtual void invalidateFunctionTables() override {
    Cache.reset();
  }

  SWIFT_DEBUG_DUMP;

  void print(llvm::raw_ostream &os) const;

  void updateCache() {
    if (!Cache)
      Cache = std::make_unique<CalleeCache>(M);
  }

  CalleeList getCalleeList(FullApplySite FAS) {
    updateCache();
    return Cache->getCalleeList(FAS);
  }

  CalleeList getCalleeListOfValue(SILValue callee) {
    updateCache();
    return Cache->getCalleeListOfValue(callee);
  }

  CalleeList getDestructors(SILType type, bool isExactType) {
    updateCache();
    return Cache->getDestructors(type, isExactType);
  }

  MemoryBehavior getMemoryBehavior(FullApplySite as, bool observeRetains);

  CalleeCache *getCalleeCache() { return Cache.get(); }
};

bool isDeinitBarrier(SILInstruction *const instruction,
                     BasicCalleeAnalysis *bca);

} // end namespace language

#endif
