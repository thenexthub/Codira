//===-- ReadMemoryRequestHandler.cpp --------------------------------------===//
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

#include "DAP.h"
#include "JSONUtils.h"
#include "RequestHandler.h"
#include "llvm/ADT/StringExtras.h"

namespace lldb_dap {

// Reads bytes from memory at the provided location.
//
// Clients should only call this request if the corresponding capability
// `supportsReadMemoryRequest` is true
llvm::Expected<protocol::ReadMemoryResponseBody>
ReadMemoryRequestHandler::Run(const protocol::ReadMemoryArguments &args) const {
  const lldb::addr_t raw_address = args.memoryReference + args.offset;

  lldb::SBProcess process = dap.target.GetProcess();
  if (!lldb::SBDebugger::StateIsStoppedState(process.GetState()))
    return llvm::make_error<NotStoppedError>();

  const uint64_t count_read = std::max<uint64_t>(args.count, 1);
  // We also need support reading 0 bytes
  // VS Code sends those requests to check if a `memoryReference`
  // can be dereferenced.
  protocol::ReadMemoryResponseBody response;
  std::vector<std::byte> &buffer = response.data;
  buffer.resize(count_read);

  lldb::SBError error;
  const size_t memory_count = dap.target.GetProcess().ReadMemory(
      raw_address, buffer.data(), buffer.size(), error);

  response.address = raw_address;

  // reading memory may fail for multiple reasons. memory not readable,
  // reading out of memory range and gaps in memory. return from
  // the last readable byte.
  if (error.Fail() && (memory_count < count_read)) {
    response.unreadableBytes = count_read - memory_count;
  }

  buffer.resize(std::min<size_t>(memory_count, args.count));
  return response;
}

} // namespace lldb_dap
