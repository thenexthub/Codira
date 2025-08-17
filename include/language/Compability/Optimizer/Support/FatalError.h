//===-- Optimizer/Support/FatalError.h --------------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_SUPPORT_FATALERROR_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_SUPPORT_FATALERROR_H

#include "mlir/IR/Diagnostics.h"
#include "toolchain/Support/ErrorHandling.h"

namespace fir {

/// Fatal error reporting helper. Report a fatal error with a source location
/// and immediately interrupt flang. If `genCrashDiag` is true, then
/// the execution is aborted and the backtrace is printed, otherwise,
/// flang exits with non-zero exit code and without backtrace printout.
[[noreturn]] inline void emitFatalError(mlir::Location loc,
                                        const toolchain::Twine &message,
                                        bool genCrashDiag = true) {
  mlir::emitError(loc, message);
  toolchain::report_fatal_error("aborting", genCrashDiag);
}

} // namespace fir

#endif // FORTRAN_OPTIMIZER_SUPPORT_FATALERROR_H
