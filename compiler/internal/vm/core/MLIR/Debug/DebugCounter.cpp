//===- DebugCounter.cpp - Debug Counter Facilities ------------------------===//
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

#include "mlir/Debug/Counter.h"
#include "vm/core/Support/CommandLine.h"
#include "vm/core/Support/Debug.h"
#include "vm/core/Support/Format.h"
#include "vm/core/Support/ManagedStatic.h"

using namespace mlir;
using namespace mlir::tracing;

//===----------------------------------------------------------------------===//
// DebugCounter CommandLine Options
//===----------------------------------------------------------------------===//

namespace {
/// This struct contains command line options that can be used to initialize
/// various bits of a DebugCounter. This uses a struct wrapper to avoid the need
/// for global command line options.
struct DebugCounterOptions {
  toolchain::cl::list<std::string> counters{
      "mlir-debug-counter",
      toolchain::cl::desc(
          "Comma separated list of debug counter skip and count arguments"),
      toolchain::cl::CommaSeparated};

  toolchain::cl::opt<bool> printCounterInfo{
      "mlir-print-debug-counter", toolchain::cl::init(false), toolchain::cl::Optional,
      toolchain::cl::desc("Print out debug counter information after all counters "
                     "have been accumulated")};
};
} // namespace

static toolchain::ManagedStatic<DebugCounterOptions> clOptions;

//===----------------------------------------------------------------------===//
// DebugCounter
//===----------------------------------------------------------------------===//

DebugCounter::DebugCounter() { applyCLOptions(); }

DebugCounter::~DebugCounter() {
  // Print information when destroyed, iff command line option is specified.
  if (clOptions.isConstructed() && clOptions->printCounterInfo)
    print(toolchain::dbgs());
}

/// Add a counter for the given debug action tag. `countToSkip` is the number
/// of counter executions to skip before enabling execution of the action.
/// `countToStopAfter` is the number of executions of the counter to allow
/// before preventing the action from executing any more.
void DebugCounter::addCounter(StringRef actionTag, int64_t countToSkip,
                              int64_t countToStopAfter) {
  assert(!counters.count(actionTag) &&
         "a counter for the given action was already registered");
  counters.try_emplace(actionTag, countToSkip, countToStopAfter);
}

void DebugCounter::operator()(toolchain::function_ref<void()> transform,
                              const Action &action) {
  if (shouldExecute(action.getTag()))
    transform();
}

bool DebugCounter::shouldExecute(StringRef tag) {
  auto counterIt = counters.find(tag);
  if (counterIt == counters.end())
    return true;

  ++counterIt->second.count;

  // We only execute while the `countToSkip` is not smaller than `count`, and
  // `countToStopAfter + countToSkip` is larger than `count`. Negative counters
  // always execute.
  if (counterIt->second.countToSkip < 0)
    return true;
  if (counterIt->second.countToSkip >= counterIt->second.count)
    return false;
  if (counterIt->second.countToStopAfter < 0)
    return true;
  return counterIt->second.countToStopAfter + counterIt->second.countToSkip >=
         counterIt->second.count;
}

void DebugCounter::print(raw_ostream &os) const {
  // Order the registered counters by name.
  SmallVector<const toolchain::StringMapEntry<Counter> *, 16> sortedCounters(
      toolchain::make_pointer_range(counters));
  toolchain::array_pod_sort(sortedCounters.begin(), sortedCounters.end(),
                       [](const decltype(sortedCounters)::value_type *lhs,
                          const decltype(sortedCounters)::value_type *rhs) {
                         return (*lhs)->getKey().compare((*rhs)->getKey());
                       });

  os << "DebugCounter counters:\n";
  for (const toolchain::StringMapEntry<Counter> *counter : sortedCounters) {
    os << toolchain::left_justify(counter->getKey(), 32) << ": {"
       << counter->second.count << "," << counter->second.countToSkip << ","
       << counter->second.countToStopAfter << "}\n";
  }
}

/// Register a set of useful command-line options that can be used to configure
/// various flags within the DebugCounter. These flags are used when
/// constructing a DebugCounter for initialization.
void DebugCounter::registerCLOptions() {
  // Make sure that the options struct has been initialized.
  *clOptions;
}

bool DebugCounter::isActivated() {
  return clOptions->counters.getNumOccurrences() ||
         clOptions->printCounterInfo.getNumOccurrences();
}

// This is called by the command line parser when it sees a value for the
// debug-counter option defined above.
void DebugCounter::applyCLOptions() {
  if (!clOptions.isConstructed())
    return;

  for (StringRef arg : clOptions->counters) {
    if (arg.empty())
      continue;

    // Debug counter arguments are expected to be in the form: `counter=value`.
    auto [counterName, counterValueStr] = arg.split('=');
    if (counterValueStr.empty()) {
      toolchain::errs() << "error: expected DebugCounter argument to have an `=` "
                      "separating the counter name and value, but the provided "
                      "argument was: `"
                   << arg << "`\n";
      toolchain::report_fatal_error(
          "Invalid DebugCounter command-line configuration");
    }

    // Extract the counter value.
    int64_t counterValue;
    if (counterValueStr.getAsInteger(0, counterValue)) {
      toolchain::errs() << "error: expected DebugCounter counter value to be "
                      "numeric, but got `"
                   << counterValueStr << "`\n";
      toolchain::report_fatal_error(
          "Invalid DebugCounter command-line configuration");
    }

    // Now we need to see if this is the skip or the count, remove the suffix,
    // and add it to the counter values.
    if (counterName.consume_back("-skip")) {
      counters[counterName].countToSkip = counterValue;

    } else if (counterName.consume_back("-count")) {
      counters[counterName].countToStopAfter = counterValue;

    } else {
      toolchain::errs() << "error: expected DebugCounter counter name to end with "
                      "either `-skip` or `-count`, but got`"
                   << counterName << "`\n";
      toolchain::report_fatal_error(
          "Invalid DebugCounter command-line configuration");
    }
  }
}
