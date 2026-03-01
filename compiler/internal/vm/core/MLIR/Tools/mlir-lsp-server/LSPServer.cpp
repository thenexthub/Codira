//===- LSPServer.cpp - MLIR Language Server -------------------------------===//
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
#include "MLIRServer.h"
#include "Protocol.h"
#include "vm/core/Support/LSP/Logging.h"
#include "vm/core/Support/LSP/Transport.h"
#include <optional>

#define DEBUG_TYPE "mlir-lsp-server"

using namespace mlir;
using namespace mlir::lsp;

using toolchain::lsp::Callback;
using toolchain::lsp::CodeAction;
using toolchain::lsp::CodeActionParams;
using toolchain::lsp::CompletionList;
using toolchain::lsp::CompletionParams;
using toolchain::lsp::DidChangeTextDocumentParams;
using toolchain::lsp::DidCloseTextDocumentParams;
using toolchain::lsp::DidOpenTextDocumentParams;
using toolchain::lsp::DocumentSymbol;
using toolchain::lsp::DocumentSymbolParams;
using toolchain::lsp::Hover;
using toolchain::lsp::InitializedParams;
using toolchain::lsp::InitializeParams;
using toolchain::lsp::JSONTransport;
using toolchain::lsp::Location;
using toolchain::lsp::Logger;
using toolchain::lsp::MessageHandler;
using toolchain::lsp::MLIRConvertBytecodeParams;
using toolchain::lsp::MLIRConvertBytecodeResult;
using toolchain::lsp::NoParams;
using toolchain::lsp::OutgoingNotification;
using toolchain::lsp::PublishDiagnosticsParams;
using toolchain::lsp::ReferenceParams;
using toolchain::lsp::TextDocumentPositionParams;
using toolchain::lsp::TextDocumentSyncKind;
using toolchain::lsp::URIForFile;

//===----------------------------------------------------------------------===//
// LSPServer
//===----------------------------------------------------------------------===//

namespace {
struct LSPServer {
  LSPServer(MLIRServer &server) : server(server) {}

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

  //===--------------------------------------------------------------------===//
  // Hover

  void onHover(const TextDocumentPositionParams &params,
               Callback<std::optional<Hover>> reply);

  //===--------------------------------------------------------------------===//
  // Document Symbols

  void onDocumentSymbol(const DocumentSymbolParams &params,
                        Callback<std::vector<DocumentSymbol>> reply);

  //===--------------------------------------------------------------------===//
  // Code Completion

  void onCompletion(const CompletionParams &params,
                    Callback<CompletionList> reply);

  //===--------------------------------------------------------------------===//
  // Code Action

  void onCodeAction(const CodeActionParams &params,
                    Callback<toolchain::json::Value> reply);

  //===--------------------------------------------------------------------===//
  // Bytecode

  void onConvertFromBytecode(const MLIRConvertBytecodeParams &params,
                             Callback<MLIRConvertBytecodeResult> reply);
  void onConvertToBytecode(const MLIRConvertBytecodeParams &params,
                           Callback<MLIRConvertBytecodeResult> reply);

  //===--------------------------------------------------------------------===//
  // Fields
  //===--------------------------------------------------------------------===//

