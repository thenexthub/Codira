//===- lib/Tooling/Execution.cpp - Implements tool execution framework. ---===//
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

#include "language/Core/Tooling/Execution.h"
#include "language/Core/Tooling/ToolExecutorPluginRegistry.h"
#include "language/Core/Tooling/Tooling.h"

LLVM_INSTANTIATE_REGISTRY(language::Core::tooling::ToolExecutorPluginRegistry)

namespace language::Core {
namespace tooling {

toolchain::cl::opt<std::string>
    ExecutorName("executor", toolchain::cl::desc("The name of the executor to use."),
                 toolchain::cl::init("standalone"));

void InMemoryToolResults::addResult(StringRef Key, StringRef Value) {
  KVResults.push_back({Strings.save(Key), Strings.save(Value)});
}

std::vector<std::pair<toolchain::StringRef, toolchain::StringRef>>
InMemoryToolResults::AllKVResults() {
  return KVResults;
}

void InMemoryToolResults::forEachResult(
    toolchain::function_ref<void(StringRef Key, StringRef Value)> Callback) {
  for (const auto &KV : KVResults) {
    Callback(KV.first, KV.second);
  }
}

void ExecutionContext::reportResult(StringRef Key, StringRef Value) {
  Results->addResult(Key, Value);
}

toolchain::Error
ToolExecutor::execute(std::unique_ptr<FrontendActionFactory> Action) {
  return execute(std::move(Action), ArgumentsAdjuster());
}

toolchain::Error ToolExecutor::execute(std::unique_ptr<FrontendActionFactory> Action,
                                  ArgumentsAdjuster Adjuster) {
  std::vector<
      std::pair<std::unique_ptr<FrontendActionFactory>, ArgumentsAdjuster>>
      Actions;
  Actions.emplace_back(std::move(Action), std::move(Adjuster));
  return execute(Actions);
}

namespace internal {
toolchain::Expected<std::unique_ptr<ToolExecutor>>
createExecutorFromCommandLineArgsImpl(int &argc, const char **argv,
                                      toolchain::cl::OptionCategory &Category,
                                      const char *Overview) {
  auto OptionsParser =
      CommonOptionsParser::create(argc, argv, Category, toolchain::cl::ZeroOrMore,
                                  /*Overview=*/Overview);
  if (!OptionsParser)
    return OptionsParser.takeError();
  for (const auto &TEPlugin : ToolExecutorPluginRegistry::entries()) {
    if (TEPlugin.getName() != ExecutorName) {
      continue;
    }
    std::unique_ptr<ToolExecutorPlugin> Plugin(TEPlugin.instantiate());
    toolchain::Expected<std::unique_ptr<ToolExecutor>> Executor =
        Plugin->create(*OptionsParser);
    if (!Executor) {
      return toolchain::make_error<toolchain::StringError>(
          toolchain::Twine("Failed to create '") + TEPlugin.getName() +
              "': " + toolchain::toString(Executor.takeError()) + "\n",
          toolchain::inconvertibleErrorCode());
    }
    return std::move(*Executor);
  }
  return toolchain::make_error<toolchain::StringError>(
      toolchain::Twine("Executor \"") + ExecutorName + "\" is not registered.",
      toolchain::inconvertibleErrorCode());
}
} // end namespace internal

toolchain::Expected<std::unique_ptr<ToolExecutor>>
createExecutorFromCommandLineArgs(int &argc, const char **argv,
                                  toolchain::cl::OptionCategory &Category,
                                  const char *Overview) {
  return internal::createExecutorFromCommandLineArgsImpl(argc, argv, Category,
                                                         Overview);
}

// This anchor is used to force the linker to link in the generated object file
// and thus register the StandaloneToolExecutorPlugin etc.
extern volatile int StandaloneToolExecutorAnchorSource;
extern volatile int AllTUsToolExecutorAnchorSource;
static int LLVM_ATTRIBUTE_UNUSED StandaloneToolExecutorAnchorDest =
    StandaloneToolExecutorAnchorSource;
static int LLVM_ATTRIBUTE_UNUSED AllTUsToolExecutorAnchorDest =
    AllTUsToolExecutorAnchorSource;

} // end namespace tooling
} // end namespace language::Core
