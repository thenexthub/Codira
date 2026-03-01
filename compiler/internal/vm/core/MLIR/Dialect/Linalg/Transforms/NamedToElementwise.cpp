//===- NamedToElementwise.cpp - convert linalg named op into elementwise --===//
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
// This file implements rewriting those linalg named ops that are essentially
// elementwise e.g. `linalg.exp`, to `linalg.elementwise`. This allows further
// optimization on `linalg.elementwise` such as folding transpose, broadcast.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Dialect/Linalg/Passes.h"
#include "mlir/Dialect/Linalg/Transforms/Transforms.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/ADT/TypeSwitch.h"

using namespace mlir;
using namespace mlir::linalg;

#define DEBUG_TYPE "linalg-named-to-elementwise"

namespace {
ElementwiseKind getKind(Operation *op) {
  return toolchain::TypeSwitch<Operation *, ElementwiseKind>(op)
      .Case([](SelectOp) { return ElementwiseKind::select; })
      .Case([](AddOp) { return ElementwiseKind::add; })
      .Case([](SubOp) { return ElementwiseKind::sub; })
      .Case([](MulOp) { return ElementwiseKind::mul; })
      .Case([](DivOp) { return ElementwiseKind::div; })
      .Case([](DivUnsignedOp) { return ElementwiseKind::div_unsigned; })
      .Case([](PowFOp) { return ElementwiseKind::powf; })
      .Case([](ExpOp) { return ElementwiseKind::exp; })
      .Case([](LogOp) { return ElementwiseKind::log; })
      .Case([](AbsOp) { return ElementwiseKind::abs; })
      .Case([](CeilOp) { return ElementwiseKind::ceil; })
      .Case([](FloorOp) { return ElementwiseKind::floor; })
      .Case([](NegFOp) { return ElementwiseKind::negf; })
      .Case([](ReciprocalOp) { return ElementwiseKind::reciprocal; })
      .Case([](RoundOp) { return ElementwiseKind::round; })
      .Case([](SqrtOp) { return ElementwiseKind::sqrt; })
      .Case([](RsqrtOp) { return ElementwiseKind::rsqrt; })
      .Case([](SquareOp) { return ElementwiseKind::square; })
      .Case([](TanhOp) { return ElementwiseKind::tanh; })
      .Case([](ErfOp) { return ElementwiseKind::erf; })
      .DefaultUnreachable("unhandled case in named to elementwise");
}

template <typename NamedOpTy>
struct NamedToElementwisePattern : public OpRewritePattern<NamedOpTy> {
  using OpRewritePattern<NamedOpTy>::OpRewritePattern;

  LogicalResult matchAndRewrite(NamedOpTy op,
                                PatternRewriter &rewriter) const override {
    SmallVector<NamedAttribute> attrs;
    auto kindAttr = ElementwiseKindAttr::get(op.getContext(), getKind(op));
    attrs.push_back(rewriter.getNamedAttr("kind", kindAttr));
    attrs.push_back(
        rewriter.getNamedAttr("indexing_maps", op.getIndexingMaps()));

    rewriter.replaceOpWithNewOp<ElementwiseOp>(op, op.getDpsInputs(),
                                               op.getDpsInits(), attrs);
    return success();
  }
};
} // namespace

void mlir::linalg::populateLinalgNamedToElementwisePatterns(
    RewritePatternSet &patterns) {
  patterns.add<NamedToElementwisePattern<SelectOp>>(patterns.getContext());
  patterns.add<NamedToElementwisePattern<AddOp>>(patterns.getContext());
  patterns.add<NamedToElementwisePattern<SubOp>>(patterns.getContext());
  patterns.add<NamedToElementwisePattern<MulOp>>(patterns.getContext());
  patterns.add<NamedToElementwisePattern<DivOp>>(patterns.getContext());
  patterns.add<NamedToElementwisePattern<DivUnsignedOp>>(patterns.getContext());
  patterns.add<NamedToElementwisePattern<PowFOp>>(patterns.getContext());
  patterns.add<NamedToElementwisePattern<ExpOp>>(patterns.getContext());
  patterns.add<NamedToElementwisePattern<LogOp>>(patterns.getContext());
  patterns.add<NamedToElementwisePattern<AbsOp>>(patterns.getContext());
  patterns.add<NamedToElementwisePattern<CeilOp>>(patterns.getContext());
  patterns.add<NamedToElementwisePattern<FloorOp>>(patterns.getContext());
  patterns.add<NamedToElementwisePattern<NegFOp>>(patterns.getContext());
  patterns.add<NamedToElementwisePattern<ReciprocalOp>>(patterns.getContext());
  patterns.add<NamedToElementwisePattern<RoundOp>>(patterns.getContext());
  patterns.add<NamedToElementwisePattern<SqrtOp>>(patterns.getContext());
  patterns.add<NamedToElementwisePattern<RsqrtOp>>(patterns.getContext());
  patterns.add<NamedToElementwisePattern<SquareOp>>(patterns.getContext());
  patterns.add<NamedToElementwisePattern<TanhOp>>(patterns.getContext());
  patterns.add<NamedToElementwisePattern<ErfOp>>(patterns.getContext());
}
