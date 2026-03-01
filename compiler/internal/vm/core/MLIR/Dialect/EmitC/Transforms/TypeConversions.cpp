//===- TypeConversions.cpp - Convert signless types into C/C++ types ------===//
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

#include "mlir/Dialect/EmitC/Transforms/TypeConversions.h"
#include "mlir/Dialect/EmitC/IR/EmitC.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/Transforms/DialectConversion.h"
#include <optional>

using namespace mlir;

namespace {

Value materializeAsUnrealizedCast(OpBuilder &builder, Type resultType,
                                  ValueRange inputs, Location loc) {
  if (inputs.size() != 1)
    return Value();

  return UnrealizedConversionCastOp::create(builder, loc, resultType, inputs)
      .getResult(0);
}

} // namespace

void mlir::populateEmitCSizeTTypeConversions(TypeConverter &converter) {
  converter.addConversion(
      [](IndexType type) { return emitc::SizeTType::get(type.getContext()); });

  converter.addSourceMaterialization(materializeAsUnrealizedCast);
  converter.addTargetMaterialization(materializeAsUnrealizedCast);
}

/// Get an unsigned integer or size data type corresponding to \p ty.
std::optional<Type> mlir::emitc::getUnsignedTypeFor(Type ty) {
  if (ty.isInteger())
    return IntegerType::get(ty.getContext(), ty.getIntOrFloatBitWidth(),
                            IntegerType::SignednessSemantics::Unsigned);
  if (isa<PtrDiffTType, SignedSizeTType>(ty))
    return SizeTType::get(ty.getContext());
  if (isa<SizeTType>(ty))
    return ty;
  return {};
}

/// Get a signed integer or size data type corresponding to \p ty that supports
/// arithmetic on negative values.
std::optional<Type> mlir::emitc::getSignedTypeFor(Type ty) {
  if (ty.isInteger())
    return IntegerType::get(ty.getContext(), ty.getIntOrFloatBitWidth(),
                            IntegerType::SignednessSemantics::Signed);
  if (isa<SizeTType, SignedSizeTType>(ty))
    return PtrDiffTType::get(ty.getContext());
  if (isa<PtrDiffTType>(ty))
    return ty;
  return {};
}
