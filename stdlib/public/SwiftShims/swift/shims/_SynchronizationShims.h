//===----------------------------------------------------------------------===//
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

#ifndef SWIFT_STDLIB_SYNCHRONIZATION_SHIMS_H
#define SWIFT_STDLIB_SYNCHRONIZATION_SHIMS_H

#include "languageStdbool.h"
#include "languageStdint.h"

#if defined(__linux__)
#include <errno.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h>

static inline __swift_uint32_t _swift_stdlib_gettid() {
  static __thread __swift_uint32_t tid = 0;

  if (tid == 0) {
    tid = syscall(SYS_gettid);
  }

  return tid;
}

static inline __swift_uint32_t _swift_stdlib_futex_lock(__swift_uint32_t *lock) {
  int ret = syscall(SYS_futex, lock, FUTEX_LOCK_PI_PRIVATE,
                    /* val */ 0, // this value is ignored by this futex op
                    /* timeout */ NULL); // block indefinitely

  if (ret == 0) {
    return ret;
  }

  return errno;
}

static inline __swift_uint32_t _swift_stdlib_futex_trylock(__swift_uint32_t *lock) {
  int ret = syscall(SYS_futex, lock, FUTEX_TRYLOCK_PI);

  if (ret == 0) {
    return ret;
  }

  return errno;
}

static inline __swift_uint32_t _swift_stdlib_futex_unlock(__swift_uint32_t *lock) {
  int ret = syscall(SYS_futex, lock, FUTEX_UNLOCK_PI_PRIVATE);

  if (ret == 0) {
    return ret;
  }

  return errno;
}

#endif // defined(__linux__)

#endif // SWIFT_STDLIB_SYNCHRONIZATION_SHIMS_H
