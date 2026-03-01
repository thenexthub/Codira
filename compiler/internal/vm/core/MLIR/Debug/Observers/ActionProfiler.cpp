//===- ActionProfiler.cpp -  Profiling Actions *- C++ -*-=====================//
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

#include "mlir/Debug/Observers/ActionProfiler.h"
#include "mlir/Debug/BreakpointManager.h"
#include "mlir/IR/Action.h"
#include "vm/core/Support/Threading.h"
#include "vm/core/Support/raw_ostream.h"
#include <chrono>

using namespace mlir;
using namespace mlir::tracing;

//===----------------------------------------------------------------------===//
// ActionProfiler
//===----------------------------------------------------------------------===//
void ActionProfiler::beforeExecute(const ActionActiveStack *action,
                                   Breakpoint *breakpoint, bool willExecute) {
  print(action, "B"); // begin event.
}

void ActionProfiler::afterExecute(const ActionActiveStack *action) {
  print(action, "E"); // end event.
}

// Print an event in JSON format.
void ActionProfiler::print(const ActionActiveStack *action,
                           toolchain::StringRef phase) {
  // Create the event.
  std::string str;
  toolchain::raw_string_ostream event(str);
  event << "{";
  event << R"("name": ")" << action->getAction().getTag() << "\", ";
  event << R"("cat": "PERF", )";
  event << R"("ph": ")" << phase << "\", ";
  event << R"("pid": 0, )";
  event << R"("tid": )" << toolchain::get_threadid() << ", ";
  auto ts = std::chrono::steady_clock::now() - startTime;
  event << R"("ts": )"
        << std::chrono::duration_cast<std::chrono::microseconds>(ts).count();
  if (phase == "B") {
    event << R"(, "args": {)";
    event << R"("desc": ")";
    action->getAction().print(event);
    event << "\"}";
  }
  event << "}";

  // Print the event.
  std::lock_guard<std::mutex> guard(mutex);
  if (printComma)
    os << ",\n";
  printComma = true;
  os << str;
  os.flush();
}
