//===-- Main.cpp - generate main runtime API calls --------------*- C++ -*-===//
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

#include "language/Compability/Optimizer/Builder/Runtime/Main.h"
#include "language/Compability/Lower/EnvironmentDefault.h"
#include "language/Compability/Optimizer/Builder/BoxValue.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/Runtime/EnvironmentDefaults.h"
#include "language/Compability/Optimizer/Builder/Runtime/RTBuilder.h"
#include "language/Compability/Optimizer/Dialect/FIROps.h"
#include "language/Compability/Optimizer/Dialect/FIRType.h"
#include "language/Compability/Runtime/CUDA/init.h"
#include "language/Compability/Runtime/main.h"
#include "language/Compability/Runtime/stop.h"

using namespace language::Compability::runtime;

/// Create a `int main(...)` that calls the Fortran entry point
void fir::runtime::genMain(
    fir::FirOpBuilder &builder, mlir::Location loc,
    const std::vector<language::Compability::lower::EnvironmentDefault> &defs,
    bool initCuda) {
  auto *context = builder.getContext();
  auto argcTy = builder.getDefaultIntegerType();
  auto ptrTy = mlir::LLVM::LLVMPointerType::get(context);

  // void ProgramStart(int argc, char** argv, char** envp,
  //                   _QQEnvironmentDefaults* env)
  auto startFn = builder.createFunction(
      loc, RTNAME_STRING(ProgramStart),
      mlir::FunctionType::get(context, {argcTy, ptrTy, ptrTy, ptrTy}, {}));
  // void ProgramStop()
  auto stopFn =
      builder.createFunction(loc, RTNAME_STRING(ProgramEndStatement),
                             mlir::FunctionType::get(context, {}, {}));

  // int main(int argc, char** argv, char** envp)
  auto mainFn = builder.createFunction(
      loc, "main",
      mlir::FunctionType::get(context, {argcTy, ptrTy, ptrTy}, argcTy));
  // void _QQmain()
  auto qqMainFn = builder.createFunction(
      loc, "_QQmain", mlir::FunctionType::get(context, {}, {}));

  mainFn.setPublic();

  auto *block = mainFn.addEntryBlock();
  mlir::OpBuilder::InsertionGuard insertGuard(builder);
  builder.setInsertionPointToStart(block);

  // Create the list of any environment defaults for the runtime to set. The
  // runtime default list is only created if there is a main program to ensure
  // it only happens once and to provide consistent results if multiple files
  // are compiled separately.
  auto env = fir::runtime::genEnvironmentDefaults(builder, loc, defs);

  toolchain::SmallVector<mlir::Value, 4> args(block->getArguments());
  args.push_back(env);

  fir::CallOp::create(builder, loc, startFn, args);

  if (initCuda) {
    auto initFn = builder.createFunction(
        loc, RTNAME_STRING(CUFInit), mlir::FunctionType::get(context, {}, {}));
    fir::CallOp::create(builder, loc, initFn);
  }

  fir::CallOp::create(builder, loc, qqMainFn);
  fir::CallOp::create(builder, loc, stopFn);

  mlir::Value ret = builder.createIntegerConstant(loc, argcTy, 0);
  mlir::func::ReturnOp::create(builder, loc, ret);
}
