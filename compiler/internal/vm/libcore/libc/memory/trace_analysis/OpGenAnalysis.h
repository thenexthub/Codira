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
#include <unordered_map>
#include <utility>

#include <memory_trace/MemoryTrace.h>
#include <memory_trace/TraceInfo.h>

#include "Analysis.h"

// Return the average execution time of each allocation operation.
class OpAverageAnalysis : public Analysis {
 public:
  OpAverageAnalysis() = default;
  virtual ~OpAverageAnalysis() = default;

  const std::string Name() override final { return "OpAverageAnalysis"; }

  bool Gather(memory_trace::TraceInfo& info) override final;

  bool StreamResult(std::ostream& os, const std::string& prefix) override final;

 private:
  std::unordered_map<memory_trace::TypeEnum, std::pair<uint64_t, uint64_t>> stats_;
};

// Return the min/max execution time of each allocation operation.
class OpMinMaxAnalysis : public Analysis {
 public:
  OpMinMaxAnalysis() = default;
  virtual ~OpMinMaxAnalysis() = default;

  const std::string Name() override final { return "OpMinMaxAnalysis"; }

  bool Gather(memory_trace::TraceInfo& info) override final;

  bool StreamResult(std::ostream& os, const std::string& prefix) override final;

 private:
  std::unordered_map<memory_trace::TypeEnum, std::pair<uint64_t, uint64_t>> stats_;
};
