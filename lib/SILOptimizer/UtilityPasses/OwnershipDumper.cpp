//===--- OwnershipDumper.cpp ----------------------------------------------===//
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
/// This is a simple utility pass that dumps the ValueOwnershipKind of all
/// SILValue in a module. It is meant to trigger assertions and verification of
/// these values.
///
//===----------------------------------------------------------------------===//

#include "language/Basic/Assertions.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILInstruction.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/PassManager/Transforms.h"

using namespace language;

//===----------------------------------------------------------------------===//
//                               Implementation
//===----------------------------------------------------------------------===//

static void dumpInstruction(SILInstruction &ii) {
  llvm::outs() << "Visiting: " << ii;

  auto ops = ii.getAllOperands();
  if (!ops.empty()) {
    llvm::outs() << "Ownership Constraint:\n";
    for (const auto &op : ops) {
      llvm::outs() << "Op #: " << op.getOperandNumber() << "\n"
                      "Constraint: " << op.getOwnershipConstraint() << "\n";
    }
  }

  // If the instruction doesn't have any results, bail.
  auto results = ii.getResults();
  if (!results.empty()) {
    llvm::outs() << "Results Ownership Kinds:\n";
    for (auto v : results) {
      auto kind = v->getOwnershipKind();
      llvm::outs() << "Result: " << v;
      llvm::outs() << "Kind: " << kind << "\n";
    }
  }
}

//===----------------------------------------------------------------------===//
//                            Top Level Entrypoint
//===----------------------------------------------------------------------===//

namespace {

class OwnershipDumper : public SILFunctionTransform {
  void run() override {
    SILFunction *f = getFunction();
    llvm::outs() << "*** Dumping Function: '" << f->getName() << "'\n";
    for (auto &bb : *f) {
      // We only dump instructions right now.
      for (auto &ii : bb) {
        dumpInstruction(ii);
      }
    }
  }
};

} // end anonymous namespace

SILTransform *swift::createOwnershipDumper() { return new OwnershipDumper(); }
