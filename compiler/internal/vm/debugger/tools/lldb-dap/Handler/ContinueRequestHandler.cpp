//===-- ContinueRequestHandler.cpp ----------------------------------------===//
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
#include "Handler/RequestHandler.h"
#include "LLDBUtils.h"
#include "Protocol/ProtocolRequests.h"
#include "lldb/API/SBError.h"
#include "lldb/API/SBProcess.h"
#include "llvm/Support/Error.h"

using namespace llvm;
using namespace lldb;
using namespace lldb_dap::protocol;

namespace lldb_dap {

/// The request resumes execution of all threads. If the debug adapter supports
/// single thread execution (see capability
/// `supportsSingleThreadExecutionRequests`), setting the `singleThread`
/// argument to true resumes only the specified thread. If not all threads were
/// resumed, the `allThreadsContinued` attribute of the response should be set
/// to false.
Expected<ContinueResponseBody>
ContinueRequestHandler::Run(const ContinueArguments &args) const {
  SBProcess process = dap.target.GetProcess();
  SBError error;

  if (!SBDebugger::StateIsStoppedState(process.GetState()))
    return make_error<NotStoppedError>();

  if (args.singleThread)
    dap.GetLLDBThread(args.threadId).Resume(error);
  else
    error = process.Continue();

  if (error.Fail())
    return ToError(error);

  ContinueResponseBody body;
  body.allThreadsContinued = !args.singleThread;
  return body;
}

} // namespace lldb_dap
