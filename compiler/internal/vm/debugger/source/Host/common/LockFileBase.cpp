//===-- LockFileBase.cpp --------------------------------------------------===//
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

#include "lldb/Host/LockFileBase.h"

using namespace lldb;
using namespace lldb_private;

static Status AlreadyLocked() {
  return Status::FromErrorString("Already locked");
}

static Status NotLocked() { return Status::FromErrorString("Not locked"); }

LockFileBase::LockFileBase(int fd)
    : m_fd(fd), m_locked(false), m_start(0), m_len(0) {}

bool LockFileBase::IsLocked() const { return m_locked; }

Status LockFileBase::WriteLock(const uint64_t start, const uint64_t len) {
  return DoLock([&](const uint64_t start,
                    const uint64_t len) { return DoWriteLock(start, len); },
                start, len);
}

Status LockFileBase::TryWriteLock(const uint64_t start, const uint64_t len) {
  return DoLock([&](const uint64_t start,
                    const uint64_t len) { return DoTryWriteLock(start, len); },
                start, len);
}

Status LockFileBase::ReadLock(const uint64_t start, const uint64_t len) {
  return DoLock([&](const uint64_t start,
                    const uint64_t len) { return DoReadLock(start, len); },
                start, len);
}

Status LockFileBase::TryReadLock(const uint64_t start, const uint64_t len) {
  return DoLock([&](const uint64_t start,
                    const uint64_t len) { return DoTryReadLock(start, len); },
                start, len);
}

Status LockFileBase::Unlock() {
  if (!IsLocked())
    return NotLocked();

  Status error = DoUnlock();
  if (error.Success()) {
    m_locked = false;
    m_start = 0;
    m_len = 0;
  }
  return error;
}

bool LockFileBase::IsValidFile() const { return m_fd != -1; }

Status LockFileBase::DoLock(const Locker &locker, const uint64_t start,
                            const uint64_t len) {
  if (!IsValidFile())
    return Status::FromErrorString("File is invalid");

  if (IsLocked())
    return AlreadyLocked();

  Status error = locker(start, len);
  if (error.Success()) {
    m_locked = true;
    m_start = start;
    m_len = len;
  }

  return error;
}
