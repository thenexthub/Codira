//===--- LinuxULock.cpp - Tests the Linux ulock implementation ------------===//
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

#if LANGUAGE_THREADING_LINUX

#include "language/Threading/Impl/Linux/ulock.h"
#include "gtest/gtest.h"

#include "LockingHelpers.h"

using namespace language;
using namespace language::threading_impl;

// -----------------------------------------------------------------------------

// Use the existing locking tests to check threaded operation
class UlockMutex {
private:
  linux::ulock_t lock_;

public:
  UlockMutex() : lock_(0) {}

  void lock() { linux::ulock_lock(&lock_); }
  void unlock() { linux::ulock_unlock(&lock_); }
  bool try_lock() { return linux::ulock_trylock(&lock_); }
};

TEST(LinuxUlockTest, SingleThreaded) {
  UlockMutex mutex;
  basicLockable(mutex);
}

TEST(LinuxUlockTest, SingleThreadedTryLock) {
  UlockMutex mutex;
  tryLockable(mutex);
}

TEST(LinuxULockTest, BasicLockableThreaded) {
  UlockMutex mutex;
  basicLockableThreaded(mutex);
}

TEST(LinuxULockTest, LockableThreaded) {
  UlockMutex mutex;
  lockableThreaded(mutex);
}

#endif // LANGUAGE_THREADING_LINUX
