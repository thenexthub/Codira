//===-- AffineDemotion.cpp -----------------------------------------------===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
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
// This transformation is a prototype that demote affine dialects operations
// after optimizations to FIR loops operations.
// It is used after the AffinePromotion pass.
// It is not part of the production pipeline and would need more work in order
// to be used in production.
// More information can be found in this presentation:
// https://slides.com/rajanwalia/deck
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Optimizer/Dialect/FIRDialect.h"
#include "language/Compability/Optimizer/Dialect/FIROps.h"
#include "language/Compability/Optimizer/Dialect/FIRType.h"
#include "language/Compability/Optimizer/Transforms/Passes.h"
#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Affine/Utils.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/IntegerSet.h"
#include "mlir/IR/Visitors.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "toolchain/ADT/DenseMap.h"
#include "toolchain/Support/CommandLine.h"
#include "toolchain/Support/Debug.h"

namespace fir {
#define GEN_PASS_DEF_AFFINEDIALECTDEMOTION
#include "language/Compability/Optimizer/Transforms/Passes.h.inc"
} // namespace fir

#define DEBUG_TYPE "flang-affine-demotion"

using namespace fir;
using namespace mlir;

namespace {

class AffineLoadConversion
    : public OpConversionPattern<mlir::affine::AffineLoadOp> {
public:
  using OpConversionPattern<mlir::affine::AffineLoadOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(mlir::affine::AffineLoadOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    SmallVector<Value> indices(adaptor.getIndices());
    auto maybeExpandedMap = affine::expandAffineMap(rewriter, op.getLoc(),
                                                    op.getAffineMap(), indices);
    if (!maybeExpandedMap)
      return failure();

    auto coorOp = fir::CoordinateOp::create(
        rewriter, op.getLoc(),
        fir::ReferenceType::get(op.getResult().getType()), adaptor.getMemref(),
        *maybeExpandedMap);

    rewriter.replaceOpWithNewOp<fir::LoadOp>(op, coorOp.getResult());
    return success();
  }
};

class AffineStoreConversion
    : public OpConversionPattern<mlir::affine::AffineStoreOp> {
public:
  using OpConversionPattern<mlir::affine::AffineStoreOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(mlir::affine::AffineStoreOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const override {
    SmallVector<Value> indices(op.getIndices());
    auto maybeExpandedMap = affine::expandAffineMap(rewriter, op.getLoc(),
                                                    op.getAffineMap(), indices);
    if (!maybeExpandedMap)
      return failure();

    auto coorOp = fir::CoordinateOp::create(
        rewriter, op.getLoc(),
        fir::ReferenceType::get(op.getValueToStore().getType()),
        adaptor.getMemref(), *maybeExpandedMap);
    rewriter.replaceOpWithNewOp<fir::StoreOp>(op, adaptor.getValue(),
                                              coorOp.getResult());
    return success();
  }
};

class ConvertConversion : public mlir::OpRewritePattern<fir::ConvertOp> {
public:
  using OpRewritePattern::OpRewritePattern;
  toolchain::LogicalResult
  matchAndRewrite(fir::ConvertOp op,
                  mlir::PatternRewriter &rewriter) const override {
    if (mlir::isa<mlir::MemRefType>(op.getRes().getType())) {
      // due to index calculation moving to affine maps we still need to
      // add converts for sequence types this has a side effect of losing
      // some information about arrays with known dimensions by creating:
      // fir.convert %arg0 : (!fir.ref<!fir.array<5xi32>>) ->
      // !fir.ref<!fir.array<?xi32>>
      if (auto refTy =
              mlir::dyn_cast<fir::ReferenceType>(op.getValue().getType()))
        if (auto arrTy = mlir::dyn_cast<fir::SequenceType>(refTy.getEleTy())) {
          fir::SequenceType::Shape flatShape = {
              fir::SequenceType::getUnknownExtent()};
          auto flatArrTy = fir::SequenceType::get(flatShape, arrTy.getEleTy());
          auto flatTy = fir::ReferenceType::get(flatArrTy);
          rewriter.replaceOpWithNewOp<fir::ConvertOp>(op, flatTy,
                                                      op.getValue());
          return success();
        }
      rewriter.startOpModification(op->getParentOp());
      op.getResult().replaceAllUsesWith(op.getValue());
      rewriter.finalizeOpModification(op->getParentOp());
      rewriter.eraseOp(op);
    }
    return success();
  }
};

mlir::Type convertMemRef(mlir::MemRefType type) {
  return fir::SequenceType::get(SmallVector<int64_t>(type.getShape()),
                                type.getElementType());
}

class StdAllocConversion : public mlir::OpRewritePattern<memref::AllocOp> {
public:
  using OpRewritePattern::OpRewritePattern;
  toolchain::LogicalResult
  matchAndRewrite(memref::AllocOp op,
                  mlir::PatternRewriter &rewriter) const override {
    rewriter.replaceOpWithNewOp<fir::AllocaOp>(op, convertMemRef(op.getType()),
                                               op.getMemref());
    return success();
  }
};

class AffineDialectDemotion
    : public fir::impl::AffineDialectDemotionBase<AffineDialectDemotion> {
public:
  void runOnOperation() override {
    auto *context = &getContext();
    auto function = getOperation();
    LLVM_DEBUG(toolchain::dbgs() << "AffineDemotion: running on function:\n";
               function.print(toolchain::dbgs()););

    mlir::RewritePatternSet patterns(context);
    patterns.insert<ConvertConversion>(context);
    patterns.insert<AffineLoadConversion>(context);
    patterns.insert<AffineStoreConversion>(context);
    patterns.insert<StdAllocConversion>(context);
    mlir::ConversionTarget target(*context);
    target.addIllegalOp<memref::AllocOp>();
    target.addDynamicallyLegalOp<fir::ConvertOp>([](fir::ConvertOp op) {
      if (mlir::isa<mlir::MemRefType>(op.getRes().getType()))
        return false;
      return true;
    });
    target
        .addLegalDialect<FIROpsDialect, mlir::scf::SCFDialect,
                         mlir::arith::ArithDialect, mlir::func::FuncDialect>();

    if (mlir::failed(mlir::applyPartialConversion(function, target,
                                                  std::move(patterns)))) {
      mlir::emitError(mlir::UnknownLoc::get(context),
                      "error in converting affine dialect\n");
      signalPassFailure();
    }
  }
};

} // namespace

std::unique_ptr<mlir::Pass> fir::createAffineDemotionPass() {
  return std::make_unique<AffineDialectDemotion>();
}
