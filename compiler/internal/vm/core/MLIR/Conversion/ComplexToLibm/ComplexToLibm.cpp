//===-- ComplexToLibm.cpp - conversion from Complex to libm calls ---------===//
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

#include "mlir/Conversion/ComplexToLibm/ComplexToLibm.h"

#include "mlir/Dialect/Complex/IR/Complex.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/PatternMatch.h"
#include <optional>

namespace mlir {
#define GEN_PASS_DEF_CONVERTCOMPLEXTOLIBM
#include "mlir/Conversion/Passes.h.inc"
} // namespace mlir

using namespace mlir;

namespace {
// Functor to resolve the function name corresponding to the given complex
// result type.
struct ComplexTypeResolver {
  std::optional<bool> operator()(Type type) const {
    auto complexType = cast<ComplexType>(type);
    auto elementType = complexType.getElementType();
    if (!isa<Float32Type, Float64Type>(elementType))
      return {};

    return elementType.getIntOrFloatBitWidth() == 64;
  }
};

// Functor to resolve the function name corresponding to the given float result
// type.
struct FloatTypeResolver {
  std::optional<bool> operator()(Type type) const {
    auto elementType = cast<FloatType>(type);
    if (!isa<Float32Type, Float64Type>(elementType))
      return {};

    return elementType.getIntOrFloatBitWidth() == 64;
  }
};

// Pattern to convert scalar complex operations to calls to libm functions.
// Additionally the libm function signatures are declared.
// TypeResolver is a functor returning the libm function name according to the
// expected type double or float.
template <typename Op, typename TypeResolver = ComplexTypeResolver>
struct ScalarOpToLibmCall : public OpRewritePattern<Op> {
public:
  using OpRewritePattern<Op>::OpRewritePattern;
  ScalarOpToLibmCall(MLIRContext *context, StringRef floatFunc,
                     StringRef doubleFunc, PatternBenefit benefit)
      : OpRewritePattern<Op>(context, benefit), floatFunc(floatFunc),
        doubleFunc(doubleFunc){};

  LogicalResult matchAndRewrite(Op op, PatternRewriter &rewriter) const final;

private:
  std::string floatFunc, doubleFunc;
};
} // namespace

template <typename Op, typename TypeResolver>
LogicalResult ScalarOpToLibmCall<Op, TypeResolver>::matchAndRewrite(
    Op op, PatternRewriter &rewriter) const {
  auto module = SymbolTable::getNearestSymbolTable(op);
  auto isDouble = TypeResolver()(op.getType());
  if (!isDouble.has_value())
    return failure();

  auto name = *isDouble ? doubleFunc : floatFunc;

  auto opFunc = dyn_cast_or_null<SymbolOpInterface>(
      SymbolTable::lookupSymbolIn(module, name));
  // Forward declare function if it hasn't already been
  if (!opFunc) {
    OpBuilder::InsertionGuard guard(rewriter);
    rewriter.setInsertionPointToStart(&module->getRegion(0).front());
    auto opFunctionTy = FunctionType::get(
        rewriter.getContext(), op->getOperandTypes(), op->getResultTypes());
    opFunc = func::FuncOp::create(rewriter, rewriter.getUnknownLoc(), name,
                                  opFunctionTy);
    opFunc.setPrivate();
  }
  assert(isa<FunctionOpInterface>(SymbolTable::lookupSymbolIn(module, name)));

  rewriter.replaceOpWithNewOp<func::CallOp>(op, name, op.getType(),
                                            op->getOperands());

  return success();
}

void mlir::populateComplexToLibmConversionPatterns(RewritePatternSet &patterns,
                                                   PatternBenefit benefit) {
  patterns.add<ScalarOpToLibmCall<complex::PowOp>>(patterns.getContext(),
                                                   "cpowf", "cpow", benefit);
  patterns.add<ScalarOpToLibmCall<complex::SqrtOp>>(patterns.getContext(),
                                                    "csqrtf", "csqrt", benefit);
  patterns.add<ScalarOpToLibmCall<complex::TanhOp>>(patterns.getContext(),
                                                    "ctanhf", "ctanh", benefit);
  patterns.add<ScalarOpToLibmCall<complex::CosOp>>(patterns.getContext(),
                                                   "ccosf", "ccos", benefit);
  patterns.add<ScalarOpToLibmCall<complex::SinOp>>(patterns.getContext(),
                                                   "csinf", "csin", benefit);
  patterns.add<ScalarOpToLibmCall<complex::ConjOp>>(patterns.getContext(),
                                                    "conjf", "conj", benefit);
  patterns.add<ScalarOpToLibmCall<complex::LogOp>>(patterns.getContext(),
                                                   "clogf", "clog", benefit);
  patterns.add<ScalarOpToLibmCall<complex::AbsOp, FloatTypeResolver>>(
      patterns.getContext(), "cabsf", "cabs", benefit);
  patterns.add<ScalarOpToLibmCall<complex::AngleOp, FloatTypeResolver>>(
      patterns.getContext(), "cargf", "carg", benefit);
  patterns.add<ScalarOpToLibmCall<complex::TanOp>>(patterns.getContext(),
                                                   "ctanf", "ctan", benefit);
}

namespace {
struct ConvertComplexToLibmPass
    : public impl::ConvertComplexToLibmBase<ConvertComplexToLibmPass> {
  void runOnOperation() override;
};
} // namespace

void ConvertComplexToLibmPass::runOnOperation() {
  auto module = getOperation();

  RewritePatternSet patterns(&getContext());
  populateComplexToLibmConversionPatterns(patterns, /*benefit=*/1);

  ConversionTarget target(getContext());
  target.addLegalDialect<func::FuncDialect>();
  target.addIllegalOp<complex::PowOp, complex::SqrtOp, complex::TanhOp,
                      complex::CosOp, complex::SinOp, complex::ConjOp,
                      complex::LogOp, complex::AbsOp, complex::AngleOp,
                      complex::TanOp>();
  if (failed(applyPartialConversion(module, target, std::move(patterns))))
    signalPassFailure();
}
