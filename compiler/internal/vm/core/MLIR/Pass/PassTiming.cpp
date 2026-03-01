//===- PassTiming.cpp -----------------------------------------------------===//
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
#include "mlir/Pass/PassManager.h"
#include "vm/core/ADT/SmallVector.h"
#include "vm/core/Support/Threading.h"

#include <optional>

using namespace mlir;
using namespace mlir::detail;

//===----------------------------------------------------------------------===//
// PassTiming
//===----------------------------------------------------------------------===//

namespace {
struct PassTiming : public PassInstrumentation {
  PassTiming(TimingScope &timingScope) : rootScope(timingScope) {}
  PassTiming(std::unique_ptr<TimingManager> tm)
      : ownedTimingManager(std::move(tm)),
        ownedTimingScope(ownedTimingManager->getRootScope()),
        rootScope(ownedTimingScope) {}

  /// If a pass can spawn additional work on other threads, it records the
  /// index to its currently active timer here. Passes that run on a
  /// newly-forked thread will check this list to find the active timer of the
  /// parent thread into which the new thread should be nested.
  DenseMap<PipelineParentInfo, unsigned> parentTimerIndices;

  /// The timing manager owned by this instrumentation (in case timing was
  /// enabled by the user on the pass manager without providing an external
  /// timing manager). This *must* appear before the `ownedTimingScope` to
  /// ensure the timing manager is destroyed *after* the scope, since the latter
  /// may hold a timer that points into the former.
  std::unique_ptr<TimingManager> ownedTimingManager;
  TimingScope ownedTimingScope;

  /// A stack of the currently active timing scopes per thread.
  DenseMap<uint64_t, SmallVector<TimingScope, 4>> activeThreadTimers;

  /// The root timing scope into which timing is reported.
  TimingScope &rootScope;

  //===--------------------------------------------------------------------===//
  // Pipeline
  //===--------------------------------------------------------------------===//

  void runBeforePipeline(std::optional<OperationName> name,
                         const PipelineParentInfo &parentInfo) override {
    auto tid = toolchain::get_threadid();
    auto &activeTimers = activeThreadTimers[tid];

    // Find the parent scope, either using the parent info or the root scope
    // (e.g. in the case of the top-level pipeline).
    TimingScope *parentScope;
    auto it = parentTimerIndices.find(parentInfo);
    if (it != parentTimerIndices.end())
      parentScope = &activeThreadTimers[parentInfo.parentThreadID][it->second];
    else
      parentScope = &rootScope;

    // Use nullptr to anchor op-agnostic pipelines, otherwise use the name of
    // the operation.
    const void *timerId = name ? name->getAsOpaquePointer() : nullptr;
    activeTimers.push_back(parentScope->nest(timerId, [name] {
      return ("'" + (name ? name->getStringRef() : "any") + "' Pipeline").str();
    }));
  }

  void runAfterPipeline(std::optional<OperationName>,
                        const PipelineParentInfo &) override {
    auto &activeTimers = activeThreadTimers[toolchain::get_threadid()];
    assert(!activeTimers.empty() && "expected active timer");
    activeTimers.pop_back();
  }

  //===--------------------------------------------------------------------===//
  // Pass
  //===--------------------------------------------------------------------===//

  void runBeforePass(Pass *pass, Operation *) override {
    auto tid = toolchain::get_threadid();
    auto &activeTimers = activeThreadTimers[tid];
    auto &parentScope = activeTimers.empty() ? rootScope : activeTimers.back();

    if (auto *adaptor = dyn_cast<OpToOpPassAdaptor>(pass)) {
      parentTimerIndices[{tid, pass}] = activeTimers.size();
      auto scope =
          parentScope.nest(pass->getThreadingSiblingOrThis(),
                           [adaptor]() { return adaptor->getAdaptorName(); });
      if (adaptor->getPassManagers().size() <= 1)
        scope.hide();
      activeTimers.push_back(std::move(scope));
    } else {
      activeTimers.push_back(
          parentScope.nest(pass->getThreadingSiblingOrThis(),
                           [pass]() { return std::string(pass->getName()); }));
    }
  }

  void runAfterPass(Pass *pass, Operation *) override {
    auto tid = toolchain::get_threadid();
    if (isa<OpToOpPassAdaptor>(pass))
      parentTimerIndices.erase({tid, pass});
    auto &activeTimers = activeThreadTimers[tid];
    assert(!activeTimers.empty() && "expected active timer");
    activeTimers.pop_back();
  }

  void runAfterPassFailed(Pass *pass, Operation *op) override {
    runAfterPass(pass, op);
  }

  //===--------------------------------------------------------------------===//
  // Analysis
  //===--------------------------------------------------------------------===//

  void runBeforeAnalysis(StringRef name, TypeID id, Operation *) override {
    auto tid = toolchain::get_threadid();
    auto &activeTimers = activeThreadTimers[tid];
    auto &parentScope = activeTimers.empty() ? rootScope : activeTimers.back();
    activeTimers.push_back(parentScope.nest(
        id.getAsOpaquePointer(), [name] { return "(A) " + name.str(); }));
  }

  void runAfterAnalysis(StringRef, TypeID, Operation *) override {
    auto &activeTimers = activeThreadTimers[toolchain::get_threadid()];
    assert(!activeTimers.empty() && "expected active timer");
    activeTimers.pop_back();
  }
};
} // namespace

//===----------------------------------------------------------------------===//
// PassManager
//===----------------------------------------------------------------------===//

/// Add an instrumentation to time the execution of passes and the computation
/// of analyses.
void PassManager::enableTiming(TimingScope &timingScope) {
  if (!timingScope)
    return;
  addInstrumentation(std::make_unique<PassTiming>(timingScope));
}

/// Add an instrumentation to time the execution of passes and the computation
/// of analyses.
void PassManager::enableTiming(std::unique_ptr<TimingManager> tm) {
  if (!tm->getRootTimer())
    return; // no need to keep the timing manager around if it's disabled
  addInstrumentation(std::make_unique<PassTiming>(std::move(tm)));
}

/// Add an instrumentation to time the execution of passes and the computation
/// of analyses.
void PassManager::enableTiming() {
  auto tm = std::make_unique<DefaultTimingManager>();
  tm->setEnabled(true);
  enableTiming(std::move(tm));
}
