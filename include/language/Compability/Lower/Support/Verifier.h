//===-- Lower/Support/Verifier.h -- verify pass for lowering ----*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_LOWER_SUPPORT_VERIFIER_H
#define LANGUAGE_COMPABILITY_LOWER_SUPPORT_VERIFIER_H

#include "mlir/IR/Verifier.h"
#include "mlir/Pass/Pass.h"

namespace language::Compability::lower {

/// A verification pass to verify the output from the bridge. This provides a
/// little bit of glue to run a verifier pass directly.
class VerifierPass
    : public mlir::PassWrapper<VerifierPass, mlir::OperationPass<>> {
  void runOnOperation() override final {
    if (mlir::failed(mlir::verify(getOperation())))
      signalPassFailure();
    markAllAnalysesPreserved();
  }
};

} // namespace language::Compability::lower

#endif // FORTRAN_LOWER_SUPPORT_VERIFIER_H
