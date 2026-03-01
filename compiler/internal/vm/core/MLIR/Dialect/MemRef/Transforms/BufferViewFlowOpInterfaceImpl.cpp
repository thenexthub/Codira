//===- BufferViewFlowOpInterfaceImpl.cpp - Buffer View Flow Analysis ------===//
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

#include "mlir/Dialect/MemRef/Transforms/BufferViewFlowOpInterfaceImpl.h"

#include "mlir/Dialect/Bufferization/IR/BufferViewFlowOpInterface.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"

using namespace mlir;
using namespace mlir::bufferization;

namespace mlir {
namespace memref {
namespace {

struct ReallocOpInterface
    : public BufferViewFlowOpInterface::ExternalModel<ReallocOpInterface,
                                                      ReallocOp> {
  void populateDependencies(
      Operation *op,
      const RegisterDependenciesFn &registerDependenciesFn) const {
    auto reallocOp = cast<ReallocOp>(op);
    // memref.realloc may return the source operand.
    registerDependenciesFn(reallocOp.getSource(), reallocOp.getResult());
  }

  bool mayBeTerminalBuffer(Operation *op, Value value) const {
    // The return value of memref.realloc is a terminal buffer because the op
    // may return a newly allocated buffer.
    return true;
  }
};

} // namespace
} // namespace memref
} // namespace mlir

void memref::registerBufferViewFlowOpInterfaceExternalModels(
    DialectRegistry &registry) {
  registry.addExtension(+[](MLIRContext *ctx, memref::MemRefDialect *dialect) {
    ReallocOp::attachInterface<ReallocOpInterface>(*ctx);
  });
}
