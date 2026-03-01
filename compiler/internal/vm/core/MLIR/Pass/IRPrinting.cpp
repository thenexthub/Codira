//===- IRPrinting.cpp -----------------------------------------------------===//
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

#include "PassDetail.h"
#include "mlir/IR/SymbolTable.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Support/FileUtilities.h"
#include "vm/core/ADT/STLExtras.h"
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/Support/FileSystem.h"
#include "vm/core/Support/FormatVariadic.h"
#include "vm/core/Support/Path.h"
#include "vm/core/Support/ToolOutputFile.h"

using namespace mlir;
using namespace mlir::detail;

namespace {
//===----------------------------------------------------------------------===//
// IRPrinter
//===----------------------------------------------------------------------===//

class IRPrinterInstrumentation : public PassInstrumentation {
public:
  IRPrinterInstrumentation(std::unique_ptr<PassManager::IRPrinterConfig> config)
      : config(std::move(config)) {}

private:
  /// Instrumentation hooks.
  void runBeforePass(Pass *pass, Operation *op) override;
  void runAfterPass(Pass *pass, Operation *op) override;
  void runAfterPassFailed(Pass *pass, Operation *op) override;

  /// Configuration to use.
  std::unique_ptr<PassManager::IRPrinterConfig> config;

  /// The following is a set of fingerprints for operations that are currently
  /// being operated on in a pass. This field is only used when the
  /// configuration asked for change detection.
  DenseMap<Pass *, OperationFingerPrint> beforePassFingerPrints;
};
} // namespace

static void printIR(Operation *op, bool printModuleScope, raw_ostream &out,
                    OpPrintingFlags flags) {
  // Otherwise, check to see if we are not printing at module scope.
  if (!printModuleScope)
    return op->print(out << " //----- //\n",
                     op->getBlock() ? flags.useLocalScope() : flags);

  // Otherwise, we are printing at module scope.
  out << " ('" << op->getName() << "' operation";
  if (auto symbolName =
          op->getAttrOfType<StringAttr>(SymbolTable::getSymbolAttrName()))
    out << ": @" << symbolName.getValue();
  out << ") //----- //\n";

  // Find the top-level operation.
  auto *topLevelOp = op;
  while (auto *parentOp = topLevelOp->getParentOp())
    topLevelOp = parentOp;
  topLevelOp->print(out, flags);
}

/// Instrumentation hooks.
void IRPrinterInstrumentation::runBeforePass(Pass *pass, Operation *op) {
  if (isa<OpToOpPassAdaptor>(pass))
    return;
  // If the config asked to detect changes, record the current fingerprint.
  if (config->shouldPrintAfterOnlyOnChange())
    beforePassFingerPrints.try_emplace(pass, op);

  config->printBeforeIfEnabled(pass, op, [&](raw_ostream &out) {
    out << "// -----// IR Dump Before " << pass->getName() << " ("
        << pass->getArgument() << ")";
    printIR(op, config->shouldPrintAtModuleScope(), out,
            config->getOpPrintingFlags());
    out << "\n\n";
  });
}

void IRPrinterInstrumentation::runAfterPass(Pass *pass, Operation *op) {
  if (isa<OpToOpPassAdaptor>(pass))
    return;

  // Check to see if we are only printing on failure.
  if (config->shouldPrintAfterOnlyOnFailure())
    return;

  // If the config asked to detect changes, compare the current fingerprint with
  // the previous.
  if (config->shouldPrintAfterOnlyOnChange()) {
    auto fingerPrintIt = beforePassFingerPrints.find(pass);
    assert(fingerPrintIt != beforePassFingerPrints.end() &&
           "expected valid fingerprint");
    // If the fingerprints are the same, we don't print the IR.
    if (fingerPrintIt->second == OperationFingerPrint(op)) {
      beforePassFingerPrints.erase(fingerPrintIt);
      return;
    }
    beforePassFingerPrints.erase(fingerPrintIt);
  }

  config->printAfterIfEnabled(pass, op, [&](raw_ostream &out) {
    out << "// -----// IR Dump After " << pass->getName() << " ("
        << pass->getArgument() << ")";
    printIR(op, config->shouldPrintAtModuleScope(), out,
            config->getOpPrintingFlags());
    out << "\n\n";
  });
}

