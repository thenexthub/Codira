//===--- FunctionOrderPrinter.cpp - Function ordering test pass -----------===//
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
// This pass prints a bottom-up ordering of functions in the module (in the
// sense that each function is printed before functions that call it).
//
//===----------------------------------------------------------------------===//

#include "language/SILOptimizer/Analysis/FunctionOrder.h"
#include "language/Demangling/Demangle.h"
#include "language/SILOptimizer/Analysis/BasicCalleeAnalysis.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILModule.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language;

#define DEBUG_TYPE "function-order-printer"

namespace {

class FunctionOrderPrinterPass : public SILModuleTransform {
  BasicCalleeAnalysis *BCA;

  /// The entry point to the transformation.
  void run() override {
    BCA = getAnalysis<BasicCalleeAnalysis>();
    auto &M = *getModule();
    BottomUpFunctionOrder Orderer(M, BCA);

    toolchain::outs() << "Bottom up function order:\n";
    auto SCCs = Orderer.getSCCs();
    for (auto &SCC : SCCs) {
      std::string Indent;

      if (SCC.size() != 1) {
        toolchain::outs() << "Non-trivial SCC:\n";
        Indent = std::string(2, ' ');
      }

      for (auto *F : SCC) {
        toolchain::outs() << Indent
                     << Demangle::demangleSymbolAsString(F->getName())
                     << "\n";
      }
    }
    toolchain::outs() << "\n";
  }

};

} // end anonymous namespace

SILTransform *language::createFunctionOrderPrinter() {
  return new FunctionOrderPrinterPass();
}
