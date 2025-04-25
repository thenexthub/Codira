//===--- ThreadSafeRefCounted.h - Thread-safe Refcounting Base --*- C++ -*-===//
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

#ifndef SWIFT_BASIC_THREADSAFEREFCOUNTED_H
#define SWIFT_BASIC_THREADSAFEREFCOUNTED_H

#include <atomic>
#include <cassert>
#include "llvm/ADT/IntrusiveRefCntPtr.h"

namespace language {

/// A class that has the same function as \c ThreadSafeRefCountedBase, but with
/// a virtual destructor.
///
/// Should be used instead of \c ThreadSafeRefCountedBase for classes that
/// already have virtual methods to enforce dynamic allocation via 'new'.
/// FIXME: This should eventually move to llvm.
class ThreadSafeRefCountedBaseVPTR {
  mutable std::atomic<unsigned> ref_cnt;
  virtual void anchor();

protected:
  ThreadSafeRefCountedBaseVPTR() : ref_cnt(0) {}
  virtual ~ThreadSafeRefCountedBaseVPTR() {}

public:
  void Retain() const {
    ref_cnt += 1;
  }

  void Release() const {
    int refCount = static_cast<int>(--ref_cnt);
    assert(refCount >= 0 && "Reference count was already zero.");
    if (refCount == 0) delete this;
  }
};

} // end namespace language

#endif // SWIFT_BASIC_THREADSAFEREFCOUNTED_H