void IRPrinterInstrumentation::runAfterPassFailed(Pass *pass, Operation *op) {
  if (isa<OpToOpPassAdaptor>(pass))
    return;
  if (config->shouldPrintAfterOnlyOnChange())
    beforePassFingerPrints.erase(pass);

  config->printAfterIfEnabled(pass, op, [&](raw_ostream &out) {
    out << formatv("// -----// IR Dump After {0} Failed ({1})", pass->getName(),
                   pass->getArgument());
    printIR(op, config->shouldPrintAtModuleScope(), out,
            config->getOpPrintingFlags());
    out << "\n\n";
  });
}

//===----------------------------------------------------------------------===//
// IRPrinterConfig
//===----------------------------------------------------------------------===//

/// Initialize the configuration.
PassManager::IRPrinterConfig::IRPrinterConfig(bool printModuleScope,
                                              bool printAfterOnlyOnChange,
                                              bool printAfterOnlyOnFailure,
                                              OpPrintingFlags opPrintingFlags)
    : printModuleScope(printModuleScope),
      printAfterOnlyOnChange(printAfterOnlyOnChange),
      printAfterOnlyOnFailure(printAfterOnlyOnFailure),
      opPrintingFlags(opPrintingFlags) {}
PassManager::IRPrinterConfig::~IRPrinterConfig() = default;

/// A hook that may be overridden by a derived config that checks if the IR
/// of 'operation' should be dumped *before* the pass 'pass' has been
/// executed. If the IR should be dumped, 'printCallback' should be invoked
/// with the stream to dump into.
void PassManager::IRPrinterConfig::printBeforeIfEnabled(
    Pass *pass, Operation *operation, PrintCallbackFn printCallback) {
  // By default, never print.
}

/// A hook that may be overridden by a derived config that checks if the IR
/// of 'operation' should be dumped *after* the pass 'pass' has been
/// executed. If the IR should be dumped, 'printCallback' should be invoked
/// with the stream to dump into.
void PassManager::IRPrinterConfig::printAfterIfEnabled(
    Pass *pass, Operation *operation, PrintCallbackFn printCallback) {
  // By default, never print.
}

//===----------------------------------------------------------------------===//
// PassManager
//===----------------------------------------------------------------------===//

namespace {
/// Simple wrapper config that allows for the simpler interface defined above.
struct BasicIRPrinterConfig : public PassManager::IRPrinterConfig {
  BasicIRPrinterConfig(
      std::function<bool(Pass *, Operation *)> shouldPrintBeforePass,
      std::function<bool(Pass *, Operation *)> shouldPrintAfterPass,
      bool printModuleScope, bool printAfterOnlyOnChange,
      bool printAfterOnlyOnFailure, OpPrintingFlags opPrintingFlags,
      raw_ostream &out)
      : IRPrinterConfig(printModuleScope, printAfterOnlyOnChange,
                        printAfterOnlyOnFailure, opPrintingFlags),
        shouldPrintBeforePass(std::move(shouldPrintBeforePass)),
        shouldPrintAfterPass(std::move(shouldPrintAfterPass)), out(out) {
    assert((this->shouldPrintBeforePass || this->shouldPrintAfterPass) &&
           "expected at least one valid filter function");
  }

  void printBeforeIfEnabled(Pass *pass, Operation *operation,
                            PrintCallbackFn printCallback) final {
    if (shouldPrintBeforePass && shouldPrintBeforePass(pass, operation))
      printCallback(out);
  }

  void printAfterIfEnabled(Pass *pass, Operation *operation,
                           PrintCallbackFn printCallback) final {
    if (shouldPrintAfterPass && shouldPrintAfterPass(pass, operation))
      printCallback(out);
  }

  /// Filter functions for before and after pass execution.
  std::function<bool(Pass *, Operation *)> shouldPrintBeforePass;
  std::function<bool(Pass *, Operation *)> shouldPrintAfterPass;

  /// The stream to output to.
  raw_ostream &out;
};
} // namespace

