//===- InlineHLFIRCopyIn.cpp - Inline hlfir.copy_in ops -------------------===//
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
// Transform hlfir.copy_in array operations into loop nests performing element
// per element assignments. For simplicity, the inlining is done for trivial
// data types when the copy_in does not require a corresponding copy_out and
// when the input array is not behind a pointer. This may change in the future.
//===----------------------------------------------------------------------===//

#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/HLFIRTools.h"
#include "language/Compability/Optimizer/Dialect/FIRType.h"
#include "language/Compability/Optimizer/HLFIR/HLFIROps.h"
#include "language/Compability/Optimizer/OpenMP/Passes.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace hlfir {
#define GEN_PASS_DEF_INLINEHLFIRCOPYIN
#include "language/Compability/Optimizer/HLFIR/Passes.h.inc"
} // namespace hlfir

#define DEBUG_TYPE "inline-hlfir-copy-in"

static toolchain::cl::opt<bool> noInlineHLFIRCopyIn(
    "no-inline-hlfir-copy-in",
    toolchain::cl::desc("Do not inline hlfir.copy_in operations"),
    toolchain::cl::init(false));

namespace {
class InlineCopyInConversion : public mlir::OpRewritePattern<hlfir::CopyInOp> {
public:
  using mlir::OpRewritePattern<hlfir::CopyInOp>::OpRewritePattern;

