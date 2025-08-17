//===- Passes.h - CIR pass entry points -------------------------*- C++ -*-===//
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
// This header file defines prototypes that expose pass constructors.
//
//===----------------------------------------------------------------------===//

#ifndef CLANG_CIR_DIALECT_PASSES_H
#define CLANG_CIR_DIALECT_PASSES_H

#include "mlir/Pass/Pass.h"

namespace language::Core {
class ASTContext;
}
namespace mlir {

std::unique_ptr<Pass> createCIRCanonicalizePass();
std::unique_ptr<Pass> createCIRFlattenCFGPass();
std::unique_ptr<Pass> createCIRSimplifyPass();
std::unique_ptr<Pass> createHoistAllocasPass();
std::unique_ptr<Pass> createLoweringPreparePass();
std::unique_ptr<Pass> createLoweringPreparePass(language::Core::ASTContext *astCtx);

void populateCIRPreLoweringPasses(mlir::OpPassManager &pm);

//===----------------------------------------------------------------------===//
// Registration
//===----------------------------------------------------------------------===//

void registerCIRDialectTranslation(mlir::MLIRContext &context);

/// Generate the code for registering passes.
#define GEN_PASS_REGISTRATION
#include "language/Core/CIR/Dialect/Passes.h.inc"

} // namespace mlir

#endif // CLANG_CIR_DIALECT_PASSES_H
