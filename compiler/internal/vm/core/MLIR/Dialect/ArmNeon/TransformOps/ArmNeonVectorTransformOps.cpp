//===- ArmNeonVectorTransformOps.cpp - Implementation transform ops -------===//
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

#include "mlir/Dialect/ArmNeon/TransformOps/ArmNeonVectorTransformOps.h"

#include "mlir/Dialect/ArmNeon/ArmNeonDialect.h"
#include "mlir/Dialect/ArmNeon/Transforms.h"
#include "mlir/Dialect/Transform/IR/TransformDialect.h"

using namespace mlir;

//===----------------------------------------------------------------------===//
// Apply...PatternsOp
//===----------------------------------------------------------------------===//

void transform::ApplyArmNeonContractionToI8MMPatternsOp::populatePatterns(
    RewritePatternSet &patterns) {
  arm_neon::populateLowerContractionToNeonI8MMPatterns(patterns);
}

void transform::ApplyArmNeonContractionToBFMMLAPatternsOp::populatePatterns(
    RewritePatternSet &patterns) {
  arm_neon::populateLowerContractionToNeonBFMMLAPatterns(patterns);
}

//===----------------------------------------------------------------------===//
// Transform op registration
//===----------------------------------------------------------------------===//

namespace {
class ArmNeonVectorTransformDialectExtension
    : public transform::TransformDialectExtension<
          ArmNeonVectorTransformDialectExtension> {
public:
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(
      ArmNeonVectorTransformDialectExtension)

  ArmNeonVectorTransformDialectExtension() {
    declareGeneratedDialect<arm_neon::ArmNeonDialect>();
    registerTransformOps<
#define GET_OP_LIST
#include "mlir/Dialect/ArmNeon/TransformOps/ArmNeonVectorTransformOps.cpp.inc"
        >();
  }
};
} // namespace

#define GET_OP_CLASSES
#include "mlir/Dialect/ArmNeon/TransformOps/ArmNeonVectorTransformOps.cpp.inc"

void mlir::arm_neon::registerTransformDialectExtension(
    DialectRegistry &registry) {
  registry.addExtensions<ArmNeonVectorTransformDialectExtension>();
}
