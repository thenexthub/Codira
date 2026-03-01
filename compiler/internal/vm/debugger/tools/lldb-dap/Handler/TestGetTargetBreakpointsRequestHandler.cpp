//===-- TestGetTargetBreakpointsRequestHandler.cpp ------------------------===//
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
#include "RequestHandler.h"

using namespace lldb_dap;
using namespace lldb_dap::protocol;

/// A request used in testing to get the details on all breakpoints that are
/// currently set in the target. This helps us to test "setBreakpoints" and
/// "setFunctionBreakpoints" requests to verify we have the correct set of
/// breakpoints currently set in LLDB.
llvm::Expected<TestGetTargetBreakpointsResponseBody>
TestGetTargetBreakpointsRequestHandler::Run(
    const TestGetTargetBreakpointsArguments &args) const {
  std::vector<protocol::Breakpoint> breakpoints;
  for (uint32_t i = 0; dap.target.GetBreakpointAtIndex(i).IsValid(); ++i) {
    auto bp = Breakpoint(dap, dap.target.GetBreakpointAtIndex(i));
    breakpoints.push_back(bp.ToProtocolBreakpoint());
  }
  return TestGetTargetBreakpointsResponseBody{std::move(breakpoints)};
}
