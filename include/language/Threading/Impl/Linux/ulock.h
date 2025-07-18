//==--- ulock.h - 32-bit futex-based lock for Linux ------------ -*-C++ -*-===//
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
// Implements a 32-bit futex-based locking primitive with priority inversion
// support.
//
// This has comparable performance to pthread_mutex_t (on x86-64, it's slower
// under contention, but not much different otherwise; other architectures may
// vary), but it only takes up 32 bits instead of 40 *bytes*.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_THREADING_IMPL_LINUX_ULOCK_H
#define LANGUAGE_THREADING_IMPL_LINUX_ULOCK_H

// This file is specific to Linux; we're just going to assume we can use
// various GCC/Clang extensions here.

#include <linux/futex.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <cstdint>

#include "language/Threading/ThreadSanitizer.h"

namespace language {
namespace threading_impl {
namespace linux {

typedef std::int32_t ulock_t;

#define ULOCK_INITIALIZER 0
#define ulock_fastpath(x) __builtin_expect((x), true)

inline int ulock_get_tid(void) {
  static __thread int tid;
  if (tid == 0)
    tid = syscall(SYS_gettid);
  return tid;
}

inline int ulock_futex(ulock_t *lock, int op) {
  return syscall(SYS_futex, lock, op | FUTEX_PRIVATE_FLAG, 0, nullptr, nullptr,
                 0);
}

inline void ulock_lock(ulock_t *lock) {
  const ulock_t tid = ulock_get_tid();
  do {
    ulock_t zero = 0;
    if (ulock_fastpath(__atomic_compare_exchange_n(lock, &zero, tid,
                                                   true, __ATOMIC_ACQUIRE,
                                                   __ATOMIC_RELAXED)))
      break;
  } while (ulock_futex(lock, FUTEX_LOCK_PI) != 0);

  tsan::acquire(lock);
}

inline bool ulock_trylock(ulock_t *lock) {
  ulock_t zero = 0;
  if (ulock_fastpath(__atomic_compare_exchange_n(lock, &zero, ulock_get_tid(),
                                                 true, __ATOMIC_ACQUIRE,
                                                 __ATOMIC_RELAXED))
      || ulock_futex(lock, FUTEX_TRYLOCK_PI) == 0) {
    tsan::acquire(lock);
    return true;
  }

  return false;
}

inline void ulock_unlock(ulock_t *lock) {
  tsan::release(lock);

  const ulock_t tid = ulock_get_tid();
  do {
    ulock_t expected = tid;
    if (ulock_fastpath(__atomic_compare_exchange_n(lock, &expected, 0,
                                                   true, __ATOMIC_RELEASE,
                                                   __ATOMIC_RELAXED)))
      break;
  } while (ulock_futex(lock, FUTEX_UNLOCK_PI) != 0);
}

} // namespace linux
} // namespace threading_impl
} // namespace language

#endif // LANGUAGE_THREADING_IMPL_LINUX_ULOCK_H
