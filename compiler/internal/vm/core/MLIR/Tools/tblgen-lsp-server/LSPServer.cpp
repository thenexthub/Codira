//===- LSPServer.cpp - TableGen Language Server ---------------------------===//
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

#include "LSPServer.h"

#include "TableGenServer.h"
#include "vm/core/Support/LSP/Logging.h"
#include "vm/core/Support/LSP/Protocol.h"
#include "vm/core/Support/LSP/Transport.h"
#include <optional>

using namespace mlir;
using namespace mlir::lsp;

using toolchain::lsp::Callback;
using toolchain::lsp::DidChangeTextDocumentParams;
using toolchain::lsp::DidCloseTextDocumentParams;
using toolchain::lsp::DidOpenTextDocumentParams;
using toolchain::lsp::DocumentLinkParams;
using toolchain::lsp::Hover;
using toolchain::lsp::InitializedParams;
using toolchain::lsp::InitializeParams;
using toolchain::lsp::JSONTransport;
using toolchain::lsp::Location;
using toolchain::lsp::Logger;
using toolchain::lsp::MessageHandler;
using toolchain::lsp::NoParams;
using toolchain::lsp::OutgoingNotification;
using toolchain::lsp::PublishDiagnosticsParams;
using toolchain::lsp::ReferenceParams;
using toolchain::lsp::TextDocumentPositionParams;
using toolchain::lsp::TextDocumentSyncKind;

//===----------------------------------------------------------------------===//
// LSPServer
//===----------------------------------------------------------------------===//

namespace {
struct LSPServer {
  LSPServer(TableGenServer &server, JSONTransport &transport)
      : server(server), transport(transport) {}

  //===--------------------------------------------------------------------===//
  // Initialization

  void onInitialize(const InitializeParams &params,
                    Callback<toolchain::json::Value> reply);
  void onInitialized(const InitializedParams &params);
  void onShutdown(const NoParams &params, Callback<std::nullptr_t> reply);

  //===--------------------------------------------------------------------===//
  // Document Change

  void onDocumentDidOpen(const DidOpenTextDocumentParams &params);
  void onDocumentDidClose(const DidCloseTextDocumentParams &params);
  void onDocumentDidChange(const DidChangeTextDocumentParams &params);

  //===--------------------------------------------------------------------===//
  // Definitions and References

  void onGoToDefinition(const TextDocumentPositionParams &params,
                        Callback<std::vector<Location>> reply);
  void onReference(const ReferenceParams &params,
                   Callback<std::vector<Location>> reply);

  //===----------------------------------------------------------------------===//
  // DocumentLink

  void onDocumentLink(const DocumentLinkParams &params,
                      Callback<std::vector<DocumentLink>> reply);

  //===--------------------------------------------------------------------===//
  // Hover

  void onHover(const TextDocumentPositionParams &params,
               Callback<std::optional<Hover>> reply);

  //===--------------------------------------------------------------------===//
  // Fields
  //===--------------------------------------------------------------------===//

  TableGenServer &server;
  JSONTransport &transport;

  /// An outgoing notification used to send diagnostics to the client when they
  /// are ready to be processed.
  OutgoingNotification<PublishDiagnosticsParams> publishDiagnostics;

  /// Used to indicate that the 'shutdown' request was received from the
  /// Language Server client.
  bool shutdownRequestReceived = false;
};
} // namespace

//===----------------------------------------------------------------------===//
// Initialization
//===----------------------------------------------------------------------===//

void LSPServer::onInitialize(const InitializeParams &params,
                             Callback<toolchain::json::Value> reply) {
  // Send a response with the capabilities of this server.
  toolchain::json::Object serverCaps{
      {"textDocumentSync",
       toolchain::json::Object{
           {"openClose", true},
           {"change", (int)TextDocumentSyncKind::Incremental},
           {"save", true},
       }},
      {"definitionProvider", true},
      {"referencesProvider", true},
      {"documentLinkProvider",
       toolchain::json::Object{
           {"resolveProvider", false},
       }},
      {"hoverProvider", true},
  };

  toolchain::json::Object result{
      {{"serverInfo", toolchain::json::Object{{"name", "tblgen-lsp-server"},
                                         {"version", "0.0.1"}}},
       {"capabilities", std::move(serverCaps)}}};
  reply(std::move(result));
}
void LSPServer::onInitialized(const InitializedParams &) {}
void LSPServer::onShutdown(const NoParams &, Callback<std::nullptr_t> reply) {
  shutdownRequestReceived = true;
  reply(nullptr);
}