/// Return pairs of (sanitized op name, symbol name) for `op` and all parent
/// operations. Op names are sanitized by replacing periods with underscores.
/// The pairs are returned in order of outer-most to inner-most (ancestors of
/// `op` first, `op` last). This information is used to construct the directory
/// tree for the `FileTreeIRPrinterConfig` below.
/// The counter for `op` will be incremented by this call.
static std::pair<SmallVector<std::pair<std::string, std::string>>, std::string>
getOpAndSymbolNames(Operation *op, StringRef passName,
                    toolchain::DenseMap<Operation *, unsigned> &counters) {
  SmallVector<std::pair<std::string, std::string>> pathElements;
  SmallVector<unsigned> countPrefix;

  Operation *iter = op;
  ++counters.try_emplace(op, -1).first->second;
  while (iter) {
    countPrefix.push_back(counters[iter]);
    StringAttr symbolNameAttr =
        iter->getAttrOfType<StringAttr>(SymbolTable::getSymbolAttrName());
    std::string symbolName =
        symbolNameAttr ? symbolNameAttr.str() : "no-symbol-name";
    toolchain::replace(symbolName, '/', '_');
    toolchain::replace(symbolName, '\\', '_');

    std::string opName =
        toolchain::join(toolchain::split(iter->getName().getStringRef().str(), '.'), "_");
    pathElements.emplace_back(std::move(opName), std::move(symbolName));
    iter = iter->getParentOp();
  }
  // Return in the order of top level (module) down to `op`.
  std::reverse(countPrefix.begin(), countPrefix.end());
  std::reverse(pathElements.begin(), pathElements.end());

  std::string passFileName = toolchain::formatv(
      "{0:$[_]}_{1}.mlir",
      toolchain::make_range(countPrefix.begin(), countPrefix.end()), passName);

  return {pathElements, passFileName};
}

static LogicalResult createDirectoryOrPrintErr(toolchain::StringRef dirPath) {
  if (std::error_code ec =
          toolchain::sys::fs::create_directory(dirPath, /*IgnoreExisting=*/true)) {
    toolchain::errs() << "Error while creating directory " << dirPath << ": "
                 << ec.message() << "\n";
    return failure();
  }
  return success();
}

/// Creates  directories (if required) and opens an output file for the
/// FileTreeIRPrinterConfig.
static std::unique_ptr<toolchain::ToolOutputFile>
createTreePrinterOutputPath(Operation *op, toolchain::StringRef passArgument,
                            toolchain::StringRef rootDir,
                            toolchain::DenseMap<Operation *, unsigned> &counters) {
  // Create the path. We will create a tree rooted at the given 'rootDir'
  // directory. The root directory will contain folders with the names of
  // modules. Sub-directories within those folders mirror the nesting
  // structure of the pass manager, using symbol names for directory names.
  auto [opAndSymbolNames, fileName] =
      getOpAndSymbolNames(op, passArgument, counters);

  // Create all the directories, starting at the root. Abort early if we fail to
  // create any directory.
  toolchain::SmallString<128> path(rootDir);
  if (failed(createDirectoryOrPrintErr(path)))
    return nullptr;

  for (const auto &[opName, symbolName] : opAndSymbolNames) {
    toolchain::sys::path::append(path, opName + "_" + symbolName);
    if (failed(createDirectoryOrPrintErr(path)))
      return nullptr;
  }

  // Open output file.
  toolchain::sys::path::append(path, fileName);
  std::string error;
  std::unique_ptr<toolchain::ToolOutputFile> file = openOutputFile(path, &error);
  if (!file) {
    toolchain::errs() << "Error opening output file " << path << ": " << error
                 << "\n";
    return nullptr;
  }
  return file;
}

