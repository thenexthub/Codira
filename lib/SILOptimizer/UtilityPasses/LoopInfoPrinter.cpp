//===--- LoopInfoPrinter.cpp - Print SIL Loop Info ------------------------===//
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

#include "language/Basic/Assertions.h"
#include "language/SILOptimizer/Analysis/LoopAnalysis.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SIL/SILModule.h"
#include "language/SIL/SILVisitor.h"
#include "toolchain/ADT/Statistic.h"

using namespace language;

namespace {

class LoopInfoPrinter : public SILModuleTransform {

  /// The entry point to the transformation.
  void run() override {
    SILLoopAnalysis *LA = PM->getAnalysis<SILLoopAnalysis>();
    assert(LA && "Invalid LoopAnalysis");
    for (auto &Fn : *getModule()) {
      if (Fn.isExternalDeclaration()) continue;

      SILLoopInfo *LI = LA->get(&Fn);
      assert(LI && "Invalid loop info for function");


      if (LI->empty()) {
        toolchain::errs() << "No loops in " << Fn.getName() << "\n";
        return;
      }

      toolchain::errs() << "Loops in " << Fn.getName() << "\n";
      for (auto *LoopIt : *LI) {
        LoopIt->dump();
      }
    }
  }
};

} // end anonymous namespace


SILTransform *language::createLoopInfoPrinter() {
  return new LoopInfoPrinter();
}

