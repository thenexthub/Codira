//===- EmulateUnsupportedFloats.cpp - Promote small floats --*- C++ -*-===//
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
// This pass promotes small floats (of some unsupported types T) to a supported
// type U by wrapping all float operations on Ts with expansion to and
// truncation from U, then operating on U.
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Arith/Transforms/Passes.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Arith/Utils/Utils.h"
#include "mlir/Dialect/Vector/IR/VectorOps.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Location.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/DialectConversion.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/Support/ErrorHandling.h"
#include <optional>

namespace mlir::arith {
#define GEN_PASS_DEF_ARITHEMULATEUNSUPPORTEDFLOATS
#include "mlir/Dialect/Arith/Transforms/Passes.h.inc"
} // namespace mlir::arith

using namespace mlir;

namespace {
struct EmulateUnsupportedFloatsPass
    : arith::impl::ArithEmulateUnsupportedFloatsBase<
          EmulateUnsupportedFloatsPass> {
  using arith::impl::ArithEmulateUnsupportedFloatsBase<
      EmulateUnsupportedFloatsPass>::ArithEmulateUnsupportedFloatsBase;

  void runOnOperation() override;
};

struct EmulateFloatPattern final : ConversionPattern {
  EmulateFloatPattern(const TypeConverter &converter, MLIRContext *ctx)
      : ConversionPattern::ConversionPattern(
            converter, Pattern::MatchAnyOpTypeTag(), 1, ctx) {}

  LogicalResult
  matchAndRewrite(Operation *op, ArrayRef<Value> operands,
                  ConversionPatternRewriter &rewriter) const override;
};
} // end namespace

LogicalResult EmulateFloatPattern::matchAndRewrite(
    Operation *op, ArrayRef<Value> operands,
    ConversionPatternRewriter &rewriter) const {
  if (getTypeConverter()->isLegal(op))
    return failure();
  // The rewrite doesn't handle cloning regions.
  if (op->getNumRegions() != 0)
    return failure();

  Location loc = op->getLoc();
  const TypeConverter *converter = getTypeConverter();
  SmallVector<Type> resultTypes;
  if (failed(converter->convertTypes(op->getResultTypes(), resultTypes))) {
    // Note to anyone looking for this error message: this is a "can't happen".
    // If you're seeing it, there's a bug.
    return op->emitOpError("type conversion failed in float emulation");
  }
  Operation *expandedOp =
      rewriter.create(loc, op->getName().getIdentifier(), operands, resultTypes,
                      op->getAttrs(), op->getSuccessors(), /*regions=*/{});
  SmallVector<Value> newResults(expandedOp->getResults());
  for (auto [res, oldType, newType] : toolchain::zip_equal(
           MutableArrayRef{newResults}, op->getResultTypes(), resultTypes)) {
    if (oldType != newType) {
      auto truncFOp = arith::TruncFOp::create(rewriter, loc, oldType, res);
      truncFOp.setFastmath(arith::FastMathFlags::contract);
      res = truncFOp.getResult();
    }
  }
  rewriter.replaceOp(op, newResults);
  return success();
}

void mlir::arith::populateEmulateUnsupportedFloatsConversions(
    TypeConverter &converter, ArrayRef<Type> sourceTypes, Type targetType) {
  converter.addConversion([sourceTypes = SmallVector<Type>(sourceTypes),
                           targetType](Type type) -> std::optional<Type> {
    if (toolchain::is_contained(sourceTypes, type))
      return targetType;
    if (auto shaped = dyn_cast<ShapedType>(type))
      if (toolchain::is_contained(sourceTypes, shaped.getElementType()))
        return shaped.clone(targetType);
    // All other types legal
    return type;
  });
  converter.addTargetMaterialization(
      [](OpBuilder &b, Type target, ValueRange input, Location loc) {
        auto extFOp = arith::ExtFOp::create(b, loc, target, input);
        extFOp.setFastmath(arith::FastMathFlags::contract);
        return extFOp;
      });
}

void mlir::arith::populateEmulateUnsupportedFloatsPatterns(
    RewritePatternSet &patterns, const TypeConverter &converter) {
  patterns.add<EmulateFloatPattern>(converter, patterns.getContext());
}

void mlir::arith::populateEmulateUnsupportedFloatsLegality(
    ConversionTarget &target, const TypeConverter &converter) {
  // Don't try to legalize functions and other ops that don't need expansion.
  target.markUnknownOpDynamicallyLegal([](Operation *op) { return true; });
  target.addDynamicallyLegalDialect<arith::ArithDialect>(
      [&](Operation *op) -> std::optional<bool> {
        return converter.isLegal(op);
      });
  // Manually mark arithmetic-performing vector instructions.
  target.addDynamicallyLegalOp<vector::ContractionOp, vector::ReductionOp,
                               vector::MultiDimReductionOp, vector::FMAOp,
                               vector::OuterProductOp, vector::ScanOp>(
      [&](Operation *op) { return converter.isLegal(op); });
  target.addLegalOp<arith::BitcastOp, arith::ExtFOp, arith::TruncFOp,
                    arith::ConstantOp, arith::SelectOp, vector::BroadcastOp>();
}

void EmulateUnsupportedFloatsPass::runOnOperation() {
  MLIRContext *ctx = &getContext();
  Operation *op = getOperation();
  SmallVector<Type> sourceTypes;
  Type targetType;

  std::optional<FloatType> maybeTargetType =
      arith::parseFloatType(ctx, targetTypeStr);
  if (!maybeTargetType) {
    emitError(UnknownLoc::get(ctx), "could not map target type '" +
                                        targetTypeStr +
                                        "' to a known floating-point type");
    return signalPassFailure();
  }
  targetType = *maybeTargetType;
  for (StringRef sourceTypeStr : sourceTypeStrs) {
    std::optional<FloatType> maybeSourceType =
        arith::parseFloatType(ctx, sourceTypeStr);
    if (!maybeSourceType) {
      emitError(UnknownLoc::get(ctx), "could not map source type '" +
                                          sourceTypeStr +
                                          "' to a known floating-point type");
      return signalPassFailure();
    }
    sourceTypes.push_back(*maybeSourceType);
  }
  if (sourceTypes.empty())
    (void)emitOptionalWarning(
        std::nullopt,
        "no source types specified, float emulation will do nothing");

  if (toolchain::is_contained(sourceTypes, targetType)) {
    emitError(UnknownLoc::get(ctx),
              "target type cannot be an unsupported source type");
    return signalPassFailure();
  }
  TypeConverter converter;
  arith::populateEmulateUnsupportedFloatsConversions(converter, sourceTypes,
                                                     targetType);
  RewritePatternSet patterns(ctx);
  arith::populateEmulateUnsupportedFloatsPatterns(patterns, converter);
  ConversionTarget target(getContext());
  arith::populateEmulateUnsupportedFloatsLegality(target, converter);

  if (failed(applyPartialConversion(op, target, std::move(patterns))))
    signalPassFailure();
}
