//===-- DoLoopHelper.h -- gen fir.do_loop ops -------------------*- C++ -*-===//
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

#ifndef LANGUAGE_COMPABILITY_OPTIMIZER_BUILDER_DOLOOPHELPER_H
#define LANGUAGE_COMPABILITY_OPTIMIZER_BUILDER_DOLOOPHELPER_H

#include "language/Compability/Optimizer/Builder/FIRBuilder.h"

namespace fir::factory {

/// Helper to build fir.do_loop Ops.
class DoLoopHelper {
public:
  explicit DoLoopHelper(fir::FirOpBuilder &builder, mlir::Location loc)
      : builder(builder), loc(loc) {}
  DoLoopHelper(const DoLoopHelper &) = delete;

  /// Type of a callback to generate the loop body.
  using BodyGenerator = std::function<void(fir::FirOpBuilder &, mlir::Value)>;

  /// Build loop [\p lb, \p ub] with step \p step.
  /// If \p step is an empty value, 1 is used for the step.
  fir::DoLoopOp createLoop(mlir::Value lb, mlir::Value ub, mlir::Value step,
                           const BodyGenerator &bodyGenerator);

  /// Build loop [\p lb,  \p ub] with step 1.
  fir::DoLoopOp createLoop(mlir::Value lb, mlir::Value ub,
                           const BodyGenerator &bodyGenerator);

  /// Build loop [0, \p count) with step 1.
  fir::DoLoopOp createLoop(mlir::Value count,
                           const BodyGenerator &bodyGenerator);

private:
  fir::FirOpBuilder &builder;
  mlir::Location loc;
};

} // namespace fir::factory

#endif // FORTRAN_OPTIMIZER_BUILDER_DOLOOPHELPER_H
