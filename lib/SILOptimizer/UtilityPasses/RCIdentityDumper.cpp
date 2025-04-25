//===--- RCIdentityDumper.cpp ---------------------------------------------===//
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
///
/// This pass applies the RCIdentityAnalysis to all SILValues in a function in
/// order to apply FileCheck testing to RCIdentityAnalysis without needing to
/// test any other passes.
///
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "sil-rc-identity-dumper"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SIL/SILArgument.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILValue.h"
#include "language/SILOptimizer/Analysis/RCIdentityAnalysis.h"
#include "language/SILOptimizer/PassManager/Transforms.h"
#include "llvm/Support/Debug.h"

using namespace language;

namespace {

/// Dumps the alias relations between all instructions of a function.
class RCIdentityDumper : public SILFunctionTransform {

  void run() override {
    auto *Fn = getFunction();
    auto *RCId = PM->getAnalysis<RCIdentityAnalysis>()->get(Fn);

    std::vector<std::pair<SILValue, SILValue>> Results;
    unsigned ValueCount = 0;
    llvm::MapVector<SILValue, uint64_t> ValueToValueIDMap;

    llvm::outs() << "@" << Fn->getName() << "@\n";

    for (auto &BB : *Fn) {
      for (auto *Arg : BB.getArguments()) {
        ValueToValueIDMap[Arg] = ValueCount++;
        Results.push_back({Arg, RCId->getRCIdentityRoot(Arg)});
      }
      for (auto &II : BB) {
        for (auto V : II.getResults()) {
          ValueToValueIDMap[V] = ValueCount++;
          Results.push_back({V, RCId->getRCIdentityRoot(V)});
        }
      }
    }

    llvm::outs() << "ValueMap:\n";
    for (auto P : ValueToValueIDMap) {
      llvm::outs() << "\tValueMap[" << P.second << "] = " << P.first;
    }

    unsigned ResultCount = 0;
    for (auto P : Results) {
      llvm::outs() << "RESULT #" << ResultCount++ << ": "
                   << ValueToValueIDMap[P.first] << " = "
                   << ValueToValueIDMap[P.second] << "\n";
    }

    llvm::outs() << "\n";
  }

};

} // end anonymous namespace

SILTransform *swift::createRCIdentityDumper() { return new RCIdentityDumper(); }
