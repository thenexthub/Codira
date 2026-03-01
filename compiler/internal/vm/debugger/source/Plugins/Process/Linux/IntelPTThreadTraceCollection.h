//===-- IntelPTThreadTraceCollection.h ------------------------ -*- C++ -*-===//
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

#ifndef liblldb_IntelPTPerThreadTraceCollection_H_
#define liblldb_IntelPTPerThreadTraceCollection_H_

#include "IntelPTSingleBufferTrace.h"
#include <optional>

namespace lldb_private {
namespace process_linux {

/// Manages a list of thread traces.
class IntelPTThreadTraceCollection {
public:
  IntelPTThreadTraceCollection() {}

  /// Dispose of all traces
  void Clear();

  /// \return
  ///   \b true if and only if this instance of tracing the provided \p tid.
  bool TracesThread(lldb::tid_t tid) const;

  /// \return
  ///   The total sum of the intel pt trace buffer sizes used by this
  ///   collection.
  size_t GetTotalBufferSize() const;

  /// Execute the provided callback on each thread that is being traced.
  ///
  /// \param[in] callback.tid
  ///   The id of the thread that is being traced.
  ///
  /// \param[in] callback.core_trace
  ///   The single-buffer trace instance for the given core.
  void ForEachThread(std::function<void(lldb::tid_t tid,
                                        IntelPTSingleBufferTrace &thread_trace)>
                         callback);

  llvm::Expected<IntelPTSingleBufferTrace &> GetTracedThread(lldb::tid_t tid);

  /// Start tracing the thread given by its \p tid.
  ///
  /// \return
  ///   An error if the operation failed.
  llvm::Error TraceStart(lldb::tid_t tid,
                         const TraceIntelPTStartRequest &request);

  /// Stop tracing the thread given by its \p tid.
  ///
  /// \return
  ///   An error if the given thread is not being traced or tracing couldn't be
  ///   stopped.
  llvm::Error TraceStop(lldb::tid_t tid);

  size_t GetTracedThreadsCount() const;

  /// \copydoc IntelPTProcessTrace::TryGetBinaryData()
  llvm::Expected<std::optional<std::vector<uint8_t>>>
  TryGetBinaryData(const TraceGetBinaryDataRequest &request);

private:
  llvm::DenseMap<lldb::tid_t, IntelPTSingleBufferTrace> m_thread_traces;
  /// Total actual thread buffer size in bytes
  size_t m_total_buffer_size = 0;
};

} // namespace process_linux
} // namespace lldb_private

#endif // liblldb_IntelPTPerThreadTraceCollection_H_
