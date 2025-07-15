//===--- MutexWin32.h - -----------------------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All rights reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// version 2 for more details (a copy is included in the LICENSE file that
// accompanied this code).
//
// Author(-s): Tunjay Akbarli
//

//===----------------------------------------------------------------------===//
//
// Mutex, ConditionVariable, Read/Write lock, and Scoped lock implementations
// using Windows Slim Reader/Writer Locks and Conditional Variables.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_RUNTIME_MUTEX_WIN32_BACKDEPLOY56_H
#define LANGUAGE_RUNTIME_MUTEX_WIN32_BACKDEPLOY56_H

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace language {

typedef CONDITION_VARIABLE ConditionHandle;
typedef SRWLOCK ConditionMutexHandle;
typedef SRWLOCK MutexHandle;
typedef SRWLOCK ReadWriteLockHandle;

#define LANGUAGE_CONDITION_SUPPORTS_CONSTEXPR 1
#define LANGUAGE_MUTEX_SUPPORTS_CONSTEXPR 1
#define LANGUAGE_READWRITELOCK_SUPPORTS_CONSTEXPR 1

struct ConditionPlatformHelper {
  static constexpr ConditionHandle staticInit() {
    return CONDITION_VARIABLE_INIT;
  };
  static void init(ConditionHandle &condition) {
    InitializeConditionVariable(&condition);
  }
  static void destroy(ConditionHandle &condition) {}
  static void notifyOne(ConditionHandle &condition) {
    WakeConditionVariable(&condition);
  }
  static void notifyAll(ConditionHandle &condition) {
    WakeAllConditionVariable(&condition);
  }
  static void wait(ConditionHandle &condition, MutexHandle &mutex);
};

struct MutexPlatformHelper {
  static constexpr MutexHandle staticInit() { return SRWLOCK_INIT; }
  static constexpr ConditionMutexHandle conditionStaticInit() {
    return SRWLOCK_INIT;
  }
  static void init(MutexHandle &mutex, bool checked = false) {
    InitializeSRWLock(&mutex);
  }
  static void destroy(MutexHandle &mutex) {}
  static void lock(MutexHandle &mutex) { AcquireSRWLockExclusive(&mutex); }
  static void unlock(MutexHandle &mutex) { ReleaseSRWLockExclusive(&mutex); }
  static bool try_lock(MutexHandle &mutex) {
    return TryAcquireSRWLockExclusive(&mutex) != 0;
  }
  // The unsafe versions don't do error checking.
  static void unsafeLock(MutexHandle &mutex) {
    AcquireSRWLockExclusive(&mutex);
  }
  static void unsafeUnlock(MutexHandle &mutex) {
    ReleaseSRWLockExclusive(&mutex);
  }
};

struct ReadWriteLockPlatformHelper {
  static constexpr ReadWriteLockHandle staticInit() { return SRWLOCK_INIT; }
  static void init(ReadWriteLockHandle &rwlock) { InitializeSRWLock(&rwlock); }
  static void destroy(ReadWriteLockHandle &rwlock) {}
  static void readLock(ReadWriteLockHandle &rwlock) {
    AcquireSRWLockShared(&rwlock);
  }
  static bool try_readLock(ReadWriteLockHandle &rwlock) {
    return TryAcquireSRWLockShared(&rwlock) != 0;
  }
  static void readUnlock(ReadWriteLockHandle &rwlock) {
    ReleaseSRWLockShared(&rwlock);
  }
  static void writeLock(ReadWriteLockHandle &rwlock) {
    AcquireSRWLockExclusive(&rwlock);
  }
  static bool try_writeLock(ReadWriteLockHandle &rwlock) {
    return TryAcquireSRWLockExclusive(&rwlock) != 0;
  }
  static void writeUnlock(ReadWriteLockHandle &rwlock) {
    ReleaseSRWLockExclusive(&rwlock);
  }
};
}

#endif // LANGUAGE_RUNTIME_MUTEX_WIN32_BACKDEPLOY56_H