  toolchain::LogicalResult
  matchAndRewrite(hlfir::CopyInOp copyIn,
                  mlir::PatternRewriter &rewriter) const override;
};

toolchain::LogicalResult
InlineCopyInConversion::matchAndRewrite(hlfir::CopyInOp copyIn,
                                        mlir::PatternRewriter &rewriter) const {
  fir::FirOpBuilder builder(rewriter, copyIn.getOperation());
  mlir::Location loc = copyIn.getLoc();
  hlfir::Entity inputVariable{copyIn.getVar()};
  mlir::Type resultAddrType = copyIn.getCopiedIn().getType();
  if (!fir::isa_trivial(inputVariable.getFortranElementType()))
    return rewriter.notifyMatchFailure(copyIn,
                                       "CopyInOp's data type is not trivial");

  // There should be exactly one user of WasCopied - the corresponding
  // CopyOutOp.
  if (!copyIn.getWasCopied().hasOneUse())
    return rewriter.notifyMatchFailure(
        copyIn, "CopyInOp's WasCopied has no single user");
  // The copy out should always be present, either to actually copy or just
  // deallocate memory.
  auto copyOut = mlir::dyn_cast<hlfir::CopyOutOp>(
      copyIn.getWasCopied().user_begin().getCurrent().getUser());

  if (!copyOut)
    return rewriter.notifyMatchFailure(copyIn,
                                       "CopyInOp has no direct CopyOut");

  if (mlir::cast<fir::BaseBoxType>(resultAddrType).isAssumedRank())
    return rewriter.notifyMatchFailure(copyIn,
                                       "The result array is assumed-rank");

  // Only inline the copy_in when copy_out does not need to be done, i.e. in
  // case of intent(in).
  if (copyOut.getVar())
    return rewriter.notifyMatchFailure(copyIn, "CopyIn needs a copy-out");

  inputVariable =
      hlfir::derefPointersAndAllocatables(loc, builder, inputVariable);
  mlir::Type sequenceType =
      hlfir::getFortranElementOrSequenceType(inputVariable.getType());
  fir::BoxType resultBoxType = fir::BoxType::get(sequenceType);
  mlir::Value isContiguous =
      fir::IsContiguousBoxOp::create(builder, loc, inputVariable);
  mlir::Operation::result_range results =
      builder
          .genIfOp(loc, {resultBoxType, builder.getI1Type()}, isContiguous,
                   /*withElseRegion=*/true)
          .genThen([&]() {
            mlir::Value result = inputVariable;
            if (fir::isPointerType(inputVariable.getType())) {
              result = fir::ReboxOp::create(builder, loc, resultBoxType,
                                            inputVariable, mlir::Value{},
                                            mlir::Value{});
            }
            fir::ResultOp::create(
                builder, loc,
                mlir::ValueRange{result, builder.createBool(loc, false)});
          })
          .genElse([&] {
            mlir::Value shape = hlfir::genShape(loc, builder, inputVariable);
            toolchain::SmallVector<mlir::Value> extents =
                hlfir::getIndexExtents(loc, builder, shape);
            toolchain::StringRef tmpName{".tmp.copy_in"};
            toolchain::SmallVector<mlir::Value> lenParams;
            mlir::Value alloc = builder.createHeapTemporary(
                loc, sequenceType, tmpName, extents, lenParams);

            auto declareOp = hlfir::DeclareOp::create(builder, loc, alloc,
                                                      tmpName, shape, lenParams,
                                                      /*dummy_scope=*/nullptr);
            hlfir::Entity temp{declareOp.getBase()};
            hlfir::LoopNest loopNest =
                hlfir::genLoopNest(loc, builder, extents, /*isUnordered=*/true,
                                   flangomp::shouldUseWorkshareLowering(copyIn),
                                   /*couldVectorize=*/false);
            builder.setInsertionPointToStart(loopNest.body);
            hlfir::Entity elem = hlfir::getElementAt(
                loc, builder, inputVariable, loopNest.oneBasedIndices);
            elem = hlfir::loadTrivialScalar(loc, builder, elem);
            hlfir::Entity tempElem = hlfir::getElementAt(
                loc, builder, temp, loopNest.oneBasedIndices);
            hlfir::AssignOp::create(builder, loc, elem, tempElem);
            builder.setInsertionPointAfter(loopNest.outerOp);

            mlir::Value result;
            // Make sure the result is always a boxed array by boxing it
            // ourselves if need be.
            if (mlir::isa<fir::BaseBoxType>(temp.getType())) {
              result = temp;
            } else {
              fir::ReferenceType refTy =
                  fir::ReferenceType::get(temp.getElementOrSequenceType());
              mlir::Value refVal = builder.createConvert(loc, refTy, temp);
              result = fir::EmboxOp::create(builder, loc, resultBoxType, refVal,
                                            shape);
            }

            fir::ResultOp::create(
                builder, loc,
                mlir::ValueRange{result, builder.createBool(loc, true)});
          })
          .getResults();

  mlir::OpResult resultBox = results[0];
  mlir::OpResult needsCleanup = results[1];

  // Prepare the corresponding copyOut to free the temporary if it is required
  auto alloca = fir::AllocaOp::create(builder, loc, resultBox.getType());
  auto store = fir::StoreOp::create(builder, loc, resultBox, alloca);
  rewriter.startOpModification(copyOut);
  copyOut->setOperand(0, store.getMemref());
  copyOut->setOperand(1, needsCleanup);
  rewriter.finalizeOpModification(copyOut);

  rewriter.replaceOp(copyIn, {resultBox, builder.genNot(loc, isContiguous)});
  return mlir::success();
}

class InlineHLFIRCopyInPass
    : public hlfir::impl::InlineHLFIRCopyInBase<InlineHLFIRCopyInPass> {
public:
  void runOnOperation() override {
    mlir::MLIRContext *context = &getContext();

    mlir::GreedyRewriteConfig config;
    // Prevent the pattern driver from merging blocks.
    config.setRegionSimplificationLevel(
        mlir::GreedySimplifyRegionLevel::Disabled);

    mlir::RewritePatternSet patterns(context);
    if (!noInlineHLFIRCopyIn) {
      patterns.insert<InlineCopyInConversion>(context);
    }

    if (mlir::failed(mlir::applyPatternsGreedily(
            getOperation(), std::move(patterns), config))) {
      mlir::emitError(getOperation()->getLoc(),
                      "failure in hlfir.copy_in inlining");
      signalPassFailure();
    }
  }
};
} // namespace
