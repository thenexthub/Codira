//===- Pattern.h - SPIRV Common Conversion Patterns -----------------------===//
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

#ifndef MLIR_CONVERSION_SPIRVCOMMON_PATTERN_H
#define MLIR_CONVERSION_SPIRVCOMMON_PATTERN_H

#include "mlir/Dialect/SPIRV/IR/SPIRVEnums.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVOpTraits.h"
#include "mlir/IR/TypeUtilities.h"
#include "mlir/Transforms/DialectConversion.h"
#include "vm/core/Support/FormatVariadic.h"

namespace mlir {
namespace spirv {

/// Converts elementwise unary, binary and ternary standard operations to SPIR-V
/// operations.
template <typename Op, typename SPIRVOp>
struct ElementwiseOpPattern : public OpConversionPattern<Op> {
  using OpConversionPattern<Op>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(Op op, typename Op::Adaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    assert(adaptor.getOperands().size() <= 3);
    Type dstType = this->getTypeConverter()->convertType(op.getType());
    if (!dstType) {
      return rewriter.notifyMatchFailure(
          op->getLoc(),
          toolchain::formatv("failed to convert type {0} for SPIR-V", op.getType()));
    }

    if (SPIRVOp::template hasTrait<OpTrait::spirv::UnsignedOp>() &&
        !getElementTypeOrSelf(op.getType()).isIndex() &&
        dstType != op.getType()) {
      op.dump();
      return op.emitError("bitwidth emulation is not implemented yet on "
                          "unsigned op pattern version");
    }
    rewriter.template replaceOpWithNewOp<SPIRVOp>(op, dstType,
                                                  adaptor.getOperands());
    return success();
  }
};

} // namespace spirv
} // namespace mlir

#endif // MLIR_CONVERSION_SPIRVCOMMON_PATTERN_H
