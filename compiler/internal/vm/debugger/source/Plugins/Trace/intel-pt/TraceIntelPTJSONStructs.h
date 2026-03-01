//===-- TraceIntelPTJSONStructs.h -----------------------------*- C++ //-*-===//
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

#ifndef LLDB_SOURCE_PLUGINS_TRACE_INTEL_PT_TRACEINTELPTJSONSTRUCTS_H
#define LLDB_SOURCE_PLUGINS_TRACE_INTEL_PT_TRACEINTELPTJSONSTRUCTS_H

#include "lldb/Utility/TraceIntelPTGDBRemotePackets.h"
#include "lldb/lldb-types.h"
#include "llvm/Support/JSON.h"
#include <intel-pt.h>
#include <optional>
#include <vector>

namespace lldb_private {
namespace trace_intel_pt {

struct JSONModule {
  std::string system_path;
  std::optional<std::string> file;
  JSONUINT64 load_address;
  std::optional<std::string> uuid;
};

struct JSONThread {
  uint64_t tid;
  std::optional<std::string> ipt_trace;
};

struct JSONProcess {
  uint64_t pid;
  std::optional<std::string> triple;
  std::vector<JSONThread> threads;
  std::vector<JSONModule> modules;
};

struct JSONCpu {
  lldb::cpu_id_t id;
  std::string ipt_trace;
  std::string context_switch_trace;
};

struct JSONKernel {
  std::optional<JSONUINT64> load_address;
  std::string file;
};

struct JSONTraceBundleDescription {
  std::string type;
  pt_cpu cpu_info;
  std::optional<std::vector<JSONProcess>> processes;
  std::optional<std::vector<JSONCpu>> cpus;
  std::optional<LinuxPerfZeroTscConversion> tsc_perf_zero_conversion;
  std::optional<JSONKernel> kernel;

  std::optional<std::vector<lldb::cpu_id_t>> GetCpuIds();
};

llvm::json::Value toJSON(const JSONModule &module);

llvm::json::Value toJSON(const JSONThread &thread);

llvm::json::Value toJSON(const JSONProcess &process);

llvm::json::Value toJSON(const JSONCpu &cpu);

llvm::json::Value toJSON(const pt_cpu &cpu_info);

llvm::json::Value toJSON(const JSONKernel &kernel);

llvm::json::Value toJSON(const JSONTraceBundleDescription &bundle_description);

bool fromJSON(const llvm::json::Value &value, JSONModule &module,
              llvm::json::Path path);

bool fromJSON(const llvm::json::Value &value, JSONThread &thread,
              llvm::json::Path path);

bool fromJSON(const llvm::json::Value &value, JSONProcess &process,
              llvm::json::Path path);

bool fromJSON(const llvm::json::Value &value, JSONCpu &cpu,
              llvm::json::Path path);

bool fromJSON(const llvm::json::Value &value, pt_cpu &cpu_info,
              llvm::json::Path path);

bool fromJSON(const llvm::json::Value &value, JSONModule &kernel,
              llvm::json::Path path);

bool fromJSON(const llvm::json::Value &value,
              JSONTraceBundleDescription &bundle_description,
              llvm::json::Path path);
} // namespace trace_intel_pt
} // namespace lldb_private

#endif // LLDB_SOURCE_PLUGINS_TRACE_INTEL_PT_TRACEINTELPTJSONSTRUCTS_H
