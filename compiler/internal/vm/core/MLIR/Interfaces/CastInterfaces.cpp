//===- CastInterfaces.cpp -------------------------------------------------===//
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

#include "mlir/Interfaces/CastInterfaces.h"

#include "mlir/IR/BuiltinDialect.h"
#include "mlir/IR/BuiltinOps.h"

using namespace mlir;

//===----------------------------------------------------------------------===//
// Helper functions for CastOpInterface
//===----------------------------------------------------------------------===//

/// Attempt to fold the given cast operation.
LogicalResult
impl::foldCastInterfaceOp(Operation *op, ArrayRef<Attribute> attrOperands,
                          SmallVectorImpl<OpFoldResult> &foldResults) {
  OperandRange operands = op->getOperands();
  if (operands.empty())
    return failure();
  ResultRange results = op->getResults();

  // Check for the case where the input and output types match 1-1.
  if (operands.getTypes() == results.getTypes()) {
    foldResults.append(operands.begin(), operands.end());
    return success();
  }

  return failure();
}

/// Attempt to verify the given cast operation.
LogicalResult impl::verifyCastInterfaceOp(Operation *op) {
  auto resultTypes = op->getResultTypes();
  if (resultTypes.empty())
    return op->emitOpError()
           << "expected at least one result for cast operation";

  auto operandTypes = op->getOperandTypes();
  if (!cast<CastOpInterface>(op).areCastCompatible(operandTypes, resultTypes)) {
    InFlightDiagnostic diag = op->emitOpError("operand type");
    if (operandTypes.empty())
      diag << "s []";
    else if (toolchain::size(operandTypes) == 1)
      diag << " " << *operandTypes.begin();
    else
      diag << "s " << operandTypes;
    return diag << " and result type" << (resultTypes.size() == 1 ? " " : "s ")
                << resultTypes << " are cast incompatible";
  }

  return success();
}

//===----------------------------------------------------------------------===//
// External model for BuiltinDialect ops
//===----------------------------------------------------------------------===//

namespace mlir {
namespace {
// This interface cannot be implemented directly on the op because the IR build
// unit cannot depend on the Interfaces build unit.
struct UnrealizedConversionCastOpInterface
    : CastOpInterface::ExternalModel<UnrealizedConversionCastOpInterface,
                                     UnrealizedConversionCastOp> {
  static bool areCastCompatible(TypeRange inputs, TypeRange outputs) {
    // `UnrealizedConversionCastOp` is agnostic of the input/output types.
    return true;
  }
};
} // namespace
} // namespace mlir

void mlir::builtin::registerCastOpInterfaceExternalModels(
    DialectRegistry &registry) {
  registry.addExtension(+[](MLIRContext *ctx, BuiltinDialect *dialect) {
    UnrealizedConversionCastOp::attachInterface<
        UnrealizedConversionCastOpInterface>(*ctx);
  });
}

//===----------------------------------------------------------------------===//
// Table-generated class definitions
//===----------------------------------------------------------------------===//

#include "mlir/Interfaces/CastInterfaces.cpp.inc"
