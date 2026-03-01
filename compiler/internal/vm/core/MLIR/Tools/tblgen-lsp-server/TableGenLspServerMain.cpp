//===- TableGenLspServerMain.cpp - TableGen Language Server main ----------===//
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

#include "mlir/Tools/tblgen-lsp-server/TableGenLspServerMain.h"
#include "LSPServer.h"
#include "TableGenServer.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/LSP/Logging.h"
#include "vm/core/Support/LSP/Transport.h"
#include "vm/core/Support/Program.h"

using namespace mlir;
using namespace mlir::lsp;

using toolchain::lsp::JSONStreamStyle;
using toolchain::lsp::JSONTransport;
using toolchain::lsp::Logger;

LogicalResult mlir::TableGenLspServerMain(int argc, char **argv) {
  toolchain::cl::opt<JSONStreamStyle> inputStyle{
      "input-style",
      toolchain::cl::desc("Input JSON stream encoding"),
      toolchain::cl::values(clEnumValN(JSONStreamStyle::Standard, "standard",
                                  "usual LSP protocol"),
                       clEnumValN(JSONStreamStyle::Delimited, "delimited",
                                  "messages delimited by `// -----` lines, "
                                  "with // comment support")),
      toolchain::cl::init(JSONStreamStyle::Standard),
      toolchain::cl::Hidden,
  };
  toolchain::cl::opt<bool> litTest{
      "lit-test",
      toolchain::cl::desc(
          "Abbreviation for -input-style=delimited -pretty -log=verbose. "
          "Intended to simplify lit tests"),
      toolchain::cl::init(false),
  };
  toolchain::cl::opt<Logger::Level> logLevel{
      "log",
      toolchain::cl::desc("Verbosity of log messages written to stderr"),
      toolchain::cl::values(
          clEnumValN(Logger::Level::Error, "error", "Error messages only"),
          clEnumValN(Logger::Level::Info, "info",
                     "High level execution tracing"),
          clEnumValN(Logger::Level::Debug, "verbose", "Low level details")),
      toolchain::cl::init(Logger::Level::Info),
  };
  toolchain::cl::opt<bool> prettyPrint{
      "pretty",
      toolchain::cl::desc("Pretty-print JSON output"),
      toolchain::cl::init(false),
  };
  toolchain::cl::list<std::string> extraIncludeDirs(
      "tablegen-extra-dir", toolchain::cl::desc("Extra directory of include files"),
      toolchain::cl::value_desc("directory"), toolchain::cl::Prefix);
  toolchain::cl::list<std::string> compilationDatabases(
      "tablegen-compilation-database",
      toolchain::cl::desc("Compilation YAML databases containing additional "
                     "compilation information for .td files"));

  toolchain::cl::ParseCommandLineOptions(argc, argv, "TableGen LSP Language Server");

  if (litTest) {
    inputStyle = JSONStreamStyle::Delimited;
    logLevel = Logger::Level::Debug;
    prettyPrint = true;
  }

  // Configure the logger.
  Logger::setLogLevel(logLevel);

  // Configure the transport used for communication.
  toolchain::sys::ChangeStdinToBinary();
  JSONTransport transport(stdin, toolchain::outs(), inputStyle, prettyPrint);

  // Configure the servers and start the main language server.
  TableGenServer::Options options(compilationDatabases, extraIncludeDirs);
  TableGenServer server(options);
  return runTableGenLSPServer(server, transport);
}
