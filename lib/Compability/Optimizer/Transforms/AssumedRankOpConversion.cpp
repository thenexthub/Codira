//===-- AssumedRankOpConversion.cpp ---------------------------------------===//
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

#include "language/Compability/Lower/BuiltinModules.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/Runtime/Support.h"
#include "language/Compability/Optimizer/Builder/Todo.h"
#include "language/Compability/Optimizer/Dialect/FIRDialect.h"
#include "language/Compability/Optimizer/Dialect/FIROps.h"
#include "language/Compability/Optimizer/Support/TypeCode.h"
#include "language/Compability/Optimizer/Support/Utils.h"
#include "language/Compability/Optimizer/Transforms/Passes.h"
#include "language/Compability/Runtime/support.h"
#include "language/Compability/Support/Fortran.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace fir {
#define GEN_PASS_DEF_ASSUMEDRANKOPCONVERSION
#include "language/Compability/Optimizer/Transforms/Passes.h.inc"
} // namespace fir

using namespace fir;
using namespace mlir;

namespace {

static int getCFIAttribute(mlir::Type boxType) {
  if (fir::isAllocatableType(boxType))
    return CFI_attribute_allocatable;
  if (fir::isPointerType(boxType))
    return CFI_attribute_pointer;
  return CFI_attribute_other;
}

static language::Compability::runtime::LowerBoundModifier
getLowerBoundModifier(fir::LowerBoundModifierAttribute modifier) {
  switch (modifier) {
  case fir::LowerBoundModifierAttribute::Preserve:
    return language::Compability::runtime::LowerBoundModifier::Preserve;
  case fir::LowerBoundModifierAttribute::SetToOnes:
    return language::Compability::runtime::LowerBoundModifier::SetToOnes;
  case fir::LowerBoundModifierAttribute::SetToZeroes:
    return language::Compability::runtime::LowerBoundModifier::SetToZeroes;
  }
  toolchain_unreachable("bad modifier code");
}

class ReboxAssumedRankConv
    : public mlir::OpRewritePattern<fir::ReboxAssumedRankOp> {
public:
  using OpRewritePattern::OpRewritePattern;

  ReboxAssumedRankConv(mlir::MLIRContext *context,
                       mlir::SymbolTable *symbolTable, fir::KindMapping kindMap)
      : mlir::OpRewritePattern<fir::ReboxAssumedRankOp>(context),
        symbolTable{symbolTable}, kindMap{kindMap} {};

  toolchain::LogicalResult
  matchAndRewrite(fir::ReboxAssumedRankOp rebox,
                  mlir::PatternRewriter &rewriter) const override {
    fir::FirOpBuilder builder{rewriter, kindMap, symbolTable};
    mlir::Location loc = rebox.getLoc();
    auto newBoxType = mlir::cast<fir::BaseBoxType>(rebox.getType());
    mlir::Type newMaxRankBoxType =
        newBoxType.getBoxTypeWithNewShape(language::Compability::common::maxRank);
    // CopyAndUpdateDescriptor FIR interface requires loading
    // !fir.ref<fir.box> input which is expensive with assumed-rank. It could
    // be best to add an entry point that takes a non "const" from to cover
    // this case, but it would be good to indicate to LLVM that from does not
    // get modified.
    if (fir::isBoxAddress(rebox.getBox().getType()))
      TODO(loc, "fir.rebox_assumed_rank codegen with fir.ref<fir.box<>> input");
    mlir::Value tempDesc = builder.createTemporary(loc, newMaxRankBoxType);
    mlir::Value newDtype;
    mlir::Type newEleType = newBoxType.unwrapInnerType();
    auto oldBoxType = mlir::cast<fir::BaseBoxType>(
        fir::unwrapRefType(rebox.getBox().getType()));
    auto newDerivedType = mlir::dyn_cast<fir::RecordType>(newEleType);
    if (newDerivedType && !fir::isPolymorphicType(newBoxType) &&
        (fir::isPolymorphicType(oldBoxType) ||
         (newEleType != oldBoxType.unwrapInnerType())) &&
        !fir::isPolymorphicType(newBoxType)) {
      newDtype = fir::TypeDescOp::create(builder, loc,
                                         mlir::TypeAttr::get(newDerivedType));
    } else {
      newDtype = builder.createNullConstant(loc);
    }
    mlir::Value newAttribute = builder.createIntegerConstant(
        loc, builder.getIntegerType(8), getCFIAttribute(newBoxType));
    int lbsModifierCode =
        static_cast<int>(getLowerBoundModifier(rebox.getLbsModifier()));
    mlir::Value lowerBoundModifier = builder.createIntegerConstant(
        loc, builder.getIntegerType(32), lbsModifierCode);
    fir::runtime::genCopyAndUpdateDescriptor(builder, loc, tempDesc,
                                             rebox.getBox(), newDtype,
                                             newAttribute, lowerBoundModifier);

    mlir::Value descValue = fir::LoadOp::create(builder, loc, tempDesc);
    mlir::Value castDesc = builder.createConvert(loc, newBoxType, descValue);
    rewriter.replaceOp(rebox, castDesc);
    return mlir::success();
  }

private:
  mlir::SymbolTable *symbolTable = nullptr;
  fir::KindMapping kindMap;
};

class IsAssumedSizeConv : public mlir::OpRewritePattern<fir::IsAssumedSizeOp> {
public:
  using OpRewritePattern::OpRewritePattern;

  IsAssumedSizeConv(mlir::MLIRContext *context, mlir::SymbolTable *symbolTable,
                    fir::KindMapping kindMap)
      : mlir::OpRewritePattern<fir::IsAssumedSizeOp>(context),
        symbolTable{symbolTable}, kindMap{kindMap} {};

  toolchain::LogicalResult
  matchAndRewrite(fir::IsAssumedSizeOp isAssumedSizeOp,
                  mlir::PatternRewriter &rewriter) const override {
    fir::FirOpBuilder builder{rewriter, kindMap, symbolTable};
    mlir::Location loc = isAssumedSizeOp.getLoc();
    mlir::Value result =
        fir::runtime::genIsAssumedSize(builder, loc, isAssumedSizeOp.getVal());
    rewriter.replaceOp(isAssumedSizeOp, result);
    return mlir::success();
  }

private:
  mlir::SymbolTable *symbolTable = nullptr;
  fir::KindMapping kindMap;
};

/// Convert FIR structured control flow ops to CFG ops.
class AssumedRankOpConversion
    : public fir::impl::AssumedRankOpConversionBase<AssumedRankOpConversion> {
public:
  void runOnOperation() override {
    auto *context = &getContext();
    mlir::ModuleOp mod = getOperation();
    mlir::SymbolTable symbolTable(mod);
    fir::KindMapping kindMap = fir::getKindMapping(mod);
    mlir::RewritePatternSet patterns(context);
    patterns.insert<ReboxAssumedRankConv>(context, &symbolTable, kindMap);
    patterns.insert<IsAssumedSizeConv>(context, &symbolTable, kindMap);
    mlir::GreedyRewriteConfig config;
    config.setRegionSimplificationLevel(
        mlir::GreedySimplifyRegionLevel::Disabled);
    (void)applyPatternsGreedily(mod, std::move(patterns), config);
  }
};
} // namespace
