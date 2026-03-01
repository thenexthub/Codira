//===- LegalizeForLLVMExport.cpp - Prepare X86Vector for LLVM translation -===//
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

#include "mlir/Dialect/X86Vector/Transforms.h"

#include "mlir/Conversion/LLVMCommon/ConversionTarget.h"
#include "mlir/Conversion/LLVMCommon/Pattern.h"
#include "mlir/Dialect/X86Vector/X86VectorDialect.h"
#include "mlir/IR/PatternMatch.h"

using namespace mlir;
using namespace mlir::x86vector;

namespace {

/// Generic one-to-one conversion of simply mappable operations into calls
/// to their respective LLVM intrinsics.
struct X86IntrinsicOpConversion
    : public ConvertOpInterfaceToLLVMPattern<x86vector::X86IntrinsicOp> {
  using ConvertOpInterfaceToLLVMPattern::ConvertOpInterfaceToLLVMPattern;

  LogicalResult
  matchAndRewrite(x86vector::X86IntrinsicOp op, ArrayRef<Value> operands,
                  ConversionPatternRewriter &rewriter) const override {
    const LLVMTypeConverter &typeConverter = *getTypeConverter();
    return LLVM::detail::intrinsicRewrite(
        op, rewriter.getStringAttr(op.getIntrinsicName()),
        op.getIntrinsicOperands(operands, typeConverter, rewriter),
        typeConverter, rewriter);
  }
};

} // namespace

/// Populate the given list with patterns that convert from X86Vector to LLVM.
void mlir::populateX86VectorLegalizeForLLVMExportPatterns(
    const LLVMTypeConverter &converter, RewritePatternSet &patterns) {
  patterns.add<X86IntrinsicOpConversion>(converter);
}

void mlir::configureX86VectorLegalizeForExportTarget(
    LLVMConversionTarget &target) {
  target.addIllegalDialect<X86VectorDialect>();
}
