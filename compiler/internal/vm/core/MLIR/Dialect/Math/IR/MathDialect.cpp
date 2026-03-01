//===- MathDialect.cpp - MLIR dialect for Math implementation -------------===//
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

#include "mlir/Conversion/ConvertToLLVM/ToLLVMInterface.h"
#include "mlir/Dialect/Math/IR/Math.h"
#include "mlir/Transforms/InliningUtils.h"

using namespace mlir;
using namespace mlir::math;

#include "mlir/Dialect/Math/IR/MathOpsDialect.cpp.inc"

namespace {
/// This class defines the interface for handling inlining with math
/// operations.
struct MathInlinerInterface : public DialectInlinerInterface {
  using DialectInlinerInterface::DialectInlinerInterface;

  /// All operations within math ops can be inlined.
  bool isLegalToInline(Operation *, Region *, bool, IRMapping &) const final {
    return true;
  }
};
} // namespace

void mlir::math::MathDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "mlir/Dialect/Math/IR/MathOps.cpp.inc"
      >();
  addInterfaces<MathInlinerInterface>();
  declarePromisedInterface<ConvertToLLVMPatternInterface, MathDialect>();
}
