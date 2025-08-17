//===-- Optimizer/Transforms/Passes.h ---------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_TRANSFORMS_PASSES_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_TRANSFORMS_PASSES_H

#include "language/Compability/Optimizer/Dialect/FIROps.h"
#include "mlir/Dialect/LLVMIR/LLVMAttrs.h"
#include "mlir/Dialect/OpenMP/OpenMPDialect.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassRegistry.h"
#include <memory>

namespace mlir {
class IRMapping;
class GreedyRewriteConfig;
class Operation;
class Pass;
class Region;
class ModuleOp;
} // namespace mlir

namespace fir {

//===----------------------------------------------------------------------===//
// Passes defined in Passes.td
//===----------------------------------------------------------------------===//

#define GEN_PASS_DECL

#include "language/Compability/Optimizer/Transforms/Passes.h.inc"

std::unique_ptr<mlir::Pass> createAffineDemotionPass();
std::unique_ptr<mlir::Pass>
createArrayValueCopyPass(fir::ArrayValueCopyOptions options = {});
std::unique_ptr<mlir::Pass> createMemDataFlowOptPass();
std::unique_ptr<mlir::Pass> createPromoteToAffinePass();
std::unique_ptr<mlir::Pass> createFIRToSCFPass();
std::unique_ptr<mlir::Pass>
createAddDebugInfoPass(fir::AddDebugInfoOptions options = {});

std::unique_ptr<mlir::Pass> createAnnotateConstantOperandsPass();
std::unique_ptr<mlir::Pass> createAlgebraicSimplificationPass();
std::unique_ptr<mlir::Pass>
createAlgebraicSimplificationPass(const mlir::GreedyRewriteConfig &config);

std::unique_ptr<mlir::Pass> createVScaleAttrPass();
std::unique_ptr<mlir::Pass>
createVScaleAttrPass(std::pair<unsigned, unsigned> vscaleAttr);

void populateCfgConversionRewrites(mlir::RewritePatternSet &patterns,
                                   bool forceLoopToExecuteOnce = false,
                                   bool setNSW = true);

void populateSimplifyFIROperationsPatterns(mlir::RewritePatternSet &patterns,
                                           bool preferInlineImplementation);

// declarative passes
#define GEN_PASS_REGISTRATION
#include "language/Compability/Optimizer/Transforms/Passes.h.inc"

} // namespace fir

#endif // FORTRAN_OPTIMIZER_TRANSFORMS_PASSES_H