  MLIRServer &server;

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
           {"change", (int)TextDocumentSyncKind::Full},
           {"save", true},
       }},
      {"completionProvider",
       toolchain::json::Object{
           {"allCommitCharacters",
            {
                "\t",
                ";",
                ",",
                ".",
                "=",
            }},
           {"resolveProvider", false},
           {"triggerCharacters",
            {".", "%", "^", "!", "#", "(", ",", "<", ":", "[", " ", "\"", "/"}},
       }},
      {"definitionProvider", true},
      {"referencesProvider", true},
      {"hoverProvider", true},

      // For now we only support documenting symbols when the client supports
      // hierarchical symbols.
      {"documentSymbolProvider",
       params.capabilities.hierarchicalDocumentSymbol},
  };

  // Per LSP, codeActionProvider can be either boolean or CodeActionOptions.
  // CodeActionOptions is only valid if the client supports action literal
  // via textDocument.codeAction.codeActionLiteralSupport.
  serverCaps["codeActionProvider"] =
      params.capabilities.codeActionStructure
          ? toolchain::json::Object{{"codeActionKinds",
                                {CodeAction::kQuickFix, CodeAction::kRefactor,
                                 CodeAction::kInfo}}}
          : toolchain::json::Value(true);

  toolchain::json::Object result{
      {{"serverInfo",
        toolchain::json::Object{{"name", "mlir-lsp-server"}, {"version", "0.0.0"}}},
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
  server.addOrUpdateDocument(params.textDocument.uri, params.textDocument.text,
                             params.textDocument.version,
                             diagParams.diagnostics);

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
  // TODO: We currently only support full document updates, we should refactor
  // to avoid this.
  if (params.contentChanges.size() != 1)
    return;
  PublishDiagnosticsParams diagParams(params.textDocument.uri,
                                      params.textDocument.version);
  server.addOrUpdateDocument(
      params.textDocument.uri, params.contentChanges.front().text,
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
// Hover
//===----------------------------------------------------------------------===//

void LSPServer::onHover(const TextDocumentPositionParams &params,
                        Callback<std::optional<Hover>> reply) {
  reply(server.findHover(params.textDocument.uri, params.position));
}

//===----------------------------------------------------------------------===//
// Document Symbols
//===----------------------------------------------------------------------===//

void LSPServer::onDocumentSymbol(const DocumentSymbolParams &params,
                                 Callback<std::vector<DocumentSymbol>> reply) {
  std::vector<DocumentSymbol> symbols;
  server.findDocumentSymbols(params.textDocument.uri, symbols);
  reply(std::move(symbols));
}

//===----------------------------------------------------------------------===//
// Code Completion
//===----------------------------------------------------------------------===//

void LSPServer::onCompletion(const CompletionParams &params,
                             Callback<CompletionList> reply) {
  reply(server.getCodeCompletion(params.textDocument.uri, params.position));
}

//===----------------------------------------------------------------------===//
// Code Action
//===----------------------------------------------------------------------===//

void LSPServer::onCodeAction(const CodeActionParams &params,
                             Callback<toolchain::json::Value> reply) {
  URIForFile uri = params.textDocument.uri;

  // Check whether a particular CodeActionKind is included in the response.
  auto isKindAllowed = [only(params.context.only)](StringRef kind) {
    if (only.empty())
      return true;
    return toolchain::any_of(only, [&](StringRef base) {
      return kind.consume_front(base) &&
             (kind.empty() || kind.starts_with("."));
    });
  };

  // We provide a code action for fixes on the specified diagnostics.
  std::vector<CodeAction> actions;
  if (isKindAllowed(CodeAction::kQuickFix))
    server.getCodeActions(uri, params.range.start, params.context, actions);
  reply(std::move(actions));
}

//===----------------------------------------------------------------------===//
// Bytecode
//===----------------------------------------------------------------------===//

void LSPServer::onConvertFromBytecode(
    const MLIRConvertBytecodeParams &params,
    Callback<MLIRConvertBytecodeResult> reply) {
  reply(server.convertFromBytecode(params.uri));
}

void LSPServer::onConvertToBytecode(const MLIRConvertBytecodeParams &params,
                                    Callback<MLIRConvertBytecodeResult> reply) {
  reply(server.convertToBytecode(params.uri));
}

//===----------------------------------------------------------------------===//
// Entry point
//===----------------------------------------------------------------------===//

LogicalResult lsp::runMlirLSPServer(MLIRServer &server,
                                    JSONTransport &transport) {
  LSPServer lspServer(server);
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

  // Hover
  messageHandler.method("textDocument/hover", &lspServer, &LSPServer::onHover);

  // Document Symbols
  messageHandler.method("textDocument/documentSymbol", &lspServer,
                        &LSPServer::onDocumentSymbol);

  // Code Completion
  messageHandler.method("textDocument/completion", &lspServer,
                        &LSPServer::onCompletion);

  // Code Action
  messageHandler.method("textDocument/codeAction", &lspServer,
                        &LSPServer::onCodeAction);

  // Bytecode
  messageHandler.method("mlir/convertFromBytecode", &lspServer,
                        &LSPServer::onConvertFromBytecode);
  messageHandler.method("mlir/convertToBytecode", &lspServer,
                        &LSPServer::onConvertToBytecode);

  // Diagnostics
  lspServer.publishDiagnostics =
      messageHandler.outgoingNotification<PublishDiagnosticsParams>(
          "textDocument/publishDiagnostics");

  // Run the main loop of the transport.
  LogicalResult result = success();
  if (toolchain::Error error = transport.run(messageHandler)) {
    Logger::error("Transport error: {0}", error);
    toolchain::consumeError(std::move(error));
    result = failure();
  } else {
    result = success(lspServer.shutdownRequestReceived);
  }
  return result;
}
