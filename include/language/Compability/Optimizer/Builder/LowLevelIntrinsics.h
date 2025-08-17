//===-- LowLevelIntrinsics.h ------------------------------------*- C++ -*-===//
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

#ifndef FLANG_OPTIMIZER_BUILDER_LOWLEVELINTRINSICS_H
#define FLANG_OPTIMIZER_BUILDER_LOWLEVELINTRINSICS_H

namespace mlir {
namespace func {
class FuncOp;
} // namespace func
} // namespace mlir
namespace fir {
class FirOpBuilder;
}

namespace fir::factory {

/// Get the C standard library `realloc` function.
mlir::func::FuncOp getRealloc(FirOpBuilder &builder);

/// Get the `toolchain.get.rounding` intrinsic.
mlir::func::FuncOp getLlvmGetRounding(FirOpBuilder &builder);

/// Get the `toolchain.set.rounding` intrinsic.
mlir::func::FuncOp getLlvmSetRounding(FirOpBuilder &builder);

/// Get the `toolchain.init.trampoline` intrinsic.
mlir::func::FuncOp getLlvmInitTrampoline(FirOpBuilder &builder);

/// Get the `toolchain.adjust.trampoline` intrinsic.
mlir::func::FuncOp getLlvmAdjustTrampoline(FirOpBuilder &builder);

/// Get the libm (fenv.h) `feclearexcept` function.
mlir::func::FuncOp getFeclearexcept(FirOpBuilder &builder);

/// Get the libm (fenv.h) `fedisableexcept` function.
mlir::func::FuncOp getFedisableexcept(FirOpBuilder &builder);

/// Get the libm (fenv.h) `feenableexcept` function.
mlir::func::FuncOp getFeenableexcept(FirOpBuilder &builder);

/// Get the libm (fenv.h) `fegetexcept` function.
mlir::func::FuncOp getFegetexcept(FirOpBuilder &builder);

/// Get the libm (fenv.h) `feraiseexcept` function.
mlir::func::FuncOp getFeraiseexcept(FirOpBuilder &builder);

/// Get the libm (fenv.h) `fetestexcept` function.
mlir::func::FuncOp getFetestexcept(FirOpBuilder &builder);

} // namespace fir::factory

#endif // FLANG_OPTIMIZER_BUILDER_LOWLEVELINTRINSICS_H
