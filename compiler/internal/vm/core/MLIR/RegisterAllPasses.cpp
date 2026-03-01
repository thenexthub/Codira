//===- RegisterAllPasses.cpp - MLIR Registration ----------------*- C++ -*-===//
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
// This file defines a helper to trigger the registration of all passes to the
// system.
//
//===----------------------------------------------------------------------===//

#include "mlir/InitAllPasses.h"

#include "mlir/Conversion/Passes.h"
#include "mlir/Dialect/AMDGPU/Transforms/Passes.h"
#include "mlir/Dialect/Affine/Transforms/Passes.h"
#include "mlir/Dialect/Arith/Transforms/Passes.h"
#include "mlir/Dialect/ArmSME/Transforms/Passes.h"
#include "mlir/Dialect/ArmSVE/Transforms/Passes.h"
#include "mlir/Dialect/Async/Passes.h"
#include "mlir/Dialect/Bufferization/Pipelines/Passes.h"
#include "mlir/Dialect/Bufferization/Transforms/Passes.h"
#include "mlir/Dialect/EmitC/Transforms/Passes.h"
#include "mlir/Dialect/Func/Transforms/Passes.h"
#include "mlir/Dialect/GPU/Pipelines/Passes.h"
#include "mlir/Dialect/GPU/Transforms/Passes.h"
#include "mlir/Dialect/LLVMIR/Transforms/Passes.h"
#include "mlir/Dialect/Linalg/Passes.h"
#include "mlir/Dialect/MLProgram/Transforms/Passes.h"
#include "mlir/Dialect/Math/Transforms/Passes.h"
#include "mlir/Dialect/MemRef/Transforms/Passes.h"
#include "mlir/Dialect/NVGPU/Transforms/Passes.h"
#include "mlir/Dialect/OpenACC/Transforms/Passes.h"
#include "mlir/Dialect/OpenMP/Transforms/Passes.h"
#include "mlir/Dialect/Quant/Transforms/Passes.h"
#include "mlir/Dialect/SCF/Transforms/Passes.h"
#include "mlir/Dialect/SPIRV/Transforms/Passes.h"
#include "mlir/Dialect/Shape/Transforms/Passes.h"
#include "mlir/Dialect/Shard/Transforms/Passes.h"
#include "mlir/Dialect/SparseTensor/Pipelines/Passes.h"
#include "mlir/Dialect/SparseTensor/Transforms/Passes.h"
#include "mlir/Dialect/Tensor/Transforms/Passes.h"
#include "mlir/Dialect/Tosa/Transforms/Passes.h"
#include "mlir/Dialect/Transform/Transforms/Passes.h"
#include "mlir/Dialect/Vector/Transforms/Passes.h"
#include "mlir/Dialect/XeGPU/Transforms/Passes.h"
#include "mlir/Target/LLVMIR/Transforms/Passes.h"
#include "mlir/Transforms/Passes.h"

// This function may be called to register the MLIR passes with the
// global registry.
// If you're building a compiler, you likely don't need this: you would build a
// pipeline programmatically without the need to register with the global
// registry, since it would already be calling the creation routine of the
// individual passes.
// The global registry is interesting to interact with the command-line tools.
void mlir::registerAllPasses() {
  // General passes
  registerTransformsPasses();

  // Conversion passes
  registerConversionPasses();

  // Dialect passes
  acc::registerOpenACCPasses();
  affine::registerAffinePasses();
  amdgpu::registerAMDGPUPasses();
  registerAsyncPasses();
  arith::registerArithPasses();
  bufferization::registerBufferizationPasses();
  func::registerFuncPasses();
  registerGPUPasses();
  registerLinalgPasses();
  registerNVGPUPasses();
  registerSparseTensorPasses();
  LLVM::registerLLVMPasses();
  LLVM::registerTargetLLVMIRTransformsPasses();
  math::registerMathPasses();
  memref::registerMemRefPasses();
  shard::registerShardPasses();
  ml_program::registerMLProgramPasses();
  omp::registerOpenMPPasses();
  quant::registerQuantPasses();
  registerSCFPasses();
  registerShapePasses();
  spirv::registerSPIRVPasses();
  tensor::registerTensorPasses();
  tosa::registerTosaPasses();
  transform::registerTransformPasses();
  vector::registerVectorPasses();
  arm_sme::registerArmSMEPasses();
  arm_sve::registerArmSVEPasses();
  emitc::registerEmitCPasses();
  xegpu::registerXeGPUPasses();

  // Dialect pipelines
  bufferization::registerBufferizationPipelines();
  sparse_tensor::registerSparseTensorPipelines();
  tosa::registerTosaToLinalgPipelines();
  gpu::registerGPUToNVVMPipeline();
  gpu::registerGPUToXeVMPipeline();
}
