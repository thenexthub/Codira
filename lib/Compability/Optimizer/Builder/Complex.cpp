//===-- Complex.cpp -------------------------------------------------------===//
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

#include "language/Compability/Optimizer/Builder/Complex.h"

//===----------------------------------------------------------------------===//
// Complex Factory implementation
//===----------------------------------------------------------------------===//

mlir::Type
fir::factory::Complex::getComplexPartType(mlir::Type complexType) const {
  return mlir::cast<mlir::ComplexType>(complexType).getElementType();
}

mlir::Type fir::factory::Complex::getComplexPartType(mlir::Value cplx) const {
  return getComplexPartType(cplx.getType());
}

mlir::Value fir::factory::Complex::createComplex(mlir::Type cplxTy,
                                                 mlir::Value real,
                                                 mlir::Value imag) {
  mlir::Value und = fir::UndefOp::create(builder, loc, cplxTy);
  return insert<Part::Imag>(insert<Part::Real>(und, real), imag);
}

mlir::Value fir::factory::Complex::createComplex(mlir::Value real,
                                                 mlir::Value imag) {
  assert(real.getType() == imag.getType() && "part types must match");
  mlir::Type cplxTy = mlir::ComplexType::get(real.getType());
  return createComplex(cplxTy, real, imag);
}
