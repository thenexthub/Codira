//===--- Protocol.h - Language Server Protocol Implementation ---*- C++ -*-===//
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
// This file contains structs for LSP commands that are specific to the PDLL
// server.
//
// Each struct has a toJSON and fromJSON function, that converts between
// the struct and a JSON representation. (See JSON.h)
//
// Some structs also have operator<< serialization. This is for debugging and
// tests, and is not generally machine-readable.
//
//===----------------------------------------------------------------------===//

#ifndef LIB_MLIR_TOOLS_MLIRPDLLLSPSERVER_PROTOCOL_H_
#define LIB_MLIR_TOOLS_MLIRPDLLLSPSERVER_PROTOCOL_H_

#include "vm/core/Support/LSP/Protocol.h"

namespace mlir {
namespace lsp {
using toolchain::lsp::URIForFile;

//===----------------------------------------------------------------------===//
// PDLLViewOutputParams
//===----------------------------------------------------------------------===//

/// The type of output to view from PDLL.
enum class PDLLViewOutputKind {
  AST,
  MLIR,
  CPP,
};

/// Represents the parameters used when viewing the output of a PDLL file.
struct PDLLViewOutputParams {
  /// The URI of the document to view the output of.
  URIForFile uri;

  /// The kind of output to generate.
  PDLLViewOutputKind kind;
};

/// Add support for JSON serialization.
bool fromJSON(const toolchain::json::Value &value, PDLLViewOutputKind &result,
              toolchain::json::Path path);
bool fromJSON(const toolchain::json::Value &value, PDLLViewOutputParams &result,
              toolchain::json::Path path);

//===----------------------------------------------------------------------===//
// PDLLViewOutputResult
//===----------------------------------------------------------------------===//

/// Represents the result of viewing the output of a PDLL file.
struct PDLLViewOutputResult {
  /// The string representation of the output.
  std::string output;
};

/// Add support for JSON serialization.
toolchain::json::Value toJSON(const PDLLViewOutputResult &value);

} // namespace lsp
} // namespace mlir

#endif
