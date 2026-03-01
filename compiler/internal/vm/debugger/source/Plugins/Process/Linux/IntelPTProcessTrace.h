//===-- IntelPTProcessTrace.h --------------------------------- -*- C++ -*-===//
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

#ifndef liblldb_IntelPTProcessTrace_H_
#define liblldb_IntelPTProcessTrace_H_

#include "lldb/Utility/TraceIntelPTGDBRemotePackets.h"
#include <memory>
#include <optional>

namespace lldb_private {
namespace process_linux {

/// Interface to be implemented by each 'process trace' strategy (per cpu, per
/// thread, etc).
class IntelPTProcessTrace {
public:
  virtual ~IntelPTProcessTrace() = default;

  virtual void ProcessDidStop() {}

  virtual void ProcessWillResume() {}

  /// Construct a minimal jLLDBTraceGetState response for this process trace.
  virtual TraceIntelPTGetStateResponse GetState() = 0;

  virtual bool TracesThread(lldb::tid_t tid) const = 0;

  /// \copydoc IntelPTThreadTraceCollection::TraceStart()
  virtual llvm::Error TraceStart(lldb::tid_t tid) = 0;

  /// \copydoc IntelPTThreadTraceCollection::TraceStop()
  virtual llvm::Error TraceStop(lldb::tid_t tid) = 0;

  /// \return
  ///   \b std::nullopt if this instance doesn't support the requested data, an
  ///   \a llvm::Error if this isntance supports it but fails at fetching it,
  ///   and \b Error::success() otherwise.
  virtual llvm::Expected<std::optional<std::vector<uint8_t>>>
  TryGetBinaryData(const TraceGetBinaryDataRequest &request) = 0;
};

using IntelPTProcessTraceUP = std::unique_ptr<IntelPTProcessTrace>;

} // namespace process_linux
} // namespace lldb_private

#endif // liblldb_IntelPTProcessTrace_H_
