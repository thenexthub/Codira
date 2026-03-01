//===- ExecutionContext.cpp - Debug Execution Context Support -------------===//
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

#include "mlir/Debug/ExecutionContext.h"

#include "vm/core/ADT/ScopeExit.h"
#include "vm/core/Support/FormatVariadic.h"

using namespace mlir;
using namespace mlir::tracing;

//===----------------------------------------------------------------------===//
// ActionActiveStack
//===----------------------------------------------------------------------===//

void ActionActiveStack::print(raw_ostream &os, bool withContext) const {
  os << "ActionActiveStack depth " << getDepth() << "\n";
  const ActionActiveStack *current = this;
  int count = 0;
  while (current) {
    toolchain::errs() << toolchain::formatv("#{0,3}: ", count++);
    current->action.print(toolchain::errs());
    toolchain::errs() << "\n";
    ArrayRef<IRUnit> context = current->action.getContextIRUnits();
    if (withContext && !context.empty()) {
      toolchain::errs() << "Context:\n";
      toolchain::interleave(
          current->action.getContextIRUnits(),
          [&](const IRUnit &unit) {
            toolchain::errs() << "  - ";
            unit.print(toolchain::errs());
          },
          [&]() { toolchain::errs() << "\n"; });
      toolchain::errs() << "\n";
    }
    current = current->parent;
  }
}

//===----------------------------------------------------------------------===//
// ExecutionContext
//===----------------------------------------------------------------------===//

static const LLVM_THREAD_LOCAL ActionActiveStack *actionStack = nullptr;

void ExecutionContext::registerObserver(Observer *observer) {
  observers.push_back(observer);
}

void ExecutionContext::operator()(toolchain::function_ref<void()> transform,
                                  const Action &action) {
  // Update the top of the stack with the current action.
  int depth = 0;
  if (actionStack)
    depth = actionStack->getDepth() + 1;
  ActionActiveStack info{actionStack, action, depth};
  actionStack = &info;
  toolchain::scope_exit raii([&]() { actionStack = info.getParent(); });
  Breakpoint *breakpoint = nullptr;

  // Invoke the callback here and handles control requests here.
  auto handleUserInput = [&]() -> bool {
    if (!onBreakpointControlExecutionCallback)
      return true;
    auto todoNext = onBreakpointControlExecutionCallback(actionStack);
    switch (todoNext) {
    case ExecutionContext::Apply:
      depthToBreak = std::nullopt;
      return true;
    case ExecutionContext::Skip:
      depthToBreak = std::nullopt;
      return false;
    case ExecutionContext::Step:
      depthToBreak = depth + 1;
      return true;
    case ExecutionContext::Next:
      depthToBreak = depth;
      return true;
    case ExecutionContext::Finish:
      depthToBreak = depth - 1;
      return true;
    }
    toolchain::report_fatal_error("Unknown control request");
  };

  // Try to find a breakpoint that would hit on this action.
  // Right now there is no way to collect them all, we stop at the first one.
  for (auto *breakpointManager : breakpoints) {
    breakpoint = breakpointManager->match(action);
    if (breakpoint)
      break;
  }
  info.setBreakpoint(breakpoint);

  bool shouldExecuteAction = true;
  // If we have a breakpoint, or if `depthToBreak` was previously set and the
  // current depth matches, we invoke the user-provided callback.
  if (breakpoint || (depthToBreak && depth <= depthToBreak))
    shouldExecuteAction = handleUserInput();

  // Notify the observers about the current action.
  for (auto *observer : observers)
    observer->beforeExecute(actionStack, breakpoint, shouldExecuteAction);

  if (shouldExecuteAction) {
    // Execute the action here.
    transform();

    // Notify the observers about completion of the action.
    for (auto *observer : observers)
      observer->afterExecute(actionStack);
  }

  if (depthToBreak && depth <= depthToBreak)
    handleUserInput();
}
