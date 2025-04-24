//===--- StripDebugInfo.cpp - Strip debug info from SIL -------------------===//
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

#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILModule.h"
#include "language/SIL/SILFunction.h"


using namespace language;

static void stripFunction(SILFunction *F) {
  for (auto &BB : *F)
    for (auto II = BB.begin(), IE = BB.end(); II != IE;) {
      SILInstruction *Inst = &*II;
      ++II;

      if (!isa<DebugValueInst>(Inst))
        continue;

      Inst->eraseFromParent();
    }
}

namespace {
class StripDebugInfo : public swift::SILFunctionTransform {
  ~StripDebugInfo() override {}

  /// The entry point to the transformation.
  void run() override {
    stripFunction(getFunction());
    invalidateAnalysis(SILAnalysis::InvalidationKind::Instructions);
  }

};
} // end anonymous namespace


SILTransform *swift::createStripDebugInfo() {
  return new StripDebugInfo();
}
