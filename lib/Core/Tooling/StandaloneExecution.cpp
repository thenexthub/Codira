//===- lib/Tooling/Execution.cpp - Standalone clang action execution. -----===//
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

#include "language/Core/Tooling/StandaloneExecution.h"
#include "language/Core/Tooling/ToolExecutorPluginRegistry.h"

namespace language::Core {
namespace tooling {

static toolchain::Error make_string_error(const toolchain::Twine &Message) {
  return toolchain::make_error<toolchain::StringError>(Message,
                                             toolchain::inconvertibleErrorCode());
}

const char *StandaloneToolExecutor::ExecutorName = "StandaloneToolExecutor";

static ArgumentsAdjuster getDefaultArgumentsAdjusters() {
  return combineAdjusters(
      getClangStripOutputAdjuster(),
      combineAdjusters(getClangSyntaxOnlyAdjuster(),
                       getClangStripDependencyFileAdjuster()));
}

StandaloneToolExecutor::StandaloneToolExecutor(
    const CompilationDatabase &Compilations,
    toolchain::ArrayRef<std::string> SourcePaths,
    IntrusiveRefCntPtr<toolchain::vfs::FileSystem> BaseFS,
    std::shared_ptr<PCHContainerOperations> PCHContainerOps)
    : Tool(Compilations, SourcePaths, std::move(PCHContainerOps),
           std::move(BaseFS)),
      Context(&Results), ArgsAdjuster(getDefaultArgumentsAdjusters()) {
  // Use self-defined default argument adjusters instead of the default
  // adjusters that come with the old `ClangTool`.
  Tool.clearArgumentsAdjusters();
}

StandaloneToolExecutor::StandaloneToolExecutor(
    CommonOptionsParser Options,
    std::shared_ptr<PCHContainerOperations> PCHContainerOps)
    : OptionsParser(std::move(Options)),
      Tool(OptionsParser->getCompilations(), OptionsParser->getSourcePathList(),
           std::move(PCHContainerOps)),
      Context(&Results), ArgsAdjuster(getDefaultArgumentsAdjusters()) {
  Tool.clearArgumentsAdjusters();
}

toolchain::Error StandaloneToolExecutor::execute(
    toolchain::ArrayRef<
        std::pair<std::unique_ptr<FrontendActionFactory>, ArgumentsAdjuster>>
        Actions) {
  if (Actions.empty())
    return make_string_error("No action to execute.");

  if (Actions.size() != 1)
    return make_string_error(
        "Only support executing exactly 1 action at this point.");

  auto &Action = Actions.front();
  Tool.appendArgumentsAdjuster(Action.second);
  Tool.appendArgumentsAdjuster(ArgsAdjuster);
  if (Tool.run(Action.first.get()))
    return make_string_error("Failed to run action.");

  return toolchain::Error::success();
}

class StandaloneToolExecutorPlugin : public ToolExecutorPlugin {
public:
  toolchain::Expected<std::unique_ptr<ToolExecutor>>
  create(CommonOptionsParser &OptionsParser) override {
    if (OptionsParser.getSourcePathList().empty())
      return make_string_error(
          "[StandaloneToolExecutorPlugin] No positional argument found.");
    return std::make_unique<StandaloneToolExecutor>(std::move(OptionsParser));
  }
};

static ToolExecutorPluginRegistry::Add<StandaloneToolExecutorPlugin>
    X("standalone", "Runs FrontendActions on a set of files provided "
                    "via positional arguments.");

// This anchor is used to force the linker to link in the generated object file
// and thus register the plugin.
volatile int StandaloneToolExecutorAnchorSource = 0;

} // end namespace tooling
} // end namespace language::Core
