//===- lib/Tooling/AllTUsExecution.cpp - Execute actions on all TUs. ------===//
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

#include "language/Core/Tooling/AllTUsExecution.h"
#include "language/Core/Tooling/ToolExecutorPluginRegistry.h"
#include "toolchain/Support/Regex.h"
#include "toolchain/Support/ThreadPool.h"
#include "toolchain/Support/Threading.h"
#include "toolchain/Support/VirtualFileSystem.h"

namespace language::Core {
namespace tooling {

const char *AllTUsToolExecutor::ExecutorName = "AllTUsToolExecutor";

namespace {
toolchain::Error make_string_error(const toolchain::Twine &Message) {
  return toolchain::make_error<toolchain::StringError>(Message,
                                             toolchain::inconvertibleErrorCode());
}

ArgumentsAdjuster getDefaultArgumentsAdjusters() {
  return combineAdjusters(
      getClangStripOutputAdjuster(),
      combineAdjusters(getClangSyntaxOnlyAdjuster(),
                       getClangStripDependencyFileAdjuster()));
}

class ThreadSafeToolResults : public ToolResults {
public:
  void addResult(StringRef Key, StringRef Value) override {
    std::unique_lock<std::mutex> LockGuard(Mutex);
    Results.addResult(Key, Value);
  }

  std::vector<std::pair<toolchain::StringRef, toolchain::StringRef>>
  AllKVResults() override {
    return Results.AllKVResults();
  }

  void forEachResult(toolchain::function_ref<void(StringRef Key, StringRef Value)>
                         Callback) override {
    Results.forEachResult(Callback);
  }

private:
  InMemoryToolResults Results;
  std::mutex Mutex;
};

} // namespace

toolchain::cl::opt<std::string>
    Filter("filter",
           toolchain::cl::desc("Only process files that match this filter. "
                          "This flag only applies to all-TUs."),
           toolchain::cl::init(".*"));

AllTUsToolExecutor::AllTUsToolExecutor(
    const CompilationDatabase &Compilations, unsigned ThreadCount,
    std::shared_ptr<PCHContainerOperations> PCHContainerOps)
    : Compilations(Compilations), Results(new ThreadSafeToolResults),
      Context(Results.get()), ThreadCount(ThreadCount) {}

AllTUsToolExecutor::AllTUsToolExecutor(
    CommonOptionsParser Options, unsigned ThreadCount,
    std::shared_ptr<PCHContainerOperations> PCHContainerOps)
    : OptionsParser(std::move(Options)),
      Compilations(OptionsParser->getCompilations()),
      Results(new ThreadSafeToolResults), Context(Results.get()),
      ThreadCount(ThreadCount) {}

toolchain::Error AllTUsToolExecutor::execute(
    toolchain::ArrayRef<
        std::pair<std::unique_ptr<FrontendActionFactory>, ArgumentsAdjuster>>
        Actions) {
  if (Actions.empty())
    return make_string_error("No action to execute.");

  if (Actions.size() != 1)
    return make_string_error(
        "Only support executing exactly 1 action at this point.");

  std::string ErrorMsg;
  std::mutex TUMutex;
  auto AppendError = [&](toolchain::Twine Err) {
    std::unique_lock<std::mutex> LockGuard(TUMutex);
    ErrorMsg += Err.str();
  };

  auto Log = [&](toolchain::Twine Msg) {
    std::unique_lock<std::mutex> LockGuard(TUMutex);
    toolchain::errs() << Msg.str() << "\n";
  };

  std::vector<std::string> Files;
  toolchain::Regex RegexFilter(Filter);
  for (const auto& File : Compilations.getAllFiles()) {
    if (RegexFilter.match(File))
      Files.push_back(File);
  }
  // Add a counter to track the progress.
  const std::string TotalNumStr = std::to_string(Files.size());
  unsigned Counter = 0;
  auto Count = [&]() {
    std::unique_lock<std::mutex> LockGuard(TUMutex);
    return ++Counter;
  };

  auto &Action = Actions.front();

  {
    toolchain::DefaultThreadPool Pool(toolchain::hardware_concurrency(ThreadCount));
    for (std::string File : Files) {
      Pool.async(
          [&](std::string Path) {
            Log("[" + std::to_string(Count()) + "/" + TotalNumStr +
                "] Processing file " + Path);
            // Each thread gets an independent copy of a VFS to allow different
            // concurrent working directories.
            IntrusiveRefCntPtr<toolchain::vfs::FileSystem> FS =
                toolchain::vfs::createPhysicalFileSystem();
            ClangTool Tool(Compilations, {Path},
                           std::make_shared<PCHContainerOperations>(), FS);
            Tool.appendArgumentsAdjuster(Action.second);
            Tool.appendArgumentsAdjuster(getDefaultArgumentsAdjusters());
            for (const auto &FileAndContent : OverlayFiles)
              Tool.mapVirtualFile(FileAndContent.first(),
                                  FileAndContent.second);
            if (Tool.run(Action.first.get()))
              AppendError(toolchain::Twine("Failed to run action on ") + Path +
                          "\n");
          },
          File);
    }
    // Make sure all tasks have finished before resetting the working directory.
    Pool.wait();
  }

  if (!ErrorMsg.empty())
    return make_string_error(ErrorMsg);

  return toolchain::Error::success();
}

toolchain::cl::opt<unsigned> ExecutorConcurrency(
    "execute-concurrency",
    toolchain::cl::desc("The number of threads used to process all files in "
                   "parallel. Set to 0 for hardware concurrency. "
                   "This flag only applies to all-TUs."),
    toolchain::cl::init(0));

class AllTUsToolExecutorPlugin : public ToolExecutorPlugin {
public:
  toolchain::Expected<std::unique_ptr<ToolExecutor>>
  create(CommonOptionsParser &OptionsParser) override {
    if (OptionsParser.getSourcePathList().empty())
      return make_string_error(
          "[AllTUsToolExecutorPlugin] Please provide a directory/file path in "
          "the compilation database.");
    return std::make_unique<AllTUsToolExecutor>(std::move(OptionsParser),
                                                 ExecutorConcurrency);
  }
};

static ToolExecutorPluginRegistry::Add<AllTUsToolExecutorPlugin>
    X("all-TUs", "Runs FrontendActions on all TUs in the compilation database. "
                 "Tool results are stored in memory.");

// This anchor is used to force the linker to link in the generated object file
// and thus register the plugin.
volatile int AllTUsToolExecutorAnchorSource = 0;

} // end namespace tooling
} // end namespace language::Core
