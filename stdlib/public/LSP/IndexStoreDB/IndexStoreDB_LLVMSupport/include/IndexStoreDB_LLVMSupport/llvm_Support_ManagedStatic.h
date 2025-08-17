//===-- toolchain/Support/ManagedStatic.h - Static Global wrapper ----*- C++ -*-===//
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
// This file defines the ManagedStatic class and the toolchain_shutdown() function.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_MANAGEDSTATIC_H
#define LLVM_SUPPORT_MANAGEDSTATIC_H

#include <IndexStoreDB_LLVMSupport/toolchain_Config_indexstoredb-prefix.h>

#include <atomic>
#include <cstddef>

namespace toolchain {

/// object_creator - Helper method for ManagedStatic.
template <class C> struct object_creator {
  static void *call() { return new C(); }
};

/// object_deleter - Helper method for ManagedStatic.
///
template <typename T> struct object_deleter {
  static void call(void *Ptr) { delete (T *)Ptr; }
};
template <typename T, size_t N> struct object_deleter<T[N]> {
  static void call(void *Ptr) { delete[](T *)Ptr; }
};

// If the current compiler is MSVC 2017 or earlier, then we have to work around
// a bug where MSVC emits code to perform dynamic initialization even if the
// class has a constexpr constructor. Instead, fall back to the C++98 strategy
// where there are no constructors or member initializers. We can remove this
// when MSVC 2019 (19.20+) is our minimum supported version.
#if !defined(__clang__) && defined(_MSC_VER) && _MSC_VER < 1920
#define LLVM_AVOID_CONSTEXPR_CTOR
#endif

/// ManagedStaticBase - Common base class for ManagedStatic instances.
class ManagedStaticBase {
protected:
#ifndef LLVM_AVOID_CONSTEXPR_CTOR
  mutable std::atomic<void *> Ptr{nullptr};
  mutable void (*DeleterFn)(void *) = nullptr;
  mutable const ManagedStaticBase *Next = nullptr;
#else
  // This should only be used as a static variable, which guarantees that this
  // will be zero initialized.
  mutable std::atomic<void *> Ptr;
  mutable void (*DeleterFn)(void *);
  mutable const ManagedStaticBase *Next;
#endif

  void RegisterManagedStatic(void *(*creator)(), void (*deleter)(void*)) const;

public:
#ifndef LLVM_AVOID_CONSTEXPR_CTOR
  constexpr ManagedStaticBase() = default;
#endif

  /// isConstructed - Return true if this object has not been created yet.
  bool isConstructed() const { return Ptr != nullptr; }

  void destroy() const;
};

/// ManagedStatic - This transparently changes the behavior of global statics to
/// be lazily constructed on demand (good for reducing startup times of dynamic
/// libraries that link in LLVM components) and for making destruction be
/// explicit through the toolchain_shutdown() function call.
///
template <class C, class Creator = object_creator<C>,
          class Deleter = object_deleter<C>>
class ManagedStatic : public ManagedStaticBase {
public:
  // Accessors.
  C &operator*() {
    void *Tmp = Ptr.load(std::memory_order_acquire);
    if (!Tmp)
      RegisterManagedStatic(Creator::call, Deleter::call);

    return *static_cast<C *>(Ptr.load(std::memory_order_relaxed));
  }

  C *operator->() { return &**this; }

  const C &operator*() const {
    void *Tmp = Ptr.load(std::memory_order_acquire);
    if (!Tmp)
      RegisterManagedStatic(Creator::call, Deleter::call);

    return *static_cast<C *>(Ptr.load(std::memory_order_relaxed));
  }

  const C *operator->() const { return &**this; }
};

/// toolchain_shutdown - Deallocate and destroy all ManagedStatic variables.
void toolchain_shutdown();

/// toolchain_shutdown_obj - This is a simple helper class that calls
/// toolchain_shutdown() when it is destroyed.
struct toolchain_shutdown_obj {
  toolchain_shutdown_obj() = default;
  ~toolchain_shutdown_obj() { toolchain_shutdown(); }
};

} // end namespace toolchain

#endif // LLVM_SUPPORT_MANAGEDSTATIC_H
