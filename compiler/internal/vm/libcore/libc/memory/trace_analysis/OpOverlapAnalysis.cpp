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

#include "OpOverlapAnalysis.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <tuple>

#include <memory_trace/MemoryTrace.h>
#include <memory_trace/TraceInfo.h>

bool OpOverlapAnalysis::Gather(memory_trace::TraceInfo& info) {
  if (info.num_entries() <= 1) return true;

  memory_trace::Entry* entries = info.entries();

  auto [cur_start_ns, cur_end_ns] = std::tie(entries[0].start_ns, entries[0].end_ns);
  total_overlapping_ = 0;
  for (size_t i = 1; i < info.num_entries(); ++i) {
    auto [start_ns, end_ns] = std::tie(entries[i].start_ns, entries[i].end_ns);
    if (end_ns <= cur_start_ns) continue;

    if (start_ns >= cur_end_ns) {
      std::tie(cur_start_ns, cur_end_ns) = std::tie(start_ns, end_ns);
    } else if (end_ns < cur_end_ns) {
      total_overlapping_ += end_ns - std::max(start_ns, cur_start_ns);
      cur_start_ns = std::min(start_ns, cur_start_ns);
    } else {
      total_overlapping_ += cur_end_ns - std::max(start_ns, cur_start_ns);
      cur_end_ns = end_ns;
    }
  }

  return true;
}

bool OpOverlapAnalysis::StreamResult(std::ostream& os, const std::string& prefix) {
  os << prefix << "Overlap of all operations is " << total_overlapping_ << " ns" << std::endl;
  return true;
}
