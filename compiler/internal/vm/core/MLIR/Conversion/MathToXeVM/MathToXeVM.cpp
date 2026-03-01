//===-- MathToXeVM.cpp - conversion from Math to XeVM ---------------------===//
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

#include "mlir/Conversion/MathToXeVM/MathToXeVM.h"
#include "mlir/Conversion/ArithCommon/AttrToLLVMConverter.h"
#include "mlir/Dialect/LLVMIR/FunctionCallUtils.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/Math/IR/Math.h"
#include "mlir/IR/BuiltinDialect.h"
#include "mlir/Pass/Pass.h"
#include "vm/core/Support/FormatVariadic.h"

namespace mlir {
#define GEN_PASS_DEF_CONVERTMATHTOXEVM
#include "mlir/Conversion/Passes.h.inc"
} // namespace mlir

using namespace mlir;

#define DEBUG_TYPE "math-to-xevm"

/// Convert math ops marked with `fast` (`afn`) to native OpenCL intrinsics.
template <typename Op>
struct ConvertNativeFuncPattern final : public OpConversionPattern<Op> {

  ConvertNativeFuncPattern(MLIRContext *context, StringRef nativeFunc,
                           PatternBenefit benefit = 1)
      : OpConversionPattern<Op>(context, benefit), nativeFunc(nativeFunc) {}

  LogicalResult
  matchAndRewrite(Op op, typename Op::Adaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    if (!isSPIRVCompatibleFloatOrVec(op.getType()))
      return failure();

    arith::FastMathFlags fastFlags = op.getFastmath();
    if (!arith::bitEnumContainsAll(fastFlags, arith::FastMathFlags::afn))
      return rewriter.notifyMatchFailure(op, "not a fastmath `afn` operation");

    SmallVector<Type, 1> operandTypes;
    for (auto operand : adaptor.getOperands()) {
      Type opTy = operand.getType();
      // This pass only supports operations on vectors that are already in SPIRV
      // supported vector sizes: Distributing unsupported vector sizes to SPIRV
      // supported vector sizes are done in other blocking optimization passes.
      if (!isSPIRVCompatibleFloatOrVec(opTy))
        return rewriter.notifyMatchFailure(
            op, toolchain::formatv("incompatible operand type: '{0}'", opTy));
      operandTypes.push_back(opTy);
    }

    auto moduleOp = op->template getParentWithTrait<OpTrait::SymbolTable>();
    auto funcOpRes = LLVM::lookupOrCreateFn(
        rewriter, moduleOp, getMangledNativeFuncName(operandTypes),
        operandTypes, op.getType());
    assert(!failed(funcOpRes));
    LLVM::LLVMFuncOp funcOp = funcOpRes.value();

    auto callOp = rewriter.replaceOpWithNewOp<LLVM::CallOp>(
        op, funcOp, adaptor.getOperands());
    // Preserve fastmath flags in our MLIR op when converting to toolchain function
    // calls, in order to allow further fastmath optimizations: We thus need to
    // convert arith fastmath attrs into attrs recognized by toolchain.
    arith::AttrConvertFastMathToLLVM<Op, LLVM::CallOp> fastAttrConverter(op);
    mlir::NamedAttribute fastAttr = fastAttrConverter.getAttrs()[0];
    callOp->setAttr(fastAttr.getName(), fastAttr.getValue());
    return success();
  }

  inline bool isSPIRVCompatibleFloatOrVec(Type type) const {
    if (type.isFloat())
      return true;
    if (auto vecType = dyn_cast<VectorType>(type)) {
      if (!vecType.getElementType().isFloat())
        return false;
      // SPIRV distinguishes between vectors and matrices: OpenCL native math
      // intrsinics are not compatible with matrices.
      ArrayRef<int64_t> shape = vecType.getShape();
      if (shape.size() != 1)
        return false;
      // SPIRV only allows vectors of size 2, 3, 4, 8, 16.
      if (shape[0] == 2 || shape[0] == 3 || shape[0] == 4 || shape[0] == 8 ||
          shape[0] == 16)
        return true;
    }
    return false;
  }

  inline std::string
  getMangledNativeFuncName(const ArrayRef<Type> operandTypes) const {
    std::string mangledFuncName =
        "_Z" + std::to_string(nativeFunc.size()) + nativeFunc.str();

    auto appendFloatToMangledFunc = [&mangledFuncName](Type type) {
      if (type.isF32())
        mangledFuncName += "f";
      else if (type.isF16())
        mangledFuncName += "Dh";
      else if (type.isF64())
        mangledFuncName += "d";
    };

    for (auto type : operandTypes) {
      if (auto vecType = dyn_cast<VectorType>(type)) {
        mangledFuncName += "Dv" + std::to_string(vecType.getShape()[0]) + "_";
        appendFloatToMangledFunc(vecType.getElementType());
      } else
        appendFloatToMangledFunc(type);
    }

    return mangledFuncName;
  }

  const StringRef nativeFunc;
};

void mlir::populateMathToXeVMConversionPatterns(RewritePatternSet &patterns,
                                                bool convertArith) {
  patterns.add<ConvertNativeFuncPattern<math::ExpOp>>(patterns.getContext(),
                                                      "__spirv_ocl_native_exp");
  patterns.add<ConvertNativeFuncPattern<math::CosOp>>(patterns.getContext(),
                                                      "__spirv_ocl_native_cos");
  patterns.add<ConvertNativeFuncPattern<math::Exp2Op>>(
      patterns.getContext(), "__spirv_ocl_native_exp2");
  patterns.add<ConvertNativeFuncPattern<math::LogOp>>(patterns.getContext(),
                                                      "__spirv_ocl_native_log");
  patterns.add<ConvertNativeFuncPattern<math::Log2Op>>(
      patterns.getContext(), "__spirv_ocl_native_log2");
  patterns.add<ConvertNativeFuncPattern<math::Log10Op>>(
      patterns.getContext(), "__spirv_ocl_native_log10");
  patterns.add<ConvertNativeFuncPattern<math::PowFOp>>(
      patterns.getContext(), "__spirv_ocl_native_powr");
  patterns.add<ConvertNativeFuncPattern<math::RsqrtOp>>(
      patterns.getContext(), "__spirv_ocl_native_rsqrt");
  patterns.add<ConvertNativeFuncPattern<math::SinOp>>(patterns.getContext(),
                                                      "__spirv_ocl_native_sin");
  patterns.add<ConvertNativeFuncPattern<math::SqrtOp>>(
      patterns.getContext(), "__spirv_ocl_native_sqrt");
  patterns.add<ConvertNativeFuncPattern<math::TanOp>>(patterns.getContext(),
                                                      "__spirv_ocl_native_tan");
  if (convertArith)
    patterns.add<ConvertNativeFuncPattern<arith::DivFOp>>(
        patterns.getContext(), "__spirv_ocl_native_divide");
}

namespace {
struct ConvertMathToXeVMPass
    : public impl::ConvertMathToXeVMBase<ConvertMathToXeVMPass> {
  using Base::Base;
  void runOnOperation() override;
};
} // namespace

void ConvertMathToXeVMPass::runOnOperation() {
  RewritePatternSet patterns(&getContext());
  populateMathToXeVMConversionPatterns(patterns, convertArith);
  ConversionTarget target(getContext());
  target.addLegalDialect<BuiltinDialect, LLVM::LLVMDialect>();
  if (failed(
          applyPartialConversion(getOperation(), target, std::move(patterns))))
    signalPassFailure();
}
