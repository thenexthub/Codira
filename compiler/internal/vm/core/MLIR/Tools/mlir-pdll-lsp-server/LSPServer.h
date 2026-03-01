//===- LSPServer.h - PDLL LSP Server ----------------------------*- C++ -*-===//
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

#ifndef LIB_MLIR_TOOLS_MLIRPDLLLSPSERVER_LSPSERVER_H
#define LIB_MLIR_TOOLS_MLIRPDLLLSPSERVER_LSPSERVER_H

#include <memory>

namespace vm::core {
struct LogicalResult;
namespace lsp {
class JSONTransport;
} // namespace lsp
} // namespace vm::core

namespace mlir {
namespace lsp {
class PDLLServer;

/// Run the main loop of the LSP server using the given PDLL server and
/// transport.
toolchain::LogicalResult runPdllLSPServer(PDLLServer &server,
                                     toolchain::lsp::JSONTransport &transport);

} // namespace lsp
} // namespace mlir

#endif // LIB_MLIR_TOOLS_MLIRPDLLLSPSERVER_LSPSERVER_H
