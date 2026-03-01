//===- BufferizationPipelines.cpp - Pipelines for bufferization -----------===//
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

#include "mlir/Dialect/Bufferization/Pipelines/Passes.h"

#include "mlir/Dialect/Bufferization/Transforms/Passes.h"
#include "mlir/Dialect/MemRef/Transforms/Passes.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Transforms/Passes.h"

//===----------------------------------------------------------------------===//
// Pipeline implementation.
//===----------------------------------------------------------------------===//

void mlir::bufferization::buildBufferDeallocationPipeline(OpPassManager &pm) {
  buildBufferDeallocationPipeline(pm, BufferDeallocationPipelineOptions());
}

void mlir::bufferization::buildBufferDeallocationPipeline(
    OpPassManager &pm, const BufferDeallocationPipelineOptions &options) {
  memref::ExpandReallocPassOptions expandAllocPassOptions{
      /*emitDeallocs=*/false};
  pm.addPass(memref::createExpandReallocPass(expandAllocPassOptions));
  pm.addPass(createCanonicalizerPass());

  OwnershipBasedBufferDeallocationPassOptions deallocationOptions{
      options.privateFunctionDynamicOwnership};
  pm.addPass(createOwnershipBasedBufferDeallocationPass(deallocationOptions));
  pm.addPass(createCanonicalizerPass());
  pm.addPass(createBufferDeallocationSimplificationPass());
  pm.addPass(createLowerDeallocationsPass());
  pm.addPass(createCSEPass());
  pm.addPass(createCanonicalizerPass());
}

//===----------------------------------------------------------------------===//
// Pipeline registration.
//===----------------------------------------------------------------------===//

void mlir::bufferization::registerBufferizationPipelines() {
  PassPipelineRegistration<BufferDeallocationPipelineOptions>(
      "buffer-deallocation-pipeline",
      "The default pipeline for automatically inserting deallocation "
      "operations after one-shot bufferization. Deallocation operations "
      "(except `memref.realloc`) may not be present already.",
      [](OpPassManager &pm, const BufferDeallocationPipelineOptions &options) {
        buildBufferDeallocationPipeline(pm, options);
      });
}
