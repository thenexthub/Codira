//===- Transform.cpp - C Interface for Transform dialect ------------------===//
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

#include "mlir-c/Dialect/Transform.h"
#include "mlir-c/Support.h"
#include "mlir/CAPI/Registration.h"
#include "mlir/Dialect/Transform/IR/TransformDialect.h"
#include "mlir/Dialect/Transform/IR/TransformTypes.h"

using namespace mlir;

MLIR_DEFINE_CAPI_DIALECT_REGISTRATION(Transform, transform,
                                      transform::TransformDialect)

//===---------------------------------------------------------------------===//
// AnyOpType
//===---------------------------------------------------------------------===//

bool mlirTypeIsATransformAnyOpType(MlirType type) {
  return isa<transform::AnyOpType>(unwrap(type));
}

MlirTypeID mlirTransformAnyOpTypeGetTypeID(void) {
  return wrap(transform::AnyOpType::getTypeID());
}

MlirType mlirTransformAnyOpTypeGet(MlirContext ctx) {
  return wrap(transform::AnyOpType::get(unwrap(ctx)));
}

MlirStringRef mlirTransformAnyOpTypeGetName(void) {
  return wrap(transform::AnyOpType::name);
}

//===---------------------------------------------------------------------===//
// AnyParamType
//===---------------------------------------------------------------------===//

bool mlirTypeIsATransformAnyParamType(MlirType type) {
  return isa<transform::AnyParamType>(unwrap(type));
}

MlirTypeID mlirTransformAnyParamTypeGetTypeID(void) {
  return wrap(transform::AnyParamType::getTypeID());
}

MlirType mlirTransformAnyParamTypeGet(MlirContext ctx) {
  return wrap(transform::AnyParamType::get(unwrap(ctx)));
}

MlirStringRef mlirTransformAnyParamTypeGetName(void) {
  return wrap(transform::AnyParamType::name);
}

//===---------------------------------------------------------------------===//
// AnyValueType
//===---------------------------------------------------------------------===//

bool mlirTypeIsATransformAnyValueType(MlirType type) {
  return isa<transform::AnyValueType>(unwrap(type));
}

MlirTypeID mlirTransformAnyValueTypeGetTypeID(void) {
  return wrap(transform::AnyValueType::getTypeID());
}

MlirType mlirTransformAnyValueTypeGet(MlirContext ctx) {
  return wrap(transform::AnyValueType::get(unwrap(ctx)));
}

MlirStringRef mlirTransformAnyValueTypeGetName(void) {
  return wrap(transform::AnyValueType::name);
}

//===---------------------------------------------------------------------===//
// OperationType
//===---------------------------------------------------------------------===//

bool mlirTypeIsATransformOperationType(MlirType type) {
  return isa<transform::OperationType>(unwrap(type));
}

MlirTypeID mlirTransformOperationTypeGetTypeID(void) {
  return wrap(transform::OperationType::getTypeID());
}

MlirType mlirTransformOperationTypeGet(MlirContext ctx,
                                       MlirStringRef operationName) {
  return wrap(
      transform::OperationType::get(unwrap(ctx), unwrap(operationName)));
}

MlirStringRef mlirTransformOperationTypeGetName(void) {
  return wrap(transform::OperationType::name);
}

MlirStringRef mlirTransformOperationTypeGetOperationName(MlirType type) {
  return wrap(cast<transform::OperationType>(unwrap(type)).getOperationName());
}

//===---------------------------------------------------------------------===//
// ParamType
//===---------------------------------------------------------------------===//

bool mlirTypeIsATransformParamType(MlirType type) {
  return isa<transform::ParamType>(unwrap(type));
}

MlirTypeID mlirTransformParamTypeGetTypeID(void) {
  return wrap(transform::ParamType::getTypeID());
}

MlirType mlirTransformParamTypeGet(MlirContext ctx, MlirType type) {
  return wrap(transform::ParamType::get(unwrap(ctx), unwrap(type)));
}

MlirStringRef mlirTransformParamTypeGetName(void) {
  return wrap(transform::ParamType::name);
}

MlirType mlirTransformParamTypeGetType(MlirType type) {
  return wrap(cast<transform::ParamType>(unwrap(type)).getType());
}
