//===- RotateWhileLoop.cpp - scf.while loop rotation ----------------------===//
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
// Rotates `scf.while` loops.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/SCF/Transforms/Patterns.h"

#include "mlir/Dialect/SCF/IR/SCF.h"

using namespace mlir;

namespace {
struct RotateWhileLoopPattern : OpRewritePattern<scf::WhileOp> {
  using OpRewritePattern<scf::WhileOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(scf::WhileOp whileOp,
                                PatternRewriter &rewriter) const final {
    // Setting this option would lead to infinite recursion on a greedy driver
    // as 'do-while' loops wouldn't be skipped.
    constexpr bool forceCreateCheck = false;
    FailureOr<scf::WhileOp> result =
        scf::wrapWhileLoopInZeroTripCheck(whileOp, rewriter, forceCreateCheck);
    // scf::wrapWhileLoopInZeroTripCheck hasn't yet implemented a failure
    // mechanism. 'do-while' loops are simply returned unmodified. In order to
    // stop recursion, we check input and output operations differ.
    return success(succeeded(result) && *result != whileOp);
  }
};
} // namespace

namespace mlir {
namespace scf {
void populateSCFRotateWhileLoopPatterns(RewritePatternSet &patterns) {
  patterns.add<RotateWhileLoopPattern>(patterns.getContext());
}
} // namespace scf
} // namespace mlir
