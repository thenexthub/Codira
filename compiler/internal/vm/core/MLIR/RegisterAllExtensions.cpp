//===- RegisterAllExtensions.cpp - MLIR Extension Registration --*- C++ -*-===//
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
// This file defines a helper to trigger the registration of all dialect
// extensions to the system.
//
//===----------------------------------------------------------------------===//

#include "mlir/InitAllExtensions.h"

#include "mlir/Conversion/ArithToEmitC/ArithToEmitC.h"
#include "mlir/Conversion/ArithToLLVM/ArithToLLVM.h"
#include "mlir/Conversion/ComplexToLLVM/ComplexToLLVM.h"
#include "mlir/Conversion/ControlFlowToLLVM/ControlFlowToLLVM.h"
#include "mlir/Conversion/FuncToEmitC/FuncToEmitC.h"
#include "mlir/Conversion/FuncToLLVM/ConvertFuncToLLVM.h"
#include "mlir/Conversion/GPUCommon/GPUToLLVM.h"
#include "mlir/Conversion/GPUToNVVM/GPUToNVVM.h"
#include "mlir/Conversion/IndexToLLVM/IndexToLLVM.h"
#include "mlir/Conversion/MPIToLLVM/MPIToLLVM.h"
#include "mlir/Conversion/MathToLLVM/MathToLLVM.h"
#include "mlir/Conversion/MemRefToEmitC/MemRefToEmitC.h"
#include "mlir/Conversion/MemRefToLLVM/MemRefToLLVM.h"
#include "mlir/Conversion/NVVMToLLVM/NVVMToLLVM.h"
#include "mlir/Conversion/OpenMPToLLVM/ConvertOpenMPToLLVM.h"
#include "mlir/Conversion/PtrToLLVM/PtrToLLVM.h"
#include "mlir/Conversion/SCFToEmitC/SCFToEmitC.h"
#include "mlir/Conversion/UBToLLVM/UBToLLVM.h"
#include "mlir/Conversion/VectorToLLVM/ConvertVectorToLLVM.h"
#include "mlir/Conversion/XeVMToLLVM/XeVMToLLVM.h"
#include "mlir/Dialect/AMX/Transforms.h"
#include "mlir/Dialect/Affine/TransformOps/AffineTransformOps.h"
#include "mlir/Dialect/ArmNeon/TransformOps/ArmNeonVectorTransformOps.h"
#include "mlir/Dialect/ArmSVE/TransformOps/ArmSVEVectorTransformOps.h"
#include "mlir/Dialect/Bufferization/TransformOps/BufferizationTransformOps.h"
#include "mlir/Dialect/DLTI/TransformOps/DLTITransformOps.h"
#include "mlir/Dialect/Func/Extensions/AllExtensions.h"
#include "mlir/Dialect/Func/TransformOps/FuncTransformOps.h"
#include "mlir/Dialect/GPU/TransformOps/GPUTransformOps.h"
#include "mlir/Dialect/Linalg/TransformOps/DialectExtension.h"
#include "mlir/Dialect/MemRef/TransformOps/MemRefTransformOps.h"
#include "mlir/Dialect/NVGPU/TransformOps/NVGPUTransformOps.h"
#include "mlir/Dialect/SCF/TransformOps/SCFTransformOps.h"
#include "mlir/Dialect/SparseTensor/TransformOps/SparseTensorTransformOps.h"
#include "mlir/Dialect/Tensor/Extensions/AllExtensions.h"
#include "mlir/Dialect/Tensor/TransformOps/TensorTransformOps.h"
#include "mlir/Dialect/Transform/DebugExtension/DebugExtension.h"
#include "mlir/Dialect/Transform/IRDLExtension/IRDLExtension.h"
#include "mlir/Dialect/Transform/LoopExtension/LoopExtension.h"
#include "mlir/Dialect/Transform/PDLExtension/PDLExtension.h"
#include "mlir/Dialect/Transform/SMTExtension/SMTExtension.h"
#include "mlir/Dialect/Transform/TuneExtension/TuneExtension.h"
#include "mlir/Dialect/Vector/TransformOps/VectorTransformOps.h"
#include "mlir/Dialect/X86Vector/TransformOps/X86VectorTransformOps.h"
#include "mlir/Dialect/XeGPU/TransformOps/XeGPUTransformOps.h"
#include "mlir/Target/LLVMIR/Dialect/Builtin/BuiltinToLLVMIRTranslation.h"
#include "mlir/Target/LLVMIR/Dialect/GPU/GPUToLLVMIRTranslation.h"
#include "mlir/Target/LLVMIR/Dialect/LLVMIR/LLVMToLLVMIRTranslation.h"
#include "mlir/Target/LLVMIR/Dialect/ROCDL/ROCDLToLLVMIRTranslation.h"
#include "mlir/Target/LLVMIR/Dialect/XeVM/XeVMToLLVMIRTranslation.h"

