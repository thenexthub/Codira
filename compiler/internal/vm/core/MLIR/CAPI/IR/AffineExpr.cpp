//===- AffineExpr.cpp - C API for MLIR Affine Expressions -----------------===//
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

#include "mlir-c/AffineExpr.h"
#include "mlir-c/AffineMap.h"
#include "mlir-c/IR.h"
#include "mlir/CAPI/AffineExpr.h"
#include "mlir/CAPI/AffineMap.h"
#include "mlir/CAPI/IR.h"
#include "mlir/CAPI/Utils.h"
#include "mlir/IR/AffineExpr.h"

using namespace mlir;

MlirContext mlirAffineExprGetContext(MlirAffineExpr affineExpr) {
  return wrap(unwrap(affineExpr).getContext());
}

bool mlirAffineExprEqual(MlirAffineExpr lhs, MlirAffineExpr rhs) {
  return unwrap(lhs) == unwrap(rhs);
}

void mlirAffineExprPrint(MlirAffineExpr affineExpr, MlirStringCallback callback,
                         void *userData) {
  mlir::detail::CallbackOstream stream(callback, userData);
  unwrap(affineExpr).print(stream);
}

void mlirAffineExprDump(MlirAffineExpr affineExpr) {
  unwrap(affineExpr).dump();
}

bool mlirAffineExprIsSymbolicOrConstant(MlirAffineExpr affineExpr) {
  return unwrap(affineExpr).isSymbolicOrConstant();
}

bool mlirAffineExprIsPureAffine(MlirAffineExpr affineExpr) {
  return unwrap(affineExpr).isPureAffine();
}

int64_t mlirAffineExprGetLargestKnownDivisor(MlirAffineExpr affineExpr) {
  return unwrap(affineExpr).getLargestKnownDivisor();
}

bool mlirAffineExprIsMultipleOf(MlirAffineExpr affineExpr, int64_t factor) {
  return unwrap(affineExpr).isMultipleOf(factor);
}

bool mlirAffineExprIsFunctionOfDim(MlirAffineExpr affineExpr,
                                   intptr_t position) {
  return unwrap(affineExpr).isFunctionOfDim(position);
}

MlirAffineExpr mlirAffineExprCompose(MlirAffineExpr affineExpr,
                                     MlirAffineMap affineMap) {
  return wrap(unwrap(affineExpr).compose(unwrap(affineMap)));
}

MlirAffineExpr mlirAffineExprShiftDims(MlirAffineExpr affineExpr,
                                       uint32_t numDims, uint32_t shift,
                                       uint32_t offset) {
  return wrap(unwrap(affineExpr).shiftDims(numDims, shift, offset));
}

MlirAffineExpr mlirAffineExprShiftSymbols(MlirAffineExpr affineExpr,
                                          uint32_t numSymbols, uint32_t shift,
                                          uint32_t offset) {
  return wrap(unwrap(affineExpr).shiftSymbols(numSymbols, shift, offset));
}

MlirAffineExpr mlirSimplifyAffineExpr(MlirAffineExpr expr, uint32_t numDims,
                                      uint32_t numSymbols) {
  return wrap(simplifyAffineExpr(unwrap(expr), numDims, numSymbols));
}

//===----------------------------------------------------------------------===//
// Affine Dimension Expression.
//===----------------------------------------------------------------------===//

bool mlirAffineExprIsADim(MlirAffineExpr affineExpr) {
  return isa<AffineDimExpr>(unwrap(affineExpr));
}

MlirAffineExpr mlirAffineDimExprGet(MlirContext ctx, intptr_t position) {
  return wrap(getAffineDimExpr(position, unwrap(ctx)));
}

intptr_t mlirAffineDimExprGetPosition(MlirAffineExpr affineExpr) {
  return cast<AffineDimExpr>(unwrap(affineExpr)).getPosition();
}

//===----------------------------------------------------------------------===//
// Affine Symbol Expression.
//===----------------------------------------------------------------------===//

bool mlirAffineExprIsASymbol(MlirAffineExpr affineExpr) {
  return isa<AffineSymbolExpr>(unwrap(affineExpr));
}

MlirAffineExpr mlirAffineSymbolExprGet(MlirContext ctx, intptr_t position) {
  return wrap(getAffineSymbolExpr(position, unwrap(ctx)));
}

intptr_t mlirAffineSymbolExprGetPosition(MlirAffineExpr affineExpr) {
  return cast<AffineSymbolExpr>(unwrap(affineExpr)).getPosition();
}

