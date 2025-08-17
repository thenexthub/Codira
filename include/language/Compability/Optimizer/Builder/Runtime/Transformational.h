//===-- Transformational.h --------------------------------------*- C++ -*-===//
// Generate transformational intrinsic runtime API calls.
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_BUILDER_RUNTIME_TRANSFORMATIONAL_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_BUILDER_RUNTIME_TRANSFORMATIONAL_H

#include "mlir/Dialect/Func/IR/FuncOps.h"

namespace fir {
class ExtendedValue;
class FirOpBuilder;
} // namespace fir

namespace fir::runtime {

void genBesselJn(fir::FirOpBuilder &builder, mlir::Location loc,
                 mlir::Value resultBox, mlir::Value n1, mlir::Value n2,
                 mlir::Value x, mlir::Value bn2, mlir::Value bn2_1);

void genBesselJnX0(fir::FirOpBuilder &builder, mlir::Location loc,
                   mlir::Type xTy, mlir::Value resultBox, mlir::Value n1,
                   mlir::Value n2);

void genBesselYn(fir::FirOpBuilder &builder, mlir::Location loc,
                 mlir::Value resultBox, mlir::Value n1, mlir::Value n2,
                 mlir::Value x, mlir::Value bn1, mlir::Value bn1_1);

void genBesselYnX0(fir::FirOpBuilder &builder, mlir::Location loc,
                   mlir::Type xTy, mlir::Value resultBox, mlir::Value n1,
                   mlir::Value n2);

void genCshift(fir::FirOpBuilder &builder, mlir::Location loc,
               mlir::Value resultBox, mlir::Value arrayBox,
               mlir::Value shiftBox, mlir::Value dimBox);

void genCshiftVector(fir::FirOpBuilder &builder, mlir::Location loc,
                     mlir::Value resultBox, mlir::Value arrayBox,
                     mlir::Value shiftBox);

void genEoshift(fir::FirOpBuilder &builder, mlir::Location loc,
                mlir::Value resultBox, mlir::Value arrayBox,
                mlir::Value shiftBox, mlir::Value boundBox, mlir::Value dimBox);

void genEoshiftVector(fir::FirOpBuilder &builder, mlir::Location loc,
                      mlir::Value resultBox, mlir::Value arrayBox,
                      mlir::Value shiftBox, mlir::Value boundBox);

void genMatmul(fir::FirOpBuilder &builder, mlir::Location loc,
               mlir::Value matrixABox, mlir::Value matrixBBox,
               mlir::Value resultBox);

void genMatmulTranspose(fir::FirOpBuilder &builder, mlir::Location loc,
                        mlir::Value matrixABox, mlir::Value matrixBBox,
                        mlir::Value resultBox);

void genPack(fir::FirOpBuilder &builder, mlir::Location loc,
             mlir::Value resultBox, mlir::Value arrayBox, mlir::Value maskBox,
             mlir::Value vectorBox);

void genShallowCopy(fir::FirOpBuilder &builder, mlir::Location loc,
                    mlir::Value resultBox, mlir::Value arrayBox,
                    bool resultIsAllocated);

void genReshape(fir::FirOpBuilder &builder, mlir::Location loc,
                mlir::Value resultBox, mlir::Value sourceBox,
                mlir::Value shapeBox, mlir::Value padBox, mlir::Value orderBox);

void genSpread(fir::FirOpBuilder &builder, mlir::Location loc,
               mlir::Value resultBox, mlir::Value sourceBox, mlir::Value dim,
               mlir::Value ncopies);

void genTranspose(fir::FirOpBuilder &builder, mlir::Location loc,
                  mlir::Value resultBox, mlir::Value sourceBox);

void genUnpack(fir::FirOpBuilder &builder, mlir::Location loc,
               mlir::Value resultBox, mlir::Value vectorBox,
               mlir::Value maskBox, mlir::Value fieldBox);

} // namespace fir::runtime

#endif // FORTRAN_OPTIMIZER_BUILDER_RUNTIME_TRANSFORMATIONAL_H
