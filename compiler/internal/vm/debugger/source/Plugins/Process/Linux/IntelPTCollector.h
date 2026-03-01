//===-- IntelPTCollector.h ------------------------------------ -*- C++ -*-===//
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

#ifndef liblldb_IntelPTCollector_H_
#define liblldb_IntelPTCollector_H_

#include "IntelPTMultiCoreTrace.h"
#include "IntelPTPerThreadProcessTrace.h"
#include "IntelPTSingleBufferTrace.h"
#include "Perf.h"
#include "lldb/Host/common/NativeProcessProtocol.h"
#include "lldb/Utility/Status.h"
#include "lldb/Utility/TraceIntelPTGDBRemotePackets.h"
#include "lldb/lldb-types.h"
#include <linux/perf_event.h>
#include <sys/mman.h>
#include <unistd.h>

namespace lldb_private {

namespace process_linux {

/// Main class that manages intel-pt process and thread tracing.
class IntelPTCollector {
public:
  /// \param[in] process
  ///     Process to be traced.
  IntelPTCollector(NativeProcessProtocol &process);

  static bool IsSupported();

  /// To be invoked as soon as we know the process stopped.
  void ProcessDidStop();

  /// To be invoked before the process will resume, so that we can capture the
  /// first instructions after the resume.
  void ProcessWillResume();

  /// If "process tracing" is enabled, then trace the given thread.
  llvm::Error OnThreadCreated(lldb::tid_t tid);

  /// Stops tracing a tracing upon a destroy event.
  llvm::Error OnThreadDestroyed(lldb::tid_t tid);

  /// Implementation of the jLLDBTraceStop packet
  llvm::Error TraceStop(const TraceStopRequest &request);

  /// Implementation of the jLLDBTraceStart packet
  llvm::Error TraceStart(const TraceIntelPTStartRequest &request);

  /// Implementation of the jLLDBTraceGetState packet
  llvm::Expected<llvm::json::Value> GetState();

  /// Implementation of the jLLDBTraceGetBinaryData packet
  llvm::Expected<std::vector<uint8_t>>
  GetBinaryData(const TraceGetBinaryDataRequest &request);

  /// Dispose of all traces
  void Clear();

private:
  llvm::Error TraceStop(lldb::tid_t tid);

  /// Start tracing a specific thread.
  llvm::Error TraceStart(lldb::tid_t tid,
                         const TraceIntelPTStartRequest &request);

  /// \return
  ///   The conversion object between TSC and wall time.
  llvm::Expected<LinuxPerfZeroTscConversion &>
  FetchPerfTscConversionParameters();

  /// The target process.
  NativeProcessProtocol &m_process;
  /// Threads traced due to "thread tracing"
  IntelPTThreadTraceCollection m_thread_traces;

  /// Only one instance of "process trace" can be active at a given time.
  /// It might be \b nullptr.
  IntelPTProcessTraceUP m_process_trace_up;
};

} // namespace process_linux
} // namespace lldb_private

#endif // liblldb_IntelPTCollector_H_
