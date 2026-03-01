//===-- HostProcessPosix.cpp ----------------------------------------------===//
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

#include "lldb/Host/Host.h"
#include "lldb/Host/FileSystem.h"
#include "lldb/Host/posix/HostProcessPosix.h"

#include "llvm/ADT/STLExtras.h"

#include <climits>
#include <csignal>
#include <unistd.h>

using namespace lldb_private;

static const int kInvalidPosixProcess = 0;

HostProcessPosix::HostProcessPosix()
    : HostNativeProcessBase(kInvalidPosixProcess) {}

HostProcessPosix::HostProcessPosix(lldb::process_t process)
    : HostNativeProcessBase(process) {}

HostProcessPosix::~HostProcessPosix() = default;

Status HostProcessPosix::Signal(int signo) const {
  if (m_process == kInvalidPosixProcess) {
    return Status::FromErrorString(
        "HostProcessPosix refers to an invalid process");
  }

  return HostProcessPosix::Signal(m_process, signo);
}

Status HostProcessPosix::Signal(lldb::process_t process, int signo) {
  Status error;

  if (-1 == ::kill(process, signo))
    return Status::FromErrno();

  return error;
}

Status HostProcessPosix::Terminate() { return Signal(SIGKILL); }

lldb::pid_t HostProcessPosix::GetProcessId() const { return m_process; }

bool HostProcessPosix::IsRunning() const {
  if (m_process == kInvalidPosixProcess)
    return false;

  // Send this process the null signal.  If it succeeds the process is running.
  Status error = Signal(0);
  return error.Success();
}

llvm::Expected<HostThread> HostProcessPosix::StartMonitoring(
    const Host::MonitorChildProcessCallback &callback) {
  return Host::StartMonitoringChildProcess(callback, m_process);
}
