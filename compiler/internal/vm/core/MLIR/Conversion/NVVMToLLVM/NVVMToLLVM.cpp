//===- NVVMToLLVM.cpp - NVVM to LLVM dialect conversion -----------------===//
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
// This file implements a translation NVVM ops which is not supported in LLVM
// core.
//
//===----------------------------------------------------------------------===//

#include "mlir/Conversion/NVVMToLLVM/NVVMToLLVM.h"

#include "mlir/Conversion/ConvertToLLVM/ToLLVMInterface.h"
#include "mlir/Conversion/LLVMCommon/Pattern.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/LLVMIR/LLVMTypes.h"
#include "mlir/Dialect/LLVMIR/NVVMDialect.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/IR/TypeUtilities.h"
#include "mlir/IR/Value.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Support/LLVM.h"
#include "vm/core/Support/DebugLog.h"
#include "vm/core/Support/LogicalResult.h"
#include "vm/core/Support/raw_ostream.h"

#define DEBUG_TYPE "nvvm-to-toolchain"

namespace mlir {
#define GEN_PASS_DEF_CONVERTNVVMTOLLVMPASS
#include "mlir/Conversion/Passes.h.inc"
} // namespace mlir

using namespace mlir;
using namespace NVVM;

namespace {

struct PtxLowering
    : public OpInterfaceRewritePattern<BasicPtxBuilderInterface> {
  using OpInterfaceRewritePattern<
      BasicPtxBuilderInterface>::OpInterfaceRewritePattern;

  PtxLowering(MLIRContext *context, PatternBenefit benefit = 2)
      : OpInterfaceRewritePattern(context, benefit) {}

  LogicalResult matchAndRewrite(BasicPtxBuilderInterface op,
                                PatternRewriter &rewriter) const override {
    if (op.hasIntrinsic()) {
      LDBG() << "Ptx Builder does not lower \n\t" << op;
      return failure();
    }

    SmallVector<std::pair<Value, PTXRegisterMod>> asmValues;
    LDBG() << op.getPtx();

    bool needsManualMapping = op.getAsmValues(rewriter, asmValues);
    PtxBuilder generator(op, rewriter, needsManualMapping);
    for (auto &[asmValue, modifier] : asmValues) {
      LDBG() << asmValue << "\t Modifier : " << modifier;
      if (failed(generator.insertValue(asmValue, modifier)))
        return failure();
    }

    generator.buildAndReplaceOp();
    return success();
  }
};

struct ConvertNVVMToLLVMPass
    : public impl::ConvertNVVMToLLVMPassBase<ConvertNVVMToLLVMPass> {
  using Base::Base;

  void getDependentDialects(DialectRegistry &registry) const override {
    registry.insert<LLVM::LLVMDialect, NVVM::NVVMDialect>();
  }

  void runOnOperation() override {
    ConversionTarget target(getContext());
    target.addLegalDialect<::mlir::LLVM::LLVMDialect>();
    RewritePatternSet pattern(&getContext());
    mlir::populateNVVMToLLVMConversionPatterns(pattern);
    if (failed(
            applyPartialConversion(getOperation(), target, std::move(pattern))))
      signalPassFailure();
  }
};

/// Implement the interface to convert NVVM to LLVM.
struct NVVMToLLVMDialectInterface : public ConvertToLLVMPatternInterface {
  using ConvertToLLVMPatternInterface::ConvertToLLVMPatternInterface;
  void loadDependentDialects(MLIRContext *context) const final {
    context->loadDialect<NVVMDialect>();
  }

  /// Hook for derived dialect interface to provide conversion patterns
  /// and mark dialect legal for the conversion target.
  void populateConvertToLLVMConversionPatterns(
      ConversionTarget &target, LLVMTypeConverter &typeConverter,
      RewritePatternSet &patterns) const final {
    populateNVVMToLLVMConversionPatterns(patterns);
  }
};

} // namespace

void mlir::populateNVVMToLLVMConversionPatterns(RewritePatternSet &patterns) {
  patterns.add<PtxLowering>(patterns.getContext());
}

void mlir::registerConvertNVVMToLLVMInterface(DialectRegistry &registry) {
  registry.addExtension(+[](MLIRContext *ctx, NVVMDialect *dialect) {
    dialect->addInterfaces<NVVMToLLVMDialectInterface>();
  });
}
