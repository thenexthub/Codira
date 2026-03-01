//===-- LocalDebugDelegate.cpp --------------------------------------------===//
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

#include "LocalDebugDelegate.h"
#include "ProcessWindows.h"

using namespace lldb;
using namespace lldb_private;

LocalDebugDelegate::LocalDebugDelegate(ProcessWP process)
    : m_process(process) {}

void LocalDebugDelegate::OnExitProcess(uint32_t exit_code) {
  if (ProcessWindowsSP process = GetProcessPointer())
    process->OnExitProcess(exit_code);
}

void LocalDebugDelegate::OnDebuggerConnected(lldb::addr_t image_base) {
  if (ProcessWindowsSP process = GetProcessPointer())
    process->OnDebuggerConnected(image_base);
}

ExceptionResult
LocalDebugDelegate::OnDebugException(bool first_chance,
                                     const ExceptionRecord &record) {
  if (ProcessWindowsSP process = GetProcessPointer())
    return process->OnDebugException(first_chance, record);
  else
    return ExceptionResult::MaskException;
}

void LocalDebugDelegate::OnCreateThread(const HostThread &thread) {
  if (ProcessWindowsSP process = GetProcessPointer())
    process->OnCreateThread(thread);
}

void LocalDebugDelegate::OnExitThread(lldb::tid_t thread_id,
                                      uint32_t exit_code) {
  if (ProcessWindowsSP process = GetProcessPointer())
    process->OnExitThread(thread_id, exit_code);
}

void LocalDebugDelegate::OnLoadDll(const lldb_private::ModuleSpec &module_spec,
                                   lldb::addr_t module_addr) {
  if (ProcessWindowsSP process = GetProcessPointer())
    process->OnLoadDll(module_spec, module_addr);
}

void LocalDebugDelegate::OnUnloadDll(lldb::addr_t module_addr) {
  if (ProcessWindowsSP process = GetProcessPointer())
    process->OnUnloadDll(module_addr);
}

void LocalDebugDelegate::OnDebugString(const std::string &string) {
  if (ProcessWindowsSP process = GetProcessPointer())
    process->OnDebugString(string);
}

void LocalDebugDelegate::OnDebuggerError(const Status &error, uint32_t type) {
  if (ProcessWindowsSP process = GetProcessPointer())
    process->OnDebuggerError(error, type);
}

ProcessWindowsSP LocalDebugDelegate::GetProcessPointer() {
  ProcessSP process = m_process.lock();
  return std::static_pointer_cast<ProcessWindows>(process);
}
