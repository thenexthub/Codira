//===- SMTTypes.cpp -------------------------------------------------------===//
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

#include "mlir/Dialect/SMT/IR/SMTTypes.h"
#include "mlir/Dialect/SMT/IR/SMTDialect.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/DialectImplementation.h"
#include "vm/core/ADT/TypeSwitch.h"

using namespace mlir;
using namespace smt;
using namespace mlir;

#define GET_TYPEDEF_CLASSES
#include "mlir/Dialect/SMT/IR/SMTTypes.cpp.inc"

void SMTDialect::registerTypes() {
  addTypes<
#define GET_TYPEDEF_LIST
#include "mlir/Dialect/SMT/IR/SMTTypes.cpp.inc"
      >();
}

bool smt::isAnyNonFuncSMTValueType(Type type) {
  return isAnySMTValueType(type) && !isa<SMTFuncType>(type);
}

bool smt::isAnySMTValueType(Type type) {
  return isa<BoolType, BitVectorType, ArrayType, IntType, SortType,
             SMTFuncType>(type);
}

//===----------------------------------------------------------------------===//
// BitVectorType
//===----------------------------------------------------------------------===//

LogicalResult
BitVectorType::verify(function_ref<InFlightDiagnostic()> emitError,
                      int64_t width) {
  if (width <= 0U)
    return emitError() << "bit-vector must have at least a width of one";
  return success();
}

//===----------------------------------------------------------------------===//
// ArrayType
//===----------------------------------------------------------------------===//

LogicalResult ArrayType::verify(function_ref<InFlightDiagnostic()> emitError,
                                Type domainType, Type rangeType) {
  if (!isAnySMTValueType(domainType))
    return emitError() << "domain must be any SMT value type";
  if (!isAnySMTValueType(rangeType))
    return emitError() << "range must be any SMT value type";

  return success();
}

//===----------------------------------------------------------------------===//
// SMTFuncType
//===----------------------------------------------------------------------===//

LogicalResult SMTFuncType::verify(function_ref<InFlightDiagnostic()> emitError,
                                  ArrayRef<Type> domainTypes, Type rangeType) {
  if (domainTypes.empty())
    return emitError() << "domain must not be empty";
  if (!toolchain::all_of(domainTypes, isAnyNonFuncSMTValueType))
    return emitError() << "domain types must be any non-function SMT type";
  if (!isAnyNonFuncSMTValueType(rangeType))
    return emitError() << "range type must be any non-function SMT type";

  return success();
}

//===----------------------------------------------------------------------===//
// SortType
//===----------------------------------------------------------------------===//

LogicalResult SortType::verify(function_ref<InFlightDiagnostic()> emitError,
                               StringAttr identifier,
                               ArrayRef<Type> sortParams) {
  if (!toolchain::all_of(sortParams, isAnyNonFuncSMTValueType))
    return emitError()
           << "sort parameter types must be any non-function SMT type";

  return success();
}
