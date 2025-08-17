//===- Passes.h - OpenMP pass entry points ----------------------*- C++ -*-===//
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
// This header declares the flang OpenMP passes.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_OPENMP_PASSES_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_OPENMP_PASSES_H

#include "language/Compability/Optimizer/OpenMP/Utils.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassRegistry.h"

#include <memory>

namespace flangomp {
#define GEN_PASS_DECL
#define GEN_PASS_REGISTRATION
#include "language/Compability/Optimizer/OpenMP/Passes.h.inc"

/// Impelements the logic specified in the 2.8.3  workshare Construct section of
/// the OpenMP standard which specifies what statements or constructs shall be
/// divided into units of work.
bool shouldUseWorkshareLowering(mlir::Operation *op);

std::unique_ptr<mlir::Pass> createDoConcurrentConversionPass(bool mapToDevice);
} // namespace flangomp

#endif // FORTRAN_OPTIMIZER_OPENMP_PASSES_H
