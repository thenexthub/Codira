//===--- EpilogueARCMatcherDumper.cpp - Find Epilogue Releases ------------===//
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

#define DEBUG_TYPE "sil-epilogue-arc-dumper"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILValue.h"
#include "language/SILOptimizer/Analysis/Analysis.h"
#include "language/SILOptimizer/Analysis/AliasAnalysis.h"
#include "language/SILOptimizer/Analysis/EpilogueARCAnalysis.h"
#include "language/SILOptimizer/Analysis/RCIdentityAnalysis.h"
#include "language/SILOptimizer/PassManager/Transforms.h"

using namespace language;

//===----------------------------------------------------------------------===//
//                              Top Level Driver
//===----------------------------------------------------------------------===//

namespace {

/// Find and dump the epilogue release instructions for the arguments.
class SILEpilogueARCMatcherDumper : public SILModuleTransform {
  void run() override {
    for (auto &F: *getModule()) {
      // Function is not definition.
      if (!F.isDefinition())
        continue;

      // Find the epilogue releases of each owned argument. 
      for (auto Arg : F.getArguments()) {
        auto *EA = PM->getAnalysis<EpilogueARCAnalysis>()->get(&F);
        llvm::outs() <<"START: " <<  F.getName() << "\n";
        llvm::outs() << *Arg;

        // Find the retain instructions for the argument.
        llvm::SmallSetVector<SILInstruction *, 1> RelInsts = 
          EA->computeEpilogueARCInstructions(EpilogueARCContext::EpilogueARCKind::Release,
                                             Arg);
        for (auto I : RelInsts) {
          llvm::outs() << *I << "\n";
        }

        // Find the release instructions for the argument.
        llvm::SmallSetVector<SILInstruction *, 1> RetInsts = 
          EA->computeEpilogueARCInstructions(EpilogueARCContext::EpilogueARCKind::Retain,
                                             Arg);
        for (auto I : RetInsts) {
          llvm::outs() << *I << "\n";
        }

        llvm::outs() <<"FINISH: " <<  F.getName() << "\n";
      }
    }
  }

};
        
} // end anonymous namespace

SILTransform *swift::createEpilogueARCMatcherDumper() {
  return new SILEpilogueARCMatcherDumper();
}
