//===- SPIRVGLCanonicalization.cpp - SPIR-V GLSL canonicalization patterns =//
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
// This file defines the canonicalization patterns for SPIR-V GLSL-specific ops.
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/SPIRV/IR/SPIRVGLCanonicalization.h"

#include "mlir/Dialect/SPIRV/IR/SPIRVOps.h"

using namespace mlir;

namespace {
#include "SPIRVCanonicalization.inc"
} // namespace

namespace mlir {
namespace spirv {
void populateSPIRVGLCanonicalizationPatterns(RewritePatternSet &results) {
  results.add<ConvertComparisonIntoClamp1_SPIRV_FOrdLessThanOp,
              ConvertComparisonIntoClamp1_SPIRV_FOrdLessThanEqualOp,
              ConvertComparisonIntoClamp1_SPIRV_SLessThanOp,
              ConvertComparisonIntoClamp1_SPIRV_SLessThanEqualOp,
              ConvertComparisonIntoClamp1_SPIRV_ULessThanOp,
              ConvertComparisonIntoClamp1_SPIRV_ULessThanEqualOp,
              ConvertComparisonIntoClamp2_SPIRV_FOrdLessThanOp,
              ConvertComparisonIntoClamp2_SPIRV_FOrdLessThanEqualOp,
              ConvertComparisonIntoClamp2_SPIRV_SLessThanOp,
              ConvertComparisonIntoClamp2_SPIRV_SLessThanEqualOp,
              ConvertComparisonIntoClamp2_SPIRV_ULessThanOp,
              ConvertComparisonIntoClamp2_SPIRV_ULessThanEqualOp>(
      results.getContext());
}
} // namespace spirv
} // namespace mlir
