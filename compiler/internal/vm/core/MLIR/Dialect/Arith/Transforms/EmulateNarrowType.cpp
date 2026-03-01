//===- EmulateNarrowType.cpp - Narrow type emulation ----*- C++
//-*-===//
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

#include "mlir/Dialect/Arith/Transforms/Passes.h"

#include "mlir/Dialect/Arith/Transforms/NarrowTypeEmulationConverter.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Func/Transforms/FuncConversions.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/TypeUtilities.h"
#include "mlir/Transforms/DialectConversion.h"
#include "vm/core/Support/MathExtras.h"
#include <cassert>

using namespace mlir;

//===----------------------------------------------------------------------===//
// Public Interface Definition
//===----------------------------------------------------------------------===//

arith::NarrowTypeEmulationConverter::NarrowTypeEmulationConverter(
    unsigned targetBitwidth)
    : loadStoreBitwidth(targetBitwidth) {
  assert(toolchain::isPowerOf2_32(targetBitwidth) &&
         "Only power-of-two integers are supported");

  // Allow unknown types.
  addConversion([](Type ty) -> std::optional<Type> { return ty; });

  // Function case.
  addConversion([this](FunctionType ty) -> std::optional<Type> {
    SmallVector<Type> inputs;
    if (failed(convertTypes(ty.getInputs(), inputs)))
      return nullptr;

    SmallVector<Type> results;
    if (failed(convertTypes(ty.getResults(), results)))
      return nullptr;

    return FunctionType::get(ty.getContext(), inputs, results);
  });
}

void arith::populateArithNarrowTypeEmulationPatterns(
    const NarrowTypeEmulationConverter &typeConverter,
    RewritePatternSet &patterns) {
  // Populate `func.*` conversion patterns.
  populateFunctionOpInterfaceTypeConversionPattern<func::FuncOp>(patterns,
                                                                 typeConverter);
  populateCallOpTypeConversionPattern(patterns, typeConverter);
  populateReturnOpTypeConversionPattern(patterns, typeConverter);
}
