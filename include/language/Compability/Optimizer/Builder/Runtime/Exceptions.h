//===-- Exceptions.h --------------------------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_BUILDER_RUNTIME_EXCEPTIONS_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_BUILDER_RUNTIME_EXCEPTIONS_H

#include "mlir/IR/Value.h"

namespace mlir {
class Location;
} // namespace mlir

namespace fir {
class FirOpBuilder;
}

namespace fir::runtime {

/// Generate a runtime call to map a set of ieee_flag_type exceptions to a
/// libm fenv.h excepts value.
mlir::Value genMapExcept(fir::FirOpBuilder &builder, mlir::Location loc,
                         mlir::Value excepts);

void genFeclearexcept(fir::FirOpBuilder &builder, mlir::Location loc,
                      mlir::Value excepts);

void genFeraiseexcept(fir::FirOpBuilder &builder, mlir::Location loc,
                      mlir::Value excepts);

mlir::Value genFetestexcept(fir::FirOpBuilder &builder, mlir::Location loc,
                            mlir::Value excepts);

void genFedisableexcept(fir::FirOpBuilder &builder, mlir::Location loc,
                        mlir::Value excepts);

void genFeenableexcept(fir::FirOpBuilder &builder, mlir::Location loc,
                       mlir::Value excepts);

mlir::Value genFegetexcept(fir::FirOpBuilder &builder, mlir::Location loc);

mlir::Value genSupportHalting(fir::FirOpBuilder &builder, mlir::Location loc,
                              mlir::Value excepts);

mlir::Value genGetUnderflowMode(fir::FirOpBuilder &builder, mlir::Location loc);
void genSetUnderflowMode(fir::FirOpBuilder &builder, mlir::Location loc,
                         mlir::Value bit);

mlir::Value genGetModesTypeSize(fir::FirOpBuilder &builder, mlir::Location loc);
mlir::Value genGetStatusTypeSize(fir::FirOpBuilder &builder,
                                 mlir::Location loc);

} // namespace fir::runtime
#endif // FORTRAN_OPTIMIZER_BUILDER_RUNTIME_EXCEPTIONS_H
