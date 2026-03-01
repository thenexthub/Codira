//===- ReconcileUnrealizedCasts.cpp - Eliminate noop unrealized casts -----===//
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

#include "mlir/Conversion/ReconcileUnrealizedCasts/ReconcileUnrealizedCasts.h"

#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"

namespace mlir {
#define GEN_PASS_DEF_RECONCILEUNREALIZEDCASTSPASS
#include "mlir/Conversion/Passes.h.inc"
} // namespace mlir

using namespace mlir;

namespace {

/// Pass to simplify and eliminate unrealized conversion casts.
///
/// This pass processes unrealized_conversion_cast ops in a worklist-driven
/// fashion. For each matched cast op, if the chain of input casts eventually
/// reaches a cast op where the input types match the output types of the
/// matched op, replace the matched op with the inputs.
///
/// Example:
/// %1 = unrealized_conversion_cast %0 : !A to !B
/// %2 = unrealized_conversion_cast %1 : !B to !C
/// %3 = unrealized_conversion_cast %2 : !C to !A
///
/// In the above example, %0 can be used instead of %3 and all cast ops are
/// folded away.
struct ReconcileUnrealizedCasts
    : public impl::ReconcileUnrealizedCastsPassBase<ReconcileUnrealizedCasts> {
  ReconcileUnrealizedCasts() = default;

  void runOnOperation() override {
    SmallVector<UnrealizedConversionCastOp> ops;
    getOperation()->walk(
        [&](UnrealizedConversionCastOp castOp) { ops.push_back(castOp); });
    reconcileUnrealizedCasts(ops);
  }
};

} // namespace
