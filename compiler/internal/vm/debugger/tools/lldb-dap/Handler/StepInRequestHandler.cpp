//===-- StepInRequestHandler.cpp ------------------------------------------===//
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
#include "Protocol/ProtocolTypes.h"
#include "RequestHandler.h"

using namespace llvm;
using namespace lldb;
using namespace lldb_dap::protocol;

namespace lldb_dap {

// The request resumes the given thread to step into a function/method and
// allows all other threads to run freely by resuming them. If the debug adapter
// supports single thread execution (see capability
// `supportsSingleThreadExecutionRequests`), setting the `singleThread` argument
// to true prevents other suspended threads from resuming. If the request cannot
// step into a target, `stepIn` behaves like the `next` request. The debug
// adapter first sends the response and then a `stopped` event (with reason
// `step`) after the step has completed. If there are multiple function/method
// calls (or other targets) on the source line, the argument `targetId` can be
// used to control into which target the `stepIn` should occur. The list of
// possible targets for a given source line can be retrieved via the
// `stepInTargets` request.
Error StepInRequestHandler::Run(const StepInArguments &args) const {
  SBThread thread = dap.GetLLDBThread(args.threadId);
  if (!thread.IsValid())
    return make_error<DAPError>("invalid thread");

  // Remember the thread ID that caused the resume so we can set the
  // "threadCausedFocus" boolean value in the "stopped" events.
  dap.focus_tid = thread.GetThreadID();

  if (!SBDebugger::StateIsStoppedState(dap.target.GetProcess().GetState()))
    return make_error<NotStoppedError>();

  lldb::SBError error;
  if (args.granularity == eSteppingGranularityInstruction) {
    thread.StepInstruction(/*step_over=*/false, error);
    return ToError(error);
  }

  std::string step_in_target;
  auto it = dap.step_in_targets.find(args.targetId.value_or(0));
  if (it != dap.step_in_targets.end())
    step_in_target = it->second;

  RunMode run_mode = args.singleThread ? eOnlyThisThread : eOnlyDuringStepping;
  thread.StepInto(step_in_target.c_str(), LLDB_INVALID_LINE_NUMBER, error,
                  run_mode);
  return ToError(error);
}

} // namespace lldb_dap
