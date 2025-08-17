//===------------------ Wasm.h - Wasm Interpreter ---------------*- C++ -*-===//
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
// This file implements interpreter support for code execution in WebAssembly.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_LIB_INTERPRETER_WASM_H
#define LANGUAGE_CORE_LIB_INTERPRETER_WASM_H

#ifndef __EMSCRIPTEN__
#error "This requires emscripten."
#endif // __EMSCRIPTEN__

#include "IncrementalExecutor.h"

namespace language::Core {

class WasmIncrementalExecutor : public IncrementalExecutor {
public:
  WasmIncrementalExecutor(toolchain::orc::ThreadSafeContext &TSC);

  toolchain::Error addModule(PartialTranslationUnit &PTU) override;
  toolchain::Error removeModule(PartialTranslationUnit &PTU) override;
  toolchain::Error runCtors() const override;
  toolchain::Error cleanUp() override;
  toolchain::Expected<toolchain::orc::ExecutorAddr>
  getSymbolAddress(toolchain::StringRef Name,
                   SymbolNameKind NameKind) const override;

  ~WasmIncrementalExecutor() override;
};

} // namespace language::Core

#endif // LANGUAGE_CORE_LIB_INTERPRETER_WASM_H
