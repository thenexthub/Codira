//==--- Linux.cpp - Threading abstraction implementation ------- -*-C++ -*-===//
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
// Implements threading support for Linux
//
//===----------------------------------------------------------------------===//

#if LANGUAGE_THREADING_LINUX

#include "language/Threading/Impl.h"
#include "language/Threading/Errors.h"
#include "language/Threading/ThreadSanitizer.h"

namespace {

#pragma clang diagnostic push
#pragma GCC diagnostic ignored "-Wglobal-constructors"

class MainThreadRememberer {
private:
  pthread_t mainThread_;

public:
  MainThreadRememberer() { mainThread_ = pthread_self(); }

  pthread_t main_thread() const { return mainThread_; }
};

MainThreadRememberer rememberer;

#if !defined(__LP64__) && !defined(_LP64)
pthread_mutex_t once_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

#pragma clang diagnostic pop

} // namespace

using namespace language;
using namespace threading_impl;

bool language::threading_impl::thread_is_main() {
  return pthread_equal(pthread_self(), rememberer.main_thread());
}

void language::threading_impl::once_slow(once_t &predicate, void (*fn)(void *),
                                      void *context) {
  // On 32-bit Linux we can't have per-once locks
#if defined(__LP64__) || defined(_LP64)
  linux::ulock_lock(&predicate.lock);
#else
  pthread_mutex_lock(&once_mutex);
#endif
  if (predicate.flag.load(std::memory_order_acquire) == 0) {
    fn(context);
    predicate.flag.store(tsan::enabled() ? 1 : -1, std::memory_order_release);
  }
#if defined(__LP64__) || defined(_LP64)
  linux::ulock_unlock(&predicate.lock);
#else
  pthread_mutex_unlock(&once_mutex);
#endif
}

std::optional<language::threading_impl::stack_bounds>
language::threading_impl::thread_get_current_stack_bounds() {
  pthread_attr_t attr;
  size_t size = 0;
  void *begin = nullptr;

  if (!pthread_getattr_np(pthread_self(), &attr)) {
    if (!pthread_attr_getstack(&attr, &begin, &size)) {
      stack_bounds result = { begin, (char *)begin + size };

      pthread_attr_destroy(&attr);

      return result;
    }

    pthread_attr_destroy(&attr);
  }

  return {};
}

#endif // LANGUAGE_THREADING_LINUX
