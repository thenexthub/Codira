//===- TransactionAcceptOrRevert.cpp - Check cost and accept/revert region ===//
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

#include "vm/core/Transforms/Vectorize/SandboxVectorizer/Passes/TransactionAcceptOrRevert.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/InstructionCost.h"
#include "vm/core/Transforms/Vectorize/SandboxVectorizer/Debug.h"

namespace vm::core {

static cl::opt<int> CostThreshold("sbvec-cost-threshold", cl::init(0),
                                  cl::Hidden,
                                  cl::desc("Vectorization cost threshold."));

namespace sandboxir {

bool TransactionAcceptOrRevert::runOnRegion(Region &Rgn, const Analyses &A) {
  const auto &SB = Rgn.getScoreboard();
  [[maybe_unused]] auto CostBefore = SB.getBeforeCost();
  [[maybe_unused]] auto CostAfter = SB.getAfterCost();
  InstructionCost CostAfterMinusBefore = SB.getAfterCost() - SB.getBeforeCost();
  LLVM_DEBUG(dbgs() << DEBUG_PREFIX << "Cost gain: " << CostAfterMinusBefore
                    << " (before/after/threshold: " << CostBefore << "/"
                    << CostAfter << "/" << CostThreshold << ")\n");
  // TODO: Print costs / write to remarks.
  auto &Tracker = Rgn.getContext().getTracker();
  if (CostAfterMinusBefore < -CostThreshold) {
    bool HasChanges = !Tracker.empty();
    Tracker.accept();
    LLVM_DEBUG(dbgs() << DEBUG_PREFIX << "*** Transaction Accept ***\n");
    return HasChanges;
  }
  // Revert the IR.
  LLVM_DEBUG(dbgs() << DEBUG_PREFIX << "*** Transaction Revert ***\n");
  Rgn.getContext().getTracker().revert();
  return false;
}

} // namespace sandboxir
} // namespace vm::core
