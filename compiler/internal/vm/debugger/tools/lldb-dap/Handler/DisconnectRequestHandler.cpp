//===-- DisconnectRequestHandler.cpp --------------------------------------===//
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
#include "llvm/Support/Error.h"
#include <optional>

using namespace llvm;
using namespace lldb_dap::protocol;

namespace lldb_dap {

/// Disconnect request; value of command field is 'disconnect'.
Error DisconnectRequestHandler::Run(
    const std::optional<DisconnectArguments> &arguments) const {
  bool terminateDebuggee = !dap.is_attach;

  if (arguments && arguments->terminateDebuggee)
    terminateDebuggee = *arguments->terminateDebuggee;

  if (Error error = dap.Disconnect(terminateDebuggee))
    return error;

  return Error::success();
}
} // namespace lldb_dap
