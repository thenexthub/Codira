//===--- Devirtualizer.cpp - Devirtualize indirect calls  -----------------===//
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
// Devirtualize indirect calls to functions, turning them into direct function
// references.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "sil-devirtualizer"

#include "language/Basic/Assertions.h"
#include "language/SIL/OptimizationRemark.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILInstruction.h"
#include "language/SILOptimizer/Analysis/ClassHierarchyAnalysis.h"
#include "language/SILOptimizer/Utils/Devirtualize.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "toolchain/ADT/SmallVector.h"

using namespace language;

namespace {

class Devirtualizer : public SILFunctionTransform {
  bool Changed = false;
  bool ChangedCFG = false;

  void devirtualizeAppliesInFunction(SILFunction &F,
                                     ClassHierarchyAnalysis *CHA);

  /// The entry point to the transformation.
  void run() override {
    SILFunction &F = *getFunction();
    ClassHierarchyAnalysis *CHA = PM->getAnalysis<ClassHierarchyAnalysis>();
    TOOLCHAIN_DEBUG(toolchain::dbgs() << "***** Devirtualizer on function:" << F.getName()
                            << " *****\n");

    Changed = false;
    ChangedCFG = false;
    devirtualizeAppliesInFunction(F, CHA);
    if (ChangedCFG)
      invalidateAnalysis(SILAnalysis::InvalidationKind::FunctionBody);
    else if (Changed)
      invalidateAnalysis(SILAnalysis::InvalidationKind::CallsAndInstructions);
  }

};

} // end anonymous namespace

// Return true if any calls changed, and true if the CFG also changed.
void Devirtualizer::devirtualizeAppliesInFunction(SILFunction &F,
                                                  ClassHierarchyAnalysis *CHA) {
  toolchain::SmallVector<ApplySite, 8> NewApplies;
  OptRemark::Emitter ORE(DEBUG_TYPE, F);

  SmallVector<ApplySite, 16> Applies;
  for (auto &BB : F) {
    for (auto It = BB.begin(), End = BB.end(); It != End;) {
      auto &I = *It++;

      // Skip non-apply instructions.

      auto Apply = ApplySite::isa(&I);
      if (!Apply)
        continue;
      Applies.push_back(Apply);
   }
  }
  for (auto Apply : Applies) {
    ApplySite NewInst;
    bool modifiedCFG;
    std::tie(NewInst, modifiedCFG) = tryDevirtualizeApply(getPassManager(), Apply, CHA, &ORE);
    if (!NewInst)
      continue;

    Changed = true;
    ChangedCFG |= modifiedCFG;

    deleteDevirtualizedApply(Apply);
    NewApplies.push_back(NewInst);
  }

  // For each new apply, attempt to link in function bodies if we do
  // not already have them, then notify the pass manager of the new
  // functions.
  //
  // We do this after deleting the old applies because otherwise we
  // hit verification errors in the linking code due to having
  // non-cond_br critical edges.
  while (!NewApplies.empty()) {
    auto Apply = NewApplies.pop_back_val();

    auto *CalleeFn = Apply.getInitiallyReferencedFunction();
    assert(CalleeFn && "Expected devirtualized callee!");

    // We need to ensure that we link after devirtualizing in order to pull in
    // everything we reference from another module, which may expose optimization
    // opportunities and is also needed for correctness if we reference functions
    // with non-public linkage. See lib/SIL/Linker.cpp for details.
    if (!CalleeFn->isDefinition())
      F.getModule().linkFunction(CalleeFn, SILModule::LinkingMode::LinkAll);

    // We may not have optimized these functions yet, and it could
    // be beneficial to rerun some earlier passes on the current
    // function now that we've made these direct references visible.
    if (CalleeFn->isDefinition() && CalleeFn->shouldOptimize())
      addFunctionToPassManagerWorklist(CalleeFn, nullptr);
  }
}

SILTransform *language::createDevirtualizer() { return new Devirtualizer(); }
