//===-- LowLevelIntrinsics.cpp --------------------------------------------===//
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
//
// Low level intrinsic functions.
//
// These include LLVM intrinsic calls and standard C library calls.
// Target-specific calls, such as OS functions, should be factored in other
// file(s).
//
//===----------------------------------------------------------------------===//

#include "language/Compability/Optimizer/Builder/LowLevelIntrinsics.h"
#include "language/Compability/Optimizer/Builder/FIRBuilder.h"

mlir::func::FuncOp fir::factory::getRealloc(fir::FirOpBuilder &builder) {
  auto ptrTy = builder.getRefType(builder.getIntegerType(8));
  toolchain::SmallVector<mlir::Type> args = {ptrTy, builder.getI64Type()};
  auto reallocTy = mlir::FunctionType::get(builder.getContext(), args, {ptrTy});
  return builder.createFunction(builder.getUnknownLoc(), "realloc", reallocTy);
}

mlir::func::FuncOp
fir::factory::getLlvmGetRounding(fir::FirOpBuilder &builder) {
  auto int32Ty = builder.getIntegerType(32);
  auto funcTy = mlir::FunctionType::get(builder.getContext(), {}, {int32Ty});
  return builder.createFunction(builder.getUnknownLoc(), "toolchain.get.rounding",
                                funcTy);
}

mlir::func::FuncOp
fir::factory::getLlvmSetRounding(fir::FirOpBuilder &builder) {
  auto int32Ty = builder.getIntegerType(32);
  auto funcTy = mlir::FunctionType::get(builder.getContext(), {int32Ty}, {});
  return builder.createFunction(builder.getUnknownLoc(), "toolchain.set.rounding",
                                funcTy);
}

mlir::func::FuncOp
fir::factory::getLlvmInitTrampoline(fir::FirOpBuilder &builder) {
  auto ptrTy = builder.getRefType(builder.getIntegerType(8));
  auto funcTy =
      mlir::FunctionType::get(builder.getContext(), {ptrTy, ptrTy, ptrTy}, {});
  return builder.createFunction(builder.getUnknownLoc(), "toolchain.init.trampoline",
                                funcTy);
}

mlir::func::FuncOp
fir::factory::getLlvmAdjustTrampoline(fir::FirOpBuilder &builder) {
  auto ptrTy = builder.getRefType(builder.getIntegerType(8));
  auto funcTy = mlir::FunctionType::get(builder.getContext(), {ptrTy}, {ptrTy});
  return builder.createFunction(builder.getUnknownLoc(),
                                "toolchain.adjust.trampoline", funcTy);
}

mlir::func::FuncOp fir::factory::getFeclearexcept(fir::FirOpBuilder &builder) {
  auto int32Ty = builder.getIntegerType(32);
  auto funcTy =
      mlir::FunctionType::get(builder.getContext(), {int32Ty}, {int32Ty});
  return builder.createFunction(builder.getUnknownLoc(), "feclearexcept",
                                funcTy);
}

mlir::func::FuncOp
fir::factory::getFedisableexcept(fir::FirOpBuilder &builder) {
  auto int32Ty = builder.getIntegerType(32);
  auto funcTy =
      mlir::FunctionType::get(builder.getContext(), {int32Ty}, {int32Ty});
  return builder.createFunction(builder.getUnknownLoc(), "fedisableexcept",
                                funcTy);
}

mlir::func::FuncOp fir::factory::getFeenableexcept(fir::FirOpBuilder &builder) {
  auto int32Ty = builder.getIntegerType(32);
  auto funcTy =
      mlir::FunctionType::get(builder.getContext(), {int32Ty}, {int32Ty});
  return builder.createFunction(builder.getUnknownLoc(), "feenableexcept",
                                funcTy);
}

mlir::func::FuncOp fir::factory::getFegetexcept(fir::FirOpBuilder &builder) {
  auto int32Ty = builder.getIntegerType(32);
  auto funcTy = mlir::FunctionType::get(builder.getContext(), {}, {int32Ty});
  return builder.createFunction(builder.getUnknownLoc(), "fegetexcept", funcTy);
}

mlir::func::FuncOp fir::factory::getFeraiseexcept(fir::FirOpBuilder &builder) {
  auto int32Ty = builder.getIntegerType(32);
  auto funcTy =
      mlir::FunctionType::get(builder.getContext(), {int32Ty}, {int32Ty});
  return builder.createFunction(builder.getUnknownLoc(), "feraiseexcept",
                                funcTy);
}

mlir::func::FuncOp fir::factory::getFetestexcept(fir::FirOpBuilder &builder) {
  auto int32Ty = builder.getIntegerType(32);
  auto funcTy =
      mlir::FunctionType::get(builder.getContext(), {int32Ty}, {int32Ty});
  return builder.createFunction(builder.getUnknownLoc(), "fetestexcept",
                                funcTy);
}
