//===--- AccessSummaryDumper.cpp - Dump access summaries for functions -----===//
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

#define DEBUG_TYPE "sil-access-summary-dumper"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILInstruction.h"
#include "language/SIL/SILValue.h"
#include "language/SILOptimizer/Analysis/AccessSummaryAnalysis.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "toolchain/Support/Debug.h"

using namespace language;

namespace {

/// Dumps summaries of kinds of accesses a function performs on its
/// @inout_aliasiable arguments.
class AccessSummaryDumper : public SILModuleTransform {

  void run() override {
    auto *analysis = PM->getAnalysis<AccessSummaryAnalysis>();

    for (auto &fn : *getModule()) {
      toolchain::outs() << "@" << fn.getName() << "\n";
      if (fn.empty()) {
        toolchain::outs() << "<unknown>\n";
        continue;
      }
      const AccessSummaryAnalysis::FunctionSummary &summary =
          analysis->getOrCreateSummary(&fn);
      summary.print(toolchain::outs(), &fn);
      toolchain::outs() << "\n";
    }
  }
};

} // end anonymous namespace

SILTransform *language::createAccessSummaryDumper() {
  return new AccessSummaryDumper();
}