/// This function may be called to register all MLIR dialect extensions with the
/// provided registry.
/// If you're building a compiler, you generally shouldn't use this: you would
/// individually register the specific extensions that are useful for the
/// pipelines and transformations you are using.
void mlir::registerAllExtensions(DialectRegistry &registry) {
  // Register all conversions to LLVM extensions.
  registerConvertArithToEmitCInterface(registry);
  arith::registerConvertArithToLLVMInterface(registry);
  registerConvertComplexToLLVMInterface(registry);
  cf::registerConvertControlFlowToLLVMInterface(registry);
  func::registerAllExtensions(registry);
  tensor::registerAllExtensions(registry);
  registerConvertFuncToEmitCInterface(registry);
  registerConvertFuncToLLVMInterface(registry);
  index::registerConvertIndexToLLVMInterface(registry);
  registerConvertMathToLLVMInterface(registry);
  mpi::registerConvertMPIToLLVMInterface(registry);
  registerConvertMemRefToEmitCInterface(registry);
  registerConvertMemRefToLLVMInterface(registry);
  registerConvertNVVMToLLVMInterface(registry);
  ptr::registerConvertPtrToLLVMInterface(registry);
  registerConvertOpenMPToLLVMInterface(registry);
  registerConvertSCFToEmitCInterface(registry);
  ub::registerConvertUBToLLVMInterface(registry);
  registerConvertAMXToLLVMInterface(registry);
  gpu::registerConvertGpuToLLVMInterface(registry);
  NVVM::registerConvertGpuToNVVMInterface(registry);
  vector::registerConvertVectorToLLVMInterface(registry);
  registerConvertXeVMToLLVMInterface(registry);

  // Register all transform dialect extensions.
  affine::registerTransformDialectExtension(registry);
  bufferization::registerTransformDialectExtension(registry);
  dlti::registerTransformDialectExtension(registry);
  func::registerTransformDialectExtension(registry);
  gpu::registerTransformDialectExtension(registry);
  linalg::registerTransformDialectExtension(registry);
  memref::registerTransformDialectExtension(registry);
  nvgpu::registerTransformDialectExtension(registry);
  scf::registerTransformDialectExtension(registry);
  sparse_tensor::registerTransformDialectExtension(registry);
  tensor::registerTransformDialectExtension(registry);
  transform::registerDebugExtension(registry);
  transform::registerIRDLExtension(registry);
  transform::registerLoopExtension(registry);
  transform::registerPDLExtension(registry);
  transform::registerSMTExtension(registry);
  transform::registerTuneExtension(registry);
  vector::registerTransformDialectExtension(registry);
  x86vector::registerTransformDialectExtension(registry);
  xegpu::registerTransformDialectExtension(registry);
  arm_neon::registerTransformDialectExtension(registry);
  arm_sve::registerTransformDialectExtension(registry);

  // Translation extensions need to be registered by calling
  // `registerAllToLLVMIRTranslations` (see All.h).
}
