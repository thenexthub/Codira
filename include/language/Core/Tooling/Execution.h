//===--- Execution.h - Executing clang frontend actions -*- C++ ---------*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
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
//  This file defines framework for executing clang frontend actions.
//
//  The framework can be extended to support different execution plans including
//  standalone execution on the given TUs or parallel execution on all TUs in
//  the codebase.
//
//  In order to enable multiprocessing execution, tool actions are expected to
//  output result into the ToolResults provided by the executor. The
//  `ToolResults` is an interface that abstracts how results are stored e.g.
//  in-memory for standalone execution or on-disk for large-scale execution.
//
//  New executors can be registered as ToolExecutorPlugins via the
//  `ToolExecutorPluginRegistry`. CLI tools can use
//  `createExecutorFromCommandLineArgs` to create a specific registered executor
//  according to the command-line arguments.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TOOLING_EXECUTION_H
#define LANGUAGE_CORE_TOOLING_EXECUTION_H

#include "language/Core/Tooling/CommonOptionsParser.h"
#include "language/Core/Tooling/Tooling.h"
#include "toolchain/Support/Error.h"
#include "toolchain/Support/Registry.h"
#include "toolchain/Support/StringSaver.h"

namespace language::Core {
namespace tooling {

extern toolchain::cl::opt<std::string> ExecutorName;

/// An abstraction for the result of a tool execution. For example, the
/// underlying result can be in-memory or on-disk.
///
/// Results should be string key-value pairs. For example, a refactoring tool
/// can use source location as key and a replacement in YAML format as value.
class ToolResults {
public:
  virtual ~ToolResults() = default;
  virtual void addResult(StringRef Key, StringRef Value) = 0;
  virtual std::vector<std::pair<toolchain::StringRef, toolchain::StringRef>>
  AllKVResults() = 0;
  virtual void forEachResult(
      toolchain::function_ref<void(StringRef Key, StringRef Value)> Callback) = 0;
};

/// Stores the key-value results in memory. It maintains the lifetime of
/// the result. Clang tools using this class are expected to generate a small
/// set of different results, or a large set of duplicated results.
class InMemoryToolResults : public ToolResults {
public:
  InMemoryToolResults() : Strings(Arena) {}
  void addResult(StringRef Key, StringRef Value) override;
  std::vector<std::pair<toolchain::StringRef, toolchain::StringRef>>
  AllKVResults() override;
  void forEachResult(toolchain::function_ref<void(StringRef Key, StringRef Value)>
                         Callback) override;

private:
  toolchain::BumpPtrAllocator Arena;
  toolchain::UniqueStringSaver Strings;

  std::vector<std::pair<toolchain::StringRef, toolchain::StringRef>> KVResults;
};

/// The context of an execution, including the information about
/// compilation and results.
class ExecutionContext {
public:
  virtual ~ExecutionContext() {}

  /// Initializes a context. This does not take ownership of `Results`.
  explicit ExecutionContext(ToolResults *Results) : Results(Results) {}

  /// Adds a KV pair to the result container of this execution.
  void reportResult(StringRef Key, StringRef Value);

  // Returns the source control system's revision number if applicable.
  // Otherwise returns an empty string.
  virtual std::string getRevision() { return ""; }

  // Returns the corpus being analyzed, e.g. "toolchain" for the LLVM codebase, if
  // applicable.
  virtual std::string getCorpus() { return ""; }

  // Returns the currently processed compilation unit if available.
  virtual std::string getCurrentCompilationUnit() { return ""; }

private:
  ToolResults *Results;
};

/// Interface for executing clang frontend actions.
///
/// This can be extended to support running tool actions in different
/// execution mode, e.g. on a specific set of TUs or many TUs in parallel.
///
///  New executors can be registered as ToolExecutorPlugins via the
///  `ToolExecutorPluginRegistry`. CLI tools can use
///  `createExecutorFromCommandLineArgs` to create a specific registered
///  executor according to the command-line arguments.
class ToolExecutor {
public:
  virtual ~ToolExecutor() {}

  /// Returns the name of a specific executor.
  virtual StringRef getExecutorName() const = 0;

  /// Executes each action with a corresponding arguments adjuster.
  virtual toolchain::Error
  execute(toolchain::ArrayRef<
          std::pair<std::unique_ptr<FrontendActionFactory>, ArgumentsAdjuster>>
              Actions) = 0;

  /// Convenient functions for the above `execute`.
  toolchain::Error execute(std::unique_ptr<FrontendActionFactory> Action);
  /// Executes an action with an argument adjuster.
  toolchain::Error execute(std::unique_ptr<FrontendActionFactory> Action,
                      ArgumentsAdjuster Adjuster);

  /// Returns a reference to the execution context.
  ///
  /// This should be passed to tool callbacks, and tool callbacks should report
  /// results via the returned context.
  virtual ExecutionContext *getExecutionContext() = 0;

  /// Returns a reference to the result container.
  ///
  /// NOTE: This should only be used after the execution finishes. Tool
  /// callbacks should report results via `ExecutionContext` instead.
  virtual ToolResults *getToolResults() = 0;

  /// Map a virtual file to be used while running the tool.
  ///
  /// \param FilePath The path at which the content will be mapped.
  /// \param Content A buffer of the file's content.
  virtual void mapVirtualFile(StringRef FilePath, StringRef Content) = 0;
};

/// Interface for factories that create specific executors. This is also
/// used as a plugin to be registered into ToolExecutorPluginRegistry.
class ToolExecutorPlugin {
public:
  virtual ~ToolExecutorPlugin() {}

  /// Create an `ToolExecutor`.
  ///
  /// `OptionsParser` can be consumed (e.g. moved) if the creation succeeds.
  virtual toolchain::Expected<std::unique_ptr<ToolExecutor>>
  create(CommonOptionsParser &OptionsParser) = 0;
};

/// This creates a ToolExecutor that is in the global registry based on
/// commandline arguments.
///
/// This picks the right executor based on the `--executor` option. This parses
/// the commandline arguments with `CommonOptionsParser`, so caller does not
/// need to parse again.
///
/// By default, this creates a `StandaloneToolExecutor` ("standalone") if
/// `--executor` is not provided.
toolchain::Expected<std::unique_ptr<ToolExecutor>>
createExecutorFromCommandLineArgs(int &argc, const char **argv,
                                  toolchain::cl::OptionCategory &Category,
                                  const char *Overview = nullptr);

namespace internal {
toolchain::Expected<std::unique_ptr<ToolExecutor>>
createExecutorFromCommandLineArgsImpl(int &argc, const char **argv,
                                      toolchain::cl::OptionCategory &Category,
                                      const char *Overview = nullptr);
} // end namespace internal

} // end namespace tooling
} // end namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_EXECUTION_H
