//===--- CFGPrinter.cpp - CFG printer pass --------------------------------===//
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
// This file defines external functions that can be called to explicitly
// instantiate the CFG printer.
//
//===----------------------------------------------------------------------===//

#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "language/SIL/CFG.h"
#include "language/SIL/SILBasicBlock.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILFunction.h"
#include "llvm/Support/CommandLine.h"

using namespace language;

//===----------------------------------------------------------------------===//
//                                  Options
//===----------------------------------------------------------------------===//

llvm::cl::opt<std::string> SILViewCFGOnlyFun(
    "sil-view-cfg-only-function", llvm::cl::init(""),
    llvm::cl::desc("Only produce a graphviz file for this function"));

llvm::cl::opt<std::string> SILViewCFGOnlyFuns(
    "sil-view-cfg-only-functions", llvm::cl::init(""),
    llvm::cl::desc("Only produce a graphviz file for the sil for the functions "
                   "whose name contains this substring"));

//===----------------------------------------------------------------------===//
//                              Top Level Driver
//===----------------------------------------------------------------------===//

namespace {

class SILCFGPrinter : public SILFunctionTransform {
  /// The entry point to the transformation.
  void run() override {
    SILFunction *F = getFunction();

    // If we are not supposed to dump view this cfg, return.
    if (!SILViewCFGOnlyFun.empty() && F && F->getName() != SILViewCFGOnlyFun)
      return;
    if (!SILViewCFGOnlyFuns.empty() && F &&
        !F->getName().contains(SILViewCFGOnlyFuns))
      return;

    F->viewCFG();
  }
};

} // end anonymous namespace

SILTransform *swift::createCFGPrinter() {
  return new SILCFGPrinter();
}
