//===-- AnnotateConstant.cpp ----------------------------------------------===//
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

// #include "PassDetail.h"
#include "language/Compability/Optimizer/Dialect/FIRDialect.h"
#include "language/Compability/Optimizer/Dialect/FIROps.h"
#include "language/Compability/Optimizer/Transforms/Passes.h"
#include "mlir/IR/BuiltinAttributes.h"

namespace fir {
#define GEN_PASS_DEF_ANNOTATECONSTANTOPERANDS
#include "language/Compability/Optimizer/Transforms/Passes.h.inc"
} // namespace fir

#define DEBUG_TYPE "flang-annotate-constant"

using namespace fir;

namespace {
struct AnnotateConstantOperands
    : public impl::AnnotateConstantOperandsBase<AnnotateConstantOperands> {
  void runOnOperation() override {
    auto *context = &getContext();
    mlir::Dialect *firDialect = context->getLoadedDialect("fir");
    getOperation()->walk([&](mlir::Operation *op) {
      // We filter out other dialects even though they may undergo merging of
      // non-equal constant values by the canonicalizer as well.
      if (op->getDialect() == firDialect) {
        toolchain::SmallVector<mlir::Attribute> attrs;
        bool hasOneOrMoreConstOpnd = false;
        for (mlir::Value opnd : op->getOperands()) {
          if (auto constOp = mlir::dyn_cast_or_null<mlir::arith::ConstantOp>(
                  opnd.getDefiningOp())) {
            attrs.push_back(constOp.getValue());
            hasOneOrMoreConstOpnd = true;
          } else if (auto addrOp = mlir::dyn_cast_or_null<fir::AddrOfOp>(
                         opnd.getDefiningOp())) {
            attrs.push_back(addrOp.getSymbol());
            hasOneOrMoreConstOpnd = true;
          } else {
            attrs.push_back(mlir::UnitAttr::get(context));
          }
        }
        if (hasOneOrMoreConstOpnd)
          op->setAttr("canonicalize_constant_operands",
                      mlir::ArrayAttr::get(context, attrs));
      }
    });
  }
};

} // namespace

std::unique_ptr<mlir::Pass> fir::createAnnotateConstantOperandsPass() {
  return std::make_unique<AnnotateConstantOperands>();
}
