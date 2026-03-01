//===-- IntelPTThreadTraceCollection.cpp ----------------------------------===//
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

#include "IntelPTThreadTraceCollection.h"
#include <optional>

using namespace lldb;
using namespace lldb_private;
using namespace process_linux;
using namespace llvm;

bool IntelPTThreadTraceCollection::TracesThread(lldb::tid_t tid) const {
  return m_thread_traces.count(tid);
}

Error IntelPTThreadTraceCollection::TraceStop(lldb::tid_t tid) {
  auto it = m_thread_traces.find(tid);
  if (it == m_thread_traces.end())
    return createStringError(inconvertibleErrorCode(),
                             "Thread %" PRIu64 " not currently traced", tid);
  m_total_buffer_size -= it->second.GetIptTraceSize();
  m_thread_traces.erase(tid);
  return Error::success();
}

Error IntelPTThreadTraceCollection::TraceStart(
    lldb::tid_t tid, const TraceIntelPTStartRequest &request) {
  if (TracesThread(tid))
    return createStringError(inconvertibleErrorCode(),
                             "Thread %" PRIu64 " already traced", tid);

  Expected<IntelPTSingleBufferTrace> trace =
      IntelPTSingleBufferTrace::Start(request, tid);
  if (!trace)
    return trace.takeError();

  m_total_buffer_size += trace->GetIptTraceSize();
  m_thread_traces.try_emplace(tid, std::move(*trace));
  return Error::success();
}

size_t IntelPTThreadTraceCollection::GetTotalBufferSize() const {
  return m_total_buffer_size;
}

void IntelPTThreadTraceCollection::ForEachThread(
    std::function<void(lldb::tid_t tid, IntelPTSingleBufferTrace &thread_trace)>
        callback) {
  for (auto &it : m_thread_traces)
    callback(it.first, it.second);
}

Expected<IntelPTSingleBufferTrace &>
IntelPTThreadTraceCollection::GetTracedThread(lldb::tid_t tid) {
  auto it = m_thread_traces.find(tid);
  if (it == m_thread_traces.end())
    return createStringError(inconvertibleErrorCode(),
                             "Thread %" PRIu64 " not currently traced", tid);
  return it->second;
}

void IntelPTThreadTraceCollection::Clear() {
  m_thread_traces.clear();
  m_total_buffer_size = 0;
}

size_t IntelPTThreadTraceCollection::GetTracedThreadsCount() const {
  return m_thread_traces.size();
}

llvm::Expected<std::optional<std::vector<uint8_t>>>
IntelPTThreadTraceCollection::TryGetBinaryData(
    const TraceGetBinaryDataRequest &request) {
  if (!request.tid)
    return std::nullopt;
  if (request.kind != IntelPTDataKinds::kIptTrace)
    return std::nullopt;

  if (!TracesThread(*request.tid))
    return std::nullopt;

  if (Expected<IntelPTSingleBufferTrace &> trace =
          GetTracedThread(*request.tid))
    return trace->GetIptTrace();
  else
    return trace.takeError();
}
