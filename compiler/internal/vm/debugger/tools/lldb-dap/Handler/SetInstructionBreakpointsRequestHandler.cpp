//===-- SetInstructionBreakpointsRequestHandler.cpp -----------------------===//
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

/// Replaces all existing instruction breakpoints. Typically, instruction
/// breakpoints would be set from a disassembly window. To clear all instruction
/// breakpoints, specify an empty array. When an instruction breakpoint is hit,
/// a stopped event (with reason instruction breakpoint) is generated. Clients
/// should only call this request if the corresponding capability
/// supportsInstructionBreakpoints is true.
llvm::Expected<protocol::SetInstructionBreakpointsResponseBody>
SetInstructionBreakpointsRequestHandler::Run(
    const protocol::SetInstructionBreakpointsArguments &args) const {
  std::vector<protocol::Breakpoint> response_breakpoints;

  // Disable any instruction breakpoints that aren't in this request.
  // There is no call to remove instruction breakpoints other than calling this
  // function with a smaller or empty "breakpoints" list.
  llvm::DenseSet<lldb::addr_t> seen(
      llvm::from_range, llvm::make_first_range(dap.instruction_breakpoints));

  for (const auto &bp : args.breakpoints) {
    // Read instruction breakpoint request.
    InstructionBreakpoint inst_bp(dap, bp);
    const auto [iv, inserted] = dap.instruction_breakpoints.try_emplace(
        inst_bp.GetInstructionAddressReference(), dap, bp);
    if (inserted)
      iv->second.SetBreakpoint();
    else
      iv->second.UpdateBreakpoint(inst_bp);
    response_breakpoints.push_back(iv->second.ToProtocolBreakpoint());
    seen.erase(inst_bp.GetInstructionAddressReference());
  }

  for (const auto &addr : seen) {
    auto inst_bp = dap.instruction_breakpoints.find(addr);
    if (inst_bp == dap.instruction_breakpoints.end())
      continue;
    dap.target.BreakpointDelete(inst_bp->second.GetID());
    dap.instruction_breakpoints.erase(addr);
  }

  return protocol::SetInstructionBreakpointsResponseBody{
      std::move(response_breakpoints)};
}

} // namespace lldb_dap
