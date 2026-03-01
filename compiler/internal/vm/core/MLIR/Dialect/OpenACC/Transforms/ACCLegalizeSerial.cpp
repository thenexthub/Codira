//===- ACCLegalizeSerial.cpp - Legalize ACC Serial region -----------------===//
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
// This pass converts acc.serial into acc.parallel with num_gangs(1)
// num_workers(1) vector_length(1).
//
// This transformation simplifies processing of acc regions by unifying the
// handling of serial and parallel constructs. Since an OpenACC serial region
// executes sequentially (like a parallel region with a single gang, worker, and
// vector), this conversion is semantically equivalent while enabling code reuse
// in later compilation stages.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/OpenACC/Transforms/Passes.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/OpenACC/OpenACC.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/Location.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/IR/Region.h"
#include "mlir/IR/Value.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Support/LogicalResult.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "vm/core/Support/Debug.h"

namespace mlir {
namespace acc {
#define GEN_PASS_DEF_ACCLEGALIZESERIAL
#include "mlir/Dialect/OpenACC/Transforms/Passes.h.inc"
} // namespace acc
} // namespace mlir

#define DEBUG_TYPE "acc-legalize-serial"

namespace {
using namespace mlir;

struct ACCSerialOpConversion : public OpRewritePattern<acc::SerialOp> {
  using OpRewritePattern<acc::SerialOp>::OpRewritePattern;

  LogicalResult matchAndRewrite(acc::SerialOp serialOp,
                                PatternRewriter &rewriter) const override {

    const Location loc = serialOp.getLoc();

    // Create a container holding the constant value of 1 for use as the
    // num_gangs, num_workers, and vector_length attributes.
    toolchain::SmallVector<mlir::Value> numValues;
    auto value = arith::ConstantIntOp::create(rewriter, loc, 1, 32);
    numValues.push_back(value);

    // Since num_gangs is specified as both attributes and values, create a
    // segment attribute.
    toolchain::SmallVector<int32_t> numGangsSegments;
    numGangsSegments.push_back(numValues.size());
    auto gangSegmentsAttr = rewriter.getDenseI32ArrayAttr(numGangsSegments);

    // Create a device_type attribute set to `none` which ensures that
    // the parallel dimensions specification applies to the default clauses.
    toolchain::SmallVector<mlir::Attribute> crtDeviceTypes;
    auto crtDeviceTypeAttr = mlir::acc::DeviceTypeAttr::get(
        rewriter.getContext(), mlir::acc::DeviceType::None);
    crtDeviceTypes.push_back(crtDeviceTypeAttr);
    auto devTypeAttr =
        mlir::ArrayAttr::get(rewriter.getContext(), crtDeviceTypes);

    LLVM_DEBUG(toolchain::dbgs() << "acc.serial OP: " << serialOp << "\n");

    // Create a new acc.parallel op with the same operands - except include the
    // num_gangs, num_workers, and vector_length attributes.
    acc::ParallelOp parOp = acc::ParallelOp::create(
        rewriter, loc, serialOp.getAsyncOperands(),
        serialOp.getAsyncOperandsDeviceTypeAttr(), serialOp.getAsyncOnlyAttr(),
        serialOp.getWaitOperands(), serialOp.getWaitOperandsSegmentsAttr(),
        serialOp.getWaitOperandsDeviceTypeAttr(),
        serialOp.getHasWaitDevnumAttr(), serialOp.getWaitOnlyAttr(), numValues,
        gangSegmentsAttr, devTypeAttr, numValues, devTypeAttr, numValues,
        devTypeAttr, serialOp.getIfCond(), serialOp.getSelfCond(),
        serialOp.getSelfAttrAttr(), serialOp.getReductionOperands(),
        serialOp.getPrivateOperands(), serialOp.getFirstprivateOperands(),
        serialOp.getDataClauseOperands(), serialOp.getDefaultAttrAttr(),
        serialOp.getCombinedAttr());

    parOp.getRegion().takeBody(serialOp.getRegion());

    LLVM_DEBUG(toolchain::dbgs() << "acc.parallel OP: " << parOp << "\n");
    rewriter.replaceOp(serialOp, parOp);

    return success();
  }
};

class ACCLegalizeSerial
    : public mlir::acc::impl::ACCLegalizeSerialBase<ACCLegalizeSerial> {
public:
  using ACCLegalizeSerialBase<ACCLegalizeSerial>::ACCLegalizeSerialBase;
  void runOnOperation() override {
    func::FuncOp funcOp = getOperation();
    MLIRContext *context = funcOp.getContext();
    RewritePatternSet patterns(context);
    patterns.insert<ACCSerialOpConversion>(context);
    (void)applyPatternsGreedily(funcOp, std::move(patterns));
  }
};

} // namespace
