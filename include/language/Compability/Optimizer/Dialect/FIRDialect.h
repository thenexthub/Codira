//===-- Optimizer/Dialect/FIRDialect.h -- FIR dialect -----------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_DIALECT_FIRDIALECT_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_DIALECT_FIRDIALECT_H

#include "mlir/IR/Dialect.h"

#include "language/Compability/Optimizer/Dialect/FIRDialect.h.inc"

namespace mlir {
class IRMapping;
} // namespace mlir

namespace fir {

/// The FIR codegen dialect is a dialect containing a small set of transient
/// operations used exclusively during code generation.
class FIRCodeGenDialect final : public mlir::Dialect {
public:
  explicit FIRCodeGenDialect(mlir::MLIRContext *ctx);
  virtual ~FIRCodeGenDialect();

  static toolchain::StringRef getDialectNamespace() { return "fircg"; }
};

/// Support for inlining on FIR.
bool canLegallyInline(mlir::Operation *op, mlir::Region *reg, bool,
                      mlir::IRMapping &map);
bool canLegallyInline(mlir::Operation *, mlir::Operation *, bool);

// Register the FIRInlinerInterface to FIROpsDialect
void addFIRInlinerExtension(mlir::DialectRegistry &registry);

// Register implementation of LLVMTranslationDialectInterface.
void addFIRToLLVMIRExtension(mlir::DialectRegistry &registry);

void registerFortranTempArrayCopyIsSafeExternalModels(
    mlir::DialectRegistry &registry);

} // namespace fir

#endif // FORTRAN_OPTIMIZER_DIALECT_FIRDIALECT_H
