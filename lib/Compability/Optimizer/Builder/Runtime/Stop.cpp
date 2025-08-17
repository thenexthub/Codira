//===-- Stop.h - generate stop runtime API calls ----------------*- C++ -*-===//
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

#include "language/Compability/Optimizer/Builder/Runtime/Stop.h"
#include "language/Compability/Optimizer/Builder/BoxValue.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/Runtime/RTBuilder.h"
#include "language/Compability/Runtime/stop.h"

using namespace language::Compability::runtime;

void fir::runtime::genExit(fir::FirOpBuilder &builder, mlir::Location loc,
                           mlir::Value status) {
  auto exitFunc = fir::runtime::getRuntimeFunc<mkRTKey(Exit)>(loc, builder);
  toolchain::SmallVector<mlir::Value> args = fir::runtime::createArguments(
      builder, loc, exitFunc.getFunctionType(), status);
  fir::CallOp::create(builder, loc, exitFunc, args);
}

void fir::runtime::genAbort(fir::FirOpBuilder &builder, mlir::Location loc) {
  mlir::func::FuncOp abortFunc =
      fir::runtime::getRuntimeFunc<mkRTKey(Abort)>(loc, builder);
  fir::CallOp::create(builder, loc, abortFunc, mlir::ValueRange{});
}

void fir::runtime::genReportFatalUserError(fir::FirOpBuilder &builder,
                                           mlir::Location loc,
                                           toolchain::StringRef message) {
  mlir::func::FuncOp crashFunc =
      fir::runtime::getRuntimeFunc<mkRTKey(ReportFatalUserError)>(loc, builder);
  mlir::FunctionType funcTy = crashFunc.getFunctionType();
  mlir::Value msgVal = fir::getBase(
      fir::factory::createStringLiteral(builder, loc, message.str() + '\0'));
  mlir::Value sourceLine =
      fir::factory::locationToLineNo(builder, loc, funcTy.getInput(2));
  mlir::Value sourceFile = fir::factory::locationToFilename(builder, loc);
  toolchain::SmallVector<mlir::Value> args = fir::runtime::createArguments(
      builder, loc, funcTy, msgVal, sourceFile, sourceLine);
  fir::CallOp::create(builder, loc, crashFunc, args);
}
