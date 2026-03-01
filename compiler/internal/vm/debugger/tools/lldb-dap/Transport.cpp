//===-- Transport.cpp -----------------------------------------------------===//
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

#include "Transport.h"
#include "DAPLog.h"
#include "lldb/lldb-forward.h"
#include "llvm/ADT/StringRef.h"

using namespace llvm;
using namespace lldb;
using namespace lldb_private;

namespace lldb_dap {

Transport::Transport(lldb_dap::Log &log, lldb::IOObjectSP input,
                     lldb::IOObjectSP output)
    : HTTPDelimitedJSONTransport(input, output), m_log(log) {}

void Transport::Log(llvm::StringRef message) {
  // Emit the message directly, since this log was forwarded.
  m_log.Emit(message);
}

} // namespace lldb_dap
