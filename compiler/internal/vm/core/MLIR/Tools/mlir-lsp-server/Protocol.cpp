//===--- Protocol.cpp - Language Server Protocol Implementation -----------===//
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
//
// This file contains the serialization code for the MLIR specific LSP structs.
//
//===----------------------------------------------------------------------===//

#include "Protocol.h"
#include "vm/core/Support/JSON.h"

//===----------------------------------------------------------------------===//
// MLIRConvertBytecodeParams
//===----------------------------------------------------------------------===//

bool toolchain::lsp::fromJSON(const toolchain::json::Value &value,
                         MLIRConvertBytecodeParams &result,
                         toolchain::json::Path path) {
  toolchain::json::ObjectMapper o(value, path);
  return o && o.map("uri", result.uri);
}

//===----------------------------------------------------------------------===//
// MLIRConvertBytecodeResult
//===----------------------------------------------------------------------===//

toolchain::json::Value toolchain::lsp::toJSON(const MLIRConvertBytecodeResult &value) {
  return toolchain::json::Object{{"output", value.output}};
}
