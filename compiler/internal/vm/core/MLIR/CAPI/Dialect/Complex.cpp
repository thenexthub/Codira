//===- Complex.cpp - C Interface for Complex dialect ----------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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

#include "mlir-c/Dialect/Complex.h"
#include "mlir-c/IR.h"
#include "mlir-c/Support.h"
#include "mlir/CAPI/Registration.h"
#include "mlir/Dialect/Complex/IR/Complex.h"

using namespace mlir;

MLIR_DEFINE_CAPI_DIALECT_REGISTRATION(Complex, complex,
                                      mlir::complex::ComplexDialect)

bool mlirAttributeIsAComplex(MlirAttribute attr) {
  return isa<complex::NumberAttr>(unwrap(attr));
}

MlirAttribute mlirComplexAttrDoubleGet(MlirContext ctx, MlirType type,
                                       double real, double imag) {
  return wrap(
      complex::NumberAttr::get(cast<ComplexType>(unwrap(type)), real, imag));
}

MlirAttribute mlirComplexAttrDoubleGetChecked(MlirLocation loc, MlirType type,
                                              double real, double imag) {
  return wrap(complex::NumberAttr::getChecked(
      unwrap(loc), cast<ComplexType>(unwrap(type)), real, imag));
}

double mlirComplexAttrGetRealDouble(MlirAttribute attr) {
  return cast<complex::NumberAttr>(unwrap(attr)).getReal().convertToDouble();
}

double mlirComplexAttrGetImagDouble(MlirAttribute attr) {
  return cast<complex::NumberAttr>(unwrap(attr)).getImag().convertToDouble();
}

MlirTypeID mlirComplexAttrGetTypeID(void) {
  return wrap(complex::NumberAttr::getTypeID());
}
