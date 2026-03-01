
//===- TosaTypeConverters.cpp ---------------------------------------------===//
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
// Type converters for lowering TOSA to linalg/arith.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Tosa/Transforms/Passes.h"

#include "mlir/Transforms/DialectConversion.h"

using namespace mlir;

void mlir::tosa::populateTosaTypeConversion(TypeConverter &converter) {
  converter.addConversion([&](Type type) -> std::optional<Type> {
    if (type.isUnsignedInteger()) {
      return IntegerType::get(type.getContext(), type.getIntOrFloatBitWidth(),
                              IntegerType::SignednessSemantics::Signless);
    }
    return type;
  });
  converter.addConversion([&](TensorType type) -> std::optional<Type> {
    auto converted = converter.convertType(type.getElementType());
    if (!converted)
      return {};
    return type.clone(converted);
  });
  converter.addSourceMaterialization([&](OpBuilder &builder, Type resultType,
                                         ValueRange inputs,
                                         Location loc) -> Value {
    if (inputs.size() != 1)
      return Value();

    return UnrealizedConversionCastOp::create(builder, loc, resultType, inputs)
        .getResult(0);
  });
  converter.addTargetMaterialization([&](OpBuilder &builder, Type resultType,
                                         ValueRange inputs,
                                         Location loc) -> Value {
    if (inputs.size() != 1)
      return Value();

    return UnrealizedConversionCastOp::create(builder, loc, resultType, inputs)
        .getResult(0);
  });
}
