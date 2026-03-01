//===- EmulateWideInt.cpp - Wide integer operation emulation ----*- C++ -*-===//
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

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Arith/Transforms/Passes.h"
#include "mlir/Dialect/Arith/Transforms/WideIntEmulationConverter.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/MemRef/Transforms/Passes.h"
#include "mlir/Dialect/MemRef/Transforms/Transforms.h"
#include "mlir/Dialect/Vector/IR/VectorOps.h"
#include "mlir/Transforms/DialectConversion.h"
#include "vm/core/Support/FormatVariadic.h"
#include "vm/core/Support/MathExtras.h"
#include <cassert>

namespace mlir::memref {
#define GEN_PASS_DEF_MEMREFEMULATEWIDEINT
#include "mlir/Dialect/MemRef/Transforms/Passes.h.inc"
} // namespace mlir::memref

using namespace mlir;

namespace {

//===----------------------------------------------------------------------===//
// ConvertMemRefAlloc
//===----------------------------------------------------------------------===//

struct ConvertMemRefAlloc final : OpConversionPattern<memref::AllocOp> {
  using OpConversionPattern::OpConversionPattern;

  LogicalResult
  matchAndRewrite(memref::AllocOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    Type newTy = getTypeConverter()->convertType(op.getType());
    if (!newTy)
      return rewriter.notifyMatchFailure(
          op->getLoc(),
          toolchain::formatv("failed to convert memref type: {0}", op.getType()));

    rewriter.replaceOpWithNewOp<memref::AllocOp>(
        op, newTy, adaptor.getDynamicSizes(), adaptor.getSymbolOperands(),
        adaptor.getAlignmentAttr());
    return success();
  }
};

//===----------------------------------------------------------------------===//
// ConvertMemRefLoad
//===----------------------------------------------------------------------===//

struct ConvertMemRefLoad final : OpConversionPattern<memref::LoadOp> {
  using OpConversionPattern::OpConversionPattern;

  LogicalResult
  matchAndRewrite(memref::LoadOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    Type newResTy = getTypeConverter()->convertType(op.getType());
    if (!newResTy)
      return rewriter.notifyMatchFailure(
          op->getLoc(), toolchain::formatv("failed to convert memref type: {0}",
                                      op.getMemRefType()));

    rewriter.replaceOpWithNewOp<memref::LoadOp>(
        op, newResTy, adaptor.getMemref(), adaptor.getIndices(),
        op.getNontemporal());
    return success();
  }
};

//===----------------------------------------------------------------------===//
// ConvertMemRefStore
//===----------------------------------------------------------------------===//

struct ConvertMemRefStore final : OpConversionPattern<memref::StoreOp> {
  using OpConversionPattern::OpConversionPattern;

  LogicalResult
  matchAndRewrite(memref::StoreOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    Type newTy = getTypeConverter()->convertType(op.getMemRefType());
    if (!newTy)
      return rewriter.notifyMatchFailure(
          op->getLoc(), toolchain::formatv("failed to convert memref type: {0}",
                                      op.getMemRefType()));

    rewriter.replaceOpWithNewOp<memref::StoreOp>(
        op, adaptor.getValue(), adaptor.getMemref(), adaptor.getIndices(),
        op.getNontemporal());
    return success();
  }
};

//===----------------------------------------------------------------------===//
// Pass Definition
//===----------------------------------------------------------------------===//

struct EmulateWideIntPass final
    : memref::impl::MemRefEmulateWideIntBase<EmulateWideIntPass> {
  using MemRefEmulateWideIntBase::MemRefEmulateWideIntBase;

  void runOnOperation() override {
    if (!toolchain::isPowerOf2_32(widestIntSupported) || widestIntSupported < 2) {
      signalPassFailure();
      return;
    }

    Operation *op = getOperation();
    MLIRContext *ctx = op->getContext();

    arith::WideIntEmulationConverter typeConverter(widestIntSupported);
    memref::populateMemRefWideIntEmulationConversions(typeConverter);
    ConversionTarget target(*ctx);
    target.addDynamicallyLegalDialect<
        arith::ArithDialect, memref::MemRefDialect, vector::VectorDialect>(
        [&typeConverter](Operation *op) { return typeConverter.isLegal(op); });

    RewritePatternSet patterns(ctx);
    // Add common patterns to support contants, functions, etc.
    arith::populateArithWideIntEmulationPatterns(typeConverter, patterns);

    memref::populateMemRefWideIntEmulationPatterns(typeConverter, patterns);

    if (failed(applyPartialConversion(op, target, std::move(patterns))))
      signalPassFailure();
  }
};

} // end anonymous namespace

//===----------------------------------------------------------------------===//
// Public Interface Definition
//===----------------------------------------------------------------------===//

void memref::populateMemRefWideIntEmulationPatterns(
    const arith::WideIntEmulationConverter &typeConverter,
    RewritePatternSet &patterns) {
  // Populate `memref.*` conversion patterns.
  patterns.add<ConvertMemRefAlloc, ConvertMemRefLoad, ConvertMemRefStore>(
      typeConverter, patterns.getContext());
}

void memref::populateMemRefWideIntEmulationConversions(
    arith::WideIntEmulationConverter &typeConverter) {
  typeConverter.addConversion(
      [&typeConverter](MemRefType ty) -> std::optional<Type> {
        auto intTy = dyn_cast<IntegerType>(ty.getElementType());
        if (!intTy)
          return ty;

        if (intTy.getIntOrFloatBitWidth() <=
            typeConverter.getMaxTargetIntBitWidth())
          return ty;

        Type newElemTy = typeConverter.convertType(intTy);
        if (!newElemTy)
          return nullptr;

        return ty.cloneWith(std::nullopt, newElemTy);
      });
}
