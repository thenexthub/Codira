//===-- language/Compability-rt/runtime/lock.h -------------------------*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
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

// Wraps a mutex

#ifndef FLANG_RT_RUNTIME_LOCK_H_
#define FLANG_RT_RUNTIME_LOCK_H_

#include "terminator.h"
#include "tools.h"

// Avoid <mutex> if possible to avoid introduction of C++ runtime
// library dependence.
#ifndef _WIN32
#define USE_PTHREADS 1
#else
#undef USE_PTHREADS
#endif

#if USE_PTHREADS
#include <pthread.h>
#elif defined(_WIN32)
#include "language/Compability/Common/windows-include.h"
#else
#include <mutex>
#endif

namespace language::Compability::runtime {

class Lock {
public:
#if RT_USE_PSEUDO_LOCK
  // No lock implementation, e.g. for using together
  // with RT_USE_PSEUDO_FILE_UNIT.
  // The users of Lock class may use it under
  // USE_PTHREADS and otherwise, so it has to provide
  // all the interfaces.
  RT_API_ATTRS void Take() {}
  RT_API_ATTRS bool Try() { return true; }
  RT_API_ATTRS void Drop() {}
  RT_API_ATTRS bool TakeIfNoDeadlock() { return true; }
#elif USE_PTHREADS
  Lock() { pthread_mutex_init(&mutex_, nullptr); }
  ~Lock() { pthread_mutex_destroy(&mutex_); }
  void Take() {
    while (pthread_mutex_lock(&mutex_)) {
    }
    holder_ = pthread_self();
    isBusy_ = true;
  }
  bool TakeIfNoDeadlock() {
    if (isBusy_) {
      auto thisThread{pthread_self()};
      if (pthread_equal(thisThread, holder_)) {
        return false;
      }
    }
    Take();
    return true;
  }
  bool Try() { return pthread_mutex_trylock(&mutex_) == 0; }
  void Drop() {
    isBusy_ = false;
    pthread_mutex_unlock(&mutex_);
  }
#elif defined(_WIN32)
  Lock() { InitializeCriticalSection(&cs_); }
  ~Lock() { DeleteCriticalSection(&cs_); }
  void Take() { EnterCriticalSection(&cs_); }
  bool Try() { return TryEnterCriticalSection(&cs_); }
  void Drop() { LeaveCriticalSection(&cs_); }
#else
  void Take() { mutex_.lock(); }
  bool Try() { return mutex_.try_lock(); }
  void Drop() { mutex_.unlock(); }
#endif

  void CheckLocked(const Terminator &terminator) {
    if (Try()) {
      Drop();
      terminator.Crash("Lock::CheckLocked() failed");
    }
  }

private:
#if RT_USE_PSEUDO_FILE_UNIT
  // No state.
#elif USE_PTHREADS
  pthread_mutex_t mutex_{};
  volatile bool isBusy_{false};
  volatile pthread_t holder_;
#elif defined(_WIN32)
  CRITICAL_SECTION cs_;
#else
  std::mutex mutex_;
#endif
};

class CriticalSection {
public:
  explicit RT_API_ATTRS CriticalSection(Lock &lock) : lock_{lock} {
    lock_.Take();
  }
  RT_API_ATTRS ~CriticalSection() { lock_.Drop(); }

private:
  Lock &lock_;
};
} // namespace language::Compability::runtime

#endif // FLANG_RT_RUNTIME_LOCK_H_
