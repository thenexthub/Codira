//===--- CallerAnalysisPrinter.cpp - Caller Analysis test pass ------------===//
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
// This pass prints all the callsites of every function in the module.
//
//===----------------------------------------------------------------------===//

#include "language/SIL/SILFunction.h"
#include "language/SIL/SILModule.h"
#include "language/SILOptimizer/Analysis/CallerAnalysis.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "toolchain/Support/YAMLTraits.h"
#include "toolchain/Support/raw_ostream.h"

using namespace language;

#define DEBUG_TYPE "caller-analysis-printer"

namespace {

/// A pass that dumps the caller analysis state in yaml form. Intended to allow
/// for visualizing of the caller analysis via external data visualization and
/// analysis programs.
class CallerAnalysisPrinterPass : public SILModuleTransform {
  /// The entry point to the transformation.
  void run() override {
    auto *CA = getAnalysis<CallerAnalysis>();
    CA->print(toolchain::outs());
  }
};

} // end anonymous namespace

SILTransform *language::createCallerAnalysisPrinter() {
  return new CallerAnalysisPrinterPass();
}
