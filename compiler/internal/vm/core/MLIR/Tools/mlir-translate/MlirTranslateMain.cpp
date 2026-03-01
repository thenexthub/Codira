//===- MlirTranslateMain.cpp - MLIR Translation entry point ---------------===//
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

#include "mlir/Tools/mlir-translate/MlirTranslateMain.h"
#include "mlir/IR/AsmState.h"
#include "mlir/Parser/Parser.h"
#include "mlir/Support/FileUtilities.h"
#include "mlir/Support/Timing.h"
#include "mlir/Support/ToolUtilities.h"
#include "mlir/Tools/mlir-translate/Translation.h"
#include "vm/core/Support/InitLLVM.h"
#include "vm/core/Support/SourceMgr.h"
#include "vm/core/Support/ToolOutputFile.h"

using namespace mlir;

//===----------------------------------------------------------------------===//
// Diagnostic Filter
//===----------------------------------------------------------------------===//

namespace {
/// A scoped diagnostic handler that marks non-error diagnostics as handled. As
/// a result, the main diagnostic handler does not print non-error diagnostics.
class ErrorDiagnosticFilter : public ScopedDiagnosticHandler {
public:
  ErrorDiagnosticFilter(MLIRContext *ctx) : ScopedDiagnosticHandler(ctx) {
    setHandler([](Diagnostic &diag) {
      if (diag.getSeverity() != DiagnosticSeverity::Error)
        return success();
      return failure();
    });
  }
};
} // namespace

//===----------------------------------------------------------------------===//
// Translate Entry Point
//===----------------------------------------------------------------------===//

