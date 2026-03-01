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
//
// These BufferizableOpInterface implementations provide analysis-related
// interface methods only. They are getting bufferized by the
// SparseTensorConversion pass.

#include "mlir/Dialect/SparseTensor/Transforms/BufferizableOpInterfaceImpl.h"

#include "mlir/Dialect/Bufferization/IR/BufferizableOpInterface.h"
#include "mlir/Dialect/SparseTensor/IR/SparseTensor.h"
#include "mlir/IR/Operation.h"
#include "mlir/IR/PatternMatch.h"

using namespace mlir::bufferization;
using namespace mlir::sparse_tensor;

namespace mlir {
namespace sparse_tensor {
namespace {

template <typename ConcreteModel, typename ConcreteOp>
struct SparseBufferizableOpInterfaceExternalModel
    : public BufferizableOpInterface::ExternalModel<ConcreteModel, ConcreteOp> {
  LogicalResult bufferize(Operation *op, RewriterBase &rewriter,
                          const BufferizationOptions &options,
                          BufferizationState &state) const {
    return op->emitError(
        "sparse_tensor ops must be bufferized with the sparsifier");
  }
};

struct ConcatenateOpInterface
    : SparseBufferizableOpInterfaceExternalModel<ConcatenateOpInterface,
                                                 sparse_tensor::ConcatenateOp> {
  bool bufferizesToAllocation(Operation *op, Value value) const { return true; }

  bool bufferizesToMemoryRead(Operation *op, OpOperand &opOperand,
                              const AnalysisState &state) const {
    return true;
  }

  bool bufferizesToMemoryWrite(Operation *op, OpOperand &opOperand,
                               const AnalysisState &state) const {
    return false;
  }

  AliasingValueList getAliasingValues(Operation *op, OpOperand &opOperand,
                                      const AnalysisState &state) const {
    return {};
  }

  bool isWritable(Operation *op, Value value,
                  const AnalysisState &state) const {
    return true;
  }
};

struct ConvertOpInterface : public SparseBufferizableOpInterfaceExternalModel<
                                ConvertOpInterface, sparse_tensor::ConvertOp> {
  bool bufferizesToAllocation(Operation *op, Value value) const {
    // ConvertOps may allocate. (Unless they convert between two identical
    // types, then they fold away.)
    return true;
  }

  bool bufferizesToMemoryRead(Operation *op, OpOperand &opOperand,
                              const AnalysisState &state) const {
    return true;
  }

  bool bufferizesToMemoryWrite(Operation *op, OpOperand &opOperand,
                               const AnalysisState &state) const {
    return false;
  }

  AliasingValueList getAliasingValues(Operation *op, OpOperand &opOperand,
                                      const AnalysisState &state) const {
    return {};
  }

  bool isWritable(Operation *op, Value value,
                  const AnalysisState &state) const {
    return true;
  }
};

struct LoadOpInterface
    : public SparseBufferizableOpInterfaceExternalModel<LoadOpInterface,
                                                        sparse_tensor::LoadOp> {
  bool bufferizesToMemoryRead(Operation *op, OpOperand &opOperand,
                              const AnalysisState &state) const {
    return false;
  }

  bool bufferizesToMemoryWrite(Operation *op, OpOperand &opOperand,
                               const AnalysisState &state) const {
    return false;
  }

  AliasingValueList getAliasingValues(Operation *op, OpOperand &opOperand,
                                      const AnalysisState &state) const {
    return {{op->getOpResult(0), BufferRelation::Equivalent}};
  }
};

struct NewOpInterface
    : public SparseBufferizableOpInterfaceExternalModel<NewOpInterface,
                                                        sparse_tensor::NewOp> {
  bool resultBufferizesToMemoryWrite(Operation *op, OpResult opResult,
                                     const AnalysisState &state) const {
    // NewOps allocate but do not write.
    return false;
  }

  bool bufferizesToAllocation(Operation *op, Value value) const { return true; }
};

struct AssembleOpInterface
    : public SparseBufferizableOpInterfaceExternalModel<
          AssembleOpInterface, sparse_tensor::AssembleOp> {
  bool bufferizesToAllocation(Operation *op, Value value) const {
    // AssembleOp reuses all the buffers instead of allocating new ones
    return false;
  }

  bool bufferizesToMemoryRead(Operation *op, OpOperand &opOperand,
                              const AnalysisState &state) const {
    return true;
  }

  bool bufferizesToMemoryWrite(Operation *op, OpOperand &opOperand,
                               const AnalysisState &state) const {
    return false;
  }

  AliasingValueList getAliasingValues(Operation *op, OpOperand &opOperand,
                                      const AnalysisState &state) const {
    assert(op->getNumResults() == 1);
    // AssembleOp reuses the input tensors as values/coordinates instead of
    // creating new ones when packing into a COO format.
    return {{op->getOpResult(0), BufferRelation::Equivalent}};
  }

  BufferRelation bufferRelation(Operation *oo, OpResult opResult,
                                const AnalysisState &state) const {
    return BufferRelation::Unknown;
  }
};

struct DisassembleOpInterface
    : public SparseBufferizableOpInterfaceExternalModel<
          DisassembleOpInterface, sparse_tensor::DisassembleOp> {
  bool bufferizesToAllocation(Operation *op, Value value) const {
    // The output buffer is pre-allocated by the user.
    return false;
  }

  bool bufferizesToMemoryRead(Operation *op, OpOperand &opOperand,
                              const AnalysisState &state) const {
    // The first operand is the sparse tensor that we are unpacking.
    return opOperand.getOperandNumber() == 0;
  }

  bool bufferizesToMemoryWrite(Operation *op, OpOperand &opOperand,
                               const AnalysisState &state) const {
    // We write into the output operand.
    assert(2 * (op->getNumOperands() - 1) == op->getNumResults());
    return opOperand.getOperandNumber() > 0;
  }

  AliasingValueList getAliasingValues(Operation *op, OpOperand &opOperand,
                                      const AnalysisState &state) const {
    assert(2 * (op->getNumOperands() - 1) == op->getNumResults());

    if (opOperand.getOperandNumber() == 0)
      return {};
    // We write directly into the output tensors and returns them.
    return {{op->getResult(opOperand.getOperandNumber() - 1),
             BufferRelation::Equivalent}};
  }
};

struct ForeachOpInterface : public SparseBufferizableOpInterfaceExternalModel<
                                ForeachOpInterface, sparse_tensor::ForeachOp> {
  bool bufferizesToMemoryRead(Operation *op, OpOperand &opOperand,
                              const AnalysisState &state) const {
    return true;
  }

  bool bufferizesToMemoryWrite(Operation *op, OpOperand &opOperand,
                               const AnalysisState &state) const {
    return false;
  }

  AliasingValueList getAliasingValues(Operation *op, OpOperand &opOperand,
                                      const AnalysisState &state) const {
    return {};
  }

  LogicalResult verifyAnalysis(Operation *op,
                               const AnalysisState &state) const {
    // A more complex analysis (similar to scf.for) is needed if the op returns
    // a tensor. That tensor would have to be bufferized (not implemented yet).
    for (OpResult result : op->getResults()) {
      if (isa<TensorType>(result.getType()))
        return op->emitOpError("tensor results are not supported yet");
    }
    return success();
  }
};

struct NumberOfEntriesOpInterface
    : public SparseBufferizableOpInterfaceExternalModel<
          NumberOfEntriesOpInterface, sparse_tensor::NumberOfEntriesOp> {
  bool bufferizesToMemoryRead(Operation *op, OpOperand &opOperand,
                              const AnalysisState &state) const {
    return true;
  }

  bool bufferizesToMemoryWrite(Operation *op, OpOperand &opOperand,
                               const AnalysisState &state) const {
    return false;
  }

  AliasingValueList getAliasingValues(Operation *op, OpOperand &opOperand,
                                      const AnalysisState &state) const {
    return {};
  }
};

struct ToCoordinatesBufferOpInterface
    : public SparseBufferizableOpInterfaceExternalModel<
          ToCoordinatesBufferOpInterface,
          sparse_tensor::ToCoordinatesBufferOp> {
  bool bufferizesToMemoryRead(Operation *op, OpOperand &opOperand,
                              const AnalysisState &state) const {
    return true;
  }

  bool bufferizesToMemoryWrite(Operation *op, OpOperand &opOperand,
                               const AnalysisState &state) const {
    // Potential writes into memory through the result of
    // `sparse_tensor.coordinates` are not considered.
    return false;
  }

  AliasingValueList getAliasingValues(Operation *op, OpOperand &opOperand,
                                      const AnalysisState &state) const {
    return {};
  }
};

struct ToCoordinatesOpInterface
    : public SparseBufferizableOpInterfaceExternalModel<
          ToCoordinatesOpInterface, sparse_tensor::ToCoordinatesOp> {
  bool bufferizesToMemoryRead(Operation *op, OpOperand &opOperand,
                              const AnalysisState &state) const {
    return true;
  }

  bool bufferizesToMemoryWrite(Operation *op, OpOperand &opOperand,
                               const AnalysisState &state) const {
    // Potential writes into memory through the result of
    // `sparse_tensor.coordinates` are not considered.
    return false;
  }

  AliasingValueList getAliasingValues(Operation *op, OpOperand &opOperand,
                                      const AnalysisState &state) const {
    return {};
  }
};

struct ToPositionsOpInterface
    : public SparseBufferizableOpInterfaceExternalModel<
          ToPositionsOpInterface, sparse_tensor::ToPositionsOp> {
  bool bufferizesToMemoryRead(Operation *op, OpOperand &opOperand,
                              const AnalysisState &state) const {
    return true;
  }

  bool bufferizesToMemoryWrite(Operation *op, OpOperand &opOperand,
                               const AnalysisState &state) const {
    // Potential writes into memory through the result of
    // `sparse_tensor.positions` are not considered.
    return false;
  }

  AliasingValueList getAliasingValues(Operation *op, OpOperand &opOperand,
                                      const AnalysisState &state) const {
    return {};
  }
};

struct ToValuesOpInterface
    : public SparseBufferizableOpInterfaceExternalModel<
          ToValuesOpInterface, sparse_tensor::ToValuesOp> {
  bool bufferizesToMemoryRead(Operation *op, OpOperand &opOperand,
                              const AnalysisState &state) const {
    return true;
  }

  bool bufferizesToMemoryWrite(Operation *op, OpOperand &opOperand,
                               const AnalysisState &state) const {
    // Potential writes into memory through the result of sparse_tensor.values
    // are not considered.
    return false;
  }

  AliasingValueList getAliasingValues(Operation *op, OpOperand &opOperand,
                                      const AnalysisState &state) const {
    return {};
  }
};

} // namespace
} // namespace sparse_tensor
} // namespace mlir

