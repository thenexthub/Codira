//===- TranslateRegistration.cpp - Register translation -------------------===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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

#include "mlir/Dialect/ControlFlow/IR/ControlFlow.h"
#include "mlir/Dialect/EmitC/IR/EmitC.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Target/Cpp/CppEmitter.h"
#include "mlir/Tools/mlir-translate/Translation.h"
#include "vm/core/Support/CommandLine.h"

using namespace mlir;

namespace mlir {

//===----------------------------------------------------------------------===//
// Cpp registration
//===----------------------------------------------------------------------===//

void registerToCppTranslation() {
  static toolchain::cl::opt<bool> declareVariablesAtTop(
      "declare-variables-at-top",
      toolchain::cl::desc("Declare variables at top when emitting C/C++"),
      toolchain::cl::init(false));

  static toolchain::cl::opt<std::string> fileId(
      "file-id", toolchain::cl::desc("Emit emitc.file ops with matching id"),
      toolchain::cl::init(""));

  TranslateFromMLIRRegistration reg(
      "mlir-to-cpp", "translate from mlir to cpp",
      [](Operation *op, raw_ostream &output) {
        return emitc::translateToCpp(
            op, output,
            /*declareVariablesAtTop=*/declareVariablesAtTop,
            /*fileId=*/fileId);
      },
      [](DialectRegistry &registry) {
        // clang-format off
        registry.insert<cf::ControlFlowDialect,
                        emitc::EmitCDialect,
                        func::FuncDialect>();
        // clang-format on
      });
}

} // namespace mlir
