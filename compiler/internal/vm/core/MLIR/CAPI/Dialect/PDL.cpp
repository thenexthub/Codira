//===- PDL.cpp - C Interface for PDL dialect ------------------------------===//
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

#include "mlir-c/Dialect/PDL.h"
#include "mlir/CAPI/Registration.h"
#include "mlir/Dialect/PDL/IR/PDL.h"
#include "mlir/Dialect/PDL/IR/PDLOps.h"
#include "mlir/Dialect/PDL/IR/PDLTypes.h"

using namespace mlir;

MLIR_DEFINE_CAPI_DIALECT_REGISTRATION(PDL, pdl, pdl::PDLDialect)

//===---------------------------------------------------------------------===//
// PDLType
//===---------------------------------------------------------------------===//

bool mlirTypeIsAPDLType(MlirType type) {
  return isa<pdl::PDLType>(unwrap(type));
}

//===---------------------------------------------------------------------===//
// AttributeType
//===---------------------------------------------------------------------===//

bool mlirTypeIsAPDLAttributeType(MlirType type) {
  return isa<pdl::AttributeType>(unwrap(type));
}

MlirTypeID mlirPDLAttributeTypeGetTypeID(void) {
  return wrap(pdl::AttributeType::getTypeID());
}

MlirType mlirPDLAttributeTypeGet(MlirContext ctx) {
  return wrap(pdl::AttributeType::get(unwrap(ctx)));
}

MlirStringRef mlirPDLAttributeTypeGetName(void) {
  return wrap(pdl::AttributeType::name);
}

//===---------------------------------------------------------------------===//
// OperationType
//===---------------------------------------------------------------------===//

bool mlirTypeIsAPDLOperationType(MlirType type) {
  return isa<pdl::OperationType>(unwrap(type));
}

MlirTypeID mlirPDLOperationTypeGetTypeID(void) {
  return wrap(pdl::OperationType::getTypeID());
}

MlirType mlirPDLOperationTypeGet(MlirContext ctx) {
  return wrap(pdl::OperationType::get(unwrap(ctx)));
}

MlirStringRef mlirPDLOperationTypeGetName(void) {
  return wrap(pdl::OperationType::name);
}

//===---------------------------------------------------------------------===//
// RangeType
//===---------------------------------------------------------------------===//

bool mlirTypeIsAPDLRangeType(MlirType type) {
  return isa<pdl::RangeType>(unwrap(type));
}

MlirTypeID mlirPDLRangeTypeGetTypeID(void) {
  return wrap(pdl::RangeType::getTypeID());
}

MlirType mlirPDLRangeTypeGet(MlirType elementType) {
  return wrap(pdl::RangeType::get(unwrap(elementType)));
}

MlirStringRef mlirPDLRangeTypeGetName(void) {
  return wrap(pdl::RangeType::name);
}

MlirType mlirPDLRangeTypeGetElementType(MlirType type) {
  return wrap(cast<pdl::RangeType>(unwrap(type)).getElementType());
}

//===---------------------------------------------------------------------===//
// TypeType
//===---------------------------------------------------------------------===//

bool mlirTypeIsAPDLTypeType(MlirType type) {
  return isa<pdl::TypeType>(unwrap(type));
}

MlirTypeID mlirPDLTypeTypeGetTypeID(void) {
  return wrap(pdl::TypeType::getTypeID());
}

MlirType mlirPDLTypeTypeGet(MlirContext ctx) {
  return wrap(pdl::TypeType::get(unwrap(ctx)));
}

MlirStringRef mlirPDLTypeTypeGetName(void) { return wrap(pdl::TypeType::name); }

//===---------------------------------------------------------------------===//
// ValueType
//===---------------------------------------------------------------------===//

bool mlirTypeIsAPDLValueType(MlirType type) {
  return isa<pdl::ValueType>(unwrap(type));
}

MlirTypeID mlirPDLValueTypeGetTypeID(void) {
  return wrap(pdl::ValueType::getTypeID());
}

MlirType mlirPDLValueTypeGet(MlirContext ctx) {
  return wrap(pdl::ValueType::get(unwrap(ctx)));
}

MlirStringRef mlirPDLValueTypeGetName(void) {
  return wrap(pdl::ValueType::name);
}
