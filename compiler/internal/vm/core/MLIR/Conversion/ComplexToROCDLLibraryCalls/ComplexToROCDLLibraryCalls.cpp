//=== ComplexToROCDLLibraryCalls.cpp - convert from Complex to ROCDL calls ===//
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

#include "mlir/Conversion/ComplexToROCDLLibraryCalls/ComplexToROCDLLibraryCalls.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Complex/IR/Complex.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/IR/TypeUtilities.h"
#include "mlir/Transforms/DialectConversion.h"

namespace mlir {
#define GEN_PASS_DEF_CONVERTCOMPLEXTOROCDLLIBRARYCALLS
#include "mlir/Conversion/Passes.h.inc"
} // namespace mlir

using namespace mlir;

namespace {

template <typename Op, typename FloatTy>
// Pattern to convert Complex ops to ROCDL function calls.
struct ComplexOpToROCDLLibraryCalls : public OpRewritePattern<Op> {
  using OpRewritePattern<Op>::OpRewritePattern;
  ComplexOpToROCDLLibraryCalls(MLIRContext *context, StringRef funcName,
                               PatternBenefit benefit = 1)
      : OpRewritePattern<Op>(context, benefit), funcName(funcName) {}

  LogicalResult matchAndRewrite(Op op, PatternRewriter &rewriter) const final {
    Operation *symTable = SymbolTable::getNearestSymbolTable(op);
    Type resType = op.getType();
    if (auto complexType = dyn_cast<ComplexType>(resType))
      resType = complexType.getElementType();
    if (!isa<FloatTy>(resType))
      return failure();

    auto opFunc = dyn_cast_or_null<SymbolOpInterface>(
        SymbolTable::lookupSymbolIn(symTable, funcName));
    if (!opFunc) {
      OpBuilder::InsertionGuard guard(rewriter);
      rewriter.setInsertionPointToStart(&symTable->getRegion(0).front());
      auto funcTy = FunctionType::get(
          rewriter.getContext(), op->getOperandTypes(), op->getResultTypes());
      opFunc = func::FuncOp::create(rewriter, rewriter.getUnknownLoc(),
                                    funcName, funcTy);
      opFunc.setPrivate();
    }
    rewriter.replaceOpWithNewOp<func::CallOp>(op, funcName, op.getType(),
                                              op->getOperands());
    return success();
  }

private:
  std::string funcName;
};

// Rewrite complex.pow(z, w) -> complex.exp(w * complex.log(z))
struct PowOpToROCDLLibraryCalls : public OpRewritePattern<complex::PowOp> {
  using OpRewritePattern<complex::PowOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(complex::PowOp op,
                                PatternRewriter &rewriter) const final {
    Location loc = op.getLoc();
    auto fastmath = op.getFastmathAttr();
    Value logBase =
        complex::LogOp::create(rewriter, loc, op.getLhs(), fastmath);
    Value mul =
        complex::MulOp::create(rewriter, loc, op.getRhs(), logBase, fastmath);
    Value exp = complex::ExpOp::create(rewriter, loc, mul, fastmath);
    rewriter.replaceOp(op, exp);
    return success();
  }
};

// Rewrite complex.powi(z, n) -> complex.pow(z, complex(float(n), 0))
struct PowiOpToROCDLLibraryCalls : public OpRewritePattern<complex::PowiOp> {
  using OpRewritePattern<complex::PowiOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(complex::PowiOp op,
                                PatternRewriter &rewriter) const final {
    auto complexType = cast<ComplexType>(getElementTypeOrSelf(op.getType()));
    Type elementType = complexType.getElementType();

    Type exponentType = op.getRhs().getType();
    Type exponentFloatType = elementType;
    if (auto shapedType = dyn_cast<ShapedType>(exponentType))
      exponentFloatType = shapedType.cloneWith(std::nullopt, elementType);

    Location loc = op.getLoc();
    Value exponentReal =
        arith::SIToFPOp::create(rewriter, loc, exponentFloatType, op.getRhs());
    Value zeroImag = arith::ConstantOp::create(
        rewriter, loc, rewriter.getZeroAttr(exponentFloatType));
    Value exponent = complex::CreateOp::create(
        rewriter, loc, op.getLhs().getType(), exponentReal, zeroImag);

    rewriter.replaceOpWithNewOp<complex::PowOp>(op, op.getType(), op.getLhs(),
                                                exponent, op.getFastmathAttr());
    return success();
  }
};
} // namespace

