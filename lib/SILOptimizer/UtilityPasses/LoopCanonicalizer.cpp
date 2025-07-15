//===--- LoopCanonicalizer.cpp --------------------------------------------===//
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
/// \file
/// This is a simple pass that can be used to apply loop canonicalizations to a
/// cfg. It also enables loop canonicalizations to be tested via FileCheck.
///
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "sil-loop-canonicalizer"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/Analysis/Analysis.h"
#include "language/SILOptimizer/Analysis/DominanceAnalysis.h"
#include "language/SILOptimizer/Analysis/LoopAnalysis.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SILOptimizer/Utils/LoopUtils.h"

using namespace language;

namespace {

class LoopCanonicalizer : public SILFunctionTransform {

  void run() override {
    SILFunction *F = getFunction();

    TOOLCHAIN_DEBUG(toolchain::dbgs() << "Attempt to canonicalize loops in "
                            << F->getName() << "\n");

    auto *LA = PM->getAnalysis<SILLoopAnalysis>();
    auto *LI = LA->get(F);

    if (LI->empty()) {
      TOOLCHAIN_DEBUG(toolchain::dbgs() << "    No loops to canonicalize!\n");
      return;
    }

    auto *DA = PM->getAnalysis<DominanceAnalysis>();
    auto *DI = DA->get(F);
    if (canonicalizeAllLoops(DI, LI)) {
      // We preserve loop info and the dominator tree.
      DA->lockInvalidation();
      LA->lockInvalidation();
      PM->invalidateAnalysis(F, SILAnalysis::InvalidationKind::FunctionBody);
      DA->unlockInvalidation();
      LA->unlockInvalidation();
    }
  }

};

} // end anonymous namespace

//===----------------------------------------------------------------------===//
//                            Top Level Entrypoint
//===----------------------------------------------------------------------===//

SILTransform *language::createLoopCanonicalizer() {
  return new LoopCanonicalizer();
}
