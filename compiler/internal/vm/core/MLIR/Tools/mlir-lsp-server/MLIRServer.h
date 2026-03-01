//===- MLIRServer.h - MLIR General Language Server --------------*- C++ -*-===//
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

#ifndef LIB_MLIR_TOOLS_MLIRLSPSERVER_SERVER_H_
#define LIB_MLIR_TOOLS_MLIRLSPSERVER_SERVER_H_

#include "Protocol.h"
#include "mlir/Support/LLVM.h"
#include "mlir/Tools/mlir-lsp-server/MlirLspRegistryFunction.h"
#include "vm/core/Support/Error.h"
#include <memory>
#include <optional>

namespace mlir {
class DialectRegistry;

namespace lsp {
using toolchain::lsp::CodeAction;
using toolchain::lsp::CodeActionContext;
using toolchain::lsp::CompletionList;
using toolchain::lsp::Diagnostic;
using toolchain::lsp::DocumentSymbol;
using toolchain::lsp::Hover;
using toolchain::lsp::Location;
using toolchain::lsp::MLIRConvertBytecodeResult;
using toolchain::lsp::Position;
using toolchain::lsp::Range;
using toolchain::lsp::URIForFile;

/// This class implements all of the MLIR related functionality necessary for a
/// language server. This class allows for keeping the MLIR specific logic
/// separate from the logic that involves LSP server/client communication.
class MLIRServer {
public:
  /// Construct a new server with the given dialect registry function.
  MLIRServer(DialectRegistryFn registry_fn);
  ~MLIRServer();

  /// Add or update the document, with the provided `version`, at the given URI.
  /// Any diagnostics emitted for this document should be added to
  /// `diagnostics`.
  void addOrUpdateDocument(const URIForFile &uri, StringRef contents,
                           int64_t version,
                           std::vector<Diagnostic> &diagnostics);

  /// Remove the document with the given uri. Returns the version of the removed
  /// document, or std::nullopt if the uri did not have a corresponding document
  /// within the server.
  std::optional<int64_t> removeDocument(const URIForFile &uri);

  /// Return the locations of the object pointed at by the given position.
  void getLocationsOf(const URIForFile &uri, const Position &defPos,
                      std::vector<Location> &locations);

  /// Find all references of the object pointed at by the given position.
  void findReferencesOf(const URIForFile &uri, const Position &pos,
                        std::vector<Location> &references);

  /// Find a hover description for the given hover position, or std::nullopt if
  /// one couldn't be found.
  std::optional<Hover> findHover(const URIForFile &uri,
                                 const Position &hoverPos);

  /// Find all of the document symbols within the given file.
  void findDocumentSymbols(const URIForFile &uri,
                           std::vector<DocumentSymbol> &symbols);

  /// Get the code completion list for the position within the given file.
  CompletionList getCodeCompletion(const URIForFile &uri,
                                   const Position &completePos);

  /// Get the set of code actions within the file.
  void getCodeActions(const URIForFile &uri, const Range &pos,
                      const CodeActionContext &context,
                      std::vector<CodeAction> &actions);

  /// Convert the given bytecode file to the textual format.
  toolchain::Expected<MLIRConvertBytecodeResult>
  convertFromBytecode(const URIForFile &uri);

  /// Convert the given textual file to the bytecode format.
  toolchain::Expected<MLIRConvertBytecodeResult>
  convertToBytecode(const URIForFile &uri);

private:
  struct Impl;

  std::unique_ptr<Impl> impl;
};

} // namespace lsp
} // namespace mlir

#endif // LIB_MLIR_TOOLS_MLIRLSPSERVER_SERVER_H_
