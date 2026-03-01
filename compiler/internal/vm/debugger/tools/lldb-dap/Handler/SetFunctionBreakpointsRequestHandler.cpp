//===-- SetFunctionBreakpointsRequestHandler.cpp --------------------------===//
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
#include "RequestHandler.h"

namespace lldb_dap {

/// Replaces all existing function breakpoints with new function breakpoints.
/// To clear all function breakpoints, specify an empty array.
/// When a function breakpoint is hit, a stopped event (with reason function
/// breakpoint) is generated. Clients should only call this request if the
/// corresponding capability supportsFunctionBreakpoints is true.
llvm::Expected<protocol::SetFunctionBreakpointsResponseBody>
SetFunctionBreakpointsRequestHandler::Run(
    const protocol::SetFunctionBreakpointsArguments &args) const {
  std::vector<protocol::Breakpoint> response_breakpoints;

  // Disable any function breakpoints that aren't in this request.
  // There is no call to remove function breakpoints other than calling this
  // function with a smaller or empty "breakpoints" list.
  const auto name_iter = dap.function_breakpoints.keys();
  llvm::DenseSet<llvm::StringRef> seen(name_iter.begin(), name_iter.end());
  for (const auto &fb : args.breakpoints) {
    FunctionBreakpoint fn_bp(dap, fb);
    const auto [it, inserted] =
        dap.function_breakpoints.try_emplace(fn_bp.GetFunctionName(), dap, fb);
    if (inserted)
      it->second.SetBreakpoint();
    else
      it->second.UpdateBreakpoint(fn_bp);

    response_breakpoints.push_back(it->second.ToProtocolBreakpoint());
    seen.erase(fn_bp.GetFunctionName());
  }

  // Remove any breakpoints that are no longer in our list
  for (const auto &name : seen) {
    auto fn_bp = dap.function_breakpoints.find(name);
    if (fn_bp == dap.function_breakpoints.end())
      continue;
    dap.target.BreakpointDelete(fn_bp->second.GetID());
    dap.function_breakpoints.erase(name);
  }

  return protocol::SetFunctionBreakpointsResponseBody{
      std::move(response_breakpoints)};
}

} // namespace lldb_dap
