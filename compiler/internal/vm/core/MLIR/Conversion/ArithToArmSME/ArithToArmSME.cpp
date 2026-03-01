//===- ArithToArmSME.cpp - Arith to ArmSME dialect conversion -------------===//
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

#include "mlir/Conversion/ArithToArmSME/ArithToArmSME.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/ArmSME/IR/ArmSME.h"
#include "mlir/Dialect/ArmSME/Utils/Utils.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
#define GEN_PASS_DEF_ARITHTOARMSMECONVERSIONPASS
#include "mlir/Conversion/Passes.h.inc"
} // namespace mlir

#define DEBUG_TYPE "arith-to-arm-sme"

using namespace mlir;

//===----------------------------------------------------------------------===//
// Conversion helpers
//===----------------------------------------------------------------------===//

/// Returns true if 'val' is a splat of zero, false otherwise.
static bool isSplatZero(Type elemType, DenseElementsAttr val) {
  if (toolchain::isa<FloatType>(elemType))
    return val && val.isSplat() && val.getSplatValue<APFloat>().isZero();
  if (toolchain::isa<IntegerType>(elemType))
    return val && val.isSplat() && val.getSplatValue<APInt>().isZero();
  return false;
}

namespace {

//===----------------------------------------------------------------------===//
// ConstantOp
//===----------------------------------------------------------------------===//

/// Conversion pattern for dense arith.constant.
struct ConstantOpToArmSMELowering : public OpRewritePattern<arith::ConstantOp> {
  using Base::Base;

  LogicalResult matchAndRewrite(arith::ConstantOp constantOp,
                                PatternRewriter &rewriter) const final {
    auto tileType = dyn_cast<VectorType>(constantOp.getType());
    if (!tileType || !arm_sme::isValidSMETileVectorType(tileType))
      return failure();

    auto denseAttr = dyn_cast<DenseElementsAttr>(constantOp.getValueAttr());
    if (!denseAttr || !denseAttr.isSplat())
      return failure();

    auto tileElementType = tileType.getElementType();

    // Lower 'arith.constant dense<0>' to 'arm_sme.zero' op.
    if (isSplatZero(tileElementType, denseAttr)) {
      rewriter.replaceOpWithNewOp<arm_sme::ZeroOp>(constantOp, tileType);
      return success();
    }

    // Lower non-zero constants to a loop of 'arm_sme.insert_tile_slice'
    // ops that broadcast the constant to each tile slice.
    auto loc = constantOp.getLoc();

    // To fill a tile with a constant, we create a 1-D splat of the constant,
    // then move that into each tile slice (the largest unit we can set at once,
    // outside of operations like the outerproduct).
    VectorType tileSliceType = VectorType::Builder(tileType).dropDim(0);
    auto denseAttr1D = DenseElementsAttr::get(
        tileSliceType, denseAttr.getSplatValue<Attribute>());
    auto constantOp1D = arith::ConstantOp::create(rewriter, loc, denseAttr1D);

    auto initTile = arm_sme::GetTileOp::create(rewriter, loc, tileType);
    auto makeLoopBody = [&](OpBuilder &b, Location loc, Value tileSliceIndex,
                            Value currentTile) {
      // Create 'arm_sme.insert_tile_slice' to write vector to tile
      // slice.
      auto nextTile = arm_sme::InsertTileSliceOp::create(
          b, loc, tileType, constantOp1D, currentTile, tileSliceIndex);
      return nextTile.getResult();
    };
    auto forOp = mlir::arm_sme::createLoopOverTileSlices(
        rewriter, loc, initTile, makeLoopBody);
    rewriter.replaceOp(constantOp, forOp.getResult(0));

    return success();
  }
};

} // namespace

//===----------------------------------------------------------------------===//
// Pattern population
//===----------------------------------------------------------------------===//

void mlir::arith::populateArithToArmSMEConversionPatterns(
    RewritePatternSet &patterns) {
  patterns.add<ConstantOpToArmSMELowering>(patterns.getContext());
}

//===----------------------------------------------------------------------===//
// Pass definition
//===----------------------------------------------------------------------===//

namespace {
struct ArithToArmSMEConversionPass final
    : impl::ArithToArmSMEConversionPassBase<ArithToArmSMEConversionPass> {
  using impl::ArithToArmSMEConversionPassBase<
      ArithToArmSMEConversionPass>::ArithToArmSMEConversionPassBase;

  void runOnOperation() override {
    RewritePatternSet patterns(&getContext());
    arith::populateArithToArmSMEConversionPatterns(patterns);
    if (failed(applyPatternsGreedily(getOperation(), std::move(patterns))))
      return signalPassFailure();
  }
};
} // namespace
