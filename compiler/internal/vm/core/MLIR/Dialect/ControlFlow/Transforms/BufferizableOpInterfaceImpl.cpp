//===- BufferizableOpInterfaceImpl.cpp - Impl. of BufferizableOpInterface -===//
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

#include "mlir/Dialect/ControlFlow/Transforms/BufferizableOpInterfaceImpl.h"

#include "mlir/Dialect/Bufferization/IR/UnstructuredControlFlow.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/IR/Operation.h"

using namespace mlir;
using namespace mlir::bufferization;

namespace mlir {
namespace cf {
namespace {

template <typename ConcreteModel, typename ConcreteOp>
struct BranchLikeOpInterface
    : public BranchOpBufferizableOpInterfaceExternalModel<ConcreteModel,
                                                          ConcreteOp> {
  bool bufferizesToMemoryRead(Operation *op, OpOperand &opOperand,
                              const AnalysisState &state) const {
    return false;
  }

  bool bufferizesToMemoryWrite(Operation *op, OpOperand &opOperand,
                               const AnalysisState &state) const {
    return false;
  }

  LogicalResult verifyAnalysis(Operation *op,
                               const AnalysisState &state) const {
    return success();
  }

  LogicalResult bufferize(Operation *op, RewriterBase &rewriter,
                          const BufferizationOptions &options,
                          BufferizationState &state) const {
    // The operands of this op are bufferized together with the block signature.
    return success();
  }
};

/// Bufferization of cf.br.
struct BranchOpInterface
    : public BranchLikeOpInterface<BranchOpInterface, cf::BranchOp> {};

/// Bufferization of cf.cond_br.
struct CondBranchOpInterface
    : public BranchLikeOpInterface<CondBranchOpInterface, cf::CondBranchOp> {};

} // namespace
} // namespace cf
} // namespace mlir

void mlir::cf::registerBufferizableOpInterfaceExternalModels(
    DialectRegistry &registry) {
  registry.addExtension(+[](MLIRContext *ctx, cf::ControlFlowDialect *dialect) {
    cf::BranchOp::attachInterface<BranchOpInterface>(*ctx);
    cf::CondBranchOp::attachInterface<CondBranchOpInterface>(*ctx);
  });
}
