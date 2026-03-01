//===-- IntelPTPerThreadProcessTrace.h ------------------------ -*- C++ -*-===//
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

#ifndef liblldb_IntelPTPerThreadProcessTrace_H_
#define liblldb_IntelPTPerThreadProcessTrace_H_

#include "IntelPTProcessTrace.h"
#include "IntelPTSingleBufferTrace.h"
#include "IntelPTThreadTraceCollection.h"
#include <optional>

namespace lldb_private {
namespace process_linux {

/// Manages a "process trace" instance by tracing each thread individually.
class IntelPTPerThreadProcessTrace : public IntelPTProcessTrace {
public:
  /// Start tracing the current process by tracing each of its tids
  /// individually.
  ///
  /// \param[in] request
  ///   Intel PT configuration parameters.
  ///
  /// \param[in] current_tids
  ///   List of tids currently alive. In the future, whenever a new thread is
  ///   spawned, they should be traced by calling the \a TraceStart(tid) method.
  ///
  /// \return
  ///   An \a IntelPTMultiCoreTrace instance if tracing was successful, or
  ///   an \a llvm::Error otherwise.
  static llvm::Expected<std::unique_ptr<IntelPTPerThreadProcessTrace>>
  Start(const TraceIntelPTStartRequest &request,
        llvm::ArrayRef<lldb::tid_t> current_tids);

  bool TracesThread(lldb::tid_t tid) const override;

  llvm::Error TraceStart(lldb::tid_t tid) override;

  llvm::Error TraceStop(lldb::tid_t tid) override;

  TraceIntelPTGetStateResponse GetState() override;

  llvm::Expected<std::optional<std::vector<uint8_t>>>
  TryGetBinaryData(const TraceGetBinaryDataRequest &request) override;

private:
  IntelPTPerThreadProcessTrace(const TraceIntelPTStartRequest &request)
      : m_tracing_params(request) {}

  IntelPTThreadTraceCollection m_thread_traces;
  /// Params used to trace threads when the user started "process tracing".
  TraceIntelPTStartRequest m_tracing_params;
};

} // namespace process_linux
} // namespace lldb_private

#endif // liblldb_IntelPTPerThreadProcessTrace_H_
