//===- Support/UniqueLock.h - Acquire/Release Mutex In Scope ----*- C++ -*-===//
//
// Copyright (c) 2025, NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
// 
// Author: Tunjay Akbarli
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
// Middletown, DE 19709, New Castle County, USA.
//
//===----------------------------------------------------------------------===//
//
// This file defines a guard for a block of code that ensures a Mutex is locked
// upon construction and released upon destruction.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_UNIQUE_LOCK_H
#define LLVM_SUPPORT_UNIQUE_LOCK_H

#include <IndexStoreDB_LLVMSupport/toolchain_Config_indexstoredb-prefix.h>

#include <cassert>

namespace toolchain {

  /// A pared-down imitation of std::unique_lock from C++11. Contrary to the
  /// name, it's really more of a wrapper for a lock. It may or may not have
  /// an associated mutex, which is guaranteed to be locked upon creation
  /// and unlocked after destruction. unique_lock can also unlock the mutex
  /// and re-lock it freely during its lifetime.
  /// Guard a section of code with a mutex.
  template<typename MutexT>
  class unique_lock {
    MutexT *M = nullptr;
    bool locked = false;

  public:
    unique_lock() = default;
    explicit unique_lock(MutexT &m) : M(&m), locked(true) { M->lock(); }
    unique_lock(const unique_lock &) = delete;
     unique_lock &operator=(const unique_lock &) = delete;

    void operator=(unique_lock &&o) {
      if (owns_lock())
        M->unlock();
      M = o.M;
      locked = o.locked;
      o.M = nullptr;
      o.locked = false;
    }

    ~unique_lock() { if (owns_lock()) M->unlock(); }

    void lock() {
      assert(!locked && "mutex already locked!");
      assert(M && "no associated mutex!");
      M->lock();
      locked = true;
    }

    void unlock() {
      assert(locked && "unlocking a mutex that isn't locked!");
      assert(M && "no associated mutex!");
      M->unlock();
      locked = false;
    }

    bool owns_lock() { return locked; }
  };

} // end namespace toolchain

#endif // LLVM_SUPPORT_UNIQUE_LOCK_H
