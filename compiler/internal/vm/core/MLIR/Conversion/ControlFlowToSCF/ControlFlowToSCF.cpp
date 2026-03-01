//===- ControlFlowToSCF.h - ControlFlow to SCF -------------*- C++ ------*-===//
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
// Define conversions from the ControlFlow dialect to the SCF dialect.
//
//===----------------------------------------------------------------------===//

#include "mlir/Conversion/ControlFlowToSCF/ControlFlowToSCF.h"

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/UB/IR/UBOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/CFGToSCF.h"

namespace mlir {
#define GEN_PASS_DEF_LIFTCONTROLFLOWTOSCFPASS
#include "mlir/Conversion/Passes.h.inc"
} // namespace mlir

using namespace mlir;

FailureOr<Operation *>
ControlFlowToSCFTransformation::createStructuredBranchRegionOp(
    OpBuilder &builder, Operation *controlFlowCondOp, TypeRange resultTypes,
    MutableArrayRef<Region> regions) {
  if (auto condBrOp = dyn_cast<cf::CondBranchOp>(controlFlowCondOp)) {
    assert(regions.size() == 2);
    auto ifOp = scf::IfOp::create(builder, controlFlowCondOp->getLoc(),
                                  resultTypes, condBrOp.getCondition());
    ifOp.getThenRegion().takeBody(regions[0]);
    ifOp.getElseRegion().takeBody(regions[1]);
    return ifOp.getOperation();
  }

  if (auto switchOp = dyn_cast<cf::SwitchOp>(controlFlowCondOp)) {
    // `getCFGSwitchValue` returns an i32 that we need to convert to index
    // fist.
    auto cast = arith::IndexCastUIOp::create(
        builder, controlFlowCondOp->getLoc(), builder.getIndexType(),
        switchOp.getFlag());
    SmallVector<int64_t> cases;
    if (auto caseValues = switchOp.getCaseValues())
      toolchain::append_range(
          cases, toolchain::map_range(*caseValues, [](const toolchain::APInt &apInt) {
            return apInt.getZExtValue();
          }));

    assert(regions.size() == cases.size() + 1);

    auto indexSwitchOp =
        scf::IndexSwitchOp::create(builder, controlFlowCondOp->getLoc(),
                                   resultTypes, cast, cases, cases.size());

    indexSwitchOp.getDefaultRegion().takeBody(regions[0]);
    for (auto &&[targetRegion, sourceRegion] :
         toolchain::zip(indexSwitchOp.getCaseRegions(), toolchain::drop_begin(regions)))
      targetRegion.takeBody(sourceRegion);

    return indexSwitchOp.getOperation();
  }

  controlFlowCondOp->emitOpError(
      "Cannot convert unknown control flow op to structured control flow");
  return failure();
}

LogicalResult
ControlFlowToSCFTransformation::createStructuredBranchRegionTerminatorOp(
    Location loc, OpBuilder &builder, Operation *branchRegionOp,
    Operation *replacedControlFlowOp, ValueRange results) {
  scf::YieldOp::create(builder, loc, results);
  return success();
}

FailureOr<Operation *>
ControlFlowToSCFTransformation::createStructuredDoWhileLoopOp(
    OpBuilder &builder, Operation *replacedOp, ValueRange loopVariablesInit,
    Value condition, ValueRange loopVariablesNextIter, Region &&loopBody) {
  Location loc = replacedOp->getLoc();
  auto whileOp = scf::WhileOp::create(
      builder, loc, loopVariablesInit.getTypes(), loopVariablesInit);

  whileOp.getBefore().takeBody(loopBody);

  builder.setInsertionPointToEnd(&whileOp.getBefore().back());
  // `getCFGSwitchValue` returns a i32. We therefore need to truncate the
  // condition to i1 first. It is guaranteed to be either 0 or 1 already.
  scf::ConditionOp::create(
      builder, loc,
      arith::TruncIOp::create(builder, loc, builder.getI1Type(), condition),
      loopVariablesNextIter);

  Block *afterBlock = builder.createBlock(&whileOp.getAfter());
  afterBlock->addArguments(
      loopVariablesInit.getTypes(),
      SmallVector<Location>(loopVariablesInit.size(), loc));
  scf::YieldOp::create(builder, loc, afterBlock->getArguments());

  return whileOp.getOperation();
}

Value ControlFlowToSCFTransformation::getCFGSwitchValue(Location loc,
                                                        OpBuilder &builder,
                                                        unsigned int value) {
  return arith::ConstantOp::create(builder, loc,
                                   builder.getI32IntegerAttr(value));
}

void ControlFlowToSCFTransformation::createCFGSwitchOp(
    Location loc, OpBuilder &builder, Value flag,
    ArrayRef<unsigned int> caseValues, BlockRange caseDestinations,
    ArrayRef<ValueRange> caseArguments, Block *defaultDest,
    ValueRange defaultArgs) {
  cf::SwitchOp::create(builder, loc, flag, defaultDest, defaultArgs,
                       toolchain::to_vector_of<int32_t>(caseValues),
                       caseDestinations, caseArguments);
}

Value ControlFlowToSCFTransformation::getUndefValue(Location loc,
                                                    OpBuilder &builder,
                                                    Type type) {
  return ub::PoisonOp::create(builder, loc, type, nullptr);
}

FailureOr<Operation *>
ControlFlowToSCFTransformation::createUnreachableTerminator(Location loc,
                                                            OpBuilder &builder,
                                                            Region &region) {

  // TODO: This should create a `ub.unreachable` op. Once such an operation
  //       exists to make the pass independent of the func dialect. For now just
  //       return poison values.
  Operation *parentOp = region.getParentOp();
  auto funcOp = dyn_cast<func::FuncOp>(parentOp);
  if (!funcOp)
    return emitError(loc, "Cannot create unreachable terminator for '")
           << parentOp->getName() << "'";

  return func::ReturnOp::create(
             builder, loc,
             toolchain::map_to_vector(
                 funcOp.getResultTypes(),
                 [&](Type type) { return getUndefValue(loc, builder, type); }))
      .getOperation();
}

namespace {

struct LiftControlFlowToSCF
    : public impl::LiftControlFlowToSCFPassBase<LiftControlFlowToSCF> {

  using Base::Base;

  void runOnOperation() override {
    ControlFlowToSCFTransformation transformation;

    bool changed = false;
    Operation *op = getOperation();
    WalkResult result = op->walk([&](func::FuncOp funcOp) {
      if (funcOp.getBody().empty())
        return WalkResult::advance();

      auto &domInfo = funcOp != op ? getChildAnalysis<DominanceInfo>(funcOp)
                                   : getAnalysis<DominanceInfo>();

      auto visitor = [&](Operation *innerOp) -> WalkResult {
        for (Region &reg : innerOp->getRegions()) {
          FailureOr<bool> changedFunc =
              transformCFGToSCF(reg, transformation, domInfo);
          if (failed(changedFunc))
            return WalkResult::interrupt();

          changed |= *changedFunc;
        }
        return WalkResult::advance();
      };

      if (funcOp->walk<WalkOrder::PostOrder>(visitor).wasInterrupted())
        return WalkResult::interrupt();

      return WalkResult::advance();
    });
    if (result.wasInterrupted())
      return signalPassFailure();

    if (!changed)
      markAllAnalysesPreserved();
  }
};
} // namespace
