//===- TableGenServer.h - TableGen Language Server --------------*- C++ -*-===//
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

#ifndef LIB_MLIR_TOOLS_TBLGENLSPSERVER_TABLEGENSERVER_H_
#define LIB_MLIR_TOOLS_TBLGENLSPSERVER_TABLEGENSERVER_H_

#include "mlir/Support/LLVM.h"
#include "vm/core/ADT/StringRef.h"
#include "vm/core/Support/LSP/Protocol.h"
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace mlir {
namespace lsp {
using toolchain::lsp::Diagnostic;
using toolchain::lsp::DocumentLink;
using toolchain::lsp::Hover;
using toolchain::lsp::Location;
using toolchain::lsp::Position;
using toolchain::lsp::TextDocumentContentChangeEvent;
using toolchain::lsp::URIForFile;

/// This class implements all of the TableGen related functionality necessary
/// for a language server. This class allows for keeping the TableGen specific
/// logic separate from the logic that involves LSP server/client communication.
class TableGenServer {
public:
  struct Options {
    Options(const std::vector<std::string> &compilationDatabases,
            const std::vector<std::string> &extraDirs)
        : compilationDatabases(compilationDatabases), extraDirs(extraDirs) {}

    /// The filenames for databases containing compilation commands for TableGen
    /// files passed to the server.
    const std::vector<std::string> &compilationDatabases;

    /// Additional list of include directories to search.
    const std::vector<std::string> &extraDirs;
  };

  TableGenServer(const Options &options);
  ~TableGenServer();

  /// Add the document, with the provided `version`, at the given URI. Any
  /// diagnostics emitted for this document should be added to `diagnostics`.
  void addDocument(const URIForFile &uri, StringRef contents, int64_t version,
                   std::vector<Diagnostic> &diagnostics);

  /// Update the document, with the provided `version`, at the given URI. Any
  /// diagnostics emitted for this document should be added to `diagnostics`.
  void updateDocument(const URIForFile &uri,
                      ArrayRef<TextDocumentContentChangeEvent> changes,
                      int64_t version, std::vector<Diagnostic> &diagnostics);

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

  /// Return the document links referenced by the given file.
  void getDocumentLinks(const URIForFile &uri,
                        std::vector<DocumentLink> &documentLinks);

  /// Find a hover description for the given hover position, or std::nullopt if
  /// one couldn't be found.
  std::optional<Hover> findHover(const URIForFile &uri,
                                 const Position &hoverPos);

private:
  struct Impl;
  std::unique_ptr<Impl> impl;
};

} // namespace lsp
} // namespace mlir

#endif // LIB_MLIR_TOOLS_TBLGENLSPSERVER_TABLEGENSERVER_H_
