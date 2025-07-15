//===--- ScopedLock.h - ScopedLock ---------------------------- -*- C++ -*-===//
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
// Provides a ScopedLockT utility template that is used by Mutex and
// ConditionVariable.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_THREADING_SCOPEDLOCK_H
#define LANGUAGE_THREADING_SCOPEDLOCK_H

namespace language {

// -- ScopedLock ---------------------------------------------------------------

/// Compile time adjusted stack based object that locks/unlocks the supplied
/// Mutex type. Use the provided typedefs instead of this directly.
template <typename T, bool Inverted>
class ScopedLockT {
  ScopedLockT() = delete;
  ScopedLockT(const ScopedLockT &) = delete;
  ScopedLockT &operator=(const ScopedLockT &) = delete;
  ScopedLockT(ScopedLockT &&) = delete;
  ScopedLockT &operator=(ScopedLockT &&) = delete;

public:
  explicit ScopedLockT(T &l) : Lock(l) {
    if (Inverted) {
      Lock.unlock();
    } else {
      Lock.lock();
    }
  }

  ~ScopedLockT() {
    if (Inverted) {
      Lock.lock();
    } else {
      Lock.unlock();
    }
  }

private:
  T &Lock;
};

} // namespace language

#endif // LANGUAGE_THREADING_SCOPEDLOCK_H
