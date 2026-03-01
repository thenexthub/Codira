//===-- EventHelper.h -----------------------------------------------------===//
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

#ifndef LLDB_TOOLS_LLDB_DAP_EVENTHELPER_H
#define LLDB_TOOLS_LLDB_DAP_EVENTHELPER_H

#include "DAPForward.h"
#include "Protocol/ProtocolEvents.h"
#include "lldb/lldb-defines.h"
#include "lldb/lldb-types.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/Error.h"

namespace lldb_dap {
struct DAP;

enum LaunchMethod { Launch, Attach, AttachForSuspendedLaunch };

/// Sends target based capabilities and lldb-dap custom capabilities.
void SendExtraCapabilities(DAP &dap);

void SendProcessEvent(DAP &dap, LaunchMethod launch_method);

llvm::Error SendThreadStoppedEvent(DAP &dap, bool on_entry = false);

void SendTerminatedEvent(DAP &dap);

void SendStdOutStdErr(DAP &dap, lldb::SBProcess &process);

void SendContinuedEvent(DAP &dap);

void SendProcessExitedEvent(DAP &dap, lldb::SBProcess &process);

void SendInvalidatedEvent(
    DAP &dap, llvm::ArrayRef<protocol::InvalidatedEventBody::Area> areas,
    lldb::tid_t tid = LLDB_INVALID_THREAD_ID);

void SendMemoryEvent(DAP &dap, lldb::SBValue variable);

/// Event thread function that handles debugger events for multiple DAP sessions
/// sharing the same debugger instance. This runs in its own thread and
/// dispatches events to the appropriate DAP instance.
///
/// \param debugger The debugger instance to listen for events from.
/// \param broadcaster The broadcaster for stop event thread notifications.
/// \param client_name The client name for thread naming/logging purposes.
/// \param log The log instance for logging.
void EventThread(lldb::SBDebugger debugger, lldb::SBBroadcaster broadcaster,
                 llvm::StringRef client_name, Log &log);

} // namespace lldb_dap

#endif
