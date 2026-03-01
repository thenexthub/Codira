//===-- HostThreadWindows.cpp ---------------------------------------------===//
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

#include "lldb/Utility/Status.h"

#include "lldb/Host/windows/HostThreadWindows.h"
#include "lldb/Host/windows/windows.h"

#include "llvm/ADT/STLExtras.h"

using namespace lldb;
using namespace lldb_private;

static void __stdcall ExitThreadProxy(ULONG_PTR dwExitCode) {
  ::ExitThread(dwExitCode);
}

HostThreadWindows::HostThreadWindows()
    : HostNativeThreadBase(), m_owns_handle(true) {}

HostThreadWindows::HostThreadWindows(lldb::thread_t thread)
    : HostNativeThreadBase(thread), m_owns_handle(true) {}

HostThreadWindows::~HostThreadWindows() { Reset(); }

void HostThreadWindows::SetOwnsHandle(bool owns) { m_owns_handle = owns; }

Status HostThreadWindows::Join(lldb::thread_result_t *result) {
  Status error;
  if (IsJoinable()) {
    DWORD wait_result = ::WaitForSingleObject(m_thread, INFINITE);
    if (WAIT_OBJECT_0 == wait_result && result) {
      DWORD exit_code = 0;
      if (!::GetExitCodeThread(m_thread, &exit_code))
        *result = 0;
      *result = exit_code;
    } else if (WAIT_OBJECT_0 != wait_result)
      error = Status(::GetLastError(), eErrorTypeWin32);
  } else
    error = Status(ERROR_INVALID_HANDLE, eErrorTypeWin32);

  Reset();
  return error;
}

Status HostThreadWindows::Cancel() {
  Status error;

  DWORD result = ::QueueUserAPC(::ExitThreadProxy, m_thread, 0);
  error = Status(result, eErrorTypeWin32);
  return error;
}

lldb::tid_t HostThreadWindows::GetThreadId() const {
  return ::GetThreadId(m_thread);
}

void HostThreadWindows::Reset() {
  if (m_owns_handle && m_thread != LLDB_INVALID_HOST_THREAD)
    ::CloseHandle(m_thread);

  HostNativeThreadBase::Reset();
}

bool HostThreadWindows::EqualsThread(lldb::thread_t thread) const {
  return GetThreadId() == ::GetThreadId(thread);
}
