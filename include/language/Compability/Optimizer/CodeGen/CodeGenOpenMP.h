//===------- Optimizer/CodeGen/CodeGenOpenMP.h - OpenMP codegen -*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_CODEGEN_CODEGENOPENMP_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_CODEGEN_CODEGENOPENMP_H

#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassRegistry.h"

namespace fir {
class LLVMTypeConverter;

/// Specialised conversion patterns of OpenMP operations for FIR to LLVM
/// dialect, utilised in cases where the default OpenMP dialect handling cannot
/// handle all cases for intermingled fir types and operations.
void populateOpenMPFIRToLLVMConversionPatterns(
    const LLVMTypeConverter &converter, mlir::RewritePatternSet &patterns);

} // namespace fir

#endif // FORTRAN_OPTIMIZER_CODEGEN_CODEGENOPENMP_H
