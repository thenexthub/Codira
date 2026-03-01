//===- mlir-reduce.cpp - The MLIR reducer ---------------------------------===//
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
// This file implements the general framework of the MLIR reducer tool. It
// parses the command line arguments, parses the initial MLIR test case and sets
// up the testing environment. It  outputs the most reduced test case variant
// after executing the reduction passes.
//
//===----------------------------------------------------------------------===//

#include "mlir/Tools/mlir-reduce/MlirReduceMain.h"
#include "mlir/Parser/Parser.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Reducer/Passes.h"
#include "mlir/Support/FileUtilities.h"
#include "mlir/Tools/ParseUtilities.h"
#include "vm/core/Support/InitLLVM.h"
#include "vm/core/Support/SourceMgr.h"
#include "vm/core/Support/ToolOutputFile.h"

using namespace mlir;

// Parse and verify the input MLIR file. Returns null on error.
static OwningOpRef<Operation *> loadModule(MLIRContext &context,
                                           StringRef inputFilename,
                                           bool insertImplictModule) {
  // Set up the input file.
  std::string errorMessage;
  auto file = openInputFile(inputFilename, &errorMessage);
  if (!file) {
    toolchain::errs() << errorMessage << "\n";
    return nullptr;
  }

  auto sourceMgr = std::make_shared<toolchain::SourceMgr>();
  sourceMgr->AddNewSourceBuffer(std::move(file), SMLoc());
  return parseSourceFileForTool(sourceMgr, &context, insertImplictModule);
}

LogicalResult mlir::mlirReduceMain(int argc, char **argv,
                                   MLIRContext &context) {
  // Override the default '-h' and use the default PrintHelpMessage() which
  // won't print options in categories.
  static toolchain::cl::opt<bool> help("h", toolchain::cl::desc("Alias for -help"),
                                  toolchain::cl::Hidden);

  static toolchain::cl::OptionCategory mlirReduceCategory("mlir-reduce options");

  static toolchain::cl::opt<std::string> inputFilename(
      toolchain::cl::Positional, toolchain::cl::desc("<input file>"),
      toolchain::cl::cat(mlirReduceCategory));

  static toolchain::cl::opt<std::string> outputFilename(
      "o", toolchain::cl::desc("Output filename for the reduced test case"),
      toolchain::cl::init("-"), toolchain::cl::cat(mlirReduceCategory));

  static toolchain::cl::opt<bool> noImplicitModule{
      "no-implicit-module",
      toolchain::cl::desc(
          "Disable implicit addition of a top-level module op during parsing"),
      toolchain::cl::init(false)};

  static toolchain::cl::opt<bool> allowUnregisteredDialects(
      "allow-unregistered-dialect",
      toolchain::cl::desc("Allow operation with no registered dialects"),
      toolchain::cl::init(false));

  toolchain::cl::HideUnrelatedOptions(mlirReduceCategory);

  toolchain::InitLLVM y(argc, argv);

  registerReducerPasses();

  PassPipelineCLParser parser("", "Reduction Passes to Run");
  toolchain::cl::ParseCommandLineOptions(argc, argv,
                                    "MLIR test case reduction tool.\n");

  if (help) {
    toolchain::cl::PrintHelpMessage();
    return success();
  }
  if (allowUnregisteredDialects)
    context.allowUnregisteredDialects();

  std::string errorMessage;

  auto output = openOutputFile(outputFilename, &errorMessage);
  if (!output)
    return failure();

  OwningOpRef<Operation *> opRef =
      loadModule(context, inputFilename, !noImplicitModule);
  if (!opRef)
    return failure();

  auto errorHandler = [&](const Twine &msg) {
    return emitError(UnknownLoc::get(&context)) << msg;
  };

  // Reduction pass pipeline.
  PassManager pm(&context, opRef.get()->getName().getStringRef());
  if (failed(parser.addToPipeline(pm, errorHandler)))
    return failure();

  OwningOpRef<Operation *> op = opRef.get()->clone();

  if (failed(pm.run(op.get())))
    return failure();

  op.get()->print(output->os());
  output->keep();

  return success();
}
