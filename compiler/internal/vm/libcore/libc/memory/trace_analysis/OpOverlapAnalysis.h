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

#pragma once

#include <ostream>
#include <string>

#include <memory_trace/TraceInfo.h>

#include "Analysis.h"

// Accumulate the overlap in time intervals of a trace with at least two allocation
// entries.
// For example,
//
//   1234: malloc 0x0 32 1 100
//   5678: malloc 0x0 32 50 120
//
// The interval [50, 100] contains two malloc calls that overlap by 51 nanoseconds.
class OpOverlapAnalysis : public Analysis {
 public:
  OpOverlapAnalysis() = default;
  virtual ~OpOverlapAnalysis() = default;

  const std::string Name() override final { return "OpOverlapAnalysis"; }

  bool Gather(memory_trace::TraceInfo& info) override final;

  bool StreamResult(std::ostream& os, const std::string& prefix) override final;

 private:
  uint64_t total_overlapping_;
};
