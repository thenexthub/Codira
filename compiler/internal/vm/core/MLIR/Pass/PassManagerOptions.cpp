//===- PassManagerOptions.cpp - PassManager Command Line Options ----------===//
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

#include "mlir/Pass/Pass.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Pass/PassRegistry.h"
#include "mlir/Support/Timing.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/ManagedStatic.h"

using namespace mlir;

namespace {
struct PassManagerOptions {
  //===--------------------------------------------------------------------===//
  // Crash Reproducer Generator
  //===--------------------------------------------------------------------===//
  toolchain::cl::opt<std::string> reproducerFile{
      "mlir-pass-pipeline-crash-reproducer",
      toolchain::cl::desc("Generate a .mlir reproducer file at the given output path"
                     " if the pass manager crashes or fails")};
  toolchain::cl::opt<bool> localReproducer{
      "mlir-pass-pipeline-local-reproducer",
      toolchain::cl::desc("When generating a crash reproducer, attempt to generated "
                     "a reproducer with the smallest pipeline."),
      toolchain::cl::init(false)};

  //===--------------------------------------------------------------------===//
  // IR Printing
  //===--------------------------------------------------------------------===//
  PassNameCLParser printBefore{"mlir-print-ir-before",
                               "Print IR before specified passes"};
  PassNameCLParser printAfter{"mlir-print-ir-after",
                              "Print IR after specified passes"};
  toolchain::cl::opt<bool> printBeforeAll{
      "mlir-print-ir-before-all", toolchain::cl::desc("Print IR before each pass"),
      toolchain::cl::init(false)};
  toolchain::cl::opt<bool> printAfterAll{"mlir-print-ir-after-all",
                                    toolchain::cl::desc("Print IR after each pass"),
                                    toolchain::cl::init(false)};
  toolchain::cl::opt<bool> printAfterChange{
      "mlir-print-ir-after-change",
      toolchain::cl::desc(
          "When printing the IR after a pass, only print if the IR changed"),
      toolchain::cl::init(false)};
  toolchain::cl::opt<bool> printAfterFailure{
      "mlir-print-ir-after-failure",
      toolchain::cl::desc(
          "When printing the IR after a pass, only print if the pass failed"),
      toolchain::cl::init(false)};
  toolchain::cl::opt<bool> printModuleScope{
      "mlir-print-ir-module-scope",
      toolchain::cl::desc("When printing IR for print-ir-[before|after]{-all} "
                     "always print the top-level operation"),
      toolchain::cl::init(false)};
  toolchain::cl::opt<std::string> printTreeDir{
      "mlir-print-ir-tree-dir",
      toolchain::cl::desc("When printing the IR before/after a pass, print file "
                     "tree rooted at this directory. Use in conjunction with "
                     "mlir-print-ir-* flags")};

  /// Add an IR printing instrumentation if enabled by any 'print-ir' flags.
  void addPrinterInstrumentation(PassManager &pm);

  //===--------------------------------------------------------------------===//
  // Pass Statistics
  //===--------------------------------------------------------------------===//
  toolchain::cl::opt<bool> passStatistics{
      "mlir-pass-statistics",
      toolchain::cl::desc("Display the statistics of each pass")};
  toolchain::cl::opt<PassDisplayMode> passStatisticsDisplayMode{
      "mlir-pass-statistics-display",
      toolchain::cl::desc("Display method for pass statistics"),
      toolchain::cl::init(PassDisplayMode::Pipeline),
      toolchain::cl::values(
          clEnumValN(
              PassDisplayMode::List, "list",
              "display the results in a merged list sorted by pass name"),
          clEnumValN(PassDisplayMode::Pipeline, "pipeline",
                     "display the results with a nested pipeline view"))};
};
} // namespace

static toolchain::ManagedStatic<PassManagerOptions> options;

/// Add an IR printing instrumentation if enabled by any 'print-ir' flags.
void PassManagerOptions::addPrinterInstrumentation(PassManager &pm) {
  std::function<bool(Pass *, Operation *)> shouldPrintBeforePass;
  std::function<bool(Pass *, Operation *)> shouldPrintAfterPass;

  // Handle print-before.
  if (printBeforeAll) {
    // If we are printing before all, then just return true for the filter.
    shouldPrintBeforePass = [](Pass *, Operation *) { return true; };
  } else if (printBefore.hasAnyOccurrences()) {
    // Otherwise if there are specific passes to print before, then check to see
    // if the pass info for the current pass is included in the list.
    shouldPrintBeforePass = [&](Pass *pass, Operation *) {
      auto *passInfo = pass->lookupPassInfo();
      return passInfo && printBefore.contains(passInfo);
    };
  }

  // Handle print-after.
  if (printAfterAll || printAfterFailure) {
    // If we are printing after all or failure, then just return true for the
    // filter.
    shouldPrintAfterPass = [](Pass *, Operation *) { return true; };
  } else if (printAfter.hasAnyOccurrences()) {
    // Otherwise if there are specific passes to print after, then check to see
    // if the pass info for the current pass is included in the list.
    shouldPrintAfterPass = [&](Pass *pass, Operation *) {
      auto *passInfo = pass->lookupPassInfo();
      return passInfo && printAfter.contains(passInfo);
    };
  }

  // If there are no valid printing filters, then just return.
  if (!shouldPrintBeforePass && !shouldPrintAfterPass)
    return;

  // Otherwise, add the IR printing instrumentation.
  if (!printTreeDir.empty()) {
    pm.enableIRPrintingToFileTree(shouldPrintBeforePass, shouldPrintAfterPass,
                                  printModuleScope, printAfterChange,
                                  printAfterFailure, printTreeDir);
    return;
  }

  pm.enableIRPrinting(shouldPrintBeforePass, shouldPrintAfterPass,
                      printModuleScope, printAfterChange, printAfterFailure,
                      toolchain::errs());
}

void mlir::registerPassManagerCLOptions() {
  // Make sure that the options struct has been constructed.
  *options;
}

LogicalResult mlir::applyPassManagerCLOptions(PassManager &pm) {
  if (!options.isConstructed())
    return failure();

  if (options->reproducerFile.getNumOccurrences() && options->localReproducer &&
      pm.getContext()->isMultithreadingEnabled()) {
    emitError(UnknownLoc::get(pm.getContext()))
        << "Local crash reproduction may not be used without disabling "
           "mutli-threading first.";
    return failure();
  }

  // Generate a reproducer on crash/failure.
  if (options->reproducerFile.getNumOccurrences())
    pm.enableCrashReproducerGeneration(options->reproducerFile,
                                       options->localReproducer);

  // Enable statistics dumping.
  if (options->passStatistics)
    pm.enableStatistics(options->passStatisticsDisplayMode);

  if (options->printModuleScope && pm.getContext()->isMultithreadingEnabled()) {
    emitError(UnknownLoc::get(pm.getContext()))
        << "IR print for module scope can't be setup on a pass-manager "
           "without disabling multi-threading first.\n";
    return failure();
  }

  // Add the IR printing instrumentation.
  options->addPrinterInstrumentation(pm);
  return success();
}

void mlir::applyDefaultTimingPassManagerCLOptions(PassManager &pm) {
  // Create a temporary timing manager for the PM to own, apply its CL options,
  // and pass it to the PM.
  auto tm = std::make_unique<DefaultTimingManager>();
  applyDefaultTimingManagerCLOptions(*tm);
  pm.enableTiming(std::move(tm));
}
