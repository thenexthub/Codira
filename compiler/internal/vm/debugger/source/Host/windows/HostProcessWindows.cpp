//===-- HostProcessWindows.cpp --------------------------------------------===//
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

#include "lldb/Host/windows/HostProcessWindows.h"
#include "lldb/Host/HostThread.h"
#include "lldb/Host/ThreadLauncher.h"
#include "lldb/Host/windows/windows.h"
#include "lldb/Utility/FileSpec.h"

#include "llvm/ADT/STLExtras.h"
#include "llvm/Support/ConvertUTF.h"
#include "llvm/Support/WindowsError.h"

#include <psapi.h>

using namespace lldb_private;

namespace {
struct MonitorInfo {
  Host::MonitorChildProcessCallback callback;
  HANDLE process_handle;
};
}

HostProcessWindows::HostProcessWindows()
    : HostNativeProcessBase(), m_owns_handle(true) {}

HostProcessWindows::HostProcessWindows(lldb::process_t process)
    : HostNativeProcessBase(process), m_owns_handle(true) {}

HostProcessWindows::~HostProcessWindows() { Close(); }

void HostProcessWindows::SetOwnsHandle(bool owns) { m_owns_handle = owns; }

Status HostProcessWindows::Terminate() {
  Status error;
  if (m_process == nullptr)
    error = Status(ERROR_INVALID_HANDLE, lldb::eErrorTypeWin32);

  if (!::TerminateProcess(m_process, 0))
    error = Status(::GetLastError(), lldb::eErrorTypeWin32);

  return error;
}

lldb::pid_t HostProcessWindows::GetProcessId() const {
  return (m_process == LLDB_INVALID_PROCESS) ? -1 : ::GetProcessId(m_process);
}

bool HostProcessWindows::IsRunning() const {
  if (m_process == nullptr)
    return false;

  DWORD code = 0;
  if (!::GetExitCodeProcess(m_process, &code))
    return false;

  return (code == STILL_ACTIVE);
}

static lldb::thread_result_t
MonitorThread(const Host::MonitorChildProcessCallback &callback,
              HANDLE process_handle) {
  DWORD exit_code;

  ::WaitForSingleObject(process_handle, INFINITE);
  ::GetExitCodeProcess(process_handle, &exit_code);
  callback(::GetProcessId(process_handle), 0, exit_code);
  ::CloseHandle(process_handle);
  return {};
}

llvm::Expected<HostThread> HostProcessWindows::StartMonitoring(
    const Host::MonitorChildProcessCallback &callback) {
  HANDLE process_handle;

  // Since the life of this HostProcessWindows instance and the life of the
  // process may be different, duplicate the handle so that the monitor thread
  // can have ownership over its own copy of the handle.
  if (::DuplicateHandle(GetCurrentProcess(), m_process, GetCurrentProcess(),
                        &process_handle, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
    return ThreadLauncher::LaunchThread(
        "ChildProcessMonitor", [callback, process_handle] {
          return MonitorThread(callback, process_handle);
        });
  } else {
    return llvm::errorCodeToError(llvm::mapWindowsError(GetLastError()));
  }
}

void HostProcessWindows::Close() {
  if (m_owns_handle && m_process != LLDB_INVALID_PROCESS)
    ::CloseHandle(m_process);
  m_process = nullptr;
}