LogicalResult mlir::mlirTranslateMain(int argc, char **argv,
                                      toolchain::StringRef toolName) {

  static toolchain::cl::opt<std::string> inputFilename(
      toolchain::cl::Positional, toolchain::cl::desc("<input file>"),
      toolchain::cl::init("-"));

  static toolchain::cl::opt<std::string> outputFilename(
      "o", toolchain::cl::desc("Output filename"), toolchain::cl::value_desc("filename"),
      toolchain::cl::init("-"));

  static toolchain::cl::opt<bool> allowUnregisteredDialects(
      "allow-unregistered-dialect",
      toolchain::cl::desc("Allow operation with no registered dialects (discouraged: testing only!)"),
      toolchain::cl::init(false));

  static toolchain::cl::opt<std::string> inputSplitMarker{
      "split-input-file", toolchain::cl::ValueOptional,
      toolchain::cl::callback([&](const std::string &str) {
        // Implicit value: use default marker if flag was used without value.
        if (str.empty())
          inputSplitMarker.setValue(kDefaultSplitMarker);
      }),
      toolchain::cl::desc("Split the input file into chunks using the given or "
                     "default marker and process each chunk independently"),
      toolchain::cl::init("")};

  static toolchain::cl::opt<SourceMgrDiagnosticVerifierHandler::Level>
      verifyDiagnostics{
          "verify-diagnostics", toolchain::cl::ValueOptional,
          toolchain::cl::desc("Check that emitted diagnostics match expected-* "
                         "lines on the corresponding line"),
          toolchain::cl::values(
              clEnumValN(
                  SourceMgrDiagnosticVerifierHandler::Level::All, "all",
                  "Check all diagnostics (expected, unexpected, near-misses)"),
              // Implicit value: when passed with no arguments, e.g.
              // `--verify-diagnostics` or `--verify-diagnostics=`.
              clEnumValN(
                  SourceMgrDiagnosticVerifierHandler::Level::All, "",
                  "Check all diagnostics (expected, unexpected, near-misses)"),
              clEnumValN(
                  SourceMgrDiagnosticVerifierHandler::Level::OnlyExpected,
                  "only-expected", "Check only expected diagnostics"))};

  static toolchain::cl::opt<bool> errorDiagnosticsOnly(
      "error-diagnostics-only",
      toolchain::cl::desc("Filter all non-error diagnostics "
                     "(discouraged: testing only!)"),
      toolchain::cl::init(false));

  static toolchain::cl::opt<std::string> outputSplitMarker(
      "output-split-marker",
      toolchain::cl::desc("Split marker to use for merging the ouput"),
      toolchain::cl::init(""));

  toolchain::InitLLVM y(argc, argv);

  // Add flags for all the registered translations.
  toolchain::cl::list<const Translation *, bool, TranslationParser>
      translationsRequested("", toolchain::cl::desc("Translations to perform"),
                            toolchain::cl::Required);
  registerAsmPrinterCLOptions();
  registerMLIRContextCLOptions();
  registerTranslationCLOptions();
  registerDefaultTimingManagerCLOptions();
  toolchain::cl::ParseCommandLineOptions(argc, argv, toolName);

  // Initialize the timing manager.
  DefaultTimingManager tm;
  applyDefaultTimingManagerCLOptions(tm);
  TimingScope timing = tm.getRootScope();

  std::string errorMessage;
  std::unique_ptr<toolchain::MemoryBuffer> input;
  if (auto inputAlignment = translationsRequested[0]->getInputAlignment())
    input = openInputFile(inputFilename, *inputAlignment, &errorMessage);
  else
    input = openInputFile(inputFilename, &errorMessage);
  if (!input) {
    toolchain::errs() << errorMessage << "\n";
    return failure();
  }

  auto output = openOutputFile(outputFilename, &errorMessage);
  if (!output) {
    toolchain::errs() << errorMessage << "\n";
    return failure();
  }

  // Processes the memory buffer with a new MLIRContext.
  auto processBuffer = [&](std::unique_ptr<toolchain::MemoryBuffer> ownedBuffer,
                           raw_ostream &os) {
    // Many of the translations expect a null-terminated buffer while splitting
    // the buffer does not guarantee null-termination. Make a copy of the buffer
    // to ensure null-termination.
    if (!ownedBuffer->getBuffer().ends_with('\0')) {
      ownedBuffer = toolchain::MemoryBuffer::getMemBufferCopy(
          ownedBuffer->getBuffer(), ownedBuffer->getBufferIdentifier());
    }
    // Temporary buffers for chained translation processing.
    std::string dataIn;
    std::string dataOut;
    LogicalResult result = LogicalResult::success();

    for (size_t i = 0, e = translationsRequested.size(); i < e; ++i) {
      toolchain::raw_ostream *stream;
      toolchain::raw_string_ostream dataStream(dataOut);

      if (i == e - 1) {
        // Output last translation to output.
        stream = &os;
      } else {
        // Output translation to temporary data buffer.
        stream = &dataStream;
      }

      const Translation *translationRequested = translationsRequested[i];
      TimingScope translationTiming =
          timing.nest(translationRequested->getDescription());

      MLIRContext context;
      context.allowUnregisteredDialects(allowUnregisteredDialects);
      context.printOpOnDiagnostic(verifyDiagnostics.getNumOccurrences() == 0);
      auto sourceMgr = std::make_shared<toolchain::SourceMgr>();
      sourceMgr->AddNewSourceBuffer(std::move(ownedBuffer), SMLoc());

      if (verifyDiagnostics.getNumOccurrences()) {
        // In the diagnostic verification flow, we ignore whether the
        // translation failed (in most cases, it is expected to fail) and we do
        // not filter non-error diagnostics even if `errorDiagnosticsOnly` is
        // set. Instead, we check if the diagnostics were produced as expected.
        SourceMgrDiagnosticVerifierHandler sourceMgrHandler(
            *sourceMgr, &context, verifyDiagnostics);
        (void)(*translationRequested)(sourceMgr, os, &context);
        result = sourceMgrHandler.verify();
      } else if (errorDiagnosticsOnly) {
        SourceMgrDiagnosticHandler sourceMgrHandler(*sourceMgr, &context);
        ErrorDiagnosticFilter diagnosticFilter(&context);
        result = (*translationRequested)(sourceMgr, *stream, &context);
      } else {
        SourceMgrDiagnosticHandler sourceMgrHandler(*sourceMgr, &context);
        result = (*translationRequested)(sourceMgr, *stream, &context);
      }
      if (failed(result))
        return result;

      if (i < e - 1) {
        // If there are further translations, create a new buffer with the
        // output data.
        dataIn = dataOut;
        dataOut.clear();
        ownedBuffer = toolchain::MemoryBuffer::getMemBuffer(dataIn);
      }
    }
    return result;
  };

  if (failed(splitAndProcessBuffer(std::move(input), processBuffer,
                                   output->os(), inputSplitMarker,
                                   outputSplitMarker)))
    return failure();

  output->keep();
  return success();
}
