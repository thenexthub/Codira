//===- X86VectorTransformOps.cpp ------------------------------------------===//
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

#include "mlir/Dialect/X86Vector/TransformOps/X86VectorTransformOps.h"
#include "mlir/Conversion/LLVMCommon/TypeConverter.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/Transform/IR/TransformDialect.h"
#include "mlir/Dialect/Transform/Interfaces/TransformInterfaces.h"
#include "mlir/Dialect/Vector/IR/VectorOps.h"
#include "mlir/Dialect/X86Vector/Transforms.h"
#include "mlir/Dialect/X86Vector/X86VectorDialect.h"

#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/RegionKindInterface.h"

using namespace mlir;
using namespace mlir::x86vector;
using namespace mlir::transform;

void mlir::transform::ApplyVectorContractToFMAPatternsOp::populatePatterns(
    RewritePatternSet &patterns) {
  x86vector::populateVectorContractToFMAPatterns(patterns);
}

void mlir::transform::ApplyVectorContractToPackedTypeDotProductPatternsOp::
    populatePatterns(RewritePatternSet &patterns) {
  x86vector::populateVectorContractToPackedTypeDotProductPatterns(patterns);
}

void mlir::transform::ApplyVectorContractBF16ToFMAPatternsOp::populatePatterns(
    RewritePatternSet &patterns) {
  x86vector::populateVectorContractBF16ToFMAPatterns(patterns);
}

void mlir::transform::ApplySinkVectorProducerOpsPatternsOp::populatePatterns(
    RewritePatternSet &patterns) {
  x86vector::populateSinkVectorProducerOpsPatterns(patterns);
}

//===----------------------------------------------------------------------===//
// Transform op registration
//===----------------------------------------------------------------------===//

namespace {
class X86VectorTransformDialectExtension
    : public transform::TransformDialectExtension<
          X86VectorTransformDialectExtension> {
public:
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(
      X86VectorTransformDialectExtension)

  X86VectorTransformDialectExtension() {
    declareGeneratedDialect<x86vector::X86VectorDialect>();
    declareGeneratedDialect<LLVM::LLVMDialect>();
    registerTransformOps<
#define GET_OP_LIST
#include "mlir/Dialect/X86Vector/TransformOps/X86VectorTransformOps.cpp.inc"
        >();
  }
};
} // namespace

#define GET_OP_CLASSES
#include "mlir/Dialect/X86Vector/TransformOps/X86VectorTransformOps.cpp.inc"

void mlir::x86vector::registerTransformDialectExtension(
    DialectRegistry &registry) {
  registry.addExtensions<X86VectorTransformDialectExtension>();
}
