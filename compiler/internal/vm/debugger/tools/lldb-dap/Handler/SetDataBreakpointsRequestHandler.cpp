//===-- SetDataBreakpointsRequestHandler.cpp ------------------------------===//
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
#include "Watchpoint.h"
#include <set>

namespace lldb_dap {

/// Replaces all existing data breakpoints with new data breakpoints.
/// To clear all data breakpoints, specify an empty array.
/// When a data breakpoint is hit, a stopped event (with reason data breakpoint)
/// is generated. Clients should only call this request if the corresponding
/// capability supportsDataBreakpoints is true.
llvm::Expected<protocol::SetDataBreakpointsResponseBody>
SetDataBreakpointsRequestHandler::Run(
    const protocol::SetDataBreakpointsArguments &args) const {
  std::vector<protocol::Breakpoint> response_breakpoints;

  dap.target.DeleteAllWatchpoints();
  std::vector<Watchpoint> watchpoints;
  for (const auto &bp : args.breakpoints)
    watchpoints.emplace_back(dap, bp);

  // If two watchpoints start at the same address, the latter overwrite the
  // former. So, we only enable those at first-seen addresses when iterating
  // backward.
  std::set<lldb::addr_t> addresses;
  for (auto iter = watchpoints.rbegin(); iter != watchpoints.rend(); ++iter) {
    if (addresses.count(iter->GetAddress()) == 0) {
      iter->SetWatchpoint();
      addresses.insert(iter->GetAddress());
    }
  }
  for (auto wp : watchpoints)
    response_breakpoints.push_back(wp.ToProtocolBreakpoint());

  return protocol::SetDataBreakpointsResponseBody{
      std::move(response_breakpoints)};
}

} // namespace lldb_dap
