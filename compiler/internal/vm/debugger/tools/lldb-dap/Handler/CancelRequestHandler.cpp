//===-- CancelRequestHandler.cpp ------------------------------------------===//
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

#include "Handler/RequestHandler.h"
#include "Protocol/ProtocolRequests.h"
#include "llvm/Support/Error.h"

using namespace llvm;
using namespace lldb_dap::protocol;

namespace lldb_dap {

/// The `cancel` request is used by the client in two situations:
///
/// - to indicate that it is no longer interested in the result produced by a
/// specific request issued earlier
/// - to cancel a progress sequence.
///
/// Clients should only call this request if the corresponding capability
/// `supportsCancelRequest` is true.
///
/// This request has a hint characteristic: a debug adapter can only be
/// expected to make a 'best effort' in honoring this request but there are no
/// guarantees.
///
/// The `cancel` request may return an error if it could not cancel
/// an operation but a client should refrain from presenting this error to end
/// users.
///
/// The request that got cancelled still needs to send a response back.
/// This can either be a normal result (`success` attribute true) or an error
/// response (`success` attribute false and the `message` set to `cancelled`).
///
/// Returning partial results from a cancelled request is possible but please
/// note that a client has no generic way for detecting that a response is
/// partial or not.
///
/// The progress that got cancelled still needs to send a `progressEnd` event
/// back.
///
/// A client cannot assume that progress just got cancelled after sending
/// the `cancel` request.
Error CancelRequestHandler::Run(const CancelArguments &arguments) const {
  // Cancel support is built into the DAP::Loop handler for detecting
  // cancellations of pending or inflight requests.
  dap.ClearCancelRequest(arguments);
  return Error::success();
}

} // namespace lldb_dap
