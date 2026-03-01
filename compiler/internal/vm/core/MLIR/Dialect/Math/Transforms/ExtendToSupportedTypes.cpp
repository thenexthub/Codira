//===- ExtendToSupportedTypes.cpp - Legalize functions on unsupported floats
//----------===//
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
// This file implements legalizing math operations on unsupported floating-point
// types through arith.extf and arith.truncf.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Arith/Utils/Utils.h"
#include "mlir/Dialect/Math/IR/Math.h"
#include "mlir/Dialect/Math/Transforms/Passes.h"
#include "mlir/IR/Diagnostics.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/IR/TypeUtilities.h"
#include "mlir/Transforms/DialectConversion.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/SetVector.h"

namespace mlir::math {
#define GEN_PASS_DEF_MATHEXTENDTOSUPPORTEDTYPES
#include "mlir/Dialect/Math/Transforms/Passes.h.inc"
} // namespace mlir::math

using namespace mlir;

namespace {
struct ExtendToSupportedTypesRewritePattern final : ConversionPattern {
  ExtendToSupportedTypesRewritePattern(const TypeConverter &converter,
                                       MLIRContext *context)
      : ConversionPattern(converter, MatchAnyOpTypeTag{}, 1, context) {}
  LogicalResult
  matchAndRewrite(Operation *op, ArrayRef<Value> operands,
                  ConversionPatternRewriter &rewriter) const override;
};

struct ExtendToSupportedTypesPass
    : mlir::math::impl::MathExtendToSupportedTypesBase<
          ExtendToSupportedTypesPass> {
  using math::impl::MathExtendToSupportedTypesBase<
      ExtendToSupportedTypesPass>::MathExtendToSupportedTypesBase;

  void runOnOperation() override;
};
} // namespace

void mlir::math::populateExtendToSupportedTypesTypeConverter(
    TypeConverter &typeConverter, const SetVector<Type> &sourceTypes,
    Type targetType) {

  typeConverter.addConversion(
      [](Type type) -> std::optional<Type> { return type; });
  typeConverter.addConversion(
      [&sourceTypes, targetType](FloatType type) -> std::optional<Type> {
        if (!sourceTypes.contains(type))
          return targetType;

        return std::nullopt;
      });
  typeConverter.addConversion(
      [&sourceTypes, targetType](ShapedType type) -> std::optional<Type> {
        if (auto elemTy = dyn_cast<FloatType>(type.getElementType()))
          if (!sourceTypes.contains(elemTy))
            return type.clone(targetType);

        return std::nullopt;
      });
  typeConverter.addTargetMaterialization(
      [](OpBuilder &b, Type target, ValueRange input, Location loc) {
        auto extFOp = arith::ExtFOp::create(b, loc, target, input);
        extFOp.setFastmath(arith::FastMathFlags::contract);
        return extFOp;
      });
}

void mlir::math::populateExtendToSupportedTypesConversionTarget(
    ConversionTarget &target, TypeConverter &typeConverter) {
  target.markUnknownOpDynamicallyLegal([&typeConverter](Operation *op) -> bool {
    if (isa<MathDialect>(op->getDialect()))
      return typeConverter.isLegal(op);
    return true;
  });
  target.addLegalOp<FmaOp>();
  target.addLegalOp<arith::ExtFOp, arith::TruncFOp>();
}

LogicalResult ExtendToSupportedTypesRewritePattern::matchAndRewrite(
    Operation *op, ArrayRef<Value> operands,
    ConversionPatternRewriter &rewriter) const {
  Location loc = op->getLoc();
  const TypeConverter *converter = getTypeConverter();
  FailureOr<Operation *> legalized =
      convertOpResultTypes(op, operands, *converter, rewriter);
  if (failed(legalized))
    return failure();

  SmallVector<Value> results = (*legalized)->getResults();
  for (auto [result, newType, origType] : toolchain::zip_equal(
           results, (*legalized)->getResultTypes(), op->getResultTypes())) {
    if (newType != origType) {
      auto truncFOp = arith::TruncFOp::create(rewriter, loc, origType, result);
      truncFOp.setFastmath(arith::FastMathFlags::contract);
      result = truncFOp.getResult();
    }
  }
  rewriter.replaceOp(op, results);
  return success();
}

void mlir::math::populateExtendToSupportedTypesPatterns(
    RewritePatternSet &patterns, const TypeConverter &typeConverter) {
  patterns.add<ExtendToSupportedTypesRewritePattern>(typeConverter,
                                                     patterns.getContext());
}

void ExtendToSupportedTypesPass::runOnOperation() {
  Operation *op = getOperation();
  MLIRContext *ctx = &getContext();

  // Parse target type
  std::optional<Type> maybeTargetType =
      arith::parseFloatType(ctx, targetTypeStr);
  if (!maybeTargetType.has_value()) {
    emitError(UnknownLoc::get(ctx), "could not map target type '" +
                                        targetTypeStr +
                                        "' to a known floating-point type");
    return signalPassFailure();
  }
  Type targetType = maybeTargetType.value();

  // Parse source types
  toolchain::SetVector<Type> sourceTypes;
  for (const auto &extraTypeStr : extraTypeStrs) {
    std::optional<FloatType> maybeExtraType =
        arith::parseFloatType(ctx, extraTypeStr);
    if (!maybeExtraType.has_value()) {
      emitError(UnknownLoc::get(ctx), "could not map source type '" +
                                          extraTypeStr +
                                          "' to a known floating-point type");
      return signalPassFailure();
    }
    sourceTypes.insert(maybeExtraType.value());
  }
  // f64 and f32 are implicitly supported
  Builder b(ctx);
  sourceTypes.insert(b.getF64Type());
  sourceTypes.insert(b.getF32Type());

  TypeConverter typeConverter;
  math::populateExtendToSupportedTypesTypeConverter(typeConverter, sourceTypes,
                                                    targetType);
  ConversionTarget target(*ctx);
  math::populateExtendToSupportedTypesConversionTarget(target, typeConverter);
  RewritePatternSet patterns(ctx);
  math::populateExtendToSupportedTypesPatterns(patterns, typeConverter);
  if (failed(applyPartialConversion(op, target, std::move(patterns))))
    return signalPassFailure();
}
