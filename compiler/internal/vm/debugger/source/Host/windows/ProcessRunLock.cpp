//===-- ProcessRunLock.cpp ------------------------------------------------===//
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

#include "lldb/Host/ProcessRunLock.h"
#include "lldb/Host/windows/windows.h"

static PSRWLOCK GetLock(lldb::rwlock_t lock) {
  return static_cast<PSRWLOCK>(lock);
}

static bool ReadLock(lldb::rwlock_t rwlock) {
  ::AcquireSRWLockShared(GetLock(rwlock));
  return true;
}

static bool ReadUnlock(lldb::rwlock_t rwlock) {
  ::ReleaseSRWLockShared(GetLock(rwlock));
  return true;
}

static bool WriteLock(lldb::rwlock_t rwlock) {
  ::AcquireSRWLockExclusive(GetLock(rwlock));
  return true;
}

static bool WriteUnlock(lldb::rwlock_t rwlock) {
  ::ReleaseSRWLockExclusive(GetLock(rwlock));
  return true;
}

using namespace lldb_private;

ProcessRunLock::ProcessRunLock() : m_running(false) {
  m_rwlock = new SRWLOCK;
  InitializeSRWLock(GetLock(m_rwlock));
}

ProcessRunLock::~ProcessRunLock() { delete static_cast<SRWLOCK *>(m_rwlock); }

bool ProcessRunLock::ReadTryLock() {
  ::ReadLock(m_rwlock);
  if (m_running == false)
    return true;
  ::ReadUnlock(m_rwlock);
  return false;
}

bool ProcessRunLock::ReadUnlock() { return ::ReadUnlock(m_rwlock); }

bool ProcessRunLock::SetRunning() {
  WriteLock(m_rwlock);
  bool was_stopped = !m_running;
  m_running = true;
  WriteUnlock(m_rwlock);
  return was_stopped;
}

bool ProcessRunLock::SetStopped() {
  WriteLock(m_rwlock);
  bool was_running = m_running;
  m_running = false;
  WriteUnlock(m_rwlock);
  return was_running;
}
