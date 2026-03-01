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
// This file contains the serialization code for the PDLL specific LSP structs.
//
//===----------------------------------------------------------------------===//

#include "Protocol.h"
#include "mlir/Support/LLVM.h"
#include "vm/core/Support/ErrorHandling.h"
#include "vm/core/Support/JSON.h"

using namespace mlir;
using namespace mlir::lsp;

// Helper that doesn't treat `null` and absent fields as failures.
template <typename T>
static bool mapOptOrNull(const toolchain::json::Value &params,
                         toolchain::StringLiteral prop, T &out,
                         toolchain::json::Path path) {
  const toolchain::json::Object *o = params.getAsObject();
  assert(o);

  // Field is missing or null.
  auto *v = o->get(prop);
  if (!v || v->getAsNull())
    return true;
  return fromJSON(*v, out, path.field(prop));
}

//===----------------------------------------------------------------------===//
// PDLLViewOutputParams
//===----------------------------------------------------------------------===//

bool mlir::lsp::fromJSON(const toolchain::json::Value &value,
                         PDLLViewOutputKind &result, toolchain::json::Path path) {
  if (std::optional<StringRef> str = value.getAsString()) {
    if (*str == "ast") {
      result = PDLLViewOutputKind::AST;
      return true;
    }
    if (*str == "mlir") {
      result = PDLLViewOutputKind::MLIR;
      return true;
    }
    if (*str == "cpp") {
      result = PDLLViewOutputKind::CPP;
      return true;
    }
  }
  return false;
}

bool mlir::lsp::fromJSON(const toolchain::json::Value &value,
                         PDLLViewOutputParams &result, toolchain::json::Path path) {
  toolchain::json::ObjectMapper o(value, path);
  return o && o.map("uri", result.uri) && o.map("kind", result.kind);
}

//===----------------------------------------------------------------------===//
// PDLLViewOutputResult
//===----------------------------------------------------------------------===//

toolchain::json::Value mlir::lsp::toJSON(const PDLLViewOutputResult &value) {
  return toolchain::json::Object{{"output", value.output}};
}
