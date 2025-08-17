//===------- Optimizer/Transforms/CUFOpConversion.h -------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_TRANSFORMS_CUFOPCONVERSION_H_
#define LANGUAGE_COMPABILITY_OPTIMIZER_TRANSFORMS_CUFOPCONVERSION_H_

#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassRegistry.h"

namespace fir {
class LLVMTypeConverter;
}

namespace mlir {
class DataLayout;
class SymbolTable;
} // namespace mlir

namespace cuf {

/// Patterns that convert CUF operations to runtime calls.
void populateCUFToFIRConversionPatterns(const fir::LLVMTypeConverter &converter,
                                        mlir::DataLayout &dl,
                                        const mlir::SymbolTable &symtab,
                                        mlir::RewritePatternSet &patterns);

/// Patterns that updates fir operations in presence of CUF.
void populateFIRCUFConversionPatterns(const mlir::SymbolTable &symtab,
                                      mlir::RewritePatternSet &patterns);

} // namespace cuf

#endif // FORTRAN_OPTIMIZER_TRANSFORMS_CUFOPCONVERSION_H_
