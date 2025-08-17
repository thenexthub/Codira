//===-- Assign.cpp -- generate assignment runtime API calls ---------------===//
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

#include "language/Compability/Optimizer/Builder/Runtime/Assign.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/Runtime/RTBuilder.h"
#include "language/Compability/Runtime/assign.h"

using namespace language::Compability::runtime;

void fir::runtime::genAssign(fir::FirOpBuilder &builder, mlir::Location loc,
                             mlir::Value destBox, mlir::Value sourceBox) {
  auto func = fir::runtime::getRuntimeFunc<mkRTKey(Assign)>(loc, builder);
  auto fTy = func.getFunctionType();
  auto sourceFile = fir::factory::locationToFilename(builder, loc);
  auto sourceLine =
      fir::factory::locationToLineNo(builder, loc, fTy.getInput(3));
  auto args = fir::runtime::createArguments(builder, loc, fTy, destBox,
                                            sourceBox, sourceFile, sourceLine);
  fir::CallOp::create(builder, loc, func, args);
}

void fir::runtime::genAssignPolymorphic(fir::FirOpBuilder &builder,
                                        mlir::Location loc, mlir::Value destBox,
                                        mlir::Value sourceBox) {
  auto func =
      fir::runtime::getRuntimeFunc<mkRTKey(AssignPolymorphic)>(loc, builder);
  auto fTy = func.getFunctionType();
  auto sourceFile = fir::factory::locationToFilename(builder, loc);
  auto sourceLine =
      fir::factory::locationToLineNo(builder, loc, fTy.getInput(3));
  auto args = fir::runtime::createArguments(builder, loc, fTy, destBox,
                                            sourceBox, sourceFile, sourceLine);
  fir::CallOp::create(builder, loc, func, args);
}

void fir::runtime::genAssignExplicitLengthCharacter(fir::FirOpBuilder &builder,
                                                    mlir::Location loc,
                                                    mlir::Value destBox,
                                                    mlir::Value sourceBox) {
  auto func =
      fir::runtime::getRuntimeFunc<mkRTKey(AssignExplicitLengthCharacter)>(
          loc, builder);
  auto fTy = func.getFunctionType();
  auto sourceFile = fir::factory::locationToFilename(builder, loc);
  auto sourceLine =
      fir::factory::locationToLineNo(builder, loc, fTy.getInput(3));
  auto args = fir::runtime::createArguments(builder, loc, fTy, destBox,
                                            sourceBox, sourceFile, sourceLine);
  fir::CallOp::create(builder, loc, func, args);
}

void fir::runtime::genAssignTemporary(fir::FirOpBuilder &builder,
                                      mlir::Location loc, mlir::Value destBox,
                                      mlir::Value sourceBox) {
  auto func =
      fir::runtime::getRuntimeFunc<mkRTKey(AssignTemporary)>(loc, builder);
  auto fTy = func.getFunctionType();
  auto sourceFile = fir::factory::locationToFilename(builder, loc);
  auto sourceLine =
      fir::factory::locationToLineNo(builder, loc, fTy.getInput(3));
  auto args = fir::runtime::createArguments(builder, loc, fTy, destBox,
                                            sourceBox, sourceFile, sourceLine);
  fir::CallOp::create(builder, loc, func, args);
}

void fir::runtime::genCopyInAssign(fir::FirOpBuilder &builder,
                                   mlir::Location loc, mlir::Value destBox,
                                   mlir::Value sourceBox) {
  auto func = fir::runtime::getRuntimeFunc<mkRTKey(CopyInAssign)>(loc, builder);
  auto fTy = func.getFunctionType();
  auto sourceFile = fir::factory::locationToFilename(builder, loc);
  auto sourceLine =
      fir::factory::locationToLineNo(builder, loc, fTy.getInput(3));
  auto args = fir::runtime::createArguments(builder, loc, fTy, destBox,
                                            sourceBox, sourceFile, sourceLine);
  fir::CallOp::create(builder, loc, func, args);
}

void fir::runtime::genCopyOutAssign(fir::FirOpBuilder &builder,
                                    mlir::Location loc, mlir::Value destBox,
                                    mlir::Value sourceBox) {
  auto func =
      fir::runtime::getRuntimeFunc<mkRTKey(CopyOutAssign)>(loc, builder);
  auto fTy = func.getFunctionType();
  auto sourceFile = fir::factory::locationToFilename(builder, loc);
  auto sourceLine =
      fir::factory::locationToLineNo(builder, loc, fTy.getInput(3));
  auto args = fir::runtime::createArguments(builder, loc, fTy, destBox,
                                            sourceBox, sourceFile, sourceLine);
  fir::CallOp::create(builder, loc, func, args);
}
