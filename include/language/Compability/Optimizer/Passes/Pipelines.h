//===-- Pipelines.h -- FIR pass pipelines -----------------------*- C++ -*-===//
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

/// This file declares some utilties to setup FIR pass pipelines. These are
/// common to flang and the test tools.

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_PASSES_PIPELINES_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_PASSES_PIPELINES_H

#include "language/Compability/Optimizer/CodeGen/CodeGen.h"
#include "language/Compability/Optimizer/HLFIR/Passes.h"
#include "language/Compability/Optimizer/OpenMP/Passes.h"
#include "language/Compability/Optimizer/Passes/CommandLineOpts.h"
#include "language/Compability/Optimizer/Transforms/Passes.h"
#include "language/Compability/Tools/CrossToolHelpers.h"
#include "mlir/Conversion/ReconcileUnrealizedCasts/ReconcileUnrealizedCasts.h"
#include "mlir/Conversion/SCFToControlFlow/SCFToControlFlow.h"
#include "mlir/Dialect/GPU/IR/GPUDialect.h"
#include "mlir/Dialect/LLVMIR/LLVMAttrs.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"
#include "mlir/Transforms/Passes.h"
#include "toolchain/Frontend/Debug/Options.h"
#include "toolchain/Passes/OptimizationLevel.h"
#include "toolchain/Support/CommandLine.h"

namespace fir {

using PassConstructor = std::unique_ptr<mlir::Pass>();

template <typename F, typename OP>
void addNestedPassToOps(mlir::PassManager &pm, F ctor) {
  pm.addNestedPass<OP>(ctor());
}

template <typename F, typename OP, typename... OPS,
          typename = std::enable_if_t<sizeof...(OPS) != 0>>
void addNestedPassToOps(mlir::PassManager &pm, F ctor) {
  addNestedPassToOps<F, OP>(pm, ctor);
  addNestedPassToOps<F, OPS...>(pm, ctor);
}

/// Generic for adding a pass to the pass manager if it is not disabled.
template <typename F>
void addPassConditionally(mlir::PassManager &pm, toolchain::cl::opt<bool> &disabled,
                          F ctor) {
  if (!disabled)
    pm.addPass(ctor());
}

template <typename OP, typename F>
void addNestedPassConditionally(mlir::PassManager &pm,
                                toolchain::cl::opt<bool> &disabled, F ctor) {
  if (!disabled)
    pm.addNestedPass<OP>(ctor());
}

template <typename F>
void addNestedPassToAllTopLevelOperations(mlir::PassManager &pm, F ctor);

template <typename F>
void addNestedPassToAllTopLevelOperationsConditionally(
    mlir::PassManager &pm, toolchain::cl::opt<bool> &disabled, F ctor);

/// Add MLIR Canonicalizer pass with region simplification disabled.
/// FIR does not support the promotion of some SSA value to block arguments (or
/// into arith.select operands) that may be done by mlir block merging in the
/// region simplification (e.g., !fir.shape<> SSA values are not supported as
/// block arguments).
/// Aside from the fir.shape issue, moving some abstract SSA value into block
/// arguments may have a heavy cost since it forces their code generation that
/// may be expensive (array temporary). The MLIR pass does not take these
/// extra costs into account when doing block merging.
void addCanonicalizerPassWithoutRegionSimplification(mlir::OpPassManager &pm);

void addCfgConversionPass(mlir::PassManager &pm,
                          const MLIRToLLVMPassPipelineConfig &config);

void addAVC(mlir::PassManager &pm, const toolchain::OptimizationLevel &optLevel);

void addMemoryAllocationOpt(mlir::PassManager &pm);

void addCodeGenRewritePass(mlir::PassManager &pm, bool preserveDeclare);

void addTargetRewritePass(mlir::PassManager &pm);

mlir::LLVM::DIEmissionKind
getEmissionKind(toolchain::codegenoptions::DebugInfoKind kind);

void addBoxedProcedurePass(mlir::PassManager &pm);

void addExternalNameConversionPass(mlir::PassManager &pm,
                                   bool appendUnderscore = true);

void addCompilerGeneratedNamesConversionPass(mlir::PassManager &pm);

void addDebugInfoPass(mlir::PassManager &pm,
                      toolchain::codegenoptions::DebugInfoKind debugLevel,
                      toolchain::OptimizationLevel optLevel,
                      toolchain::StringRef inputFilename);

void addFIRToLLVMPass(mlir::PassManager &pm,
                      const MLIRToLLVMPassPipelineConfig &config);

void addLLVMDialectToLLVMPass(mlir::PassManager &pm, toolchain::raw_ostream &output);

/// Use inliner extension point callback to register the default inliner pass.
void registerDefaultInlinerPass(MLIRToLLVMPassPipelineConfig &config);

/// Create a pass pipeline for running default optimization passes for
/// incremental conversion of FIR.
///
/// \param pm - MLIR pass manager that will hold the pipeline definition
void createDefaultFIROptimizerPassPipeline(mlir::PassManager &pm,
                                           MLIRToLLVMPassPipelineConfig &pc);

/// Select which mode to enable OpenMP support in.
enum class EnableOpenMP { None, Simd, Full };

/// Create a pass pipeline for lowering from HLFIR to FIR
///
/// \param pm - MLIR pass manager that will hold the pipeline definition
/// \param optLevel - optimization level used for creating FIR optimization
///   passes pipeline
void createHLFIRToFIRPassPipeline(
    mlir::PassManager &pm, EnableOpenMP enableOpenMP,
    toolchain::OptimizationLevel optLevel = defaultOptLevel);

struct OpenMPFIRPassPipelineOpts {
  /// Whether code is being generated for a target device rather than the host
  /// device
  bool isTargetDevice;

  /// Controls how to map `do concurrent` loops; to device, host, or none at
  /// all.
  language::Compability::frontend::CodeGenOptions::DoConcurrentMappingKind
      doConcurrentMappingKind;
};

/// Create a pass pipeline for handling certain OpenMP transformations needed
/// prior to FIR lowering.
///
/// WARNING: These passes must be run immediately after the lowering to ensure
/// that the FIR is correct with respect to OpenMP operations/attributes.
///
/// \param pm - MLIR pass manager that will hold the pipeline definition.
/// \param opts - options to control OpenMP code-gen; see struct docs for more
/// details.
void createOpenMPFIRPassPipeline(mlir::PassManager &pm,
                                 OpenMPFIRPassPipelineOpts opts);

#if !defined(FLANG_EXCLUDE_CODEGEN)
void createDebugPasses(mlir::PassManager &pm,
                       toolchain::codegenoptions::DebugInfoKind debugLevel,
                       toolchain::OptimizationLevel OptLevel,
                       toolchain::StringRef inputFilename);

void createDefaultFIRCodeGenPassPipeline(mlir::PassManager &pm,
                                         MLIRToLLVMPassPipelineConfig config,
                                         toolchain::StringRef inputFilename = {});

/// Create a pass pipeline for lowering from MLIR to LLVM IR
///
/// \param pm - MLIR pass manager that will hold the pipeline definition
/// \param optLevel - optimization level used for creating FIR optimization
///   passes pipeline
void createMLIRToLLVMPassPipeline(mlir::PassManager &pm,
                                  MLIRToLLVMPassPipelineConfig &config,
                                  toolchain::StringRef inputFilename = {});
#undef FLANG_EXCLUDE_CODEGEN
#endif

} // namespace fir

#endif // FORTRAN_OPTIMIZER_PASSES_PIPELINES_H