//===----------------------------------------------------------------------===//
// Document Change
//===----------------------------------------------------------------------===//

void LSPServer::onDocumentDidOpen(const DidOpenTextDocumentParams &params) {
  PublishDiagnosticsParams diagParams(params.textDocument.uri,
                                      params.textDocument.version);
  server.addDocument(params.textDocument.uri, params.textDocument.text,
                     params.textDocument.version, diagParams.diagnostics);

  // Publish any recorded diagnostics.
  publishDiagnostics(diagParams);
}
void LSPServer::onDocumentDidClose(const DidCloseTextDocumentParams &params) {
  std::optional<int64_t> version =
      server.removeDocument(params.textDocument.uri);
  if (!version)
    return;

  // Empty out the diagnostics shown for this document. This will clear out
  // anything currently displayed by the client for this document (e.g. in the
  // "Problems" pane of VSCode).
  publishDiagnostics(
      PublishDiagnosticsParams(params.textDocument.uri, *version));
}
void LSPServer::onDocumentDidChange(const DidChangeTextDocumentParams &params) {
  PublishDiagnosticsParams diagParams(params.textDocument.uri,
                                      params.textDocument.version);
  server.updateDocument(params.textDocument.uri, params.contentChanges,
                        params.textDocument.version, diagParams.diagnostics);

  // Publish any recorded diagnostics.
  publishDiagnostics(diagParams);
}

//===----------------------------------------------------------------------===//
// Definitions and References
//===----------------------------------------------------------------------===//

void LSPServer::onGoToDefinition(const TextDocumentPositionParams &params,
                                 Callback<std::vector<Location>> reply) {
  std::vector<Location> locations;
  server.getLocationsOf(params.textDocument.uri, params.position, locations);
  reply(std::move(locations));
}

void LSPServer::onReference(const ReferenceParams &params,
                            Callback<std::vector<Location>> reply) {
  std::vector<Location> locations;
  server.findReferencesOf(params.textDocument.uri, params.position, locations);
  reply(std::move(locations));
}

//===----------------------------------------------------------------------===//
// DocumentLink
//===----------------------------------------------------------------------===//

void LSPServer::onDocumentLink(const DocumentLinkParams &params,
                               Callback<std::vector<DocumentLink>> reply) {
  std::vector<DocumentLink> links;
  server.getDocumentLinks(params.textDocument.uri, links);
  reply(std::move(links));
}

//===----------------------------------------------------------------------===//
// Hover
//===----------------------------------------------------------------------===//

void LSPServer::onHover(const TextDocumentPositionParams &params,
                        Callback<std::optional<Hover>> reply) {
  reply(server.findHover(params.textDocument.uri, params.position));
}

//===----------------------------------------------------------------------===//
// Entry Point
//===----------------------------------------------------------------------===//

LogicalResult mlir::lsp::runTableGenLSPServer(TableGenServer &server,
                                              JSONTransport &transport) {
  LSPServer lspServer(server, transport);
  MessageHandler messageHandler(transport);

  // Initialization
  messageHandler.method("initialize", &lspServer, &LSPServer::onInitialize);
  messageHandler.notification("initialized", &lspServer,
                              &LSPServer::onInitialized);
  messageHandler.method("shutdown", &lspServer, &LSPServer::onShutdown);

  // Document Changes
  messageHandler.notification("textDocument/didOpen", &lspServer,
                              &LSPServer::onDocumentDidOpen);
  messageHandler.notification("textDocument/didClose", &lspServer,
                              &LSPServer::onDocumentDidClose);
  messageHandler.notification("textDocument/didChange", &lspServer,
                              &LSPServer::onDocumentDidChange);

  // Definitions and References
  messageHandler.method("textDocument/definition", &lspServer,
                        &LSPServer::onGoToDefinition);
  messageHandler.method("textDocument/references", &lspServer,
                        &LSPServer::onReference);

  // Document Link
  messageHandler.method("textDocument/documentLink", &lspServer,
                        &LSPServer::onDocumentLink);

  // Hover
  messageHandler.method("textDocument/hover", &lspServer, &LSPServer::onHover);

  // Diagnostics
  lspServer.publishDiagnostics =
      messageHandler.outgoingNotification<PublishDiagnosticsParams>(
          "textDocument/publishDiagnostics");

  // Run the main loop of the transport.
  if (toolchain::Error error = transport.run(messageHandler)) {
    Logger::error("Transport error: {0}", error);
    toolchain::consumeError(std::move(error));
    return failure();
  }
  return success(lspServer.shutdownRequestReceived);
}
