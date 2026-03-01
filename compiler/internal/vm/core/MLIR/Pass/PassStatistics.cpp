//===- PassStatistics.cpp -------------------------------------------------===//
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
#include "vm/core/ADT/StringExtras.h"
#include "vm/core/Support/Format.h"

using namespace mlir;
using namespace mlir::detail;

constexpr StringLiteral kPassStatsDescription =
    "... Pass statistics report ...";

namespace {
/// Information pertaining to a specific statistic.
struct Statistic {
  const char *name, *desc;
  uint64_t value;
};
} // namespace

/// Utility to print a pass entry in the statistics output.
static void printPassEntry(raw_ostream &os, unsigned indent, StringRef pass,
                           MutableArrayRef<Statistic> stats = {}) {
  os.indent(indent) << pass << "\n";
  if (stats.empty())
    return;

  // Make sure to sort the statistics by name.
  toolchain::array_pod_sort(
      stats.begin(), stats.end(), [](const auto *lhs, const auto *rhs) {
        return StringRef{lhs->name}.compare(StringRef{rhs->name});
      });

  // Collect the largest name and value length from each of the statistics.
  size_t largestName = 0, largestValue = 0;
  for (auto &stat : stats) {
    largestName = std::max(largestName, (size_t)strlen(stat.name));
    largestValue =
        std::max(largestValue, (size_t)toolchain::utostr(stat.value).size());
  }

  // Print each of the statistics.
  for (auto &stat : stats) {
    os.indent(indent + 2) << toolchain::format("(S) %*u %-*s - %s\n", largestValue,
                                          stat.value, largestName, stat.name,
                                          stat.desc);
  }
}

/// Print the statistics results in a list form, where each pass is sorted by
/// name.
static void printResultsAsList(raw_ostream &os, OpPassManager &pm) {
  toolchain::StringMap<std::vector<Statistic>> mergedStats;
  std::function<void(Pass *)> addStats = [&](Pass *pass) {
    auto *adaptor = dyn_cast<OpToOpPassAdaptor>(pass);

    // If this is not an adaptor, add the stats to the list if there are any.
    if (!adaptor) {
#if LLVM_ENABLE_STATS
      auto statistics = pass->getStatistics();
      if (statistics.empty())
        return;

      auto &passEntry = mergedStats[pass->getName()];
      if (passEntry.empty()) {
        for (Pass::Statistic *it : pass->getStatistics())
          passEntry.push_back({it->getName(), it->getDesc(), it->getValue()});
      } else {
        for (auto [idx, statistic] : toolchain::enumerate(pass->getStatistics()))
          passEntry[idx].value += statistic->getValue();
      }
#endif
      return;
    }

    // Otherwise, recursively add each of the children.
    for (auto &mgr : adaptor->getPassManagers())
      for (Pass &pass : mgr.getPasses())
        addStats(&pass);
  };
  for (Pass &pass : pm.getPasses())
    addStats(&pass);

  // Sort the statistics by pass name and then by record name.
  auto passAndStatistics =
      toolchain::to_vector<16>(toolchain::make_pointer_range(mergedStats));
  toolchain::array_pod_sort(passAndStatistics.begin(), passAndStatistics.end(),
                       [](const decltype(passAndStatistics)::value_type *lhs,
                          const decltype(passAndStatistics)::value_type *rhs) {
                         return (*lhs)->getKey().compare((*rhs)->getKey());
                       });

  // Print the timing information sequentially.
  for (auto &statData : passAndStatistics)
    printPassEntry(os, /*indent=*/2, statData->first(), statData->second);
}

/// Print the results in pipeline mode that mirrors the internal pass manager
/// structure.
static void printResultsAsPipeline(raw_ostream &os, OpPassManager &pm) {
#if LLVM_ENABLE_STATS
  std::function<void(unsigned, Pass *)> printPass = [&](unsigned indent,
                                                        Pass *pass) {
    if (auto *adaptor = dyn_cast<OpToOpPassAdaptor>(pass)) {
      // If this adaptor has more than one internal pipeline, print an entry for
      // it.
      auto mgrs = adaptor->getPassManagers();
      if (mgrs.size() > 1) {
        printPassEntry(os, indent, adaptor->getAdaptorName());
        indent += 2;
      }

      // Print each of the children passes.
      for (OpPassManager &mgr : mgrs) {
        auto name = ("'" + mgr.getOpAnchorName() + "' Pipeline").str();
        printPassEntry(os, indent, name);
        for (Pass &pass : mgr.getPasses())
          printPass(indent + 2, &pass);
      }
      return;
    }

    // Otherwise, we print the statistics for this pass.
    std::vector<Statistic> stats;
    for (Pass::Statistic *stat : pass->getStatistics())
      stats.push_back({stat->getName(), stat->getDesc(), stat->getValue()});
    printPassEntry(os, indent, pass->getName(), stats);
  };
  for (Pass &pass : pm.getPasses())
    printPass(/*indent=*/0, &pass);
#endif
}

