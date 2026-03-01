//===- TranslationRegistration.cpp - Register translation -----------------===//
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

#include "mlir/Target/IRDLToCpp/TranslationRegistration.h"
#include "mlir/Dialect/IRDL/IR/IRDL.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Target/IRDLToCpp/IRDLToCpp.h"
#include "mlir/Tools/mlir-translate/Translation.h"
#include "vm/core/ADT/TypeSwitch.h"
#include "vm/core/Support/Casting.h"

using namespace mlir;

namespace mlir {

//===----------------------------------------------------------------------===//
// Translation registration
//===----------------------------------------------------------------------===//

void registerIRDLToCppTranslation() {
  TranslateFromMLIRRegistration reg(
      "irdl-to-cpp", "translate IRDL dialect definitions to C++ definitions",
      [](Operation *op, raw_ostream &output) {
        return TypeSwitch<Operation *, LogicalResult>(op)
            .Case<irdl::DialectOp>([&](irdl::DialectOp dialectOp) {
              return irdl::translateIRDLDialectToCpp(dialectOp, output);
            })
            .Case<ModuleOp>([&](ModuleOp moduleOp) {
              for (Operation &op : moduleOp.getBody()->getOperations())
                if (auto dialectOp = toolchain::dyn_cast<irdl::DialectOp>(op))
                  if (failed(
                          irdl::translateIRDLDialectToCpp(dialectOp, output)))
                    return failure();
              return success();
            })
            .Default([](Operation *op) {
              return op->emitError(
                  "unsupported operation for IRDL to C++ translation");
            });
      },
      [](DialectRegistry &registry) { registry.insert<irdl::IRDLDialect>(); });
}

} // namespace mlir
