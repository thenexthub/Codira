//===-- Allocatable.cpp -- generate allocatable runtime API calls----------===//
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

#include "language/Compability/Optimizer/Builder/Runtime/Allocatable.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/Runtime/RTBuilder.h"
#include "language/Compability/Runtime/allocatable.h"

using namespace language::Compability::runtime;

mlir::Value fir::runtime::genMoveAlloc(fir::FirOpBuilder &builder,
                                       mlir::Location loc, mlir::Value to,
                                       mlir::Value from, mlir::Value hasStat,
                                       mlir::Value errMsg) {
  mlir::func::FuncOp func{
      fir::runtime::getRuntimeFunc<mkRTKey(MoveAlloc)>(loc, builder)};
  mlir::FunctionType fTy{func.getFunctionType()};
  mlir::Value sourceFile{fir::factory::locationToFilename(builder, loc)};
  mlir::Value sourceLine{
      fir::factory::locationToLineNo(builder, loc, fTy.getInput(6))};
  mlir::Value declaredTypeDesc;
  if (fir::isPolymorphicType(from.getType()) &&
      !fir::isUnlimitedPolymorphicType(from.getType())) {
    fir::ClassType clTy =
        mlir::dyn_cast<fir::ClassType>(fir::dyn_cast_ptrEleTy(from.getType()));
    mlir::Type derivedType = fir::unwrapInnerType(clTy.getEleTy());
    declaredTypeDesc =
        fir::TypeDescOp::create(builder, loc, mlir::TypeAttr::get(derivedType));
  } else {
    declaredTypeDesc = builder.createNullConstant(loc);
  }
  toolchain::SmallVector<mlir::Value> args{fir::runtime::createArguments(
      builder, loc, fTy, to, from, declaredTypeDesc, hasStat, errMsg,
      sourceFile, sourceLine)};

  return fir::CallOp::create(builder, loc, func, args).getResult(0);
}

void fir::runtime::genAllocatableApplyMold(fir::FirOpBuilder &builder,
                                           mlir::Location loc, mlir::Value desc,
                                           mlir::Value mold, int rank) {
  mlir::func::FuncOp func{
      fir::runtime::getRuntimeFunc<mkRTKey(AllocatableApplyMold)>(loc,
                                                                  builder)};
  mlir::FunctionType fTy = func.getFunctionType();
  mlir::Value rankVal =
      builder.createIntegerConstant(loc, fTy.getInput(2), rank);
  toolchain::SmallVector<mlir::Value> args{
      fir::runtime::createArguments(builder, loc, fTy, desc, mold, rankVal)};
  fir::CallOp::create(builder, loc, func, args);
}

void fir::runtime::genAllocatableSetBounds(fir::FirOpBuilder &builder,
                                           mlir::Location loc, mlir::Value desc,
                                           mlir::Value dimIndex,
                                           mlir::Value lowerBound,
                                           mlir::Value upperBound) {
  mlir::func::FuncOp func{
      fir::runtime::getRuntimeFunc<mkRTKey(AllocatableSetBounds)>(loc,
                                                                  builder)};
  mlir::FunctionType fTy{func.getFunctionType()};
  toolchain::SmallVector<mlir::Value> args{fir::runtime::createArguments(
      builder, loc, fTy, desc, dimIndex, lowerBound, upperBound)};
  fir::CallOp::create(builder, loc, func, args);
}

void fir::runtime::genAllocatableAllocate(fir::FirOpBuilder &builder,
                                          mlir::Location loc, mlir::Value desc,
                                          mlir::Value hasStat,
                                          mlir::Value errMsg) {
  mlir::func::FuncOp func{
      fir::runtime::getRuntimeFunc<mkRTKey(AllocatableAllocate)>(loc, builder)};
  mlir::FunctionType fTy{func.getFunctionType()};
  mlir::Value asyncObject = builder.createNullConstant(loc);
  mlir::Value sourceFile{fir::factory::locationToFilename(builder, loc)};
  mlir::Value sourceLine{
      fir::factory::locationToLineNo(builder, loc, fTy.getInput(5))};
  if (!hasStat)
    hasStat = builder.createBool(loc, false);
  if (!errMsg) {
    mlir::Type boxNoneTy = fir::BoxType::get(builder.getNoneType());
    errMsg = fir::AbsentOp::create(builder, loc, boxNoneTy).getResult();
  }
  toolchain::SmallVector<mlir::Value> args{
      fir::runtime::createArguments(builder, loc, fTy, desc, asyncObject,
                                    hasStat, errMsg, sourceFile, sourceLine)};
  fir::CallOp::create(builder, loc, func, args);
}
