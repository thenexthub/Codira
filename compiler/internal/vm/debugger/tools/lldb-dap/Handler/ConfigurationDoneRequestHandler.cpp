//===-- ConfigurationDoneRequestHandler..cpp ------------------------------===//
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
#include "LLDBUtils.h"
#include "Protocol/ProtocolRequests.h"
#include "ProtocolUtils.h"
#include "RequestHandler.h"
#include "lldb/API/SBDebugger.h"

using namespace llvm;
using namespace lldb_dap::protocol;

namespace lldb_dap {

/// This request indicates that the client has finished initialization of the
/// debug adapter.
///
/// So it is the last request in the sequence of configuration requests (which
/// was started by the `initialized` event).
///
/// Clients should only call this request if the corresponding capability
/// `supportsConfigurationDoneRequest` is true.
llvm::Error
ConfigurationDoneRequestHandler::Run(const ConfigurationDoneArguments &) const {
  dap.configuration_done = true;

  // Ensure any command scripts did not leave us in an unexpected state.
  lldb::SBProcess process = dap.target.GetProcess();
  if (!process.IsValid() ||
      !lldb::SBDebugger::StateIsStoppedState(process.GetState()))
    return make_error<DAPError>(
        "Expected process to be stopped.\r\n\r\nProcess is in an unexpected "
        "state and may have missed an initial configuration. Please check that "
        "any debugger command scripts are not resuming the process during the "
        "launch sequence.");

  // Waiting until 'configurationDone' to send target based capabilities in case
  // the launch or attach scripts adjust the target. The initial dummy target
  // may have different capabilities than the final target.

  /// Also send here custom capabilities to the client, which is consumed by the
  /// lldb-dap specific editor extension.
  SendExtraCapabilities(dap);

  PrintIntroductionMessage();

  // Clients can request a baseline of currently existing threads after
  // we acknowledge the configurationDone request.
  // Client requests the baseline of currently existing threads after
  // a successful or attach by sending a 'threads' request
  // right after receiving the configurationDone response.
  // Obtain the list of threads before we resume the process
  dap.initial_thread_list = GetThreads(process, dap.thread_format);

  SendProcessEvent(dap, dap.is_attach ? Attach : Launch);

  if (dap.stop_at_entry)
    return SendThreadStoppedEvent(dap, /*on_entry=*/true);

  return ToError(process.Continue());
}

void ConfigurationDoneRequestHandler::PostRun() const {
  if (!dap.on_configuration_done)
    return;

  dap.on_configuration_done();
  // Clear the callback to ensure any captured resources are released.
  dap.on_configuration_done = nullptr;
}

} // namespace lldb_dap
