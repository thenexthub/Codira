//===----------------------------------------------------------------------===//
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
// Generic OpenACC lowering functions not Stmt, Decl, or clause specific.
//
//===----------------------------------------------------------------------===//

#include "CIRGenFunction.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/OpenACC/OpenACC.h"
#include "language/Core/AST/ExprCXX.h"

using namespace language::Core;
using namespace language::Core::CIRGen;

namespace {
mlir::Value createBound(CIRGenFunction &cgf, CIRGen::CIRGenBuilderTy &builder,
                        mlir::Location boundLoc, mlir::Value lowerBound,
                        mlir::Value upperBound, mlir::Value extent) {
  // Arrays always have a start-idx of 0.
  mlir::Value startIdx = cgf.createOpenACCConstantInt(boundLoc, 64, 0);
  // Stride is always 1 in C/C++.
  mlir::Value stride = cgf.createOpenACCConstantInt(boundLoc, 64, 1);

  auto bound =
      builder.create<mlir::acc::DataBoundsOp>(boundLoc, lowerBound, upperBound);
  bound.getStartIdxMutable().assign(startIdx);
  if (extent)
    bound.getExtentMutable().assign(extent);
  bound.getStrideMutable().assign(stride);

  return bound;
}
} // namespace

mlir::Value CIRGenFunction::emitOpenACCIntExpr(const Expr *intExpr) {
  mlir::Value expr = emitScalarExpr(intExpr);
  mlir::Location exprLoc = cgm.getLoc(intExpr->getBeginLoc());

  mlir::IntegerType targetType = mlir::IntegerType::get(
      &getMLIRContext(), getContext().getIntWidth(intExpr->getType()),
      intExpr->getType()->isSignedIntegerOrEnumerationType()
          ? mlir::IntegerType::SignednessSemantics::Signed
          : mlir::IntegerType::SignednessSemantics::Unsigned);

  auto conversionOp = builder.create<mlir::UnrealizedConversionCastOp>(
      exprLoc, targetType, expr);
  return conversionOp.getResult(0);
}

mlir::Value CIRGenFunction::createOpenACCConstantInt(mlir::Location loc,
                                                     unsigned width,
                                                     int64_t value) {
  mlir::IntegerType ty =
      mlir::IntegerType::get(&getMLIRContext(), width,
                             mlir::IntegerType::SignednessSemantics::Signless);
  auto constOp = builder.create<mlir::arith::ConstantOp>(
      loc, builder.getIntegerAttr(ty, value));

  return constOp.getResult();
}

CIRGenFunction::OpenACCDataOperandInfo
CIRGenFunction::getOpenACCDataOperandInfo(const Expr *e) {
  const Expr *curVarExpr = e->IgnoreParenImpCasts();

  mlir::Location exprLoc = cgm.getLoc(curVarExpr->getBeginLoc());
  toolchain::SmallVector<mlir::Value> bounds;

  std::string exprString;
  toolchain::raw_string_ostream os(exprString);
  e->printPretty(os, nullptr, getContext().getPrintingPolicy());

  while (isa<ArraySectionExpr, ArraySubscriptExpr>(curVarExpr)) {
    mlir::Location boundLoc = cgm.getLoc(curVarExpr->getBeginLoc());
    mlir::Value lowerBound;
    mlir::Value upperBound;
    mlir::Value extent;

    if (const auto *section = dyn_cast<ArraySectionExpr>(curVarExpr)) {
      if (const Expr *lb = section->getLowerBound())
        lowerBound = emitOpenACCIntExpr(lb);
      else
        lowerBound = createOpenACCConstantInt(boundLoc, 64, 0);

      if (const Expr *len = section->getLength()) {
        extent = emitOpenACCIntExpr(len);
      } else {
        QualType baseTy = ArraySectionExpr::getBaseOriginalType(
            section->getBase()->IgnoreParenImpCasts());
        // We know this is the case as implicit lengths are only allowed for
        // array types with a constant size, or a dependent size.  AND since
        // we are codegen we know we're not dependent.
        auto *arrayTy = getContext().getAsConstantArrayType(baseTy);
        // Rather than trying to calculate the extent based on the
        // lower-bound, we can just emit this as an upper bound.
        upperBound = createOpenACCConstantInt(boundLoc, 64,
                                              arrayTy->getLimitedSize() - 1);
      }

      curVarExpr = section->getBase()->IgnoreParenImpCasts();
    } else {
      const auto *subscript = cast<ArraySubscriptExpr>(curVarExpr);

      lowerBound = emitOpenACCIntExpr(subscript->getIdx());
      // Length of an array index is always 1.
      extent = createOpenACCConstantInt(boundLoc, 64, 1);
      curVarExpr = subscript->getBase()->IgnoreParenImpCasts();
    }

    bounds.push_back(createBound(*this, this->builder, boundLoc, lowerBound,
                                 upperBound, extent));
  }

  if (const auto *memExpr = dyn_cast<MemberExpr>(curVarExpr))
    return {exprLoc, emitMemberExpr(memExpr).getPointer(), exprString,
            curVarExpr->getType().getNonReferenceType().getUnqualifiedType(),
            std::move(bounds)};

  // Sema has made sure that only 4 types of things can get here, array
  // subscript, array section, member expr, or DRE to a var decl (or the
  // former 3 wrapping a var-decl), so we should be able to assume this is
  // right.
  const auto *dre = cast<DeclRefExpr>(curVarExpr);
  return {exprLoc, emitDeclRefLValue(dre).getPointer(), exprString,
          curVarExpr->getType().getNonReferenceType().getUnqualifiedType(),
          std::move(bounds)};
}
