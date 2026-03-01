//===- MemRefToEmitC.cpp - MemRef to EmitC conversion ---------------------===//
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
// This file implements a pass to convert memref ops into emitc ops.
//
//===----------------------------------------------------------------------===//

#include "mlir/Conversion/MemRefToEmitC/MemRefToEmitCPass.h"

#include "mlir/Conversion/MemRefToEmitC/MemRefToEmitC.h"
#include "mlir/Dialect/EmitC/IR/EmitC.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/IR/Attributes.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "vm/core/ADT/SmallSet.h"
#include "vm/core/ADT/StringRef.h"

namespace mlir {
#define GEN_PASS_DEF_CONVERTMEMREFTOEMITC
#include "mlir/Conversion/Passes.h.inc"
} // namespace mlir

using namespace mlir;

namespace {

emitc::IncludeOp addStandardHeader(OpBuilder &builder, ModuleOp module,
                                   StringRef headerName) {
  StringAttr includeAttr = builder.getStringAttr(headerName);
  return emitc::IncludeOp::create(
      builder, module.getLoc(), includeAttr,
      /*is_standard_include=*/builder.getUnitAttr());
}

struct ConvertMemRefToEmitCPass
    : public impl::ConvertMemRefToEmitCBase<ConvertMemRefToEmitCPass> {
  using Base::Base;
  void runOnOperation() override {
    TypeConverter converter;
    ConvertMemRefToEmitCOptions options;
    options.lowerToCpp = this->lowerToCpp;
    // Fallback for other types.
    converter.addConversion([](Type type) -> std::optional<Type> {
      if (!emitc::isSupportedEmitCType(type))
        return {};
      return type;
    });

    populateMemRefToEmitCTypeConversion(converter);

    RewritePatternSet patterns(&getContext());
    populateMemRefToEmitCConversionPatterns(patterns, converter);

    ConversionTarget target(getContext());
    target.addIllegalDialect<memref::MemRefDialect>();
    target.addLegalDialect<emitc::EmitCDialect>();

    if (failed(applyPartialConversion(getOperation(), target,
                                      std::move(patterns))))
      return signalPassFailure();

    mlir::ModuleOp module = getOperation();
    toolchain::SmallSet<StringRef, 4> existingHeaders;
    mlir::OpBuilder builder(module.getBody(), module.getBody()->begin());
    module.walk([&](mlir::emitc::IncludeOp includeOp) {
      if (includeOp.getIsStandardInclude())
        existingHeaders.insert(includeOp.getInclude());
    });

    module.walk([&](mlir::emitc::CallOpaqueOp callOp) {
      StringRef expectedHeader;
      if (callOp.getCallee() == alignedAllocFunctionName ||
          callOp.getCallee() == mallocFunctionName)
        expectedHeader = options.lowerToCpp ? cppStandardLibraryHeader
                                            : cStandardLibraryHeader;
      else if (callOp.getCallee() == memcpyFunctionName)
        expectedHeader =
            options.lowerToCpp ? cppStringLibraryHeader : cStringLibraryHeader;
      else
        return mlir::WalkResult::advance();
      if (!existingHeaders.contains(expectedHeader)) {
        addStandardHeader(builder, module, expectedHeader);
        existingHeaders.insert(expectedHeader);
      }
      return mlir::WalkResult::advance();
    });
  }
};
} // namespace
