//===-- Command.cpp -- generate command line runtime API calls ------------===//
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

#include "language/Compability/Optimizer/Builder/Runtime/Command.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/Runtime/RTBuilder.h"
#include "language/Compability/Runtime/command.h"
#include "language/Compability/Runtime/extensions.h"

using namespace language::Compability::runtime;

// Certain runtime intrinsics should only be run when select parameters of the
// intrisic are supplied. In certain cases one of these parameters may not be
// given, however the intrinsic needs to be run due to another required
// parameter being supplied. In this case the missing parameter is assigned to
// have an "absent" value. This typically happens in IntrinsicCall.cpp. For this
// reason the extra indirection with `isAbsent` is needed for testing whether a
// given parameter is actually present (so that parameters with "value" absent
// are not considered as present).
inline bool isAbsent(mlir::Value val) {
  return mlir::isa_and_nonnull<fir::AbsentOp>(val.getDefiningOp());
}

mlir::Value fir::runtime::genCommandArgumentCount(fir::FirOpBuilder &builder,
                                                  mlir::Location loc) {
  auto argumentCountFunc =
      fir::runtime::getRuntimeFunc<mkRTKey(ArgumentCount)>(loc, builder);
  return fir::CallOp::create(builder, loc, argumentCountFunc).getResult(0);
}

mlir::Value fir::runtime::genGetCommand(fir::FirOpBuilder &builder,
                                        mlir::Location loc, mlir::Value command,
                                        mlir::Value length,
                                        mlir::Value errmsg) {
  auto runtimeFunc =
      fir::runtime::getRuntimeFunc<mkRTKey(GetCommand)>(loc, builder);
  mlir::FunctionType runtimeFuncTy = runtimeFunc.getFunctionType();
  mlir::Value sourceFile = fir::factory::locationToFilename(builder, loc);
  mlir::Value sourceLine =
      fir::factory::locationToLineNo(builder, loc, runtimeFuncTy.getInput(4));
  toolchain::SmallVector<mlir::Value> args =
      fir::runtime::createArguments(builder, loc, runtimeFuncTy, command,
                                    length, errmsg, sourceFile, sourceLine);
  return fir::CallOp::create(builder, loc, runtimeFunc, args).getResult(0);
}

mlir::Value fir::runtime::genGetPID(fir::FirOpBuilder &builder,
                                    mlir::Location loc) {
  auto runtimeFunc =
      fir::runtime::getRuntimeFunc<mkRTKey(GetPID)>(loc, builder);

  return fir::CallOp::create(builder, loc, runtimeFunc).getResult(0);
}

mlir::Value fir::runtime::genGetCommandArgument(
    fir::FirOpBuilder &builder, mlir::Location loc, mlir::Value number,
    mlir::Value value, mlir::Value length, mlir::Value errmsg) {
  auto runtimeFunc =
      fir::runtime::getRuntimeFunc<mkRTKey(GetCommandArgument)>(loc, builder);
  mlir::FunctionType runtimeFuncTy = runtimeFunc.getFunctionType();
  mlir::Value sourceFile = fir::factory::locationToFilename(builder, loc);
  mlir::Value sourceLine =
      fir::factory::locationToLineNo(builder, loc, runtimeFuncTy.getInput(5));
  toolchain::SmallVector<mlir::Value> args =
      fir::runtime::createArguments(builder, loc, runtimeFuncTy, number, value,
                                    length, errmsg, sourceFile, sourceLine);
  return fir::CallOp::create(builder, loc, runtimeFunc, args).getResult(0);
}

mlir::Value fir::runtime::genGetEnvVariable(fir::FirOpBuilder &builder,
                                            mlir::Location loc,
                                            mlir::Value name, mlir::Value value,
                                            mlir::Value length,
                                            mlir::Value trimName,
                                            mlir::Value errmsg) {
  auto runtimeFunc =
      fir::runtime::getRuntimeFunc<mkRTKey(GetEnvVariable)>(loc, builder);
  mlir::FunctionType runtimeFuncTy = runtimeFunc.getFunctionType();
  mlir::Value sourceFile = fir::factory::locationToFilename(builder, loc);
  mlir::Value sourceLine =
      fir::factory::locationToLineNo(builder, loc, runtimeFuncTy.getInput(6));
  toolchain::SmallVector<mlir::Value> args = fir::runtime::createArguments(
      builder, loc, runtimeFuncTy, name, value, length, trimName, errmsg,
      sourceFile, sourceLine);
  return fir::CallOp::create(builder, loc, runtimeFunc, args).getResult(0);
}

