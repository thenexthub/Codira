//===-- Derived.cpp -- derived type runtime API ---------------------------===//
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

#include "language/Compability/Optimizer/Builder/Runtime/Derived.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/Runtime/RTBuilder.h"
#include "language/Compability/Optimizer/Support/FatalError.h"
#include "language/Compability/Optimizer/Support/InternalNames.h"
#include "language/Compability/Runtime/derived-api.h"
#include "language/Compability/Runtime/pointer.h"

using namespace language::Compability::runtime;

void fir::runtime::genDerivedTypeInitialize(fir::FirOpBuilder &builder,
                                            mlir::Location loc,
                                            mlir::Value box) {
  auto func = fir::runtime::getRuntimeFunc<mkRTKey(Initialize)>(loc, builder);
  auto fTy = func.getFunctionType();
  auto sourceFile = fir::factory::locationToFilename(builder, loc);
  auto sourceLine =
      fir::factory::locationToLineNo(builder, loc, fTy.getInput(2));
  auto args = fir::runtime::createArguments(builder, loc, fTy, box, sourceFile,
                                            sourceLine);
  fir::CallOp::create(builder, loc, func, args);
}

void fir::runtime::genDerivedTypeInitializeClone(fir::FirOpBuilder &builder,
                                                 mlir::Location loc,
                                                 mlir::Value newBox,
                                                 mlir::Value box) {
  auto func =
      fir::runtime::getRuntimeFunc<mkRTKey(InitializeClone)>(loc, builder);
  auto fTy = func.getFunctionType();
  auto sourceFile = fir::factory::locationToFilename(builder, loc);
  auto sourceLine =
      fir::factory::locationToLineNo(builder, loc, fTy.getInput(3));
  auto args = fir::runtime::createArguments(builder, loc, fTy, newBox, box,
                                            sourceFile, sourceLine);
  fir::CallOp::create(builder, loc, func, args);
}

void fir::runtime::genDerivedTypeDestroy(fir::FirOpBuilder &builder,
                                         mlir::Location loc, mlir::Value box) {
  auto func = fir::runtime::getRuntimeFunc<mkRTKey(Destroy)>(loc, builder);
  auto fTy = func.getFunctionType();
  auto args = fir::runtime::createArguments(builder, loc, fTy, box);
  fir::CallOp::create(builder, loc, func, args);
}

void fir::runtime::genDerivedTypeFinalize(fir::FirOpBuilder &builder,
                                          mlir::Location loc, mlir::Value box) {
  auto func = fir::runtime::getRuntimeFunc<mkRTKey(Finalize)>(loc, builder);
  auto fTy = func.getFunctionType();
  auto sourceFile = fir::factory::locationToFilename(builder, loc);
  auto sourceLine =
      fir::factory::locationToLineNo(builder, loc, fTy.getInput(2));
  auto args = fir::runtime::createArguments(builder, loc, fTy, box, sourceFile,
                                            sourceLine);
  fir::CallOp::create(builder, loc, func, args);
}

void fir::runtime::genDerivedTypeDestroyWithoutFinalization(
    fir::FirOpBuilder &builder, mlir::Location loc, mlir::Value box) {
  auto func = fir::runtime::getRuntimeFunc<mkRTKey(DestroyWithoutFinalization)>(
      loc, builder);
  auto fTy = func.getFunctionType();
  auto args = fir::runtime::createArguments(builder, loc, fTy, box);
  fir::CallOp::create(builder, loc, func, args);
}

void fir::runtime::genNullifyDerivedType(fir::FirOpBuilder &builder,
                                         mlir::Location loc, mlir::Value box,
                                         fir::RecordType derivedType,
                                         unsigned rank) {
  mlir::Value typeDesc =
      fir::TypeDescOp::create(builder, loc, mlir::TypeAttr::get(derivedType));
  mlir::func::FuncOp callee =
      fir::runtime::getRuntimeFunc<mkRTKey(PointerNullifyDerived)>(loc,
                                                                   builder);
  toolchain::ArrayRef<mlir::Type> inputTypes = callee.getFunctionType().getInputs();
  toolchain::SmallVector<mlir::Value> args;
  args.push_back(builder.createConvert(loc, inputTypes[0], box));
  args.push_back(builder.createConvert(loc, inputTypes[1], typeDesc));
  mlir::Value rankCst = builder.createIntegerConstant(loc, inputTypes[2], rank);
  mlir::Value c0 = builder.createIntegerConstant(loc, inputTypes[3], 0);
  args.push_back(rankCst);
  args.push_back(c0);
  fir::CallOp::create(builder, loc, callee, args);
}

mlir::Value fir::runtime::genSameTypeAs(fir::FirOpBuilder &builder,
                                        mlir::Location loc, mlir::Value a,
                                        mlir::Value b) {
  mlir::func::FuncOp sameTypeAsFunc =
      fir::runtime::getRuntimeFunc<mkRTKey(SameTypeAs)>(loc, builder);
  auto fTy = sameTypeAsFunc.getFunctionType();
  auto args = fir::runtime::createArguments(builder, loc, fTy, a, b);
  return fir::CallOp::create(builder, loc, sameTypeAsFunc, args).getResult(0);
}

mlir::Value fir::runtime::genExtendsTypeOf(fir::FirOpBuilder &builder,
                                           mlir::Location loc, mlir::Value a,
                                           mlir::Value mold) {
  mlir::func::FuncOp extendsTypeOfFunc =
      fir::runtime::getRuntimeFunc<mkRTKey(ExtendsTypeOf)>(loc, builder);
  auto fTy = extendsTypeOfFunc.getFunctionType();
  auto args = fir::runtime::createArguments(builder, loc, fTy, a, mold);
  return fir::CallOp::create(builder, loc, extendsTypeOfFunc, args)
      .getResult(0);
}
