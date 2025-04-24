//===--- ComputeDominanceInfo.cpp -----------------------------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
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
