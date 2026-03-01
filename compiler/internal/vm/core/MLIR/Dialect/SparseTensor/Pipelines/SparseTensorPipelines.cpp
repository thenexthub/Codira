//===- SparseTensorPipelines.cpp - Pipelines for sparse tensor code -------===//
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

#include "mlir/Conversion/Passes.h"
#include "mlir/Dialect/Arith/Transforms/Passes.h"
#include "mlir/Dialect/Bufferization/Transforms/Passes.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/GPU/IR/GPUDialect.h"
#include "mlir/Dialect/GPU/Transforms/Passes.h"
#include "mlir/Dialect/Linalg/Passes.h"
#include "mlir/Dialect/MemRef/Transforms/Passes.h"
#include "mlir/Dialect/SparseTensor/Pipelines/Passes.h"
#include "mlir/Dialect/SparseTensor/Transforms/Passes.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Transforms/Passes.h"

//===----------------------------------------------------------------------===//
// Pipeline implementation.
//===----------------------------------------------------------------------===//

void mlir::sparse_tensor::buildSparsifier(OpPassManager &pm,
                                          const SparsifierOptions &options) {
  // Rewrite named linalg ops into generic ops and apply fusion.
  pm.addNestedPass<func::FuncOp>(createLinalgGeneralizeNamedOpsPass());
  pm.addNestedPass<func::FuncOp>(createLinalgElementwiseOpFusionPass());

  // Sparsification and bufferization mini-pipeline.
  pm.addPass(createSparsificationAndBufferizationPass(
      getBufferizationOptionsForSparsification(
          options.testBufferizationAnalysisOnly),
      options.sparsificationOptions(), options.createSparseDeallocs,
      options.enableRuntimeLibrary, options.enableBufferInitialization,
      options.vectorLength,
      /*enableVLAVectorization=*/options.armSVE,
      /*enableSIMDIndex32=*/options.force32BitVectorIndices,
      options.enableGPULibgen,
      options.sparsificationOptions().sparseEmitStrategy,
      options.sparsificationOptions().parallelizationStrategy));

  // Bail-early for test setup.
  if (options.testBufferizationAnalysisOnly)
    return;

  // Storage specifier lowering and bufferization wrap-up.
  pm.addPass(createStorageSpecifierToLLVMPass());
  pm.addNestedPass<func::FuncOp>(createCanonicalizerPass());

  // GPU code generation.
  const bool gpuCodegen = options.gpuTriple.hasValue();
  if (gpuCodegen) {
    pm.addPass(createSparseGPUCodegenPass());
    pm.addNestedPass<gpu::GPUModuleOp>(createStripDebugInfoPass());
    pm.addNestedPass<gpu::GPUModuleOp>(createSCFToControlFlowPass());
    pm.addNestedPass<gpu::GPUModuleOp>(createConvertGpuOpsToNVVMOps());
  }

  // Progressively lower to LLVM. Note that the convert-vector-to-toolchain
  // pass is repeated on purpose.
  // TODO(springerm): Add sparse support to the BufferDeallocation pass and add
  // it to this pipeline.
  pm.addNestedPass<func::FuncOp>(createConvertLinalgToLoopsPass());
  pm.addNestedPass<func::FuncOp>(createConvertVectorToSCFPass());
  pm.addNestedPass<func::FuncOp>(memref::createExpandReallocPass());
  pm.addNestedPass<func::FuncOp>(createSCFToControlFlowPass());
  pm.addPass(memref::createExpandStridedMetadataPass());
  pm.addPass(createLowerAffinePass());
  pm.addPass(
      createConvertVectorToLLVMPass(options.convertVectorToLLVMOptions()));
  pm.addNestedPass<func::FuncOp>(createConvertComplexToStandardPass());
  pm.addNestedPass<func::FuncOp>(arith::createArithExpandOpsPass());
  pm.addNestedPass<func::FuncOp>(createConvertMathToLLVMPass());
  pm.addPass(createConvertMathToLibmPass());
  pm.addPass(createConvertComplexToLibm());
  pm.addPass(
      createConvertVectorToLLVMPass(options.convertVectorToLLVMOptions()));

  // Finalize GPU code generation.
  if (gpuCodegen) {
    GpuNVVMAttachTargetOptions nvvmTargetOptions;
    nvvmTargetOptions.triple = options.gpuTriple;
    nvvmTargetOptions.chip = options.gpuChip;
    nvvmTargetOptions.features = options.gpuFeatures;
    pm.addPass(createGpuNVVMAttachTarget(nvvmTargetOptions));
    pm.addPass(createGpuToLLVMConversionPass());
    GpuModuleToBinaryPassOptions gpuModuleToBinaryPassOptions;
    gpuModuleToBinaryPassOptions.compilationTarget = options.gpuFormat;
    pm.addPass(createGpuModuleToBinaryPass(gpuModuleToBinaryPassOptions));
  }

  // Convert to LLVM.
  pm.addPass(createConvertToLLVMPass());

  // Ensure all casts are realized.
  pm.addPass(createReconcileUnrealizedCastsPass());
}

//===----------------------------------------------------------------------===//
// Pipeline registration.
//===----------------------------------------------------------------------===//

void mlir::sparse_tensor::registerSparseTensorPipelines() {
  PassPipelineRegistration<SparsifierOptions>(
      "sparsifier",
      "The standard pipeline for taking sparsity-agnostic IR using the"
      " sparse-tensor type, and lowering it to LLVM IR with concrete"
      " representations and algorithms for sparse tensors.",
      buildSparsifier);
}
