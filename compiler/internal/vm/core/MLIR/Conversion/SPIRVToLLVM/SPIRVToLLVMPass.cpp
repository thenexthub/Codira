//===- SPIRVToLLVMPass.cpp - SPIR-V to LLVM Passes ------------------------===//
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
// This file implements a pass to convert MLIR SPIR-V ops into LLVM ops
//
//===----------------------------------------------------------------------===//

#include "mlir/Conversion/SPIRVToLLVM/SPIRVToLLVMPass.h"

#include "mlir/Conversion/LLVMCommon/TypeConverter.h"
#include "mlir/Conversion/SPIRVToLLVM/SPIRVToLLVM.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVDialect.h"
#include "mlir/Dialect/SPIRV/IR/SPIRVEnums.h"
#include "mlir/Pass/Pass.h"

namespace mlir {
#define GEN_PASS_DEF_CONVERTSPIRVTOLLVMPASS
#include "mlir/Conversion/Passes.h.inc"
} // namespace mlir

using namespace mlir;

namespace {
/// A pass converting MLIR SPIR-V operations into LLVM dialect.
class ConvertSPIRVToLLVMPass
    : public impl::ConvertSPIRVToLLVMPassBase<ConvertSPIRVToLLVMPass> {
  void runOnOperation() override;

public:
  using Base::Base;
};
} // namespace

void ConvertSPIRVToLLVMPass::runOnOperation() {
  MLIRContext *context = &getContext();
  ModuleOp module = getOperation();

  LowerToLLVMOptions options(&getContext());

  LLVMTypeConverter converter(&getContext(), options);

  // Encode global variable's descriptor set and binding if they exist.
  encodeBindAttribute(module);

  RewritePatternSet patterns(context);

  populateSPIRVToLLVMTypeConversion(converter, clientAPI);

  populateSPIRVToLLVMModuleConversionPatterns(converter, patterns);
  populateSPIRVToLLVMConversionPatterns(converter, patterns, clientAPI);
  populateSPIRVToLLVMFunctionConversionPatterns(converter, patterns);

  ConversionTarget target(*context);
  target.addIllegalDialect<spirv::SPIRVDialect>();
  target.addLegalDialect<LLVM::LLVMDialect>();

  if (clientAPI != spirv::ClientAPI::OpenCL &&
      clientAPI != spirv::ClientAPI::Unknown)
    getOperation()->emitWarning()
        << "address space mapping for client '"
        << spirv::stringifyClientAPI(clientAPI) << "' not implemented";

  // Set `ModuleOp` as legal for `spirv.module` conversion.
  target.addLegalOp<ModuleOp>();
  if (failed(applyPartialConversion(module, target, std::move(patterns))))
    signalPassFailure();
}
