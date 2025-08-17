//===-- Execute.cpp -- generate command line runtime API calls ------------===//
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

#include "language/Compability/Optimizer/Builder/Runtime/Execute.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/Runtime/RTBuilder.h"
#include "language/Compability/Runtime/execute.h"

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

void fir::runtime::genExecuteCommandLine(fir::FirOpBuilder &builder,
                                         mlir::Location loc,
                                         mlir::Value command, mlir::Value wait,
                                         mlir::Value exitstat,
                                         mlir::Value cmdstat,
                                         mlir::Value cmdmsg) {
  auto runtimeFunc =
      fir::runtime::getRuntimeFunc<mkRTKey(ExecuteCommandLine)>(loc, builder);
  mlir::FunctionType runtimeFuncTy = runtimeFunc.getFunctionType();
  mlir::Value sourceFile = fir::factory::locationToFilename(builder, loc);
  mlir::Value sourceLine =
      fir::factory::locationToLineNo(builder, loc, runtimeFuncTy.getInput(6));
  toolchain::SmallVector<mlir::Value> args = fir::runtime::createArguments(
      builder, loc, runtimeFuncTy, command, wait, exitstat, cmdstat, cmdmsg,
      sourceFile, sourceLine);
  fir::CallOp::create(builder, loc, runtimeFunc, args);
}
