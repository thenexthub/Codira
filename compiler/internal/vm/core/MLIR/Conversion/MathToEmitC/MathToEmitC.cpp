//===- MathToEmitC.cpp - Math to EmitC Patterns -----------------*- C++ -*-===//
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

#include "mlir/Conversion/MathToEmitC/MathToEmitC.h"

#include "mlir/Dialect/EmitC/IR/EmitC.h"
#include "mlir/Dialect/Math/IR/Math.h"
#include "mlir/Transforms/DialectConversion.h"

using namespace mlir;

namespace {
template <typename OpType>
class LowerToEmitCCallOpaque : public OpRewritePattern<OpType> {
  std::string calleeStr;
  emitc::LanguageTarget languageTarget;

public:
  LowerToEmitCCallOpaque(MLIRContext *context, std::string calleeStr,
                         emitc::LanguageTarget languageTarget)
      : OpRewritePattern<OpType>(context), calleeStr(std::move(calleeStr)),
        languageTarget(languageTarget) {}

  LogicalResult matchAndRewrite(OpType op,
                                PatternRewriter &rewriter) const override;
};

template <typename OpType>
LogicalResult LowerToEmitCCallOpaque<OpType>::matchAndRewrite(
    OpType op, PatternRewriter &rewriter) const {
  if (!toolchain::all_of(op->getOperandTypes(),
                    toolchain::IsaPred<Float32Type, Float64Type>) ||
      !toolchain::all_of(op->getResultTypes(),
                    toolchain::IsaPred<Float32Type, Float64Type>))
    return rewriter.notifyMatchFailure(
        op.getLoc(),
        "expected all operands and results to be of type f32 or f64");
  std::string modifiedCalleeStr = calleeStr;
  if (languageTarget == emitc::LanguageTarget::cpp11) {
    modifiedCalleeStr = "std::" + calleeStr;
  } else if (languageTarget == emitc::LanguageTarget::c99) {
    auto operandType = op->getOperandTypes()[0];
    if (operandType.isF32())
      modifiedCalleeStr = calleeStr + "f";
  }
  rewriter.replaceOpWithNewOp<emitc::CallOpaqueOp>(
      op, op.getType(), modifiedCalleeStr, op->getOperands());
  return success();
}

} // namespace

// Populates patterns to replace `math` operations with `emitc.call_opaque`,
// using function names consistent with those in <math.h>.
void mlir::populateConvertMathToEmitCPatterns(
    RewritePatternSet &patterns, emitc::LanguageTarget languageTarget) {
  auto *context = patterns.getContext();
  patterns.insert<LowerToEmitCCallOpaque<math::FloorOp>>(context, "floor",
                                                         languageTarget);
  patterns.insert<LowerToEmitCCallOpaque<math::RoundOp>>(context, "round",
                                                         languageTarget);
  patterns.insert<LowerToEmitCCallOpaque<math::ExpOp>>(context, "exp",
                                                       languageTarget);
  patterns.insert<LowerToEmitCCallOpaque<math::CosOp>>(context, "cos",
                                                       languageTarget);
  patterns.insert<LowerToEmitCCallOpaque<math::SinOp>>(context, "sin",
                                                       languageTarget);
  patterns.insert<LowerToEmitCCallOpaque<math::AcosOp>>(context, "acos",
                                                        languageTarget);
  patterns.insert<LowerToEmitCCallOpaque<math::AsinOp>>(context, "asin",
                                                        languageTarget);
  patterns.insert<LowerToEmitCCallOpaque<math::Atan2Op>>(context, "atan2",
                                                         languageTarget);
  patterns.insert<LowerToEmitCCallOpaque<math::CeilOp>>(context, "ceil",
                                                        languageTarget);
  patterns.insert<LowerToEmitCCallOpaque<math::AbsFOp>>(context, "fabs",
                                                        languageTarget);
  patterns.insert<LowerToEmitCCallOpaque<math::PowFOp>>(context, "pow",
                                                        languageTarget);
}
