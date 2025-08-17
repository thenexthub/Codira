//===-- Complex.h -- lowering of complex values -----------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_BUILDER_COMPLEX_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_BUILDER_COMPLEX_H

#include "language/Compability/Optimizer/Builder/FIRBuilder.h"

namespace fir::factory {

/// Helper to facilitate lowering of COMPLEX manipulations in FIR.
class Complex {
public:
  explicit Complex(FirOpBuilder &builder, mlir::Location loc)
      : builder(builder), loc(loc) {}
  Complex(const Complex &) = delete;

  // The values of part enum members are meaningful for
  // InsertValueOp and ExtractValueOp so they are explicit.
  enum class Part { Real = 0, Imag = 1 };

  /// Get the Complex Type. Determine the type. Do not create MLIR operations.
  mlir::Type getComplexPartType(mlir::Value cplx) const;
  mlir::Type getComplexPartType(mlir::Type complexType) const;

  /// Create a complex value.
  mlir::Value createComplex(mlir::Type complexType, mlir::Value real,
                            mlir::Value imag);
  /// Create a complex value given the real and imag parts real type (which
  /// must be the same).
  mlir::Value createComplex(mlir::Value real, mlir::Value imag);

  /// Returns the Real/Imag part of \p cplx
  mlir::Value extractComplexPart(mlir::Value cplx, bool isImagPart) {
    return isImagPart ? extract<Part::Imag>(cplx) : extract<Part::Real>(cplx);
  }

  /// Returns (Real, Imag) pair of \p cplx
  std::pair<mlir::Value, mlir::Value> extractParts(mlir::Value cplx) {
    return {extract<Part::Real>(cplx), extract<Part::Imag>(cplx)};
  }

  mlir::Value insertComplexPart(mlir::Value cplx, mlir::Value part,
                                bool isImagPart) {
    return isImagPart ? insert<Part::Imag>(cplx, part)
                      : insert<Part::Real>(cplx, part);
  }

protected:
  template <Part partId>
  mlir::Value extract(mlir::Value cplx) {
    return fir::ExtractValueOp::create(
        builder, loc, getComplexPartType(cplx), cplx,
        builder.getArrayAttr({builder.getIntegerAttr(
            builder.getIndexType(), static_cast<int>(partId))}));
  }

  template <Part partId>
  mlir::Value insert(mlir::Value cplx, mlir::Value part) {
    return fir::InsertValueOp::create(
        builder, loc, cplx.getType(), cplx, part,
        builder.getArrayAttr({builder.getIntegerAttr(
            builder.getIndexType(), static_cast<int>(partId))}));
  }

  template <Part partId>
  mlir::Value createPartId() {
    return builder.createIntegerConstant(loc, builder.getIndexType(),
                                         static_cast<int>(partId));
  }

private:
  FirOpBuilder &builder;
  mlir::Location loc;
};

} // namespace fir::factory

#endif // FORTRAN_OPTIMIZER_BUILDER_COMPLEX_H
