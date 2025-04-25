//===--- ComputeDominanceInfo.cpp -----------------------------------------===//
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

#define DEBUG_TYPE "sil-compute-dominance-info"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SILOptimizer/Analysis/DominanceAnalysis.h"

using namespace language;

class ComputeDominanceInfo : public SILFunctionTransform {

  void run() override {
    PM->getAnalysis<DominanceAnalysis>()->get(getFunction());
    PM->getAnalysis<PostDominanceAnalysis>()->get(getFunction());
  }

};

SILTransform *swift::createComputeDominanceInfo() { return new ComputeDominanceInfo(); }
