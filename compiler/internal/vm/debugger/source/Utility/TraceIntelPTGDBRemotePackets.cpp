//===-- TraceIntelPTGDBRemotePackets.cpp ------------------------*- C++ -*-===//
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

#include "lldb/Utility/TraceIntelPTGDBRemotePackets.h"

using namespace llvm;
using namespace llvm::json;

namespace lldb_private {

const char *IntelPTDataKinds::kProcFsCpuInfo = "procfsCpuInfo";
const char *IntelPTDataKinds::kIptTrace = "iptTrace";
const char *IntelPTDataKinds::kPerfContextSwitchTrace =
    "perfContextSwitchTrace";

bool TraceIntelPTStartRequest::IsPerCpuTracing() const {
  return per_cpu_tracing.value_or(false);
}

json::Value toJSON(const JSONUINT64 &uint64, bool hex) {
  if (hex)
    return json::Value(formatv("{0:x+}", uint64.value));
  else
    return json::Value(formatv("{0}", uint64.value));
}

bool fromJSON(const json::Value &value, JSONUINT64 &uint64, Path path) {
  if (std::optional<uint64_t> val = value.getAsUINT64()) {
    uint64.value = *val;
    return true;
  } else if (std::optional<StringRef> val = value.getAsString()) {
    if (!val->getAsInteger(/*radix=*/0, uint64.value))
      return true;
    path.report("invalid string number");
  }
  path.report("invalid number or string number");
  return false;
}

bool fromJSON(const json::Value &value, TraceIntelPTStartRequest &packet,
              Path path) {
  ObjectMapper o(value, path);
  if (!(o && fromJSON(value, (TraceStartRequest &)packet, path) &&
        o.map("enableTsc", packet.enable_tsc) &&
        o.map("psbPeriod", packet.psb_period) &&
        o.map("iptTraceSize", packet.ipt_trace_size)))
    return false;

  if (packet.IsProcessTracing()) {
    if (!o.map("processBufferSizeLimit", packet.process_buffer_size_limit) ||
        !o.map("perCpuTracing", packet.per_cpu_tracing) ||
        !o.map("disableCgroupTracing", packet.disable_cgroup_filtering))
      return false;
  }
  return true;
}

json::Value toJSON(const TraceIntelPTStartRequest &packet) {
  json::Value base = toJSON((const TraceStartRequest &)packet);
  json::Object &obj = *base.getAsObject();
  obj.try_emplace("iptTraceSize", packet.ipt_trace_size);
  obj.try_emplace("processBufferSizeLimit", packet.process_buffer_size_limit);
  obj.try_emplace("psbPeriod", packet.psb_period);
  obj.try_emplace("enableTsc", packet.enable_tsc);
  obj.try_emplace("perCpuTracing", packet.per_cpu_tracing);
  obj.try_emplace("disableCgroupTracing", packet.disable_cgroup_filtering);
  return base;
}

uint64_t LinuxPerfZeroTscConversion::ToNanos(uint64_t tsc) const {
  uint64_t quot = tsc >> time_shift;
  uint64_t rem_flag = (((uint64_t)1 << time_shift) - 1);
  uint64_t rem = tsc & rem_flag;
  return time_zero.value + quot * time_mult + ((rem * time_mult) >> time_shift);
}

uint64_t LinuxPerfZeroTscConversion::ToTSC(uint64_t nanos) const {
  uint64_t time = nanos - time_zero.value;
  uint64_t quot = time / time_mult;
  uint64_t rem = time % time_mult;
  return (quot << time_shift) + (rem << time_shift) / time_mult;
}

json::Value toJSON(const LinuxPerfZeroTscConversion &packet) {
  return json::Value(json::Object{
      {"timeMult", packet.time_mult},
      {"timeShift", packet.time_shift},
      {"timeZero", toJSON(packet.time_zero, /*hex=*/false)},
  });
}

bool fromJSON(const json::Value &value, LinuxPerfZeroTscConversion &packet,
              json::Path path) {
  ObjectMapper o(value, path);
  uint64_t time_mult, time_shift;
  if (!(o && o.map("timeMult", time_mult) && o.map("timeShift", time_shift) &&
        o.map("timeZero", packet.time_zero)))
    return false;
  packet.time_mult = time_mult;
  packet.time_shift = time_shift;
  return true;
}

bool fromJSON(const json::Value &value, TraceIntelPTGetStateResponse &packet,
              json::Path path) {
  ObjectMapper o(value, path);
  return o && fromJSON(value, (TraceGetStateResponse &)packet, path) &&
         o.map("tscPerfZeroConversion", packet.tsc_perf_zero_conversion) &&
         o.map("usingCgroupFiltering", packet.using_cgroup_filtering);
}

json::Value toJSON(const TraceIntelPTGetStateResponse &packet) {
  json::Value base = toJSON((const TraceGetStateResponse &)packet);
  json::Object &obj = *base.getAsObject();
  obj.insert({"tscPerfZeroConversion", packet.tsc_perf_zero_conversion});
  obj.insert({"usingCgroupFiltering", packet.using_cgroup_filtering});
  return base;
}

} // namespace lldb_private
