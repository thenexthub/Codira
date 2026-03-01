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

#ifndef _WIN32
#include "lldb/Host/ProcessRunLock.h"

namespace lldb_private {

ProcessRunLock::ProcessRunLock() {
  int err = ::pthread_rwlock_init(&m_rwlock, nullptr);
  (void)err;
}

ProcessRunLock::~ProcessRunLock() {
  int err = ::pthread_rwlock_destroy(&m_rwlock);
  (void)err;
}

bool ProcessRunLock::ReadTryLock() {
  ::pthread_rwlock_rdlock(&m_rwlock);
  if (!m_running) {
    // coverity[missing_unlock]
    return true;
  }
  ::pthread_rwlock_unlock(&m_rwlock);
  return false;
}

bool ProcessRunLock::ReadUnlock() {
  return ::pthread_rwlock_unlock(&m_rwlock) == 0;
}

bool ProcessRunLock::SetRunning() {
  ::pthread_rwlock_wrlock(&m_rwlock);
  bool was_stopped = !m_running;
  m_running = true;
  ::pthread_rwlock_unlock(&m_rwlock);
  return was_stopped;
}

bool ProcessRunLock::SetStopped() {
  ::pthread_rwlock_wrlock(&m_rwlock);
  bool was_running = m_running;
  m_running = false;
  ::pthread_rwlock_unlock(&m_rwlock);
  return was_running;
}

} // namespace lldb_private

#endif
