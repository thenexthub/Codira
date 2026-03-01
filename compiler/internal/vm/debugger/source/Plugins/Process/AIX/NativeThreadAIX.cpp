//===-- NativeThreadAIX.cpp ---------------------------------------------===//
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

#include "NativeThreadAIX.h"
#include "NativeProcessAIX.h"
#include "lldb/Utility/State.h"

using namespace lldb;
using namespace lldb_private;
using namespace lldb_private::process_aix;

NativeThreadAIX::NativeThreadAIX(NativeProcessAIX &process, lldb::tid_t tid)
    : NativeThreadProtocol(process, tid), m_state(StateType::eStateInvalid) {}

std::string NativeThreadAIX::GetName() { return ""; }

lldb::StateType NativeThreadAIX::GetState() { return m_state; }

bool NativeThreadAIX::GetStopReason(ThreadStopInfo &stop_info,
                                    std::string &description) {
  return false;
}

Status NativeThreadAIX::SetWatchpoint(lldb::addr_t addr, size_t size,
                                      uint32_t watch_flags, bool hardware) {
  return Status("Unable to Set hardware watchpoint.");
}

Status NativeThreadAIX::RemoveWatchpoint(lldb::addr_t addr) {
  return Status("Clearing hardware watchpoint failed.");
}

Status NativeThreadAIX::SetHardwareBreakpoint(lldb::addr_t addr, size_t size) {
  return Status("Unable to set hardware breakpoint.");
}

Status NativeThreadAIX::RemoveHardwareBreakpoint(lldb::addr_t addr) {
  return Status("Clearing hardware breakpoint failed.");
}

NativeProcessAIX &NativeThreadAIX::GetProcess() {
  return static_cast<NativeProcessAIX &>(m_process);
}

const NativeProcessAIX &NativeThreadAIX::GetProcess() const {
  return static_cast<const NativeProcessAIX &>(m_process);
}

llvm::Expected<std::unique_ptr<llvm::MemoryBuffer>>
NativeThreadAIX::GetSiginfo() const {
  return llvm::createStringError(llvm::inconvertibleErrorCode(),
                                 "Not implemented");
}