//===----------------------------------------------------------------------===//
// Affine Constant Expression.
//===----------------------------------------------------------------------===//

bool mlirAffineExprIsAConstant(MlirAffineExpr affineExpr) {
  return isa<AffineConstantExpr>(unwrap(affineExpr));
}

MlirAffineExpr mlirAffineConstantExprGet(MlirContext ctx, int64_t constant) {
  return wrap(getAffineConstantExpr(constant, unwrap(ctx)));
}

int64_t mlirAffineConstantExprGetValue(MlirAffineExpr affineExpr) {
  return cast<AffineConstantExpr>(unwrap(affineExpr)).getValue();
}

//===----------------------------------------------------------------------===//
// Affine Add Expression.
//===----------------------------------------------------------------------===//

bool mlirAffineExprIsAAdd(MlirAffineExpr affineExpr) {
  return unwrap(affineExpr).getKind() == mlir::AffineExprKind::Add;
}

MlirAffineExpr mlirAffineAddExprGet(MlirAffineExpr lhs, MlirAffineExpr rhs) {
  return wrap(getAffineBinaryOpExpr(mlir::AffineExprKind::Add, unwrap(lhs),
                                    unwrap(rhs)));
}

//===----------------------------------------------------------------------===//
// Affine Mul Expression.
//===----------------------------------------------------------------------===//

bool mlirAffineExprIsAMul(MlirAffineExpr affineExpr) {
  return unwrap(affineExpr).getKind() == mlir::AffineExprKind::Mul;
}

MlirAffineExpr mlirAffineMulExprGet(MlirAffineExpr lhs, MlirAffineExpr rhs) {
  return wrap(getAffineBinaryOpExpr(mlir::AffineExprKind::Mul, unwrap(lhs),
                                    unwrap(rhs)));
}

//===----------------------------------------------------------------------===//
// Affine Mod Expression.
//===----------------------------------------------------------------------===//

bool mlirAffineExprIsAMod(MlirAffineExpr affineExpr) {
  return unwrap(affineExpr).getKind() == mlir::AffineExprKind::Mod;
}

MlirAffineExpr mlirAffineModExprGet(MlirAffineExpr lhs, MlirAffineExpr rhs) {
  return wrap(getAffineBinaryOpExpr(mlir::AffineExprKind::Mod, unwrap(lhs),
                                    unwrap(rhs)));
}

//===----------------------------------------------------------------------===//
// Affine FloorDiv Expression.
//===----------------------------------------------------------------------===//

bool mlirAffineExprIsAFloorDiv(MlirAffineExpr affineExpr) {
  return unwrap(affineExpr).getKind() == mlir::AffineExprKind::FloorDiv;
}

MlirAffineExpr mlirAffineFloorDivExprGet(MlirAffineExpr lhs,
                                         MlirAffineExpr rhs) {
  return wrap(getAffineBinaryOpExpr(mlir::AffineExprKind::FloorDiv, unwrap(lhs),
                                    unwrap(rhs)));
}

//===----------------------------------------------------------------------===//
// Affine CeilDiv Expression.
//===----------------------------------------------------------------------===//

bool mlirAffineExprIsACeilDiv(MlirAffineExpr affineExpr) {
  return unwrap(affineExpr).getKind() == mlir::AffineExprKind::CeilDiv;
}

MlirAffineExpr mlirAffineCeilDivExprGet(MlirAffineExpr lhs,
                                        MlirAffineExpr rhs) {
  return wrap(getAffineBinaryOpExpr(mlir::AffineExprKind::CeilDiv, unwrap(lhs),
                                    unwrap(rhs)));
}

//===----------------------------------------------------------------------===//
// Affine Binary Operation Expression.
//===----------------------------------------------------------------------===//

bool mlirAffineExprIsABinary(MlirAffineExpr affineExpr) {
  return isa<AffineBinaryOpExpr>(unwrap(affineExpr));
}

MlirAffineExpr mlirAffineBinaryOpExprGetLHS(MlirAffineExpr affineExpr) {
  return wrap(cast<AffineBinaryOpExpr>(unwrap(affineExpr)).getLHS());
}

MlirAffineExpr mlirAffineBinaryOpExprGetRHS(MlirAffineExpr affineExpr) {
  return wrap(cast<AffineBinaryOpExpr>(unwrap(affineExpr)).getRHS());
}
