//===--- DeadEndBlocksAnalysis.h ------------------------------------------===//
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

#ifndef LANGUAGE_SILOPTIMIZER_DEADENDBLOCKSANALYSIS_H
#define LANGUAGE_SILOPTIMIZER_DEADENDBLOCKSANALYSIS_H

#include "language/SIL/BasicBlockUtils.h"
#include "language/SILOptimizer/Analysis/Analysis.h"

namespace language {

class DeadEndBlocksAnalysis final : public FunctionAnalysisBase<DeadEndBlocks> {
public:
  DeadEndBlocksAnalysis()
      : FunctionAnalysisBase<DeadEndBlocks>(SILAnalysisKind::DeadEndBlocks) {}

  DeadEndBlocksAnalysis(const DeadEndBlocksAnalysis &) = delete;
  DeadEndBlocksAnalysis &operator=(const DeadEndBlocksAnalysis &) = delete;

  static SILAnalysisKind getAnalysisKind() {
    return SILAnalysisKind::DeadEndBlocks;
  }

  static bool classof(const SILAnalysis *s) {
    return s->getKind() == SILAnalysisKind::DeadEndBlocks;
  }

  std::unique_ptr<DeadEndBlocks> newFunctionAnalysis(SILFunction *f) override {
    return std::make_unique<DeadEndBlocks>(f);
  }

  bool shouldInvalidate(SILAnalysis::InvalidationKind k) override {
    return k & InvalidationKind::Branches;
  }

protected:
  void verify(DeadEndBlocks *deBlocks) const override;
};

} // namespace language

#endif
