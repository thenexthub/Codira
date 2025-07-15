//===--- DifferentiabilityWitnessDevirtualizer.cpp ------------------------===//
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
//
// Devirtualizes `differentiability_witness_function` instructions into
// `function_ref` instructions for differentiability witness definitions.
//
//===----------------------------------------------------------------------===//

#include "language/Basic/Assertions.h"
#include "language/SIL/SILBuilder.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILInstruction.h"
#include "language/SILOptimizer/PassManager/Transforms.h"

using namespace language;

namespace {
class DifferentiabilityWitnessDevirtualizer : public SILFunctionTransform {

  /// Returns true if any changes were made.
  bool devirtualizeDifferentiabilityWitnessesInFunction(SILFunction &f);

  /// The entry point to the transformation.
  void run() override {
    if (devirtualizeDifferentiabilityWitnessesInFunction(*getFunction()))
      invalidateAnalysis(SILAnalysis::InvalidationKind::CallsAndInstructions);
  }
};
} // end anonymous namespace

bool DifferentiabilityWitnessDevirtualizer::
    devirtualizeDifferentiabilityWitnessesInFunction(SILFunction &f) {
  bool changed = false;
  toolchain::SmallVector<DifferentiabilityWitnessFunctionInst *, 8> insts;
  for (auto &bb : f) {
    for (auto &inst : bb) {
      auto *dfwi = dyn_cast<DifferentiabilityWitnessFunctionInst>(&inst);
      if (!dfwi)
        continue;
      insts.push_back(dfwi);
    }
  }
  for (auto *inst : insts) {
    auto *witness = inst->getWitness();
    if (witness->isDeclaration())
      f.getModule().loadDifferentiabilityWitness(witness);
    if (witness->isDeclaration())
      continue;
    changed = true;
    SILBuilderWithScope builder(inst);
    auto kind = inst->getWitnessKind().getAsDerivativeFunctionKind();
    assert(kind.has_value());
    auto *newInst = builder.createFunctionRefFor(inst->getLoc(),
                                                 witness->getDerivative(*kind));
    inst->replaceAllUsesWith(newInst);
    inst->getParent()->erase(inst);
  }
  return changed;
}

SILTransform *language::createDifferentiabilityWitnessDevirtualizer() {
  return new DifferentiabilityWitnessDevirtualizer();
}
