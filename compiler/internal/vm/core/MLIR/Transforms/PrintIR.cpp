//===- PrintIR.cpp - Pass to dump IR on debug stream ----------------------===//
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

#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/Passes.h"
#include "vm/core/Support/Debug.h"

namespace mlir {
namespace {

#define GEN_PASS_DEF_PRINTIRPASS
#include "mlir/Transforms/Passes.h.inc"

struct PrintIRPass : public impl::PrintIRPassBase<PrintIRPass> {
  using impl::PrintIRPassBase<PrintIRPass>::PrintIRPassBase;

  void runOnOperation() override {
    toolchain::dbgs() << "// -----// IR Dump";
    if (!this->label.empty())
      toolchain::dbgs() << " " << this->label;
    toolchain::dbgs() << " //----- //\n";
    getOperation()->dump();
    markAllAnalysesPreserved();
  }
};

} // namespace

std::unique_ptr<Pass> createPrintIRPass(const PrintIRPassOptions &options) {
  return std::make_unique<PrintIRPass>(options);
}

} // namespace mlir
