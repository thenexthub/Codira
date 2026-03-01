//===- LegalizeForLLVMExport.cpp - Prepare AMX for LLVM translation ----===//
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

#include "mlir/Dialect/AMX/Transforms.h"

#include "mlir/Conversion/ConvertToLLVM/ToLLVMInterface.h"
#include "mlir/Conversion/LLVMCommon/ConversionTarget.h"
#include "mlir/Conversion/LLVMCommon/Pattern.h"
#include "mlir/Dialect/AMX/AMXDialect.h"
#include "mlir/IR/PatternMatch.h"

using namespace mlir;
using namespace mlir::amx;

namespace {

/// Generic one-to-one conversion of simply mappable operations into calls
/// to their respective LLVM intrinsics.
struct AMXIntrinsicOpConversion
    : public ConvertOpInterfaceToLLVMPattern<amx::AMXIntrinsicOp> {
  using ConvertOpInterfaceToLLVMPattern::ConvertOpInterfaceToLLVMPattern;

  LogicalResult
  matchAndRewrite(amx::AMXIntrinsicOp op, ArrayRef<Value> operands,
                  ConversionPatternRewriter &rewriter) const override {
    const LLVMTypeConverter &typeConverter = *getTypeConverter();
    return LLVM::detail::intrinsicRewrite(
        op, rewriter.getStringAttr(op.getIntrinsicName()),
        op.getIntrinsicOperands(operands, typeConverter, rewriter),
        typeConverter, rewriter);
  }
};

} // namespace

void mlir::populateAMXLegalizeForLLVMExportPatterns(
    LLVMTypeConverter &converter, RewritePatternSet &patterns) {
  patterns.add<AMXIntrinsicOpConversion>(converter);
  converter.addConversion([&](amx::TileType type) {
    return LLVM::LLVMX86AMXType::get(&converter.getContext());
  });
}

void mlir::configureAMXLegalizeForExportTarget(LLVMConversionTarget &target) {
  target.addIllegalDialect<AMXDialect>();
}

namespace {
/// Implement the interface to convert AMX to LLVM.
struct AMXToLLVMDialectInterface : public ConvertToLLVMPatternInterface {
  using ConvertToLLVMPatternInterface::ConvertToLLVMPatternInterface;

  void populateConvertToLLVMConversionPatterns(
      ConversionTarget &target, LLVMTypeConverter &typeConverter,
      RewritePatternSet &patterns) const final {
    populateAMXLegalizeForLLVMExportPatterns(typeConverter, patterns);
  }
};
} // namespace

void mlir::registerConvertAMXToLLVMInterface(DialectRegistry &registry) {
  registry.addExtension(+[](MLIRContext *ctx, amx::AMXDialect *dialect) {
    dialect->addInterfaces<AMXToLLVMDialectInterface>();
  });
}
