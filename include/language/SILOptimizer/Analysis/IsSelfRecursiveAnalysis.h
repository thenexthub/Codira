//===--- IsSelfRecursiveAnalysis.h ----------------------------------------===//
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

#ifndef SWIFT_SILOPTIMIZER_ISSELFRECURSIVEANALYSIS_H
#define SWIFT_SILOPTIMIZER_ISSELFRECURSIVEANALYSIS_H

#include "language/SILOptimizer/Analysis/Analysis.h"

namespace language {
class SILFunction;

class IsSelfRecursive {
  const SILFunction *f;
  bool didComputeValue = false;
  bool isSelfRecursive = false;

  void compute();

public:
  IsSelfRecursive(const SILFunction *f) : f(f) {}

  ~IsSelfRecursive();

  bool isComputed() const { return didComputeValue; }

  bool get() {
    if (!didComputeValue) {
      compute();
      didComputeValue = true;
    }
    return isSelfRecursive;
  }

  SILFunction *getFunction() { return const_cast<SILFunction *>(f); }
};

class IsSelfRecursiveAnalysis final
    : public FunctionAnalysisBase<IsSelfRecursive> {
public:
  IsSelfRecursiveAnalysis()
      : FunctionAnalysisBase<IsSelfRecursive>(
            SILAnalysisKind::IsSelfRecursive) {}

  IsSelfRecursiveAnalysis(const IsSelfRecursiveAnalysis &) = delete;
  IsSelfRecursiveAnalysis &operator=(const IsSelfRecursiveAnalysis &) = delete;

  static SILAnalysisKind getAnalysisKind() {
    return SILAnalysisKind::IsSelfRecursive;
  }

  static bool classof(const SILAnalysis *s) {
    return s->getKind() == SILAnalysisKind::IsSelfRecursive;
  }

  std::unique_ptr<IsSelfRecursive> newFunctionAnalysis(SILFunction *f) override {
    return std::make_unique<IsSelfRecursive>(f);
  }

  bool shouldInvalidate(SILAnalysis::InvalidationKind k) override {
    return k & InvalidationKind::Calls;
  }
};

} // namespace language

#endif
