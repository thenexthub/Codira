//===- MlirQueryMain.cpp - MLIR Query main --------------------------------===//
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
// This file implements the general framework of the MLIR query tool. It
// parses the command line arguments, parses the MLIR file and outputs the query
// results.
//
//===----------------------------------------------------------------------===//

#include "mlir/Tools/mlir-query/MlirQueryMain.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Parser/Parser.h"
#include "mlir/Query/Query.h"
#include "mlir/Query/QuerySession.h"
#include "mlir/Support/FileUtilities.h"
#include "vm/core/LineEditor/LineEditor.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/InitLLVM.h"
#include "vm/core/Support/Process.h"
#include "vm/core/Support/SourceMgr.h"

//===----------------------------------------------------------------------===//
// Query Parser
//===----------------------------------------------------------------------===//

toolchain::LogicalResult
mlir::mlirQueryMain(int argc, char **argv, MLIRContext &context,
                    const mlir::query::matcher::Registry &matcherRegistry) {

  // Override the default '-h' and use the default PrintHelpMessage() which
  // won't print options in categories.
  static toolchain::cl::opt<bool> help("h", toolchain::cl::desc("Alias for -help"),
                                  toolchain::cl::Hidden);

  static toolchain::cl::OptionCategory mlirQueryCategory("mlir-query options");

  static toolchain::cl::list<std::string> commands(
      "c", toolchain::cl::desc("Specify command to run"),
      toolchain::cl::value_desc("command"), toolchain::cl::cat(mlirQueryCategory));

  static toolchain::cl::opt<std::string> inputFilename(
      toolchain::cl::Positional, toolchain::cl::desc("<input file>"), toolchain::cl::init("-"),
      toolchain::cl::cat(mlirQueryCategory));

  static toolchain::cl::opt<bool> noImplicitModule{
      "no-implicit-module",
      toolchain::cl::desc(
          "Disable implicit addition of a top-level module op during parsing"),
      toolchain::cl::init(false)};

  static toolchain::cl::opt<bool> allowUnregisteredDialects(
      "allow-unregistered-dialect",
      toolchain::cl::desc("Allow operation with no registered dialects"),
      toolchain::cl::init(false));

  toolchain::cl::HideUnrelatedOptions(mlirQueryCategory);

  toolchain::InitLLVM y(argc, argv);

  toolchain::cl::ParseCommandLineOptions(argc, argv, "MLIR test case query tool.\n");

  if (help) {
    toolchain::cl::PrintHelpMessage();
    return mlir::success();
  }

  // When reading from stdin and the input is a tty, it is often a user mistake
  // and the process "appears to be stuck". Print a message to let the user
  // know!
  if (inputFilename == "-" &&
      toolchain::sys::Process::FileDescriptorIsDisplayed(fileno(stdin)))
    toolchain::errs() << "(processing input from stdin now, hit ctrl-c/ctrl-d to "
                    "interrupt)\n";

  // Set up the input file.
  std::string errorMessage;
  auto file = openInputFile(inputFilename, &errorMessage);
  if (!file) {
    toolchain::errs() << errorMessage << "\n";
    return mlir::failure();
  }

  auto sourceMgr = toolchain::SourceMgr();
  auto bufferId = sourceMgr.AddNewSourceBuffer(std::move(file), SMLoc());

  context.allowUnregisteredDialects(allowUnregisteredDialects);

  // Parse the input MLIR file.
  OwningOpRef<Operation *> opRef =
      noImplicitModule ? parseSourceFile(sourceMgr, &context)
                       : parseSourceFile<mlir::ModuleOp>(sourceMgr, &context);
  if (!opRef)
    return mlir::failure();

  mlir::query::QuerySession qs(opRef.get(), sourceMgr, bufferId,
                               matcherRegistry);
  if (!commands.empty()) {
    for (auto &command : commands) {
      mlir::query::QueryRef queryRef = mlir::query::parse(command, qs);
      if (mlir::failed(queryRef->run(toolchain::outs(), qs)))
        return mlir::failure();
    }
  } else {
    toolchain::LineEditor le("mlir-query");
    le.setListCompleter([&qs](toolchain::StringRef line, size_t pos) {
      return mlir::query::complete(line, pos, qs);
    });
    while (std::optional<std::string> line = le.readLine()) {
      mlir::query::QueryRef queryRef = mlir::query::parse(*line, qs);
      (void)queryRef->run(toolchain::outs(), qs);
      toolchain::outs().flush();
      if (qs.terminate)
        break;
    }
  }

  return mlir::success();
}
