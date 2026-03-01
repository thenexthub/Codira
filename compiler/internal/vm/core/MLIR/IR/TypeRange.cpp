//===- TypeRange.cpp ------------------------------------------------------===//
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

#include "mlir/IR/TypeRange.h"

using namespace mlir;

//===----------------------------------------------------------------------===//
// TypeRange
//===----------------------------------------------------------------------===//

TypeRange::TypeRange(ArrayRef<Type> types)
    : TypeRange(types.data(), types.size()) {
  assert(toolchain::all_of(types, [](Type t) { return t; }) &&
         "attempting to construct a TypeRange with null types");
}
TypeRange::TypeRange(OperandRange values)
    : TypeRange(values.begin().getBase(), values.size()) {}
TypeRange::TypeRange(ResultRange values)
    : TypeRange(values.getBase(), values.size()) {}
TypeRange::TypeRange(ValueRange values) : TypeRange(OwnerT(), values.size()) {
  if (count == 0)
    return;
  ValueRange::OwnerT owner = values.begin().getBase();
  if (auto *result = toolchain::dyn_cast_if_present<detail::OpResultImpl *>(owner))
    this->base = result;
  else if (auto *operand = toolchain::dyn_cast_if_present<OpOperand *>(owner))
    this->base = operand;
  else
    this->base = cast<const Value *>(owner);
}

/// See `toolchain::detail::indexed_accessor_range_base` for details.
TypeRange::OwnerT TypeRange::offset_base(OwnerT object, ptrdiff_t index) {
  if (const auto *value = toolchain::dyn_cast_if_present<const Value *>(object))
    return {value + index};
  if (auto *operand = toolchain::dyn_cast_if_present<OpOperand *>(object))
    return {operand + index};
  if (auto *result = toolchain::dyn_cast_if_present<detail::OpResultImpl *>(object))
    return {result->getNextResultAtOffset(index)};
  return {toolchain::dyn_cast_if_present<const Type *>(object) + index};
}

/// See `toolchain::detail::indexed_accessor_range_base` for details.
Type TypeRange::dereference_iterator(OwnerT object, ptrdiff_t index) {
  if (const auto *value = toolchain::dyn_cast_if_present<const Value *>(object))
    return (value + index)->getType();
  if (auto *operand = toolchain::dyn_cast_if_present<OpOperand *>(object))
    return (operand + index)->get().getType();
  if (auto *result = toolchain::dyn_cast_if_present<detail::OpResultImpl *>(object))
    return result->getNextResultAtOffset(index)->getType();
  return toolchain::dyn_cast_if_present<const Type *>(object)[index];
}
