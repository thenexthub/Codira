//===-- ThreadsRequestHandler.cpp -----------------------------------------===//
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
#include "EventHelper.h"
#include "Protocol/ProtocolRequests.h"
#include "ProtocolUtils.h"
#include "RequestHandler.h"
#include "lldb/API/SBDebugger.h"
#include "lldb/API/SBDefines.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace lldb_dap::protocol;

namespace lldb_dap {

/// The request retrieves a list of all threads.
Expected<ThreadsResponseBody>
ThreadsRequestHandler::Run(const ThreadsArguments &) const {
  lldb::SBProcess process = dap.target.GetProcess();
  std::vector<Thread> threads;

  // Client requests the baseline of currently existing threads after
  // a successful launch or attach by sending a 'threads' request
  // right after receiving the configurationDone response.
  // If no thread has reported to the client, it prevents something
  // like the pause request from working in the running state.
  // Return the cache of initial threads as the process might have resumed
  if (!dap.initial_thread_list.empty()) {
    threads = dap.initial_thread_list;
    dap.initial_thread_list.clear();
  } else {
    if (!lldb::SBDebugger::StateIsStoppedState(process.GetState()))
      return make_error<NotStoppedError>();

    threads = GetThreads(process, dap.thread_format);
  }

  if (threads.size() == 0)
    return make_error<DAPError>("failed to retrieve threads from process");

  return ThreadsResponseBody{threads};
}

} // namespace lldb_dap
