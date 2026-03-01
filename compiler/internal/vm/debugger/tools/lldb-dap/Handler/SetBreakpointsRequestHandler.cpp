//===-- SetBreakpointsRequestHandler.cpp ----------------------------------===//
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
#include "Protocol/ProtocolRequests.h"
#include "RequestHandler.h"

namespace lldb_dap {

/// Sets multiple breakpoints for a single source and clears all previous
/// breakpoints in that source. To clear all breakpoint for a source, specify an
/// empty array. When a breakpoint is hit, a `stopped` event (with reason
/// `breakpoint`) is generated.
llvm::Expected<protocol::SetBreakpointsResponseBody>
SetBreakpointsRequestHandler::Run(
    const protocol::SetBreakpointsArguments &args) const {
  std::vector<protocol::Breakpoint> response_breakpoints =
      dap.SetSourceBreakpoints(args.source, args.breakpoints);
  return protocol::SetBreakpointsResponseBody{std::move(response_breakpoints)};
}

} // namespace lldb_dap
