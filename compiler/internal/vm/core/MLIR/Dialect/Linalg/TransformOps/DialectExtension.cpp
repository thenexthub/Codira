//===- DialectExtension.cpp - Linalg transform dialect extension ----------===//
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

#include "mlir/Dialect/Linalg/TransformOps/DialectExtension.h"
#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/GPU/IR/GPUDialect.h"
#include "mlir/Dialect/Index/IR/IndexDialect.h"
#include "mlir/Dialect/Linalg/IR/Linalg.h"
#include "mlir/Dialect/Linalg/TransformOps/LinalgMatchOps.h"
#include "mlir/Dialect/Linalg/TransformOps/LinalgTransformOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/Tensor/IR/Tensor.h"
#include "mlir/Dialect/Transform/IR/TransformDialect.h"
#include "mlir/Dialect/Vector/IR/VectorOps.h"

using namespace mlir;

namespace {
/// Registers new ops and declares PDL as dependent dialect since the
/// additional ops are using PDL types for operands and results.
class LinalgTransformDialectExtension
    : public transform::TransformDialectExtension<
          LinalgTransformDialectExtension> {
public:
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(LinalgTransformDialectExtension)

  using Base::Base;

  void init() {
    declareDependentDialect<linalg::LinalgDialect>();

    declareGeneratedDialect<affine::AffineDialect>();
    declareGeneratedDialect<arith::ArithDialect>();
    declareGeneratedDialect<index::IndexDialect>();
    declareGeneratedDialect<scf::SCFDialect>();
    declareGeneratedDialect<vector::VectorDialect>();
    declareGeneratedDialect<gpu::GPUDialect>();
    declareGeneratedDialect<tensor::TensorDialect>();

    registerTransformOps<
#define GET_OP_LIST
#include "mlir/Dialect/Linalg/TransformOps/LinalgTransformOps.cpp.inc"
        >();
    registerTransformOps<
#define GET_OP_LIST
#include "mlir/Dialect/Linalg/TransformOps/LinalgMatchOps.cpp.inc"
        >();
  }
};
} // namespace

void mlir::linalg::registerTransformDialectExtension(
    DialectRegistry &registry) {
  registry.addExtensions<LinalgTransformDialectExtension>();
}
