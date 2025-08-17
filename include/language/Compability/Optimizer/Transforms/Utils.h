//===-- Optimizer/Transforms/Utils.h ----------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_TRANSFORMS_UTILS_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_TRANSFORMS_UTILS_H

namespace fir {

using MinlocBodyOpGeneratorTy = toolchain::function_ref<mlir::Value(
    fir::FirOpBuilder &, mlir::Location, const mlir::Type &, mlir::Value,
    mlir::Value, mlir::Value, const toolchain::SmallVectorImpl<mlir::Value> &)>;
using InitValGeneratorTy = toolchain::function_ref<mlir::Value(
    fir::FirOpBuilder &, mlir::Location, const mlir::Type &)>;
using AddrGeneratorTy = toolchain::function_ref<mlir::Value(
    fir::FirOpBuilder &, mlir::Location, const mlir::Type &, mlir::Value,
    mlir::Value)>;

// Produces a loop nest for a Minloc intrinsic.
void genMinMaxlocReductionLoop(fir::FirOpBuilder &builder, mlir::Value array,
                               fir::InitValGeneratorTy initVal,
                               fir::MinlocBodyOpGeneratorTy genBody,
                               fir::AddrGeneratorTy getAddrFn, unsigned rank,
                               mlir::Type elementType, mlir::Location loc,
                               mlir::Type maskElemType, mlir::Value resultArr,
                               bool maskMayBeLogicalScalar);

} // namespace fir

#endif // FORTRAN_OPTIMIZER_TRANSFORMS_UTILS_H
