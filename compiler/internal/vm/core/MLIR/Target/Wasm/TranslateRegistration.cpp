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
#include "mlir/Dialect/WasmSSA/IR/WasmSSA.h"
#include "mlir/IR/DialectRegistry.h"
#include "mlir/IR/OwningOpRef.h"
#include "mlir/Target/Wasm/WasmImporter.h"
#include "mlir/Tools/mlir-translate/Translation.h"

using namespace mlir;

namespace mlir {
void registerFromWasmTranslation() {
  TranslateToMLIRRegistration registration{
      "import-wasm", "Translate WASM to MLIR",
      [](toolchain::SourceMgr &sourceMgr,
         MLIRContext *context) -> OwningOpRef<Operation *> {
        return wasm::importWebAssemblyToModule(sourceMgr, context);
      },
      [](DialectRegistry &registry) {
        registry.insert<wasmssa::WasmSSADialect>();
      }};
}
} // namespace mlir
