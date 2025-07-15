//===--- MutexPThread.cpp - Supports Mutex.h using PThreads ---------------===//
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
// using PThreads.
//
//===----------------------------------------------------------------------===//

#if __has_include(<unistd.h>)
#include <unistd.h>
#endif

#if defined(_POSIX_THREADS) && !defined(LANGUAGE_STDLIB_SINGLE_THREADED_RUNTIME)

// Notes: language::fatalError is not shared between liblanguageCore and liblanguage_Concurrency
// and liblanguage_Concurrency uses language_Concurrency_fatalError instead.
/* #ifndef LANGUAGE_FATAL_ERROR */
/* #define LANGUAGE_FATAL_ERROR language::fatalError */
/* #endif */

// This is the concurrency Mutex implementation. We're forcing the concurrency
// fatalError here.
#include "Concurrency/Error.h"
#define LANGUAGE_FATAL_ERROR language_Concurrency_fatalError

#include "Concurrency/Threading/Mutex.h"

#include "language/Runtime/Debug.h"
#include <errno.h>
#include <stdlib.h>

using namespace language;

#define reportError(PThreadFunction)                                           \
  do {                                                                         \
    int errorcode = PThreadFunction;                                           \
    if (errorcode != 0) {                                                      \
      LANGUAGE_FATAL_ERROR(/* flags = */ 0, "'%s' failed with error '%s'(%d)\n",  \
                        #PThreadFunction, errorName(errorcode), errorcode);    \
    }                                                                          \
  } while (false)

#define returnTrueOrReportError(PThreadFunction, returnFalseOnEBUSY)           \
  do {                                                                         \
    int errorcode = PThreadFunction;                                           \
    if (errorcode == 0)                                                        \
      return true;                                                             \
    if (returnFalseOnEBUSY && errorcode == EBUSY)                              \
      return false;                                                            \
    LANGUAGE_FATAL_ERROR(/* flags = */ 0, "'%s' failed with error '%s'(%d)\n",    \
                      #PThreadFunction, errorName(errorcode), errorcode);      \
  } while (false)

static const char *errorName(int errorcode) {
  switch (errorcode) {
  case EINVAL:
    return "EINVAL";
  case EPERM:
    return "EPERM";
  case EDEADLK:
    return "EDEADLK";
  case ENOMEM:
    return "ENOMEM";
  case EAGAIN:
    return "EAGAIN";
  case EBUSY:
    return "EBUSY";
  default:
    return "<unknown>";
  }
}

void ConditionPlatformHelper::init(pthread_cond_t &condition) {
  reportError(pthread_cond_init(&condition, nullptr));
}

void ConditionPlatformHelper::destroy(pthread_cond_t &condition) {
  reportError(pthread_cond_destroy(&condition));
}

void ConditionPlatformHelper::notifyOne(pthread_cond_t &condition) {
  reportError(pthread_cond_signal(&condition));
}

void ConditionPlatformHelper::notifyAll(pthread_cond_t &condition) {
  reportError(pthread_cond_broadcast(&condition));
}

void ConditionPlatformHelper::wait(pthread_cond_t &condition,
                                   pthread_mutex_t &mutex) {
  reportError(pthread_cond_wait(&condition, &mutex));
}

void MutexPlatformHelper::init(pthread_mutex_t &mutex, bool checked) {
  pthread_mutexattr_t attr;
  int kind = (checked ? PTHREAD_MUTEX_ERRORCHECK : PTHREAD_MUTEX_NORMAL);
  reportError(pthread_mutexattr_init(&attr));
  reportError(pthread_mutexattr_settype(&attr, kind));
  reportError(pthread_mutex_init(&mutex, &attr));
  reportError(pthread_mutexattr_destroy(&attr));
}

void MutexPlatformHelper::destroy(pthread_mutex_t &mutex) {
  reportError(pthread_mutex_destroy(&mutex));
}

void MutexPlatformHelper::lock(pthread_mutex_t &mutex) {
  reportError(pthread_mutex_lock(&mutex));
}

void MutexPlatformHelper::unlock(pthread_mutex_t &mutex) {
  reportError(pthread_mutex_unlock(&mutex));
}

bool MutexPlatformHelper::try_lock(pthread_mutex_t &mutex) {
  returnTrueOrReportError(pthread_mutex_trylock(&mutex),
                          /* returnFalseOnEBUSY = */ true);
}

#if HAS_OS_UNFAIR_LOCK

void MutexPlatformHelper::init(os_unfair_lock &lock, bool checked) {
  (void)checked; // Unfair locks are always checked.
  lock = OS_UNFAIR_LOCK_INIT;
}

void MutexPlatformHelper::destroy(os_unfair_lock &lock) {}

void MutexPlatformHelper::lock(os_unfair_lock &lock) {
  os_unfair_lock_lock(&lock);
}

void MutexPlatformHelper::unlock(os_unfair_lock &lock) {
  os_unfair_lock_unlock(&lock);
}

bool MutexPlatformHelper::try_lock(os_unfair_lock &lock) {
  return os_unfair_lock_trylock(&lock);
}

#endif

void ReadWriteLockPlatformHelper::init(pthread_rwlock_t &rwlock) {
  reportError(pthread_rwlock_init(&rwlock, nullptr));
}

void ReadWriteLockPlatformHelper::destroy(pthread_rwlock_t &rwlock) {
  reportError(pthread_rwlock_destroy(&rwlock));
}

void ReadWriteLockPlatformHelper::readLock(pthread_rwlock_t &rwlock) {
  reportError(pthread_rwlock_rdlock(&rwlock));
}

bool ReadWriteLockPlatformHelper::try_readLock(pthread_rwlock_t &rwlock) {
  returnTrueOrReportError(pthread_rwlock_tryrdlock(&rwlock),
                          /* returnFalseOnEBUSY = */ true);
}

void ReadWriteLockPlatformHelper::writeLock(pthread_rwlock_t &rwlock) {
  reportError(pthread_rwlock_wrlock(&rwlock));
}

bool ReadWriteLockPlatformHelper::try_writeLock(pthread_rwlock_t &rwlock) {
  returnTrueOrReportError(pthread_rwlock_trywrlock(&rwlock),
                          /* returnFalseOnEBUSY = */ true);
}

void ReadWriteLockPlatformHelper::readUnlock(pthread_rwlock_t &rwlock) {
  reportError(pthread_rwlock_unlock(&rwlock));
}

void ReadWriteLockPlatformHelper::writeUnlock(pthread_rwlock_t &rwlock) {
  reportError(pthread_rwlock_unlock(&rwlock));
}
#endif
