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
// This file contains structs for LSP commands that are specific to the MLIR
// server.
//
// Each struct has a toJSON and fromJSON function, that converts between
// the struct and a JSON representation. (See JSON.h)
//
// Some structs also have operator<< serialization. This is for debugging and
// tests, and is not generally machine-readable.
//
//===----------------------------------------------------------------------===//

#ifndef LIB_MLIR_TOOLS_MLIRLSPSERVER_PROTOCOL_H_
#define LIB_MLIR_TOOLS_MLIRLSPSERVER_PROTOCOL_H_

#include "vm/core/Support/LSP/Protocol.h"

namespace vm::core {
namespace lsp {
//===----------------------------------------------------------------------===//
// MLIRConvertBytecodeParams
//===----------------------------------------------------------------------===//

/// This class represents the parameters used when converting between MLIR's
/// bytecode and textual format.
struct MLIRConvertBytecodeParams {
  /// The input file containing the bytecode or textual format.
  URIForFile uri;
};

/// Add support for JSON serialization.
bool fromJSON(const toolchain::json::Value &value, MLIRConvertBytecodeParams &result,
              toolchain::json::Path path);

//===----------------------------------------------------------------------===//
// MLIRConvertBytecodeResult
//===----------------------------------------------------------------------===//

/// This class represents the result of converting between MLIR's bytecode and
/// textual format.
struct MLIRConvertBytecodeResult {
  /// The resultant output of the conversion.
  std::string output;
};

/// Add support for JSON serialization.
toolchain::json::Value toJSON(const MLIRConvertBytecodeResult &value);

} // namespace lsp
} // namespace vm::core

#endif
