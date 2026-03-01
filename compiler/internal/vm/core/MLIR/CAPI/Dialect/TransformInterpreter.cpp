//===- TransformTransforms.cpp - C Interface for Transform dialect --------===//
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
//
// C interface to transforms for the transform dialect.
//
//===----------------------------------------------------------------------===//

#include "mlir-c/Dialect/Transform/Interpreter.h"
#include "mlir-c/Support.h"
#include "mlir/CAPI/IR.h"
#include "mlir/CAPI/Support.h"
#include "mlir/CAPI/Wrap.h"
#include "mlir/Dialect/Transform/IR/Utils.h"
#include "mlir/Dialect/Transform/Interfaces/TransformInterfaces.h"
#include "mlir/Dialect/Transform/Transforms/TransformInterpreterUtils.h"

using namespace mlir;

DEFINE_C_API_PTR_METHODS(MlirTransformOptions, transform::TransformOptions)

extern "C" {

MlirTransformOptions mlirTransformOptionsCreate() {
  return wrap(new transform::TransformOptions);
}

void mlirTransformOptionsEnableExpensiveChecks(
    MlirTransformOptions transformOptions, bool enable) {
  unwrap(transformOptions)->enableExpensiveChecks(enable);
}

bool mlirTransformOptionsGetExpensiveChecksEnabled(
    MlirTransformOptions transformOptions) {
  return unwrap(transformOptions)->getExpensiveChecksEnabled();
}

void mlirTransformOptionsEnforceSingleTopLevelTransformOp(
    MlirTransformOptions transformOptions, bool enable) {
  unwrap(transformOptions)->enableEnforceSingleToplevelTransformOp(enable);
}

bool mlirTransformOptionsGetEnforceSingleTopLevelTransformOp(
    MlirTransformOptions transformOptions) {
  return unwrap(transformOptions)->getEnforceSingleToplevelTransformOp();
}

void mlirTransformOptionsDestroy(MlirTransformOptions transformOptions) {
  delete unwrap(transformOptions);
}

MlirLogicalResult mlirTransformApplyNamedSequence(
    MlirOperation payload, MlirOperation transformRoot,
    MlirOperation transformModule, MlirTransformOptions transformOptions) {
  Operation *transformRootOp = unwrap(transformRoot);
  Operation *transformModuleOp = unwrap(transformModule);
  if (!isa<transform::TransformOpInterface>(transformRootOp)) {
    transformRootOp->emitError()
        << "must implement TransformOpInterface to be used as transform root";
    return mlirLogicalResultFailure();
  }
  if (!isa<ModuleOp>(transformModuleOp)) {
    transformModuleOp->emitError()
        << "must be a " << ModuleOp::getOperationName();
    return mlirLogicalResultFailure();
  }
  return wrap(transform::applyTransformNamedSequence(
      unwrap(payload), unwrap(transformRoot),
      cast<ModuleOp>(unwrap(transformModule)), *unwrap(transformOptions)));
}

MlirLogicalResult mlirMergeSymbolsIntoFromClone(MlirOperation target,
                                                MlirOperation other) {
  OwningOpRef<Operation *> otherOwning(unwrap(other)->clone());
  LogicalResult result = transform::detail::mergeSymbolsInto(
      unwrap(target), std::move(otherOwning));
  return wrap(result);
}
}