void mlir::sparse_tensor::registerBufferizableOpInterfaceExternalModels(
    DialectRegistry &registry) {
  registry.addExtension(+[](MLIRContext *ctx,
                            sparse_tensor::SparseTensorDialect *dialect) {
    sparse_tensor::ConcatenateOp::attachInterface<ConcatenateOpInterface>(*ctx);
    sparse_tensor::ConvertOp::attachInterface<ConvertOpInterface>(*ctx);
    sparse_tensor::LoadOp::attachInterface<LoadOpInterface>(*ctx);
    sparse_tensor::NewOp::attachInterface<NewOpInterface>(*ctx);
    sparse_tensor::NumberOfEntriesOp::attachInterface<
        NumberOfEntriesOpInterface>(*ctx);
    sparse_tensor::AssembleOp::attachInterface<AssembleOpInterface>(*ctx);
    sparse_tensor::DisassembleOp::attachInterface<DisassembleOpInterface>(*ctx);
    sparse_tensor::ForeachOp::attachInterface<ForeachOpInterface>(*ctx);
    sparse_tensor::ToCoordinatesBufferOp::attachInterface<
        ToCoordinatesBufferOpInterface>(*ctx);
    sparse_tensor::ToCoordinatesOp::attachInterface<ToCoordinatesOpInterface>(
        *ctx);
    sparse_tensor::ToPositionsOp::attachInterface<ToPositionsOpInterface>(*ctx);
    sparse_tensor::ToValuesOp::attachInterface<ToValuesOpInterface>(*ctx);
  });
}
