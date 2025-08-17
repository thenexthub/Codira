//===-- Support/MutexGuard.h - Acquire/Release Mutex In Scope ---*- C++ -*-===//
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

#ifndef LLVM_SUPPORT_MUTEXGUARD_H
#define LLVM_SUPPORT_MUTEXGUARD_H

#include <IndexStoreDB_LLVMSupport/toolchain_Support_Mutex.h>

namespace toolchain {
  /// Instances of this class acquire a given Mutex Lock when constructed and
  /// hold that lock until destruction. The intention is to instantiate one of
  /// these on the stack at the top of some scope to be assured that C++
  /// destruction of the object will always release the Mutex and thus avoid
  /// a host of nasty multi-threading problems in the face of exceptions, etc.
  /// Guard a section of code with a Mutex.
  class MutexGuard {
    sys::Mutex &M;
    MutexGuard(const MutexGuard &) = delete;
    void operator=(const MutexGuard &) = delete;
  public:
    MutexGuard(sys::Mutex &m) : M(m) { M.lock(); }
    ~MutexGuard() { M.unlock(); }
    /// holds - Returns true if this locker instance holds the specified lock.
    /// This is mostly used in assertions to validate that the correct mutex
    /// is held.
    bool holds(const sys::Mutex& lock) const { return &M == &lock; }
  };
}

#endif // LLVM_SUPPORT_MUTEXGUARD_H