static void printStatistics(OpPassManager &pm, PassDisplayMode displayMode) {
  auto os = toolchain::CreateInfoOutputFile();

  // Print the stats header.
  *os << "===" << std::string(73, '-') << "===\n";
  // Figure out how many spaces for the description name.
  unsigned padding = (80 - kPassStatsDescription.size()) / 2;
  os->indent(padding) << kPassStatsDescription << '\n';
  *os << "===" << std::string(73, '-') << "===\n";

  // Defer to a specialized printer for each display mode.
  switch (displayMode) {
  case PassDisplayMode::List:
    printResultsAsList(*os, pm);
    break;
  case PassDisplayMode::Pipeline:
    printResultsAsPipeline(*os, pm);
    break;
  }
  *os << "\n";
  os->flush();
}

//===----------------------------------------------------------------------===//
// PassStatistics
//===----------------------------------------------------------------------===//

Pass::Statistic::Statistic(Pass *owner, const char *name,
                           const char *description)
    : toolchain::Statistic{/*DebugType=*/"", name, description} {
#if LLVM_ENABLE_STATS
  // Always set the 'initialized' bit to true so that this statistic isn't
  // placed in the static registry.
  // TODO: This is sort of hack as `toolchain::Statistic`s can't be setup to avoid
  // automatic registration with the global registry. We should either add
  // support for this in LLVM, or just write our own statistics classes.
  Initialized = true;
#endif

  // Register this statistic with the parent.
  owner->statistics.push_back(this);
}

auto Pass::Statistic::operator=(unsigned value) -> Statistic & {
  toolchain::Statistic::operator=(value);
  return *this;
}

//===----------------------------------------------------------------------===//
// PassManager
//===----------------------------------------------------------------------===//

/// Merge the pass statistics of this class into 'other'.
void OpPassManager::mergeStatisticsInto(OpPassManager &other) {
  auto passes = getPasses(), otherPasses = other.getPasses();

  for (auto passPair : toolchain::zip(passes, otherPasses)) {
    Pass &pass = std::get<0>(passPair), &otherPass = std::get<1>(passPair);

    // If this is an adaptor, then recursively merge the pass managers.
    if (auto *adaptorPass = dyn_cast<OpToOpPassAdaptor>(&pass)) {
      auto *otherAdaptorPass = cast<OpToOpPassAdaptor>(&otherPass);
      for (auto mgrs : toolchain::zip(adaptorPass->getPassManagers(),
                                 otherAdaptorPass->getPassManagers()))
        std::get<0>(mgrs).mergeStatisticsInto(std::get<1>(mgrs));
      continue;
    }
    // Otherwise, merge the statistics for the current pass.
    assert(pass.statistics.size() == otherPass.statistics.size());
    for (unsigned i = 0, e = pass.statistics.size(); i != e; ++i) {
      assert(pass.statistics[i]->getName() ==
             StringRef(otherPass.statistics[i]->getName()));
      *otherPass.statistics[i] += *pass.statistics[i];
      *pass.statistics[i] = 0;
    }
  }
}

/// Prepare the statistics of passes within the given pass manager for
/// consumption(e.g. dumping).
static void prepareStatistics(OpPassManager &pm) {
  for (Pass &pass : pm.getPasses()) {
    OpToOpPassAdaptor *adaptor = dyn_cast<OpToOpPassAdaptor>(&pass);
    if (!adaptor)
      continue;
    MutableArrayRef<OpPassManager> nestedPms = adaptor->getPassManagers();

    // Merge the statistics from the async pass managers into the main nested
    // pass managers.  Prepare recursively before merging.
    for (auto &asyncPM : adaptor->getParallelPassManagers()) {
      for (unsigned i = 0, e = asyncPM.size(); i != e; ++i) {
        prepareStatistics(asyncPM[i]);
        asyncPM[i].mergeStatisticsInto(nestedPms[i]);
      }
    }

    // Prepare the statistics of each of the nested passes.
    for (OpPassManager &nestedPM : nestedPms)
      prepareStatistics(nestedPM);
  }
}

/// Dump the statistics of the passes within this pass manager.
void PassManager::dumpStatistics() {
  prepareStatistics(*this);
  printStatistics(*this, *passStatisticsMode);
}

/// Dump the statistics for each pass after running.
void PassManager::enableStatistics(PassDisplayMode displayMode) {
  passStatisticsMode = displayMode;
}
