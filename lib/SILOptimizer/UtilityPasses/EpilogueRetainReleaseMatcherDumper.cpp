//===--- EpilogueRetainReleaseMatcherDumper.cpp - Find Epilogue Releases --===//
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
/// This pass finds the epilogue releases matched to each argument of the
/// function.
///
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "sil-epilogue-release-dumper"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILValue.h"
#include "language/SILOptimizer/Analysis/AliasAnalysis.h"
#include "language/SILOptimizer/Analysis/ARCAnalysis.h"
#include "language/SILOptimizer/Analysis/Analysis.h"
#include "language/SILOptimizer/Analysis/RCIdentityAnalysis.h"
#include "language/SILOptimizer/PassManager/Transforms.h"

using namespace language;

//===----------------------------------------------------------------------===//
//                              Top Level Driver
//===----------------------------------------------------------------------===//

namespace {

/// Find and dump the epilogue release instructions for the arguments.
class SILEpilogueRetainReleaseMatcherDumper : public SILModuleTransform {

  void run() override {
    auto *RCIA = getAnalysis<RCIdentityAnalysis>();
    for (auto &Fn: *getModule()) {
      // Function is not definition.
      if (!Fn.isDefinition())
        continue;

      auto *AA = PM->getAnalysis<AliasAnalysis>(&Fn);

      toolchain::outs() << "START: sil @" << Fn.getName() << "\n";

      // Handle @owned return value.
      ConsumedResultToEpilogueRetainMatcher RetMap(RCIA->get(&Fn), AA, &Fn); 
      for (auto &RI : RetMap)
        toolchain::outs() << *RI;

      // Handle @owned function arguments.
      ConsumedArgToEpilogueReleaseMatcher RelMap(RCIA->get(&Fn), &Fn); 
      // Iterate over arguments and dump their epilogue releases.
      for (auto Arg : Fn.getArguments()) {
        toolchain::outs() << *Arg;
        // Can not find an epilogue release instruction for the argument.
        for (auto &RI : RelMap.getReleasesForArgument(Arg))
          toolchain::outs() << *RI;
      }

      toolchain::outs() << "END: sil @" << Fn.getName() << "\n";
    }
  }

};
        
} // end anonymous namespace

SILTransform *language::createEpilogueRetainReleaseMatcherDumper() {
  return new SILEpilogueRetainReleaseMatcherDumper();
}
