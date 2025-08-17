
//===-- Allocatable.cpp -- Allocatable statements lowering ----------------===//
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
//
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Optimizer/Builder/Runtime/CUDA/Descriptor.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"
#include "language/Compability/Optimizer/Builder/Runtime/RTBuilder.h"
#include "language/Compability/Runtime/CUDA/descriptor.h"

using namespace language::Compability::runtime::cuda;

void fir::runtime::cuda::genSyncGlobalDescriptor(fir::FirOpBuilder &builder,
                                                 mlir::Location loc,
                                                 mlir::Value hostPtr) {
  mlir::func::FuncOp callee =
      fir::runtime::getRuntimeFunc<mkRTKey(CUFSyncGlobalDescriptor)>(loc,
                                                                     builder);
  auto fTy = callee.getFunctionType();
  mlir::Value sourceFile = fir::factory::locationToFilename(builder, loc);
  mlir::Value sourceLine =
      fir::factory::locationToLineNo(builder, loc, fTy.getInput(2));
  toolchain::SmallVector<mlir::Value> args{fir::runtime::createArguments(
      builder, loc, fTy, hostPtr, sourceFile, sourceLine)};
  fir::CallOp::create(builder, loc, callee, args);
}

void fir::runtime::cuda::genDescriptorCheckSection(fir::FirOpBuilder &builder,
                                                   mlir::Location loc,
                                                   mlir::Value desc) {
  mlir::func::FuncOp func =
      fir::runtime::getRuntimeFunc<mkRTKey(CUFDescriptorCheckSection)>(loc,
                                                                       builder);
  auto fTy = func.getFunctionType();
  mlir::Value sourceFile = fir::factory::locationToFilename(builder, loc);
  mlir::Value sourceLine =
      fir::factory::locationToLineNo(builder, loc, fTy.getInput(2));
  toolchain::SmallVector<mlir::Value> args{fir::runtime::createArguments(
      builder, loc, fTy, desc, sourceFile, sourceLine)};
  fir::CallOp::create(builder, loc, func, args);
}

void fir::runtime::cuda::genSetAllocatorIndex(fir::FirOpBuilder &builder,
                                              mlir::Location loc,
                                              mlir::Value desc,
                                              mlir::Value index) {
  mlir::func::FuncOp func =
      fir::runtime::getRuntimeFunc<mkRTKey(CUFSetAllocatorIndex)>(loc, builder);
  auto fTy = func.getFunctionType();
  mlir::Value sourceFile = fir::factory::locationToFilename(builder, loc);
  mlir::Value sourceLine =
      fir::factory::locationToLineNo(builder, loc, fTy.getInput(3));
  toolchain::SmallVector<mlir::Value> args{fir::runtime::createArguments(
      builder, loc, fTy, desc, index, sourceFile, sourceLine)};
  fir::CallOp::create(builder, loc, func, args);
}
