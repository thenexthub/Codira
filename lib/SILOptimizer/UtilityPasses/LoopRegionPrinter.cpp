//===--- LoopRegionPrinter.cpp --------------------------------------------===//
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
// Simple pass for testing the new loop region dumper analysis. Prints out
// information suitable for checking with filecheck.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "sil-loop-region-printer"

#include "language/SILOptimizer/Analysis/LoopRegionAnalysis.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "llvm/Support/CommandLine.h"

using namespace language;

static llvm::cl::opt<std::string>
    SILViewCFGOnlyFun("sil-loop-region-view-cfg-only-function",
                      llvm::cl::init(""),
                      llvm::cl::desc("Only produce a graphviz file for the "
                                     "loop region info of this function"));

static llvm::cl::opt<std::string>
    SILViewCFGOnlyFuns("sil-loop-region-view-cfg-only-functions",
                       llvm::cl::init(""),
                       llvm::cl::desc("Only produce a graphviz file for the "
                                      "loop region info for the functions "
                                      "whose name contains this substring"));

namespace {

class LoopRegionViewText : public SILModuleTransform {
  void run() override {
    invalidateAll();
    auto *lra = PM->getAnalysis<LoopRegionAnalysis>();

    for (auto &fn : *getModule()) {
      if (fn.isExternalDeclaration())
        continue;
      if (!SILViewCFGOnlyFun.empty() && fn.getName() != SILViewCFGOnlyFun)
        continue;
      if (!SILViewCFGOnlyFuns.empty() &&
          !fn.getName().contains(SILViewCFGOnlyFuns))
        continue;

      // Ok, we are going to analyze this function. Invalidate all state
      // associated with it so we recompute the loop regions.
      llvm::outs() << "Start @" << fn.getName() << "@\n";
      lra->get(&fn)->dump();
      llvm::outs() << "End @" << fn.getName() << "@\n";
      llvm::outs().flush();
    }
  }
};

class LoopRegionViewCFG : public SILModuleTransform {
  void run() override {
    invalidateAll();
    auto *lra = PM->getAnalysis<LoopRegionAnalysis>();

    for (auto &fn : *getModule()) {
      if (fn.isExternalDeclaration())
        continue;
      if (!SILViewCFGOnlyFun.empty() && fn.getName() != SILViewCFGOnlyFun)
        continue;
      if (!SILViewCFGOnlyFuns.empty() &&
          !fn.getName().contains(SILViewCFGOnlyFuns))
        continue;

      // Ok, we are going to analyze this function. Invalidate all state
      // associated with it so we recompute the loop regions.
      lra->get(&fn)->viewLoopRegions();
    }
  }
};

} // end anonymous namespace

SILTransform *swift::createLoopRegionViewText() {
  return new LoopRegionViewText();
}

SILTransform *swift::createLoopRegionViewCFG() {
  return new LoopRegionViewCFG();
}
