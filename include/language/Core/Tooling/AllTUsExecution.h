//===--- AllTUsExecution.h - Execute actions on all TUs. -*- C++ --------*-===//
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
//  This file defines a tool executor that runs given actions on all TUs in the
//  compilation database. Tool results are deuplicated by the result key.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_CORE_TOOLING_ALLTUSEXECUTION_H
#define LANGUAGE_CORE_TOOLING_ALLTUSEXECUTION_H

#include "language/Core/Tooling/ArgumentsAdjusters.h"
#include "language/Core/Tooling/Execution.h"
#include <optional>

namespace language::Core {
namespace tooling {

/// Executes given frontend actions on all files/TUs in the compilation
/// database.
class AllTUsToolExecutor : public ToolExecutor {
public:
  static const char *ExecutorName;

  /// Init with \p CompilationDatabase.
  /// This uses \p ThreadCount threads to exececute the actions on all files in
  /// parallel. If \p ThreadCount is 0, this uses `toolchain::hardware_concurrency`.
  AllTUsToolExecutor(const CompilationDatabase &Compilations,
                     unsigned ThreadCount,
                     std::shared_ptr<PCHContainerOperations> PCHContainerOps =
                         std::make_shared<PCHContainerOperations>());

  /// Init with \p CommonOptionsParser. This is expected to be used by
  /// `createExecutorFromCommandLineArgs` based on commandline options.
  ///
  /// The executor takes ownership of \p Options.
  AllTUsToolExecutor(CommonOptionsParser Options, unsigned ThreadCount,
                     std::shared_ptr<PCHContainerOperations> PCHContainerOps =
                         std::make_shared<PCHContainerOperations>());

  StringRef getExecutorName() const override { return ExecutorName; }

  using ToolExecutor::execute;

  toolchain::Error
  execute(toolchain::ArrayRef<
          std::pair<std::unique_ptr<FrontendActionFactory>, ArgumentsAdjuster>>
              Actions) override;

  ExecutionContext *getExecutionContext() override { return &Context; };

  ToolResults *getToolResults() override { return Results.get(); }

  void mapVirtualFile(StringRef FilePath, StringRef Content) override {
    OverlayFiles[FilePath] = std::string(Content);
  }

private:
  // Used to store the parser when the executor is initialized with parser.
  std::optional<CommonOptionsParser> OptionsParser;
  const CompilationDatabase &Compilations;
  std::unique_ptr<ToolResults> Results;
  ExecutionContext Context;
  toolchain::StringMap<std::string> OverlayFiles;
  unsigned ThreadCount;
};

extern toolchain::cl::opt<unsigned> ExecutorConcurrency;
extern toolchain::cl::opt<std::string> Filter;

} // end namespace tooling
} // end namespace language::Core

#endif // LANGUAGE_CORE_TOOLING_ALLTUSEXECUTION_H
