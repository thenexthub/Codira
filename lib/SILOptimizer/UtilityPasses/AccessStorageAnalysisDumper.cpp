//===--- AccessStorageAnalysisDumper.cpp - accessed storage analysis ----===//
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

#define DEBUG_TYPE "sil-accessed-storage-analysis-dumper"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILValue.h"
#include "language/SILOptimizer/Analysis/AccessStorageAnalysis.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "llvm/Support/Debug.h"

using namespace language;

namespace {

/// Dumps per-function information on dynamically enforced formal accesses.
class AccessStorageAnalysisDumper : public SILModuleTransform {

  void run() override {
    auto *analysis = PM->getAnalysis<AccessStorageAnalysis>();

    for (auto &fn : *getModule()) {
      llvm::outs() << "@" << fn.getName() << "\n";
      if (fn.empty()) {
        llvm::outs() << "<unknown>\n";
        continue;
      }
      const FunctionAccessStorage &summary = analysis->getEffects(&fn);
      summary.print(llvm::outs());
    }
  }
};

} // end anonymous namespace

SILTransform *swift::createAccessStorageAnalysisDumper() {
  return new AccessStorageAnalysisDumper();
}
