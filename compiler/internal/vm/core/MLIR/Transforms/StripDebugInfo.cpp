//===- StripDebugInfo.cpp - Pass to strip debug information ---------------===//
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

#include "mlir/Transforms/Passes.h"

#include "mlir/IR/Operation.h"
#include "mlir/Pass/Pass.h"

namespace mlir {
#define GEN_PASS_DEF_STRIPDEBUGINFO
#include "mlir/Transforms/Passes.h.inc"
} // namespace mlir

using namespace mlir;

namespace {
struct StripDebugInfo : public impl::StripDebugInfoBase<StripDebugInfo> {
  void runOnOperation() override;
};
} // namespace

void StripDebugInfo::runOnOperation() {
  auto unknownLoc = UnknownLoc::get(&getContext());

  // Strip the debug info from all operations.
  getOperation()->walk([&](Operation *op) {
    op->setLoc(unknownLoc);
    // Strip block arguments debug info.
    for (Region &region : op->getRegions()) {
      for (Block &block : region.getBlocks()) {
        for (BlockArgument &arg : block.getArguments()) {
          arg.setLoc(unknownLoc);
        }
      }
    }
  });
}

/// Creates a pass to strip debug information from a function.
std::unique_ptr<Pass> mlir::createStripDebugInfoPass() {
  return std::make_unique<StripDebugInfo>();
}
