//===- ActionLogging.cpp -  Logging Actions *- C++ -*-========================//
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

#include "mlir/Debug/Observers/ActionLogging.h"
#include "mlir/Debug/BreakpointManager.h"
#include "mlir/IR/Action.h"
#include "vm/core/Support/InterleavedRange.h"
#include "vm/core/Support/Threading.h"
#include "vm/core/Support/raw_ostream.h"

using namespace mlir;
using namespace mlir::tracing;

//===----------------------------------------------------------------------===//
// ActionLogger
//===----------------------------------------------------------------------===//

bool ActionLogger::shouldLog(const ActionActiveStack *action) {
  // If some condition was set, we ensured it is met before logging.
  if (breakpointManagers.empty())
    return true;
  return toolchain::any_of(breakpointManagers,
                      [&](const BreakpointManager *manager) {
                        return manager->match(action->getAction());
                      });
}

void ActionLogger::beforeExecute(const ActionActiveStack *action,
                                 Breakpoint *breakpoint, bool willExecute) {
  if (!shouldLog(action))
    return;
  SmallVector<char> name;
  toolchain::get_thread_name(name);
  if (name.empty()) {
    toolchain::raw_svector_ostream os(name);
    os << toolchain::get_threadid();
  }
  os << "[thread " << name << "] ";
  if (willExecute)
    os << "begins ";
  else
    os << "skipping ";
  if (printBreakpoints) {
    if (breakpoint)
      os << "(on breakpoint: " << *breakpoint << ") ";
    else
      os << "(no breakpoint) ";
  }
  os << "Action ";
  if (printActions)
    action->getAction().print(os);
  else
    os << action->getAction().getTag();
  if (printIRUnits)
    os << " (" << toolchain::interleaved(action->getAction().getContextIRUnits())
       << ")";
  os << "`\n";
}

void ActionLogger::afterExecute(const ActionActiveStack *action) {
  if (!shouldLog(action))
    return;
  SmallVector<char> name;
  toolchain::get_thread_name(name);
  if (name.empty()) {
    toolchain::raw_svector_ostream os(name);
    os << toolchain::get_threadid();
  }
  os << "[thread " << name << "] completed `" << action->getAction().getTag()
     << "`\n";
}
