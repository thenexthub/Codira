//===----------------------------------------------------------------------===//
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

#include "lldb/Protocol/MCP/Transport.h"
#include "llvm/ADT/StringRef.h"
#include <utility>

using namespace lldb_protocol::mcp;
using namespace llvm;

Transport::Transport(lldb::IOObjectSP in, lldb::IOObjectSP out,
                     LogCallback log_callback)
    : JSONRPCTransport(in, out), m_log_callback(std::move(log_callback)) {}

void Transport::Log(StringRef message) {
  if (m_log_callback)
    m_log_callback(message);
}
