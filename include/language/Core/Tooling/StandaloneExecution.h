//===--- StandaloneExecution.h - Standalone execution. -*- C++ ----------*-===//
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
//  This file defines standalone execution of clang tools.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TOOLING_STANDALONEEXECUTION_H
#define LANGUAGE_CORE_TOOLING_STANDALONEEXECUTION_H

#include "language/Core/Tooling/ArgumentsAdjusters.h"
#include "language/Core/Tooling/Execution.h"
#include <optional>

namespace language::Core {
namespace tooling {

/// A standalone executor that runs FrontendActions on a given set of
/// TUs in sequence.
///
/// By default, this executor uses the following arguments adjusters (as defined
/// in `clang/Tooling/ArgumentsAdjusters.h`):
///   - `getClangStripOutputAdjuster()`
///   - `getClangSyntaxOnlyAdjuster()`
///   - `getClangStripDependencyFileAdjuster()`
class StandaloneToolExecutor : public ToolExecutor {
public:
  static const char *ExecutorName;

  /// Init with \p CompilationDatabase and the paths of all files to be
  /// proccessed.
  StandaloneToolExecutor(
      const CompilationDatabase &Compilations,
      toolchain::ArrayRef<std::string> SourcePaths,
      IntrusiveRefCntPtr<toolchain::vfs::FileSystem> BaseFS =
          toolchain::vfs::getRealFileSystem(),
      std::shared_ptr<PCHContainerOperations> PCHContainerOps =
          std::make_shared<PCHContainerOperations>());

  /// Init with \p CommonOptionsParser. This is expected to be used by
  /// `createExecutorFromCommandLineArgs` based on commandline options.
  ///
  /// The executor takes ownership of \p Options.
  StandaloneToolExecutor(
      CommonOptionsParser Options,
      std::shared_ptr<PCHContainerOperations> PCHContainerOps =
          std::make_shared<PCHContainerOperations>());

  StringRef getExecutorName() const override { return ExecutorName; }

  using ToolExecutor::execute;

  toolchain::Error
  execute(toolchain::ArrayRef<
          std::pair<std::unique_ptr<FrontendActionFactory>, ArgumentsAdjuster>>
              Actions) override;

  /// Set a \c DiagnosticConsumer to use during parsing.
  void setDiagnosticConsumer(DiagnosticConsumer *DiagConsumer) {
    Tool.setDiagnosticConsumer(DiagConsumer);
  }

  ExecutionContext *getExecutionContext() override { return &Context; };

  ToolResults *getToolResults() override { return &Results; }

  toolchain::ArrayRef<std::string> getSourcePaths() const {
    return Tool.getSourcePaths();
  }

  void mapVirtualFile(StringRef FilePath, StringRef Content) override {
    Tool.mapVirtualFile(FilePath, Content);
  }

  /// Returns the file manager used in the tool.
  ///
  /// The file manager is shared between all translation units.
  FileManager &getFiles() { return Tool.getFiles(); }

private:
  // Used to store the parser when the executor is initialized with parser.
  std::optional<CommonOptionsParser> OptionsParser;
  // FIXME: The standalone executor is currently just a wrapper of `ClangTool`.
  // Merge `ClangTool` implementation into the this.
  ClangTool Tool;
  ExecutionContext Context;
  InMemoryToolResults Results;
  ArgumentsAdjuster ArgsAdjuster;
};

} // end namespace tooling
} // end namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_STANDALONEEXECUTION_H
