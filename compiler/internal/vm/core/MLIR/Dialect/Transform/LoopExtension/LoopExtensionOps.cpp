//===- LoopExtensionOps.cpp - Loop extension for the Transform dialect ----===//
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

#include "mlir/Dialect/Transform/LoopExtension/LoopExtensionOps.h"

#include "mlir/Transforms/LoopInvariantCodeMotionUtils.h"

using namespace mlir;

#define GET_OP_CLASSES
#include "mlir/Dialect/Transform/LoopExtension/LoopExtensionOps.cpp.inc"

//===----------------------------------------------------------------------===//
// HoistLoopInvariantSubsetsOp
//===----------------------------------------------------------------------===//

DiagnosedSilenceableFailure transform::HoistLoopInvariantSubsetsOp::applyToOne(
    transform::TransformRewriter &rewriter, LoopLikeOpInterface loopLikeOp,
    transform::ApplyToEachResultList &results,
    transform::TransformState &state) {
  hoistLoopInvariantSubsets(rewriter, loopLikeOp);
  return DiagnosedSilenceableFailure::success();
}

void transform::HoistLoopInvariantSubsetsOp::getEffects(
    SmallVectorImpl<MemoryEffects::EffectInstance> &effects) {
  transform::onlyReadsHandle(getTargetMutable(), effects);
  transform::modifiesPayload(effects);
}
