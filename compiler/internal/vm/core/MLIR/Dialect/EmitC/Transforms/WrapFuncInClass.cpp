//===- WrapFuncInClass.cpp - Wrap Emitc Funcs in classes -------------===//
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

#include "mlir/Dialect/EmitC/IR/EmitC.h"
#include "mlir/Dialect/EmitC/Transforms/Passes.h"
#include "mlir/Dialect/EmitC/Transforms/Transforms.h"
#include "mlir/IR/Attributes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/WalkPatternRewriteDriver.h"

using namespace mlir;
using namespace emitc;

namespace mlir {
namespace emitc {
#define GEN_PASS_DEF_WRAPFUNCINCLASSPASS
#include "mlir/Dialect/EmitC/Transforms/Passes.h.inc"

namespace {
struct WrapFuncInClassPass
    : public impl::WrapFuncInClassPassBase<WrapFuncInClassPass> {
  using WrapFuncInClassPassBase::WrapFuncInClassPassBase;
  void runOnOperation() override {
    Operation *rootOp = getOperation();

    RewritePatternSet patterns(&getContext());
    populateFuncPatterns(patterns);

    walkAndApplyPatterns(rootOp, std::move(patterns));
  }
};

} // namespace
} // namespace emitc
} // namespace mlir

class WrapFuncInClass : public OpRewritePattern<emitc::FuncOp> {
public:
  WrapFuncInClass(MLIRContext *context)
      : OpRewritePattern<emitc::FuncOp>(context) {}

  LogicalResult matchAndRewrite(emitc::FuncOp funcOp,
                                PatternRewriter &rewriter) const override {

    auto className = funcOp.getSymNameAttr().str() + "Class";
    ClassOp newClassOp = ClassOp::create(rewriter, funcOp.getLoc(), className);

    SmallVector<std::pair<StringAttr, TypeAttr>> fields;
    rewriter.createBlock(&newClassOp.getBody());
    rewriter.setInsertionPointToStart(&newClassOp.getBody().front());

    auto argAttrs = funcOp.getArgAttrs();
    for (auto [idx, val] : toolchain::enumerate(funcOp.getArguments())) {
      StringAttr fieldName =
          rewriter.getStringAttr("fieldName" + std::to_string(idx));

      TypeAttr typeAttr = TypeAttr::get(val.getType());
      fields.push_back({fieldName, typeAttr});

      FieldOp fieldop = emitc::FieldOp::create(rewriter, funcOp->getLoc(),
                                               fieldName, typeAttr, nullptr);

      if (argAttrs && idx < argAttrs->size()) {
        fieldop->setDiscardableAttrs(funcOp.getArgAttrDict(idx));
      }
    }

    rewriter.setInsertionPointToEnd(&newClassOp.getBody().front());
    FunctionType funcType = funcOp.getFunctionType();
    Location loc = funcOp.getLoc();
    FuncOp newFuncOp =
        emitc::FuncOp::create(rewriter, loc, ("execute"), funcType);

    rewriter.createBlock(&newFuncOp.getBody());
    newFuncOp.getBody().takeBody(funcOp.getBody());

    rewriter.setInsertionPointToStart(&newFuncOp.getBody().front());
    std::vector<Value> newArguments;
    newArguments.reserve(fields.size());
    for (auto &[fieldName, attr] : fields) {
      GetFieldOp arg =
          emitc::GetFieldOp::create(rewriter, loc, attr.getValue(), fieldName);
      newArguments.push_back(arg);
    }

    for (auto [oldArg, newArg] :
         toolchain::zip(newFuncOp.getArguments(), newArguments)) {
      rewriter.replaceAllUsesWith(oldArg, newArg);
    }

    toolchain::BitVector argsToErase(newFuncOp.getNumArguments(), true);
    if (failed(newFuncOp.eraseArguments(argsToErase)))
      newFuncOp->emitOpError("failed to erase all arguments using BitVector");

    rewriter.replaceOp(funcOp, newClassOp);
    return success();
  }
};

void mlir::emitc::populateFuncPatterns(RewritePatternSet &patterns) {
  patterns.add<WrapFuncInClass>(patterns.getContext());
}
