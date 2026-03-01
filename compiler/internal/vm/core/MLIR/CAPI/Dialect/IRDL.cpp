//===- IRDL.cpp - C Interface for IRDL dialect ----------------------------===//
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

#include "mlir-c/Dialect/IRDL.h"
#include "mlir/CAPI/Registration.h"
#include "mlir/Dialect/IRDL/IR/IRDL.h"
#include "mlir/Dialect/IRDL/IRDLLoading.h"

MLIR_DEFINE_CAPI_DIALECT_REGISTRATION(IRDL, irdl, mlir::irdl::IRDLDialect)

MlirLogicalResult mlirLoadIRDLDialects(MlirModule module) {
  return wrap(mlir::irdl::loadDialects(unwrap(module)));
}

//===----------------------------------------------------------------------===//
// VariadicityAttr
//===----------------------------------------------------------------------===//

MlirAttribute mlirIRDLVariadicityAttrGet(MlirContext ctx, MlirStringRef value) {
  return wrap(mlir::irdl::VariadicityAttr::get(
      unwrap(ctx), mlir::irdl::symbolizeVariadicity(unwrap(value)).value()));
}

MlirStringRef mlirIRDLVariadicityAttrGetName(void) {
  return wrap(mlir::irdl::VariadicityAttr::name);
}

//===----------------------------------------------------------------------===//
// VariadicityArrayAttr
//===----------------------------------------------------------------------===//

MlirAttribute mlirIRDLVariadicityArrayAttrGet(MlirContext ctx, intptr_t nValues,
                                              MlirAttribute const *values) {
  toolchain::SmallVector<mlir::Attribute> attrs;
  toolchain::ArrayRef<mlir::Attribute> unwrappedAttrs =
      unwrapList(nValues, values, attrs);

  toolchain::SmallVector<mlir::irdl::VariadicityAttr> variadicities;
  for (auto attr : unwrappedAttrs)
    variadicities.push_back(toolchain::cast<mlir::irdl::VariadicityAttr>(attr));

  return wrap(
      mlir::irdl::VariadicityArrayAttr::get(unwrap(ctx), variadicities));
}

MlirStringRef mlirIRDLVariadicityArrayAttrGetName(void) {
  return wrap(mlir::irdl::VariadicityArrayAttr::name);
}
