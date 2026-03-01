//===-- HostProcess.cpp ---------------------------------------------------===//
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

#include "lldb/Host/HostProcess.h"
#include "lldb/Host/HostNativeProcess.h"
#include "lldb/Host/HostThread.h"

using namespace lldb;
using namespace lldb_private;

HostProcess::HostProcess() : m_native_process(new HostNativeProcess) {}

HostProcess::HostProcess(lldb::process_t process)
    : m_native_process(new HostNativeProcess(process)) {}

HostProcess::~HostProcess() = default;

Status HostProcess::Terminate() { return m_native_process->Terminate(); }

lldb::pid_t HostProcess::GetProcessId() const {
  return m_native_process->GetProcessId();
}

bool HostProcess::IsRunning() const { return m_native_process->IsRunning(); }

llvm::Expected<HostThread> HostProcess::StartMonitoring(
    const Host::MonitorChildProcessCallback &callback) {
  return m_native_process->StartMonitoring(callback);
}

HostNativeProcessBase &HostProcess::GetNativeProcess() {
  return *m_native_process;
}

const HostNativeProcessBase &HostProcess::GetNativeProcess() const {
  return *m_native_process;
}