mlir::Value fir::runtime::genGetCwd(fir::FirOpBuilder &builder,
                                    mlir::Location loc, mlir::Value cwd) {
  mlir::func::FuncOp func =
      fir::runtime::getRuntimeFunc<mkRTKey(GetCwd)>(loc, builder);
  auto runtimeFuncTy = func.getFunctionType();
  mlir::Value sourceFile = fir::factory::locationToFilename(builder, loc);
  mlir::Value sourceLine =
      fir::factory::locationToLineNo(builder, loc, runtimeFuncTy.getInput(2));
  toolchain::SmallVector<mlir::Value> args = fir::runtime::createArguments(
      builder, loc, runtimeFuncTy, cwd, sourceFile, sourceLine);
  return fir::CallOp::create(builder, loc, func, args).getResult(0);
}

mlir::Value fir::runtime::genHostnm(fir::FirOpBuilder &builder,
                                    mlir::Location loc, mlir::Value res) {
  mlir::func::FuncOp func =
      fir::runtime::getRuntimeFunc<mkRTKey(Hostnm)>(loc, builder);
  auto runtimeFuncTy = func.getFunctionType();
  mlir::Value sourceFile = fir::factory::locationToFilename(builder, loc);
  mlir::Value sourceLine =
      fir::factory::locationToLineNo(builder, loc, runtimeFuncTy.getInput(2));
  toolchain::SmallVector<mlir::Value> args = fir::runtime::createArguments(
      builder, loc, runtimeFuncTy, res, sourceFile, sourceLine);
  return fir::CallOp::create(builder, loc, func, args).getResult(0);
}

void fir::runtime::genPerror(fir::FirOpBuilder &builder, mlir::Location loc,
                             mlir::Value string) {
  auto runtimeFunc =
      fir::runtime::getRuntimeFunc<mkRTKey(Perror)>(loc, builder);
  mlir::FunctionType runtimeFuncTy = runtimeFunc.getFunctionType();
  toolchain::SmallVector<mlir::Value> args =
      fir::runtime::createArguments(builder, loc, runtimeFuncTy, string);
  fir::CallOp::create(builder, loc, runtimeFunc, args);
}

mlir::Value fir::runtime::genPutEnv(fir::FirOpBuilder &builder,
                                    mlir::Location loc, mlir::Value str,
                                    mlir::Value strLength) {
  mlir::func::FuncOp func =
      fir::runtime::getRuntimeFunc<mkRTKey(PutEnv)>(loc, builder);
  auto runtimeFuncTy = func.getFunctionType();
  mlir::Value sourceFile = fir::factory::locationToFilename(builder, loc);
  mlir::Value sourceLine =
      fir::factory::locationToLineNo(builder, loc, runtimeFuncTy.getInput(1));
  toolchain::SmallVector<mlir::Value> args = fir::runtime::createArguments(
      builder, loc, runtimeFuncTy, str, strLength, sourceFile, sourceLine);
  return fir::CallOp::create(builder, loc, func, args).getResult(0);
}

mlir::Value fir::runtime::genUnlink(fir::FirOpBuilder &builder,
                                    mlir::Location loc, mlir::Value path,
                                    mlir::Value pathLength) {
  mlir::func::FuncOp func =
      fir::runtime::getRuntimeFunc<mkRTKey(Unlink)>(loc, builder);
  auto runtimeFuncTy = func.getFunctionType();
  mlir::Value sourceFile = fir::factory::locationToFilename(builder, loc);
  mlir::Value sourceLine =
      fir::factory::locationToLineNo(builder, loc, runtimeFuncTy.getInput(1));
  toolchain::SmallVector<mlir::Value> args = fir::runtime::createArguments(
      builder, loc, runtimeFuncTy, path, pathLength, sourceFile, sourceLine);
  return fir::CallOp::create(builder, loc, func, args).getResult(0);
}
