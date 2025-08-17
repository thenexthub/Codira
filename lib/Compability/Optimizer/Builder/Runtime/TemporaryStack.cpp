//===- TemporaryStack.cpp ---- temporary stack runtime API calls ----------===//
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

#include "language/Compability/Optimizer/Builder/Runtime/TemporaryStack.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/Runtime/RTBuilder.h"
#include "language/Compability/Runtime/temporary-stack.h"

using namespace language::Compability::runtime;

mlir::Value fir::runtime::genCreateValueStack(mlir::Location loc,
                                              fir::FirOpBuilder &builder) {
  mlir::func::FuncOp func =
      fir::runtime::getRuntimeFunc<mkRTKey(CreateValueStack)>(loc, builder);
  mlir::FunctionType funcType = func.getFunctionType();
  mlir::Value sourceFile = fir::factory::locationToFilename(builder, loc);
  mlir::Value sourceLine =
      fir::factory::locationToLineNo(builder, loc, funcType.getInput(1));
  auto args = fir::runtime::createArguments(builder, loc, funcType, sourceFile,
                                            sourceLine);
  return fir::CallOp::create(builder, loc, func, args).getResult(0);
}

void fir::runtime::genPushValue(mlir::Location loc, fir::FirOpBuilder &builder,
                                mlir::Value opaquePtr, mlir::Value boxValue) {
  mlir::func::FuncOp func =
      fir::runtime::getRuntimeFunc<mkRTKey(PushValue)>(loc, builder);
  mlir::FunctionType funcType = func.getFunctionType();
  auto args = fir::runtime::createArguments(builder, loc, funcType, opaquePtr,
                                            boxValue);
  fir::CallOp::create(builder, loc, func, args);
}

void fir::runtime::genValueAt(mlir::Location loc, fir::FirOpBuilder &builder,
                              mlir::Value opaquePtr, mlir::Value i,
                              mlir::Value retValueBox) {
  mlir::func::FuncOp func =
      fir::runtime::getRuntimeFunc<mkRTKey(ValueAt)>(loc, builder);
  mlir::FunctionType funcType = func.getFunctionType();
  auto args = fir::runtime::createArguments(builder, loc, funcType, opaquePtr,
                                            i, retValueBox);
  fir::CallOp::create(builder, loc, func, args);
}

void fir::runtime::genDestroyValueStack(mlir::Location loc,
                                        fir::FirOpBuilder &builder,
                                        mlir::Value opaquePtr) {
  mlir::func::FuncOp func =
      fir::runtime::getRuntimeFunc<mkRTKey(DestroyValueStack)>(loc, builder);
  mlir::FunctionType funcType = func.getFunctionType();
  auto args = fir::runtime::createArguments(builder, loc, funcType, opaquePtr);
  fir::CallOp::create(builder, loc, func, args);
}

mlir::Value fir::runtime::genCreateDescriptorStack(mlir::Location loc,
                                                   fir::FirOpBuilder &builder) {
  mlir::func::FuncOp func =
      fir::runtime::getRuntimeFunc<mkRTKey(CreateDescriptorStack)>(loc,
                                                                   builder);
  mlir::FunctionType funcType = func.getFunctionType();
  mlir::Value sourceFile = fir::factory::locationToFilename(builder, loc);
  mlir::Value sourceLine =
      fir::factory::locationToLineNo(builder, loc, funcType.getInput(1));
  auto args = fir::runtime::createArguments(builder, loc, funcType, sourceFile,
                                            sourceLine);
  return fir::CallOp::create(builder, loc, func, args).getResult(0);
}

void fir::runtime::genPushDescriptor(mlir::Location loc,
                                     fir::FirOpBuilder &builder,
                                     mlir::Value opaquePtr,
                                     mlir::Value boxDescriptor) {
  mlir::func::FuncOp func =
      fir::runtime::getRuntimeFunc<mkRTKey(PushDescriptor)>(loc, builder);
  mlir::FunctionType funcType = func.getFunctionType();
  auto args = fir::runtime::createArguments(builder, loc, funcType, opaquePtr,
                                            boxDescriptor);
  fir::CallOp::create(builder, loc, func, args);
}

void fir::runtime::genDescriptorAt(mlir::Location loc,
                                   fir::FirOpBuilder &builder,
                                   mlir::Value opaquePtr, mlir::Value i,
                                   mlir::Value retDescriptorBox) {
  mlir::func::FuncOp func =
      fir::runtime::getRuntimeFunc<mkRTKey(DescriptorAt)>(loc, builder);
  mlir::FunctionType funcType = func.getFunctionType();
  auto args = fir::runtime::createArguments(builder, loc, funcType, opaquePtr,
                                            i, retDescriptorBox);
  fir::CallOp::create(builder, loc, func, args);
}

void fir::runtime::genDestroyDescriptorStack(mlir::Location loc,
                                             fir::FirOpBuilder &builder,
                                             mlir::Value opaquePtr) {
  mlir::func::FuncOp func =
      fir::runtime::getRuntimeFunc<mkRTKey(DestroyDescriptorStack)>(loc,
                                                                    builder);
  mlir::FunctionType funcType = func.getFunctionType();
  auto args = fir::runtime::createArguments(builder, loc, funcType, opaquePtr);
  fir::CallOp::create(builder, loc, func, args);
}
