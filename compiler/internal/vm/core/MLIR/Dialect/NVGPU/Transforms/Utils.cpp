//===- Utils.cpp - Transform utilities ------------------------------------===//
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

#include "mlir/Dialect/NVGPU/Transforms/Utils.h"

#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/NVGPU/IR/NVGPUDialect.h"
#include "mlir/Dialect/Vector/IR/VectorOps.h"

using namespace mlir;
using namespace mlir::nvgpu;

Operation::operand_range nvgpu::getIndices(Operation *op) {
  if (auto ldmatrixOp = dyn_cast<LdMatrixOp>(op))
    return ldmatrixOp.getIndices();
  if (auto copyOp = dyn_cast<DeviceAsyncCopyOp>(op))
    return copyOp.getDstIndices();
  if (auto loadOp = dyn_cast<memref::LoadOp>(op))
    return loadOp.getIndices();
  if (auto storeOp = dyn_cast<memref::StoreOp>(op))
    return storeOp.getIndices();
  if (auto vectorReadOp = dyn_cast<vector::LoadOp>(op))
    return vectorReadOp.getIndices();
  if (auto vectorStoreOp = dyn_cast<vector::StoreOp>(op))
    return vectorStoreOp.getIndices();
  if (auto transferReadOp = dyn_cast<vector::TransferReadOp>(op))
    return transferReadOp.getIndices();
  if (auto transferWriteOp = dyn_cast<vector::TransferWriteOp>(op))
    return transferWriteOp.getIndices();
  llvm_unreachable("unsupported op type");
}

void nvgpu::setIndices(Operation *op, ArrayRef<Value> indices) {
  if (auto ldmatrixOp = dyn_cast<LdMatrixOp>(op))
    return ldmatrixOp.getIndicesMutable().assign(indices);
  if (auto copyOp = dyn_cast<DeviceAsyncCopyOp>(op))
    return copyOp.getDstIndicesMutable().assign(indices);
  if (auto loadOp = dyn_cast<memref::LoadOp>(op))
    return loadOp.getIndicesMutable().assign(indices);
  if (auto storeOp = dyn_cast<memref::StoreOp>(op))
    return storeOp.getIndicesMutable().assign(indices);
  if (auto vectorReadOp = dyn_cast<vector::LoadOp>(op))
    return vectorReadOp.getIndicesMutable().assign(indices);
  if (auto vectorStoreOp = dyn_cast<vector::StoreOp>(op))
    return vectorStoreOp.getIndicesMutable().assign(indices);
  if (auto transferReadOp = dyn_cast<vector::TransferReadOp>(op))
    return transferReadOp.getIndicesMutable().assign(indices);
  if (auto transferWriteOp = dyn_cast<vector::TransferWriteOp>(op))
    return transferWriteOp.getIndicesMutable().assign(indices);
  llvm_unreachable("unsupported op type");
}

Value nvgpu::getValueStored(Operation *op) {
  if (auto storeOp = dyn_cast<memref::StoreOp>(op))
    return storeOp.getValueToStore();
  if (auto transferWrite = dyn_cast<vector::TransferWriteOp>(op))
    return transferWrite.getValue();
  if (auto storeOp = dyn_cast<vector::StoreOp>(op))
    return storeOp.getValueToStore();
  llvm_unreachable("unsupported op type");
}

Value nvgpu::getMemrefOperand(Operation *op) {
  if (auto loadOp = dyn_cast<memref::LoadOp>(op))
    return loadOp.getMemref();
  if (auto storeOp = dyn_cast<memref::StoreOp>(op))
    return storeOp.getMemref();
  if (auto transferWrite = dyn_cast<vector::TransferWriteOp>(op))
    return transferWrite.getBase();
  if (auto transferRead = dyn_cast<vector::TransferReadOp>(op))
    return transferRead.getBase();
  if (auto storeOp = dyn_cast<vector::StoreOp>(op))
    return storeOp.getBase();
  if (auto loadOp = dyn_cast<vector::LoadOp>(op))
    return loadOp.getBase();
  llvm_unreachable("unsupported op type");
}
