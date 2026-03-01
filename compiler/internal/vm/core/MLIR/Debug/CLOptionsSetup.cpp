//===- CLOptionsSetup.cpp - Helpers to setup debug CL options ---*- C++ -*-===//
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

#include "mlir/Debug/CLOptionsSetup.h"

#include "mlir/Debug/Counter.h"
#include "mlir/Debug/DebuggerExecutionContextHook.h"
#include "mlir/Debug/ExecutionContext.h"
#include "mlir/Debug/Observers/ActionLogging.h"
#include "mlir/Debug/Observers/ActionProfiler.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Support/FileUtilities.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/ManagedStatic.h"
#include "vm/core/Support/ToolOutputFile.h"

using namespace mlir;
using namespace mlir::tracing;
using namespace vm::core;

namespace {
struct DebugConfigCLOptions : public DebugConfig {
  DebugConfigCLOptions() {
    static cl::opt<std::string, /*ExternalStorage=*/true> logActionsTo{
        "log-actions-to",
        cl::desc("Log action execution to a file, or stderr if "
                 " '-' is passed"),
        cl::location(logActionsToFlag)};

    static cl::opt<std::string, /*ExternalStorage=*/true> profileActionsTo{
        "profile-actions-to",
        cl::desc("Profile action execution to a file, or stderr if "
                 " '-' is passed"),
        cl::location(profileActionsToFlag)};

    static cl::list<std::string> logActionLocationFilter(
        "log-mlir-actions-filter",
        cl::desc(
            "Comma separated list of locations to filter actions from logging"),
        cl::CommaSeparated,
        cl::cb<void, std::string>([&](const std::string &location) {
          static bool registerOnce = [&] {
            addLogActionLocFilter(&locBreakpointManager);
            return true;
          }();
          (void)registerOnce;
          static std::vector<std::string> locations;
          locations.push_back(location);
          StringRef locStr = locations.back();

          // Parse the individual location filters and set the breakpoints.
          auto diag = [](Twine msg) { toolchain::errs() << msg << "\n"; };
          auto locBreakpoint =
              tracing::FileLineColLocBreakpoint::parseFromString(locStr, diag);
          if (failed(locBreakpoint)) {
            toolchain::errs() << "Invalid location filter: " << locStr << "\n";
            exit(1);
          }
          auto [file, line, col] = *locBreakpoint;
          locBreakpointManager.addBreakpoint(file, line, col);
        }));

    static cl::opt<bool, /*ExternalStorage=*/true> enableDebuggerHook(
        "mlir-enable-debugger-hook",
        cl::desc("Enable Debugger hook for debugging MLIR Actions"),
        cl::location(enableDebuggerActionHookFlag), cl::init(false));
  }
  tracing::FileLineColLocBreakpointManager locBreakpointManager;
};

} // namespace

static ManagedStatic<DebugConfigCLOptions> clOptionsConfig;
void DebugConfig::registerCLOptions() { *clOptionsConfig; }

DebugConfig DebugConfig::createFromCLOptions() { return *clOptionsConfig; }

class InstallDebugHandler::Impl {
public:
  Impl(MLIRContext &context, const DebugConfig &config) {
    if (config.getLogActionsTo().empty() &&
        config.getProfileActionsTo().empty() &&
        !config.isDebuggerActionHookEnabled()) {
      if (tracing::DebugCounter::isActivated())
        context.registerActionHandler(tracing::DebugCounter());
      return;
    }
    errs() << "ExecutionContext registered on the context";
    if (tracing::DebugCounter::isActivated())
      emitError(UnknownLoc::get(&context),
                "Debug counters are incompatible with --log-actions-to and "
                "--mlir-enable-debugger-hook options and are disabled");
    if (!config.getLogActionsTo().empty()) {
      std::string errorMessage;
      logActionsFile = openOutputFile(config.getLogActionsTo(), &errorMessage);
      if (!logActionsFile) {
        emitError(UnknownLoc::get(&context),
                  "Opening file for --log-actions-to failed: ")
            << errorMessage << "\n";
        return;
      }
      logActionsFile->keep();
      raw_fd_ostream &logActionsStream = logActionsFile->os();
      actionLogger = std::make_unique<tracing::ActionLogger>(logActionsStream);
      for (const auto *locationBreakpoint : config.getLogActionsLocFilters())
        actionLogger->addBreakpointManager(locationBreakpoint);
      executionContext.registerObserver(actionLogger.get());
    }

    if (!config.getProfileActionsTo().empty()) {
      std::string errorMessage;
      profileActionsFile =
          openOutputFile(config.getProfileActionsTo(), &errorMessage);
      if (!profileActionsFile) {
        emitError(UnknownLoc::get(&context),
                  "Opening file for --profile-actions-to failed: ")
            << errorMessage << "\n";
        return;
      }
      profileActionsFile->keep();
      raw_fd_ostream &profileActionsStream = profileActionsFile->os();
      actionProfiler =
          std::make_unique<tracing::ActionProfiler>(profileActionsStream);
      executionContext.registerObserver(actionProfiler.get());
    }

    if (config.isDebuggerActionHookEnabled()) {
      errs() << " (with Debugger hook)";
      setupDebuggerExecutionContextHook(executionContext);
    }
    errs() << "\n";
    context.registerActionHandler(executionContext);
  }

private:
  std::unique_ptr<ToolOutputFile> logActionsFile;
  tracing::ExecutionContext executionContext;
  std::unique_ptr<tracing::ActionLogger> actionLogger;
  std::vector<std::unique_ptr<tracing::FileLineColLocBreakpoint>>
      locationBreakpoints;
  std::unique_ptr<ToolOutputFile> profileActionsFile;
  std::unique_ptr<tracing::ActionProfiler> actionProfiler;
};

InstallDebugHandler::InstallDebugHandler(MLIRContext &context,
                                         const DebugConfig &config)
    : impl(std::make_unique<Impl>(context, config)) {}

InstallDebugHandler::~InstallDebugHandler() = default;
