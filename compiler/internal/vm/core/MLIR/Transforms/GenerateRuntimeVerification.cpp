//===- RuntimeOpVerification.cpp - Op Verification ------------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//

#include "mlir/IR/AsmState.h"
#include "mlir/Transforms/Passes.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/Operation.h"
#include "mlir/Interfaces/RuntimeVerifiableOpInterface.h"

namespace mlir {
#define GEN_PASS_DEF_GENERATERUNTIMEVERIFICATION
#include "mlir/Transforms/Passes.h.inc"
} // namespace mlir

using namespace mlir;

namespace {
struct GenerateRuntimeVerificationPass
    : public impl::GenerateRuntimeVerificationBase<
          GenerateRuntimeVerificationPass> {
  void runOnOperation() override;
};

/// Default error message generator for runtime verification failures.
///
/// This class generates error messages with different levels of verbosity:
/// - Level 0: Shows only the error message and operation location
/// - Level 1: Shows the full operation string, error message, and location
///
/// Clients can call getVerboseLevel() to retrieve the current verbose level
/// and use it to customize their own error message generators with similar
/// behavior patterns.
class DefaultErrMsgGenerator {
private:
  unsigned vLevel;
  AsmState &state;

public:
  DefaultErrMsgGenerator(unsigned verboseLevel, AsmState &asmState)
      : vLevel(verboseLevel), state(asmState) {}

  std::string operator()(Operation *op, StringRef msg) {
    std::string buffer;
    toolchain::raw_string_ostream stream(buffer);
    stream << "ERROR: Runtime op verification failed\n";
    if (vLevel == 1) {
      op->print(stream, state);
      stream << "\n^ " << msg;
    } else {
      stream << "^ " << msg;
    }
    stream << "\nLocation: ";
    op->getLoc().print(stream);
    return buffer;
  }

  unsigned getVerboseLevel() const { return vLevel; }
};
} // namespace

void GenerateRuntimeVerificationPass::runOnOperation() {
  // Check verboseLevel is in range [0, 1].
  if (verboseLevel > 1) {
    getOperation()->emitError(
        "generate-runtime-verification pass: set verboseLevel to 0 or 1");
    signalPassFailure();
    return;
  }

  // The implementation of the RuntimeVerifiableOpInterface may create ops that
  // can be verified. We don't want to generate verification for IR that
  // performs verification, so gather all runtime-verifiable ops first.
  SmallVector<RuntimeVerifiableOpInterface> ops;
  getOperation()->walk([&](RuntimeVerifiableOpInterface verifiableOp) {
    ops.push_back(verifiableOp);
  });

  // We may generate a lot of error messages and so we need to ensure the
  // printing is fast.
  OpPrintingFlags flags;
  flags.elideLargeElementsAttrs();
  flags.skipRegions();
  flags.useLocalScope();
  AsmState state(getOperation(), flags);

  // Client can call getVerboseLevel() to fetch verbose level.
  DefaultErrMsgGenerator defaultErrMsgGenerator(verboseLevel.getValue(), state);

  OpBuilder builder(getOperation()->getContext());
  for (RuntimeVerifiableOpInterface verifiableOp : ops) {
    builder.setInsertionPoint(verifiableOp);
    verifiableOp.generateRuntimeVerification(builder, verifiableOp.getLoc(),
                                             defaultErrMsgGenerator);
  };
}

std::unique_ptr<Pass> mlir::createGenerateRuntimeVerificationPass() {
  return std::make_unique<GenerateRuntimeVerificationPass>();
}
