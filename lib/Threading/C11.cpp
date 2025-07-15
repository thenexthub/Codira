//==--- C11.cpp - Threading abstraction implementation --------- -*-C++ -*-===//
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
// Implements threading support for C11 threads
//
//===----------------------------------------------------------------------===//

#if LANGUAGE_THREADING_C11

#include "language/Threading/Impl.h"
#include "language/Threading/Errors.h"

namespace {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"

class C11ThreadingHelper {
private:
  thrd_t mainThread_;
  mtx_t onceMutex_;
  cnd_t onceCond_;

public:
  C11ThreadingHelper() {
    mainThread_ = thrd_current();
    LANGUAGE_C11THREADS_CHECK(::mtx_init(&onceMutex_, ::mtx_plain));
    LANGUAGE_C11THREADS_CHECK(::cnd_init(&onceCond_));
  }

  thrd_t main_thread() const { return mainThread_; }

  void once_lock() { LANGUAGE_C11THREADS_CHECK(mtx_lock(&onceMutex_)); }
  void once_unlock() { LANGUAGE_C11THREADS_CHECK(mtx_unlock(&onceMutex_)); }
  void once_broadcast() { LANGUAGE_C11THREADS_CHECK(cnd_broadcast(&onceCond_)); }
  void once_wait() {
    // The mutex must be locked when this function is entered.  It will
    // be locked again before the function returns.
    LANGUAGE_C11THREADS_CHECK(cnd_wait(&onceCond_, &onceMutex_));
  }
};

C11ThreadingHelper helper;

#pragma clang diagnostic pop

} // namespace

using namespace language;
using namespace threading_impl;

bool language::threading_impl::thread_is_main() {
  return thrd_equal(thrd_current(), helper.main_thread());
}

void language::threading_impl::once_slow(once_t &predicate, void (*fn)(void *),
                                      void *context) {
  std::intptr_t zero = 0;
  if (predicate.compare_exchange_strong(zero, (std::intptr_t)1,
                                        std::memory_order_relaxed,
                                        std::memory_order_relaxed)) {
    fn(context);

    predicate.store((std::intptr_t)-1, std::memory_order_release);

    helper.once_lock();
    helper.once_unlock();
    helper.once_broadcast();
    return;
  }

  helper.once_lock();
  while (predicate.load(std::memory_order_acquire) >= (std::intptr_t)0) {
    helper.once_wait();
  }
  helper.once_unlock();
}

#endif // LANGUAGE_THREADING_C11
