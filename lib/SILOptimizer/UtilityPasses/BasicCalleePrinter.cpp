//===--- BasicCalleePrinter.cpp - Callee cache printing pass --------------===//
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
// This pass prints the callees of functions as determined by the
// BasicCalleeAnalysis. The pass is strictly for testing that
// analysis.
//
//===----------------------------------------------------------------------===//

#include "language/SILOptimizer/Analysis/BasicCalleeAnalysis.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILModule.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

using namespace language;

#define DEBUG_TYPE "basic-callee-printer"

namespace {

class BasicCalleePrinterPass : public SILModuleTransform {
  BasicCalleeAnalysis *BCA;

  void printCallees(FullApplySite FAS) {
    llvm::outs() << "Function call site:\n";
    if (auto *Callee = FAS.getCallee()->getDefiningInstruction())
      llvm::outs() << *Callee;
    llvm::outs() << *FAS.getInstruction();

    auto Callees = BCA->getCalleeList(FAS);
    Callees.print(llvm::outs());
  }

  /// The entry point to the transformation.
  void run() override {
    BCA = getAnalysis<BasicCalleeAnalysis>();
    for (auto &Fn : *getModule()) {
      if (Fn.isExternalDeclaration()) continue;
      for (auto &B : Fn)
        for (auto &I : B)
          if (auto FAS = FullApplySite::isa(&I))
            printCallees(FAS);
    }
  }

};

} // end anonymous namespace

SILTransform *swift::createBasicCalleePrinter() {
  return new BasicCalleePrinterPass();
}
