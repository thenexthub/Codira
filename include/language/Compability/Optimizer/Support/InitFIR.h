//===-- Optimizer/Support/InitFIR.h -----------------------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
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
// Coding style: https://mlir.toolchain.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_SUPPORT_INITFIR_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_SUPPORT_INITFIR_H

#include "language/Compability/Optimizer/Dialect/CUF/CUFDialect.h"
#include "language/Compability/Optimizer/Dialect/CUF/CUFToLLVMIRTranslation.h"
#include "language/Compability/Optimizer/Dialect/FIRDialect.h"
#include "language/Compability/Optimizer/HLFIR/HLFIRDialect.h"
#include "language/Compability/Optimizer/OpenACC/Support/RegisterOpenACCExtensions.h"
#include "language/Compability/Optimizer/OpenMP/Support/RegisterOpenMPExtensions.h"
#include "mlir/Conversion/Passes.h"
#include "mlir/Dialect/Affine/IR/AffineOps.h"
#include "mlir/Dialect/Affine/Passes.h"
#include "mlir/Dialect/Complex/IR/Complex.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlow.h"
#include "mlir/Dialect/DLTI/DLTI.h"
#include "mlir/Dialect/Func/Extensions/InlinerExtension.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Index/IR/IndexDialect.h"
#include "mlir/Dialect/LLVMIR/NVVMDialect.h"
#include "mlir/Dialect/LLVMIR/Transforms/InlinerInterfaceImpl.h"
#include "mlir/Dialect/Math/IR/Math.h"
#include "mlir/Dialect/OpenACC/OpenACC.h"
#include "mlir/Dialect/OpenACC/Transforms/Passes.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Dialect/SCF/Transforms/Passes.h"
#include "mlir/InitAllDialects.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassRegistry.h"
#include "mlir/Transforms/LocationSnapshot.h"
#include "mlir/Transforms/Passes.h"

namespace fir::support {

#define FLANG_NONCODEGEN_DIALECT_LIST                                          \
  mlir::affine::AffineDialect, FIROpsDialect, hlfir::hlfirDialect,             \
      mlir::acc::OpenACCDialect, mlir::omp::OpenMPDialect,                     \
      mlir::scf::SCFDialect, mlir::arith::ArithDialect,                        \
      mlir::cf::ControlFlowDialect, mlir::func::FuncDialect,                   \
      mlir::vector::VectorDialect, mlir::math::MathDialect,                    \
      mlir::complex::ComplexDialect, mlir::DLTIDialect, cuf::CUFDialect,       \
      mlir::NVVM::NVVMDialect, mlir::gpu::GPUDialect,                          \
      mlir::index::IndexDialect

#define FLANG_CODEGEN_DIALECT_LIST FIRCodeGenDialect, mlir::LLVM::LLVMDialect

// The definitive list of dialects used by flang.
#define FLANG_DIALECT_LIST                                                     \
  FLANG_NONCODEGEN_DIALECT_LIST, FLANG_CODEGEN_DIALECT_LIST

inline void registerNonCodegenDialects(mlir::DialectRegistry &registry) {
  registry.insert<FLANG_NONCODEGEN_DIALECT_LIST>();
  mlir::func::registerInlinerExtension(registry);
  mlir::LLVM::registerInlinerInterface(registry);
}

/// Register all the dialects used by flang.
inline void registerDialects(mlir::DialectRegistry &registry) {
  registerNonCodegenDialects(registry);
  registry.insert<FLANG_CODEGEN_DIALECT_LIST>();
}

// Register FIR Extensions
inline void addFIRExtensions(mlir::DialectRegistry &registry,
                             bool addFIRInlinerInterface = true) {
  if (addFIRInlinerInterface)
    addFIRInlinerExtension(registry);
  addFIRToLLVMIRExtension(registry);
  cuf::registerCUFDialectTranslation(registry);
  fir::acc::registerOpenACCExtensions(registry);
  fir::omp::registerOpenMPExtensions(registry);
}

inline void loadNonCodegenDialects(mlir::MLIRContext &context) {
  mlir::DialectRegistry registry;
  registerNonCodegenDialects(registry);
  context.appendDialectRegistry(registry);

  context.loadDialect<FLANG_NONCODEGEN_DIALECT_LIST>();
}

/// Forced load of all the dialects used by flang.  Lowering is not an MLIR
/// pass, but a producer of FIR and MLIR. It is therefore a requirement that the
/// dialects be preloaded to be able to build the IR.
inline void loadDialects(mlir::MLIRContext &context) {
  mlir::DialectRegistry registry;
  registerDialects(registry);
  context.appendDialectRegistry(registry);

  context.loadDialect<FLANG_DIALECT_LIST>();
}

/// Register the standard passes we use. This comes from registerAllPasses(),
/// but is a smaller set since we aren't using many of the passes found there.
inline void registerMLIRPassesForFortranTools() {
  mlir::acc::registerOpenACCPasses();
  mlir::registerCanonicalizerPass();
  mlir::registerCSEPass();
  mlir::affine::registerAffineLoopFusionPass();
  mlir::registerLoopInvariantCodeMotionPass();
  mlir::affine::registerLoopCoalescingPass();
  mlir::registerStripDebugInfoPass();
  mlir::registerPrintOpStatsPass();
  mlir::registerInlinerPass();
  mlir::registerSCCPPass();
  mlir::registerSCFPasses();
  mlir::affine::registerAffineScalarReplacementPass();
  mlir::registerSymbolDCEPass();
  mlir::registerLocationSnapshotPass();
  mlir::affine::registerAffinePipelineDataTransferPass();

  mlir::affine::registerAffineVectorizePass();
  mlir::affine::registerAffineLoopUnrollPass();
  mlir::affine::registerAffineLoopUnrollAndJamPass();
  mlir::affine::registerSimplifyAffineStructuresPass();
  mlir::affine::registerAffineLoopInvariantCodeMotionPass();
  mlir::affine::registerAffineLoopTilingPass();
  mlir::affine::registerAffineDataCopyGenerationPass();

  mlir::registerLowerAffinePass();
}

/// Register the interfaces needed to lower to LLVM IR.
void registerLLVMTranslation(mlir::MLIRContext &context);

} // namespace fir::support

#endif // FORTRAN_OPTIMIZER_SUPPORT_INITFIR_H