namespace {
/// A configuration that prints the IR before/after each pass to a set of files
/// in the specified directory. The files are organized into subdirectories that
/// mirror the nesting structure of the IR.
struct FileTreeIRPrinterConfig : public PassManager::IRPrinterConfig {
  FileTreeIRPrinterConfig(
      std::function<bool(Pass *, Operation *)> shouldPrintBeforePass,
      std::function<bool(Pass *, Operation *)> shouldPrintAfterPass,
      bool printModuleScope, bool printAfterOnlyOnChange,
      bool printAfterOnlyOnFailure, OpPrintingFlags opPrintingFlags,
      toolchain::StringRef treeDir)
      : IRPrinterConfig(printModuleScope, printAfterOnlyOnChange,
                        printAfterOnlyOnFailure, opPrintingFlags),
        shouldPrintBeforePass(std::move(shouldPrintBeforePass)),
        shouldPrintAfterPass(std::move(shouldPrintAfterPass)),
        treeDir(treeDir) {
    assert((this->shouldPrintBeforePass || this->shouldPrintAfterPass) &&
           "expected at least one valid filter function");
  }

  void printBeforeIfEnabled(Pass *pass, Operation *operation,
                            PrintCallbackFn printCallback) final {
    if (!shouldPrintBeforePass || !shouldPrintBeforePass(pass, operation))
      return;
    std::unique_ptr<toolchain::ToolOutputFile> file = createTreePrinterOutputPath(
        operation, pass->getArgument(), treeDir, counters);
    if (!file)
      return;
    printCallback(file->os());
    file->keep();
  }

  void printAfterIfEnabled(Pass *pass, Operation *operation,
                           PrintCallbackFn printCallback) final {
    if (!shouldPrintAfterPass || !shouldPrintAfterPass(pass, operation))
      return;
    std::unique_ptr<toolchain::ToolOutputFile> file = createTreePrinterOutputPath(
        operation, pass->getArgument(), treeDir, counters);
    if (!file)
      return;
    printCallback(file->os());
    file->keep();
  }

  /// Filter functions for before and after pass execution.
  std::function<bool(Pass *, Operation *)> shouldPrintBeforePass;
  std::function<bool(Pass *, Operation *)> shouldPrintAfterPass;

  /// Directory that should be used as the root of the file tree.
  std::string treeDir;

  /// Counters used for labeling the prefix. Every op which could be targeted by
  /// a pass gets its own counter.
  toolchain::DenseMap<Operation *, unsigned> counters;
};

} // namespace

/// Add an instrumentation to print the IR before and after pass execution,
/// using the provided configuration.
void PassManager::enableIRPrinting(std::unique_ptr<IRPrinterConfig> config) {
  if (config->shouldPrintAtModuleScope() &&
      getContext()->isMultithreadingEnabled())
    toolchain::report_fatal_error("IR printing can't be setup on a pass-manager "
                             "without disabling multi-threading first.");
  addInstrumentation(
      std::make_unique<IRPrinterInstrumentation>(std::move(config)));
}

/// Add an instrumentation to print the IR before and after pass execution.
void PassManager::enableIRPrinting(
    std::function<bool(Pass *, Operation *)> shouldPrintBeforePass,
    std::function<bool(Pass *, Operation *)> shouldPrintAfterPass,
    bool printModuleScope, bool printAfterOnlyOnChange,
    bool printAfterOnlyOnFailure, raw_ostream &out,
    OpPrintingFlags opPrintingFlags) {
  enableIRPrinting(std::make_unique<BasicIRPrinterConfig>(
      std::move(shouldPrintBeforePass), std::move(shouldPrintAfterPass),
      printModuleScope, printAfterOnlyOnChange, printAfterOnlyOnFailure,
      opPrintingFlags, out));
}

/// Add an instrumentation to print the IR before and after pass execution.
void PassManager::enableIRPrintingToFileTree(
    std::function<bool(Pass *, Operation *)> shouldPrintBeforePass,
    std::function<bool(Pass *, Operation *)> shouldPrintAfterPass,
    bool printModuleScope, bool printAfterOnlyOnChange,
    bool printAfterOnlyOnFailure, StringRef printTreeDir,
    OpPrintingFlags opPrintingFlags) {
  enableIRPrinting(std::make_unique<FileTreeIRPrinterConfig>(
      std::move(shouldPrintBeforePass), std::move(shouldPrintAfterPass),
      printModuleScope, printAfterOnlyOnChange, printAfterOnlyOnFailure,
      opPrintingFlags, printTreeDir));
}
