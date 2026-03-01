//===-- Transport.h -------------------------------------------------------===//
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
//
// Debug Adapter Protocol transport layer for encoding and decoding protocol
// messages.
//
//===----------------------------------------------------------------------===//

#ifndef LLDB_TOOLS_LLDB_DAP_TRANSPORT_H
#define LLDB_TOOLS_LLDB_DAP_TRANSPORT_H

#include "DAPForward.h"
#include "Protocol/ProtocolBase.h"
#include "lldb/Host/JSONTransport.h"
#include "lldb/lldb-forward.h"
#include "llvm/ADT/StringRef.h"

namespace lldb_dap {

struct ProtocolDescriptor {
  using Id = protocol::Id;
  using Req = protocol::Request;
  using Resp = protocol::Response;
  using Evt = protocol::Event;
};

/// A transport class that performs the Debug Adapter Protocol communication
/// with the client.
class Transport final
    : public lldb_private::transport::HTTPDelimitedJSONTransport<
          ProtocolDescriptor> {
public:
  Transport(lldb_dap::Log &log, lldb::IOObjectSP input,
            lldb::IOObjectSP output);
  virtual ~Transport() = default;

  void Log(llvm::StringRef message) override;

private:
  lldb_dap::Log &m_log;
};

} // namespace lldb_dap

#endif
