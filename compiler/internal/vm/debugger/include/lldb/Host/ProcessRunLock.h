//===-- ProcessRunLock.h ----------------------------------------*- C++ -*-===//
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

#ifndef LLDB_HOST_PROCESSRUNLOCK_H
#define LLDB_HOST_PROCESSRUNLOCK_H

#include <cstdint>
#include <ctime>

#include "lldb/lldb-defines.h"

/// Enumerations for broadcasting.
namespace lldb_private {

/// \class ProcessRunLock ProcessRunLock.h "lldb/Host/ProcessRunLock.h"
/// A class used to prevent the process from starting while other
/// threads are accessing its data, and prevent access to its data while it is
/// running.

class ProcessRunLock {
public:
  ProcessRunLock();
  ~ProcessRunLock();

  bool ReadTryLock();
  bool ReadUnlock();

  /// Set the process to running. Returns true if the process was stopped.
  /// Return false if the process was running.
  bool SetRunning();

  /// Set the process to stopped. Returns true if the process was running.
  /// Returns false if the process was stopped.
  bool SetStopped();

  class ProcessRunLocker {
  public:
    ProcessRunLocker() = default;
    ProcessRunLocker(ProcessRunLocker &&other) : m_lock(other.m_lock) {
      other.m_lock = nullptr;
    }
    ProcessRunLocker &operator=(ProcessRunLocker &&other) {
      if (this != &other) {
        Unlock();
        m_lock = other.m_lock;
        other.m_lock = nullptr;
      }
      return *this;
    }

    ~ProcessRunLocker() { Unlock(); }

    bool IsLocked() const { return m_lock; }

    // Try to lock the read lock, but only do so if there are no writers.
    bool TryLock(ProcessRunLock *lock) {
      if (m_lock) {
        if (m_lock == lock)
          return true; // We already have this lock locked
        else
          Unlock();
      }
      if (lock) {
        if (lock->ReadTryLock()) {
          m_lock = lock;
          return true;
        }
      }
      return false;
    }

  protected:
    void Unlock() {
      if (m_lock) {
        m_lock->ReadUnlock();
        m_lock = nullptr;
      }
    }

    ProcessRunLock *m_lock = nullptr;

  private:
    ProcessRunLocker(const ProcessRunLocker &) = delete;
    const ProcessRunLocker &operator=(const ProcessRunLocker &) = delete;
  };

protected:
  lldb::rwlock_t m_rwlock;
  bool m_running = false;

private:
  ProcessRunLock(const ProcessRunLock &) = delete;
  const ProcessRunLock &operator=(const ProcessRunLock &) = delete;
};

} // namespace lldb_private

#endif // LLDB_HOST_PROCESSRUNLOCK_H
