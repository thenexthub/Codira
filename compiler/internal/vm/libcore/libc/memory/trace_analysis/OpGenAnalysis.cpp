/*
 * Copyright (C) 2025 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "OpGenAnalysis.h"

#include <algorithm>
#include <ostream>
#include <string>
#include <unordered_map>
#include <utility>

#include <memory_trace/MemoryTrace.h>
#include <memory_trace/TraceInfo.h>

bool OpAverageAnalysis::Gather(memory_trace::TraceInfo& info) {
  memory_trace::Entry* entries = info.entries();
  for (size_t i = 0; i < info.num_entries(); i++) {
    auto& iter = stats_[entries[i].type];
    iter.first += entries[i].end_ns - entries[i].start_ns;
    ++iter.second;
  }
  return true;
}

bool OpAverageAnalysis::StreamResult(std::ostream& os, const std::string& prefix) {
  constexpr memory_trace::TypeEnum outputOrder[] = {memory_trace::MALLOC, memory_trace::CALLOC,
                                                    memory_trace::MEMALIGN, memory_trace::REALLOC,
                                                    memory_trace::FREE};
  for (memory_trace::TypeEnum type : outputOrder) {
    auto [total_time, total_count] = stats_[type];
    os << prefix << memory_trace::TypeToName(type) << ": avg exec time = ";
    if (total_count == 0) {
      os << "0.0 ns" << std::endl;
    } else {
      os << total_time / total_count;
      uint64_t fraction = total_time % total_count;
      os << "." << (fraction * 1000) / total_count << " ns" << std::endl;
    }
  }

  return true;
}

bool OpMinMaxAnalysis::Gather(memory_trace::TraceInfo& info) {
  memory_trace::Entry* entries = info.entries();
  for (size_t i = 0; i < info.num_entries(); i++) {
    auto& iter = stats_[entries[i].type];
    uint64_t period = entries[i].end_ns - entries[i].start_ns;
    iter.first = iter.first == 0 ? period : std::min(iter.first, period);
    iter.second = std::max(iter.second, period);
  }

  return true;
}

bool OpMinMaxAnalysis::StreamResult(std::ostream& os, const std::string& prefix) {
  constexpr memory_trace::TypeEnum outputOrder[] = {memory_trace::MALLOC, memory_trace::CALLOC,
                                                    memory_trace::MEMALIGN, memory_trace::REALLOC,
                                                    memory_trace::FREE};
  for (memory_trace::TypeEnum type : outputOrder) {
    auto [min_exec, max_exec] = stats_[type];
    os << prefix << memory_trace::TypeToName(type) << ": min exec time = " << min_exec
       << " ns, max exec time = " << max_exec << " ns" << std::endl;
  }

  return true;
}
