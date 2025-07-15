//===--- ConditionVariable.cpp - ConditionVariable Tests ------------------===//
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

#include "language/Threading/ConditionVariable.h"
#include "gtest/gtest.h"

#include <chrono>
#include <vector>

#include "ThreadingHelpers.h"
#include "LockingHelpers.h"

using namespace language;

template <typename Body>
std::chrono::duration<double> measureDuration(Body body) {
  auto start = std::chrono::steady_clock::now();
  body();
  return std::chrono::steady_clock::now() - start;
}

// -----------------------------------------------------------------------------

// Check we can lock, unlock, lock and unlock an unlocked condition variable
TEST(ConditionVariableTest, BasicLockable) {
  ConditionVariable cond;
  basicLockable(cond);
}

// Test ScopedLock and ScopedUnlock
TEST(ConditionVariableTest, ScopedLockThreaded) {
  ConditionVariable cond;
  scopedLockThreaded<ConditionVariable::ScopedLock>(cond);
}

TEST(ConditionVariableTest, ScopedUnlockUnderScopedLockThreaded) {
  ConditionVariable cond;
  scopedUnlockUnderScopedLockThreaded<ConditionVariable::ScopedLock,
                                      ConditionVariable::ScopedUnlock>(cond);
}

// Test withLock()
TEST(ConditionVariableTest, CriticalSectionThreaded) {
  ConditionVariable cond;
  criticalSectionThreaded(cond);
}

#if !LANGUAGE_THREADING_NONE
// Check that timeouts work
TEST(ConditionVariableTest, Timeout) {
  using namespace std::chrono_literals;

  ConditionVariable cond;

  auto duration = measureDuration([&] {
    ConditionVariable::ScopedLock lock(cond);
    bool ret = cond.wait(0.01s);
    ASSERT_FALSE(ret);
  });

  ASSERT_GE(duration.count(), 0.01);

  duration = measureDuration([&] {
    ConditionVariable::ScopedLock lock(cond);
    bool ret = cond.wait(0.1s);
    ASSERT_FALSE(ret);
  });

  ASSERT_GE(duration.count(), 0.1);

  duration = measureDuration([&] {
    ConditionVariable::ScopedLock lock(cond);
    bool ret = cond.wait(1s);
    ASSERT_FALSE(ret);
  });

  ASSERT_GE(duration.count(), 1.0);

  auto deadline = std::chrono::system_clock::now() + 0.5s;

  duration = measureDuration([&] {
    ConditionVariable::ScopedLock lock(cond);
    bool ret = cond.waitUntil(deadline);
    ASSERT_FALSE(ret);
  });

  ASSERT_GE(duration.count(), 0.5);
}

// Check that signal() wakes exactly one waiter
TEST(ConditionVariableTest, Signal) {
  ConditionVariable cond;

  int ready = 0, done = 0;
  std::vector<std::thread> threads;
  const int thread_count = 10;

  for (int i = 0; i < thread_count; ++i) {
    threads.push_back(std::thread([&] {
      cond.lock();
      ++ready;
      cond.wait();
      ++done;
      cond.unlock();
    }));
  }

  // Wait for all threads to be ready
  cond.withLock([&] {
    int tries = 1000;
    while (ready < thread_count && tries--) {
      ConditionVariable::ScopedUnlock unlock(cond);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  });

  ASSERT_EQ(ready, thread_count);

  // Signal the condition variable and check that done increments by one
  // each time
  for (int i = 0; i < thread_count; ++i) {
    cond.signal();

    cond.withLock([&] {
      int tries = 500;
      while (done == i && tries--) {
        ConditionVariable::ScopedUnlock unlock(cond);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
    });

    ASSERT_EQ(done, i + 1);
  }

  ASSERT_EQ(done, thread_count);

  for (auto &thread : threads) {
    thread.join();
  }
}

// Check that broadcast() wakes all waiters
TEST(ConditionVariableTest, Broadcast) {
  ConditionVariable cond;

  int ready = 0, done = 0;
  std::vector<std::thread> threads;
  const int thread_count = 10;

  for (int i = 0; i < thread_count; ++i) {
    threads.push_back(std::thread([&] {
      cond.lock();
      ++ready;
      cond.wait();
      ++done;
      cond.unlock();
    }));
  }

  // Wait for all threads to be ready
  cond.withLock([&] {
    int tries = 1000;
    while (ready < thread_count && tries--) {
      ConditionVariable::ScopedUnlock unlock(cond);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  });

  ASSERT_EQ(ready, thread_count);

  // Broadcast and wait
  cond.broadcast();

  cond.withLock([&] {
    int tries = 1000;
    while (done < thread_count && tries--) {
      ConditionVariable::ScopedUnlock unlock(cond);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  });

  ASSERT_EQ(done, thread_count);

  for (auto &thread : threads) {
    thread.join();
  }
}

#endif
