//===-- Exceptions.cpp ----------------------------------------------------===//
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

#include "language/Compability/Optimizer/Builder/Runtime/Exceptions.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/Runtime/RTBuilder.h"
#include "language/Compability/Runtime/exceptions.h"

using namespace language::Compability::runtime;

mlir::Value fir::runtime::genMapExcept(fir::FirOpBuilder &builder,
                                       mlir::Location loc,
                                       mlir::Value excepts) {
  mlir::func::FuncOp func{
      fir::runtime::getRuntimeFunc<mkRTKey(MapException)>(loc, builder)};
  return fir::CallOp::create(builder, loc, func, excepts).getResult(0);
}

void fir::runtime::genFeclearexcept(fir::FirOpBuilder &builder,
                                    mlir::Location loc, mlir::Value excepts) {
  mlir::func::FuncOp func{
      fir::runtime::getRuntimeFunc<mkRTKey(feclearexcept)>(loc, builder)};
  fir::CallOp::create(builder, loc, func, excepts);
}

void fir::runtime::genFeraiseexcept(fir::FirOpBuilder &builder,
                                    mlir::Location loc, mlir::Value excepts) {
  mlir::func::FuncOp func{
      fir::runtime::getRuntimeFunc<mkRTKey(feraiseexcept)>(loc, builder)};
  fir::CallOp::create(builder, loc, func, excepts);
}

mlir::Value fir::runtime::genFetestexcept(fir::FirOpBuilder &builder,
                                          mlir::Location loc,
                                          mlir::Value excepts) {
  mlir::func::FuncOp func{
      fir::runtime::getRuntimeFunc<mkRTKey(fetestexcept)>(loc, builder)};
  return fir::CallOp::create(builder, loc, func, excepts).getResult(0);
}

void fir::runtime::genFedisableexcept(fir::FirOpBuilder &builder,
                                      mlir::Location loc, mlir::Value excepts) {
  mlir::func::FuncOp func{
      fir::runtime::getRuntimeFunc<mkRTKey(fedisableexcept)>(loc, builder)};
  fir::CallOp::create(builder, loc, func, excepts);
}

void fir::runtime::genFeenableexcept(fir::FirOpBuilder &builder,
                                     mlir::Location loc, mlir::Value excepts) {
  mlir::func::FuncOp func{
      fir::runtime::getRuntimeFunc<mkRTKey(feenableexcept)>(loc, builder)};
  fir::CallOp::create(builder, loc, func, excepts);
}

mlir::Value fir::runtime::genFegetexcept(fir::FirOpBuilder &builder,
                                         mlir::Location loc) {
  mlir::func::FuncOp func{
      fir::runtime::getRuntimeFunc<mkRTKey(fegetexcept)>(loc, builder)};
  return fir::CallOp::create(builder, loc, func).getResult(0);
}

mlir::Value fir::runtime::genSupportHalting(fir::FirOpBuilder &builder,
                                            mlir::Location loc,
                                            mlir::Value excepts) {
  mlir::func::FuncOp func{
      fir::runtime::getRuntimeFunc<mkRTKey(SupportHalting)>(loc, builder)};
  return fir::CallOp::create(builder, loc, func, excepts).getResult(0);
}

mlir::Value fir::runtime::genGetUnderflowMode(fir::FirOpBuilder &builder,
                                              mlir::Location loc) {
  mlir::func::FuncOp func{
      fir::runtime::getRuntimeFunc<mkRTKey(GetUnderflowMode)>(loc, builder)};
  return fir::CallOp::create(builder, loc, func).getResult(0);
}

void fir::runtime::genSetUnderflowMode(fir::FirOpBuilder &builder,
                                       mlir::Location loc, mlir::Value flag) {
  mlir::func::FuncOp func{
      fir::runtime::getRuntimeFunc<mkRTKey(SetUnderflowMode)>(loc, builder)};
  fir::CallOp::create(builder, loc, func, flag);
}

mlir::Value fir::runtime::genGetModesTypeSize(fir::FirOpBuilder &builder,
                                              mlir::Location loc) {
  mlir::func::FuncOp func{
      fir::runtime::getRuntimeFunc<mkRTKey(GetModesTypeSize)>(loc, builder)};
  return fir::CallOp::create(builder, loc, func).getResult(0);
}

mlir::Value fir::runtime::genGetStatusTypeSize(fir::FirOpBuilder &builder,
                                               mlir::Location loc) {
  mlir::func::FuncOp func{
      fir::runtime::getRuntimeFunc<mkRTKey(GetStatusTypeSize)>(loc, builder)};
  return fir::CallOp::create(builder, loc, func).getResult(0);
}
