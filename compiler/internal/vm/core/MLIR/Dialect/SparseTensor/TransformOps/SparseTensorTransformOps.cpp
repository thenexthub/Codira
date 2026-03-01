//===- SparseTensorTransformOps.cpp - sparse tensor transform ops impl ----===//
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

#include "mlir/Dialect/SparseTensor/TransformOps/SparseTensorTransformOps.h"
#include "mlir/Dialect/Linalg/TransformOps/Syntax.h"
#include "mlir/Dialect/SparseTensor/IR/SparseTensor.h"

using namespace mlir;
using namespace mlir::sparse_tensor;

//===----------------------------------------------------------------------===//
// Transform op implementation
//===----------------------------------------------------------------------===//

DiagnosedSilenceableFailure transform::MatchSparseInOut::matchOperation(
    mlir::Operation *current, mlir::transform::TransformResults &results,
    mlir::transform::TransformState &state) {
  bool hasSparseInOut = hasAnySparseOperandOrResult(current);
  if (!hasSparseInOut) {
    return emitSilenceableFailure(current->getLoc(),
                                  "operation has no sparse input or output");
  }
  results.set(cast<OpResult>(getResult()), state.getPayloadOps(getTarget()));
  return DiagnosedSilenceableFailure::success();
}

//===----------------------------------------------------------------------===//
// Transform op registration
//===----------------------------------------------------------------------===//

namespace {
class SparseTensorTransformDialectExtension
    : public transform::TransformDialectExtension<
          SparseTensorTransformDialectExtension> {
public:
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(
      SparseTensorTransformDialectExtension)

  SparseTensorTransformDialectExtension() {
    declareGeneratedDialect<sparse_tensor::SparseTensorDialect>();
    registerTransformOps<
#define GET_OP_LIST
#include "mlir/Dialect/SparseTensor/TransformOps/SparseTensorTransformOps.cpp.inc"
        >();
  }
};
} // namespace

#define GET_OP_CLASSES
#include "mlir/Dialect/SparseTensor/TransformOps/SparseTensorTransformOps.cpp.inc"

void mlir::sparse_tensor::registerTransformDialectExtension(
    DialectRegistry &registry) {
  registry.addExtensions<SparseTensorTransformDialectExtension>();
}
