//===----------------------------------------------------------------------===//
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
// This file implements machinery for any CIR <-> CIR passes used by clang.
//
//===----------------------------------------------------------------------===//

// #include "language/Core/AST/ASTContext.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/PassManager.h"
#include "language/Core/CIR/Dialect/Passes.h"
#include "toolchain/Support/TimeProfiler.h"

namespace cir {
mlir::LogicalResult runCIRToCIRPasses(mlir::ModuleOp theModule,
                                      mlir::MLIRContext &mlirContext,
                                      language::Core::ASTContext &astContext,
                                      bool enableVerifier,
                                      bool enableCIRSimplify) {

  toolchain::TimeTraceScope scope("CIR To CIR Passes");

  mlir::PassManager pm(&mlirContext);
  pm.addPass(mlir::createCIRCanonicalizePass());

  if (enableCIRSimplify)
    pm.addPass(mlir::createCIRSimplifyPass());

  pm.addPass(mlir::createLoweringPreparePass(&astContext));

  pm.enableVerifier(enableVerifier);
  (void)mlir::applyPassManagerCLOptions(pm);
  return pm.run(theModule);
}

} // namespace cir

namespace mlir {

void populateCIRPreLoweringPasses(OpPassManager &pm) {
  pm.addPass(createHoistAllocasPass());
  pm.addPass(createCIRFlattenCFGPass());
}

} // namespace mlir
