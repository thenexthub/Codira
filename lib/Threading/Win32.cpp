//==--- Win32.cpp - Threading abstraction implementation ------- -*-C++ -*-===//
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
// Implements threading support for Windows threads
//
//===----------------------------------------------------------------------===//

#if LANGUAGE_THREADING_WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#pragma comment(lib, "synchronization.lib")

#include "language/Threading/Errors.h"
#include "language/Threading/Impl.h"

namespace {

#pragma clang diagnostic push
#pragma GCC diagnostic ignored "-Wglobal-constructors"

class MainThreadRememberer {
private:
  DWORD dwMainThread_;

public:
  MainThreadRememberer() { dwMainThread_ = ::GetCurrentThreadId(); }

  DWORD main_thread() const { return dwMainThread_; }
};

MainThreadRememberer rememberer;

// Prior to Windows 8, we have to use a global lock
#if _WIN32_WINNT < 0x0602
SRWLOCK onceMutex = SRWLOCK_INIT;
CONDITION_VARIABLE onceCond = CONDITION_VARIABLE_INIT;
#endif

#pragma clang diagnostic pop

} // namespace

using namespace language;
using namespace threading_impl;

bool language::threading_impl::thread_is_main() {
  return ::GetCurrentThreadId() == rememberer.main_thread();
}

void language::threading_impl::once_slow(once_t &predicate, void (*fn)(void *),
                                      void *context) {
  intptr_t expected = 0;
  if (predicate.compare_exchange_strong(expected, static_cast<intptr_t>(1),
                                        std::memory_order_relaxed,
                                        std::memory_order_relaxed)) {
    fn(context);

    predicate.store(static_cast<intptr_t>(-1), std::memory_order_release);

#if _WIN32_WINNT >= 0x0602
    // On Windows 8, use WakeByAddressAll() to wake waiters
    WakeByAddressAll(&predicate);
#else
    // On Windows 7 and earlier, we use a global lock and condition variable;
    // this will wake *all* waiters on *all* onces, which might result in a
    // thundering herd problem, but it's the best we can do.
    AcquireSRWLockExclusive(&onceMutex);
    WakeAllConditionVariable(&onceCond);
    ReleaseSRWLockExclusive(&onceMutex);
#endif
    return;
  }

#if _WIN32_WINNT >= 0x0602
  // On Windows 8, loop waiting on the address until it changes to -1
  while (expected >= 0) {
    WaitOnAddress(&predicate, &expected, sizeof(expected), INFINITE);
    expected = predicate.load(std::memory_order_acquire);
  }
#else
  // On Windows 7 and earlier, wait on the global condition variable
  AcquireSRWLockExclusive(&onceMutex);
  while (predicate.load(std::memory_order_acquire) >= 0) {
    SleepConditionVariableSRW(&onceCond, &onceMutex, INFINITE, 0);
  }
  ReleaseSRWLockExclusive(&onceMutex);
#endif
}

std::optional<language::threading_impl::stack_bounds>
language::threading_impl::thread_get_current_stack_bounds() {
#if _WIN32_WINNT >= 0x0602
  ULONG_PTR lowLimit = 0;
  ULONG_PTR highLimit = 0;

  GetCurrentThreadStackLimits(&lowLimit, &highLimit);

  stack_bounds result = { (void *)lowLimit, (void *)highLimit };
#else
  MEMORY_BASIC_INFORMATION mbi;
  VirtualQuery(&mbi, &mbi, sizeof(mbi));

  stack_bounds result = {
    mbi.AllocationBase,
    (char *)mbi.BaseAddress + mbi.RegionSize
  };
#endif

  return result;
}

#endif // LANGUAGE_THREADING_WIN32
