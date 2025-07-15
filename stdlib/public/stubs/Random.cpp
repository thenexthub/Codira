//===--- Random.cpp -------------------------------------------------------===//
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

// language_stdlib_random
//
// Should the implementation of this function add a new platform/change for a
// platform, make sure to also update the documentation regarding platform
// implementation of this function.
// This can be found at: /docs/Random.md

#if defined(_WIN32) && !defined(__CYGWIN__)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <Bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#elif !defined(__APPLE__)

#include <errno.h>
#include <fcntl.h>

#if __has_include(<sys/random.h>)
#include <sys/random.h>
#endif
#if __has_include(<sys/stat.h>)
#include <sys/stat.h>
#endif
#if __has_include(<sys/syscall.h>)
#include <sys/syscall.h>
#endif

#endif

#if __has_include(<unistd.h>)
#include <unistd.h>
#endif

#include <stdlib.h>

#include "language/shims/Random.h"
#include "language/Runtime/Debug.h"
#include "language/Threading/Mutex.h"

#include <algorithm> // required for std::min

using namespace language;

#if defined(__APPLE__)

LANGUAGE_RUNTIME_STDLIB_API
void language_stdlib_random(void *buf, __language_size_t nbytes) {
  arc4random_buf(buf, nbytes);
}

#elif defined(_WIN32) && !defined(__CYGWIN__)

LANGUAGE_RUNTIME_STDLIB_API
void language_stdlib_random(void *buf, __language_size_t nbytes) {
  while (nbytes > 0) {
    __language_size_t actual_nbytes =
        std::min(nbytes, static_cast<__language_size_t>(ULONG_MAX));
    NTSTATUS status = BCryptGenRandom(nullptr,
                                      static_cast<PUCHAR>(buf),
                                      static_cast<ULONG>(actual_nbytes),
                                      BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (!BCRYPT_SUCCESS(status))
      fatalError(0, "Fatal error: BCryptGenRandom returned 0x%lX in '%s'\n",
                 status, __func__);

    buf = static_cast<uint8_t *>(buf) + actual_nbytes;
    nbytes -= actual_nbytes;
  }
}

#else

#undef  WHILE_EINTR
#define WHILE_EINTR(expression) ([&] () -> decltype(expression) {              \
  decltype(expression) result = -1;                                            \
  do { result = (expression); } while (result == -1 && errno == EINTR);        \
  return result;                                                               \
}())

LANGUAGE_RUNTIME_STDLIB_API
void language_stdlib_random(void *buf, __language_size_t nbytes) {
  while (nbytes > 0) {
    __language_ssize_t actual_nbytes = -1;

#if defined(__NR_getrandom)
    static const bool getrandom_available =
      !(syscall(__NR_getrandom, nullptr, 0, 0) == -1 && errno == ENOSYS);
  
    if (getrandom_available) {
      actual_nbytes = WHILE_EINTR(syscall(__NR_getrandom, buf, nbytes, 0));
    }
#elif __has_include(<sys/random.h>) && (defined(__CYGWIN__) || defined(__Fuchsia__) || defined(__wasi__))
    __language_size_t getentropy_nbytes = std::min(nbytes, __language_size_t{256});
    
    if (0 == getentropy(buf, getentropy_nbytes)) {
      actual_nbytes = getentropy_nbytes;
    }
#endif

    if (actual_nbytes == -1) {
      static const int fd = 
        WHILE_EINTR(open("/dev/urandom", O_RDONLY | O_CLOEXEC, 0));
        
      if (fd != -1) {
        // ###FIXME: Why is this locked?  None of the others are.
        static LazyMutex mutex;
        mutex.withLock([&] {
          actual_nbytes = WHILE_EINTR(read(fd, buf, nbytes));
        });
      }
    }
    
    if (actual_nbytes == -1) {
      fatalError(0, "Fatal error: %d in '%s'\n", errno, __func__);
    }
    
    buf = static_cast<uint8_t *>(buf) + actual_nbytes;
    nbytes -= actual_nbytes;
  }
}

#endif

