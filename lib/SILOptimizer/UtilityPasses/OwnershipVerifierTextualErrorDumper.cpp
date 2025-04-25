//===--- OwnershipVerifierStateDumper.cpp ---------------------------------===//
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
/// This is a simple utility pass that verifies the ownership of all SILValue in
/// a module with a special flag set that causes the verifier to emit textual
/// errors instead of asserting. This is done one function at a time so that we
/// can number the errors as we emit them so in FileCheck tests, we can be 100%
/// sure we exactly matched the number of errors.
///
//===----------------------------------------------------------------------===//

#include "language/Basic/Assertions.h"
#include "language/SIL/BasicBlockUtils.h"
#include "language/SIL/SILFunction.h"
#include "language/SIL/SILInstruction.h"
#include "language/SILOptimizer/Analysis/DeadEndBlocksAnalysis.h"
#include "language/SILOptimizer/PassManager/Passes.h"
#include "language/SILOptimizer/PassManager/Transforms.h"

using namespace language;

//===----------------------------------------------------------------------===//
//                            Top Level Entrypoint
//===----------------------------------------------------------------------===//

namespace {

class OwnershipVerifierTextualErrorDumper : public SILFunctionTransform {
  void run() override {
    SILFunction *f = getFunction();
    auto *deBlocksAnalysis = getAnalysis<DeadEndBlocksAnalysis>();
    f->verifyOwnership(f->getModule().getOptions().OSSAVerifyComplete
                           ? nullptr
                           : deBlocksAnalysis->get(f));
  }
};

} // end anonymous namespace

SILTransform *swift::createOwnershipVerifierTextualErrorDumper() {
  return new OwnershipVerifierTextualErrorDumper();
}
