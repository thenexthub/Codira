//===----------------------------------------------------------------------===//
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
// Implementation of external dialect interfaces for CIR.
//
//===----------------------------------------------------------------------===//

#include "language/Core/CIR/Dialect/OpenACC/CIROpenACCTypeInterfaces.h"
#include "language/Core/CIR/Dialect/IR/CIRDialect.h"
#include "language/Core/CIR/Dialect/IR/CIRTypes.h"

namespace cir::acc {

mlir::Type getBaseType(mlir::Value varPtr) {
  mlir::Operation *op = varPtr.getDefiningOp();
  assert(op && "Expected a defining operation");

  // This is the variable definition we're looking for.
  if (auto allocaOp = mlir::dyn_cast<cir::AllocaOp>(*op))
    return allocaOp.getAllocaType();

  // Look through casts to the source pointer.
  if (auto castOp = mlir::dyn_cast<cir::CastOp>(*op))
    return getBaseType(castOp.getSrc());

  // Follow the source of ptr strides.
  if (auto ptrStrideOp = mlir::dyn_cast<cir::PtrStrideOp>(*op))
    return getBaseType(ptrStrideOp.getBase());

  if (auto getMemberOp = mlir::dyn_cast<cir::GetMemberOp>(*op))
    return getBaseType(getMemberOp.getAddr());

  return mlir::cast<cir::PointerType>(varPtr.getType()).getPointee();
}

template <>
mlir::acc::VariableTypeCategory
OpenACCPointerLikeModel<cir::PointerType>::getPointeeTypeCategory(
    mlir::Type pointer, mlir::TypedValue<mlir::acc::PointerLikeType> varPtr,
    mlir::Type varType) const {
  mlir::Type eleTy = getBaseType(varPtr);

  if (auto mappableTy = mlir::dyn_cast<mlir::acc::MappableType>(eleTy))
    return mappableTy.getTypeCategory(varPtr);

  if (isAnyIntegerOrFloatingPointType(eleTy) ||
      mlir::isa<cir::BoolType>(eleTy) || mlir::isa<cir::PointerType>(eleTy))
    return mlir::acc::VariableTypeCategory::scalar;
  if (mlir::isa<cir::ArrayType>(eleTy))
    return mlir::acc::VariableTypeCategory::array;
  if (mlir::isa<cir::RecordType>(eleTy))
    return mlir::acc::VariableTypeCategory::composite;
  if (mlir::isa<cir::FuncType>(eleTy) || mlir::isa<cir::VectorType>(eleTy))
    return mlir::acc::VariableTypeCategory::nonscalar;

  // Without further checking, this type cannot be categorized.
  return mlir::acc::VariableTypeCategory::uncategorized;
}

} // namespace cir::acc
