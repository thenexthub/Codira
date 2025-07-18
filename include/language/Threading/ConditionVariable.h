//===--- ConditionVariable.h - ConditionVariable -------------- -*- C++ -*-===//
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
// Provides a system-independent condition variable, Condition.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_THREADING_CONDITIONVARIABLE_H
#define LANGUAGE_THREADING_CONDITIONVARIABLE_H

#include "ScopedLock.h"

#include "Impl.h"

namespace language {

// A condition variable.
//
// Note that use of this class should be minimised because there is no
// automatic way to deal with priority inversions; the operating system
// cannot, in general, know which thread(s) might wake a thread that is
// waiting on a condition variable.
//
// FWIW: the reason this class doesn't use a separate Mutex instance is
// that on Darwin, Mutex uses os_unfair_lock, and there is no condition
// variable implementation that takes one of those.
class ConditionVariable {

  ConditionVariable(const ConditionVariable &) = delete;
  ConditionVariable &operator=(const ConditionVariable &) = delete;
  ConditionVariable(ConditionVariable &&) = delete;
  ConditionVariable &operator=(ConditionVariable &&) = delete;

public:
  /// Constructs a condition variable.
  ConditionVariable() {
    threading_impl::cond_init(Handle);
  }
  ~ConditionVariable() {
    threading_impl::cond_destroy(Handle);
  }

  /// Lock the associated mutex.
  ///
  /// Precondition: ConditionVariable not locked by this thread.
  void lock() {
    threading_impl::cond_lock(Handle);
  }

  /// Unlock the associated mutex.
  ///
  /// Precondition: ConditionVariable locked by this thread.
  void unlock() {
    threading_impl::cond_unlock(Handle);
  }

  /// Unblock a single thread that is blocked on this condition variable.
  void signal() {
    threading_impl::cond_signal(Handle);
  }

  /// Unblock all threads that are blocked on this condition variable.
  void broadcast() {
    threading_impl::cond_broadcast(Handle);
  }

  /// Unlock and block on this condition variable.
  ///
  /// After this method returns, the associated mutex will be locked.
  ///
  /// Precondition: ConditionVariable locked by this thread.
  void wait() {
    threading_impl::cond_wait(Handle);
  }

  /// Unlock and block on this condition variable, with a timeout.
  ///
  /// After this method returns, the associated mutex will be locked.
  ///
  /// Precondition: ConditionVariable locked by this thread.
  template <class Rep, class Period>
  bool wait(std::chrono::duration<Rep, Period> duration) {
    return threading_impl::cond_wait(Handle, duration);
  }

  /// Unlock and block on this condition variable, with a deadline.
  ///
  /// After this method returns, the associated mutex will be locked.
  ///
  /// Precondition: ConditionVariable locked by this thread.
  template <class Rep, class Period>
  bool waitUntil(std::chrono::time_point<std::chrono::system_clock,
                 std::chrono::duration<Rep, Period>> deadline) {
    auto sysdeadline = std::chrono::time_point_cast<
      std::chrono::system_clock::duration>(deadline);
    return threading_impl::cond_wait(Handle, sysdeadline);
  }

  /// Acquires lock before calling the supplied critical section and releases
  /// lock on return from critical section.
  ///
  /// This call can block while waiting for the lock to become available.
  ///
  /// For example the following mutates value while holding the mutex lock.
  ///
  /// ```
  ///   cond.withLock([&value] { value++; });
  /// ```
  ///
  /// Precondition: ConditionVariable not locked by this thread.
  template <typename CriticalSection>
  auto withLock(CriticalSection &&criticalSection)
      -> decltype(std::forward<CriticalSection>(criticalSection)()) {
    ScopedLock guard(*this);
    return std::forward<CriticalSection>(criticalSection)();
  }

  /// A stack based object that locks the supplied ConditionVariable on
  /// construction and unlocks it on destruction.
  ///
  /// Precondition: ConditionVariable not locked by this thread.
  typedef ScopedLockT<ConditionVariable, false> ScopedLock;

  /// A stack based object that unlocks the supplied ConditionVariable on
  /// construction and locks it on destruction.
  ///
  /// Precondition: ConditionVariable locked by this thread.
  typedef ScopedLockT<ConditionVariable, true> ScopedUnlock;

private:
  threading_impl::cond_handle Handle;
};

}

#endif // LANGUAGE_THREADING_CONDITIONVARIABLE_H