void mlir::populateComplexToROCDLLibraryCallsConversionPatterns(
    RewritePatternSet &patterns) {
  patterns.add<PowiOpToROCDLLibraryCalls>(patterns.getContext());
  patterns.add<PowOpToROCDLLibraryCalls>(patterns.getContext());
  patterns.add<ComplexOpToROCDLLibraryCalls<complex::AbsOp, Float32Type>>(
      patterns.getContext(), "__ocml_cabs_f32");
  patterns.add<ComplexOpToROCDLLibraryCalls<complex::AbsOp, Float64Type>>(
      patterns.getContext(), "__ocml_cabs_f64");
  patterns.add<ComplexOpToROCDLLibraryCalls<complex::CosOp, Float32Type>>(
      patterns.getContext(), "__ocml_ccos_f32");
  patterns.add<ComplexOpToROCDLLibraryCalls<complex::CosOp, Float64Type>>(
      patterns.getContext(), "__ocml_ccos_f64");
  patterns.add<ComplexOpToROCDLLibraryCalls<complex::ExpOp, Float32Type>>(
      patterns.getContext(), "__ocml_cexp_f32");
  patterns.add<ComplexOpToROCDLLibraryCalls<complex::ExpOp, Float64Type>>(
      patterns.getContext(), "__ocml_cexp_f64");
  patterns.add<ComplexOpToROCDLLibraryCalls<complex::LogOp, Float32Type>>(
      patterns.getContext(), "__ocml_clog_f32");
  patterns.add<ComplexOpToROCDLLibraryCalls<complex::LogOp, Float64Type>>(
      patterns.getContext(), "__ocml_clog_f64");
  patterns.add<ComplexOpToROCDLLibraryCalls<complex::SinOp, Float32Type>>(
      patterns.getContext(), "__ocml_csin_f32");
  patterns.add<ComplexOpToROCDLLibraryCalls<complex::SinOp, Float64Type>>(
      patterns.getContext(), "__ocml_csin_f64");
  patterns.add<ComplexOpToROCDLLibraryCalls<complex::SqrtOp, Float32Type>>(
      patterns.getContext(), "__ocml_csqrt_f32");
  patterns.add<ComplexOpToROCDLLibraryCalls<complex::SqrtOp, Float64Type>>(
      patterns.getContext(), "__ocml_csqrt_f64");
  patterns.add<ComplexOpToROCDLLibraryCalls<complex::TanOp, Float32Type>>(
      patterns.getContext(), "__ocml_ctan_f32");
  patterns.add<ComplexOpToROCDLLibraryCalls<complex::TanOp, Float64Type>>(
      patterns.getContext(), "__ocml_ctan_f64");
  patterns.add<ComplexOpToROCDLLibraryCalls<complex::TanhOp, Float32Type>>(
      patterns.getContext(), "__ocml_ctanh_f32");
  patterns.add<ComplexOpToROCDLLibraryCalls<complex::TanhOp, Float64Type>>(
      patterns.getContext(), "__ocml_ctanh_f64");
}

namespace {
struct ConvertComplexToROCDLLibraryCallsPass
    : public impl::ConvertComplexToROCDLLibraryCallsBase<
          ConvertComplexToROCDLLibraryCallsPass> {
  void runOnOperation() override;
};
} // namespace

void ConvertComplexToROCDLLibraryCallsPass::runOnOperation() {
  Operation *op = getOperation();

  RewritePatternSet patterns(&getContext());
  populateComplexToROCDLLibraryCallsConversionPatterns(patterns);

  ConversionTarget target(getContext());
  target.addLegalDialect<arith::ArithDialect, func::FuncDialect>();
  target.addLegalOp<complex::CreateOp, complex::MulOp>();
  target.addIllegalOp<complex::AbsOp, complex::CosOp, complex::ExpOp,
                      complex::LogOp, complex::PowOp, complex::PowiOp,
                      complex::SinOp, complex::SqrtOp, complex::TanOp,
                      complex::TanhOp>();
  if (failed(applyPartialConversion(op, target, std::move(patterns))))
    signalPassFailure();
}
