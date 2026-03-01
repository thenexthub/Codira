//===- CompositePass.cpp - Composite pass code ----------------------------===//
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
//
// CompositePass allows to run set of passes until fixed point is reached.
//
//===----------------------------------------------------------------------===//

#include "mlir/Transforms/Passes.h"

#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassManager.h"

namespace mlir {
#define GEN_PASS_DEF_COMPOSITEFIXEDPOINTPASS
#include "mlir/Transforms/Passes.h.inc"
} // namespace mlir

using namespace mlir;

namespace {
struct CompositeFixedPointPass final
    : public impl::CompositeFixedPointPassBase<CompositeFixedPointPass> {
  using CompositeFixedPointPassBase::CompositeFixedPointPassBase;

  CompositeFixedPointPass(
      std::string name_, toolchain::function_ref<void(OpPassManager &)> populateFunc,
      int maxIterations) {
    name = std::move(name_);
    maxIter = maxIterations;
    populateFunc(dynamicPM);

    toolchain::raw_string_ostream os(pipelineStr);
    toolchain::interleave(
        dynamicPM, [&](mlir::Pass &pass) { pass.printAsTextualPipeline(os); },
        [&]() { os << ","; });
  }

  LogicalResult initializeOptions(
      StringRef options,
      function_ref<LogicalResult(const Twine &)> errorHandler) override {
    if (failed(CompositeFixedPointPassBase::initializeOptions(options,
                                                              errorHandler)))
      return failure();

    if (failed(parsePassPipeline(pipelineStr, dynamicPM)))
      return errorHandler("Failed to parse composite pass pipeline");

    return success();
  }

  LogicalResult initialize(MLIRContext *context) override {
    if (maxIter <= 0)
      return emitError(UnknownLoc::get(context))
             << "Invalid maxIterations value: " << maxIter << "\n";

    return success();
  }

  void getDependentDialects(DialectRegistry &registry) const override {
    dynamicPM.getDependentDialects(registry);
  }

  void runOnOperation() override {
    auto *op = getOperation();
    OperationFingerPrint fp(op);

    int currentIter = 0;
    int maxIterVal = maxIter;
    while (true) {
      if (failed(runPipeline(dynamicPM, op)))
        return signalPassFailure();

      if (currentIter++ >= maxIterVal) {
        op->emitWarning("Composite pass \"" + toolchain::Twine(name) +
                        "\"+ didn't converge in " + toolchain::Twine(maxIterVal) +
                        " iterations");
        break;
      }

      OperationFingerPrint newFp(op);
      if (newFp == fp)
        break;

      fp = newFp;
    }
  }

protected:
  toolchain::StringRef getName() const override { return name; }

private:
  OpPassManager dynamicPM;
};
} // namespace

std::unique_ptr<Pass> mlir::createCompositeFixedPointPass(
    std::string name, toolchain::function_ref<void(OpPassManager &)> populateFunc,
    int maxIterations) {

  return std::make_unique<CompositeFixedPointPass>(std::move(name),
                                                   populateFunc, maxIterations);
}
