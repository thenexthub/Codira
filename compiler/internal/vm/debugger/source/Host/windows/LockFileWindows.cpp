//===-- LockFileWindows.cpp -----------------------------------------------===//
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

#include "lldb/Host/windows/LockFileWindows.h"

#include <io.h>

using namespace lldb;
using namespace lldb_private;

static Status fileLock(HANDLE file_handle, DWORD flags, const uint64_t start,
                       const uint64_t len) {
  if (start != 0)
    return Status::FromErrorString(
        "Non-zero start lock regions are not supported");

  OVERLAPPED overlapped = {};

  if (!::LockFileEx(file_handle, flags, 0, len, 0, &overlapped) &&
      ::GetLastError() != ERROR_IO_PENDING)
    return Status(::GetLastError(), eErrorTypeWin32);

  DWORD bytes;
  if (!::GetOverlappedResult(file_handle, &overlapped, &bytes, TRUE))
    return Status(::GetLastError(), eErrorTypeWin32);

  return Status();
}

LockFileWindows::LockFileWindows(int fd)
    : LockFileBase(fd), m_file(reinterpret_cast<HANDLE>(_get_osfhandle(fd))) {}

LockFileWindows::~LockFileWindows() { Unlock(); }

bool LockFileWindows::IsValidFile() const {
  return LockFileBase::IsValidFile() && m_file != INVALID_HANDLE_VALUE;
}

Status LockFileWindows::DoWriteLock(const uint64_t start, const uint64_t len) {
  return fileLock(m_file, LOCKFILE_EXCLUSIVE_LOCK, start, len);
}

Status LockFileWindows::DoTryWriteLock(const uint64_t start,
                                       const uint64_t len) {
  return fileLock(m_file, LOCKFILE_EXCLUSIVE_LOCK | LOCKFILE_FAIL_IMMEDIATELY,
                  start, len);
}

Status LockFileWindows::DoReadLock(const uint64_t start, const uint64_t len) {
  return fileLock(m_file, 0, start, len);
}

Status LockFileWindows::DoTryReadLock(const uint64_t start,
                                      const uint64_t len) {
  return fileLock(m_file, LOCKFILE_FAIL_IMMEDIATELY, start, len);
}

Status LockFileWindows::DoUnlock() {
  OVERLAPPED overlapped = {};

  if (!::UnlockFileEx(m_file, 0, m_len, 0, &overlapped) &&
      ::GetLastError() != ERROR_IO_PENDING)
    return Status(::GetLastError(), eErrorTypeWin32);

  DWORD bytes;
  if (!::GetOverlappedResult(m_file, &overlapped, &bytes, TRUE))
    return Status(::GetLastError(), eErrorTypeWin32);

  return Status();
}
