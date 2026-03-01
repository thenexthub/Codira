//===- Memory.h -------------------------------------------------*- C++ -*-===//
//
// Copyright (c) NeXTHub Corporation. All Rights Reserved.
// DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
//
// Author: Tunjay Akbarli
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at:
//
// http://www.apache.org/licenses/LICENSE-2.0
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
// This file defines arena allocators.
//
// Almost all large objects, such as files, sections or symbols, are
// used for the entire lifetime of the linker once they are created.
// This usage characteristic makes arena allocator an attractive choice
// where the entire linker is one arena. With an arena, newly created
// objects belong to the arena and freed all at once when everything is done.
// Arena allocators are efficient and easy to understand.
// Most objects are allocated using the arena allocators defined by this file.
//
//===----------------------------------------------------------------------===//

#ifndef LLD_COMMON_MEMORY_H
#define LLD_COMMON_MEMORY_H

#include "llvm/Support/Allocator.h"

namespace lld {
// A base class only used by the CommonLinkerContext to keep track of the
// SpecificAlloc<> instances.
struct SpecificAllocBase {
  virtual ~SpecificAllocBase() = default;
  static SpecificAllocBase *getOrCreate(void *tag, size_t size, size_t align,
                                        SpecificAllocBase *(&creator)(void *));
};

// An arena of specific types T, created on-demand.
template <class T> struct SpecificAlloc : public SpecificAllocBase {
  static SpecificAllocBase *create(void *storage) {
    return new (storage) SpecificAlloc<T>();
  }
  llvm::SpecificBumpPtrAllocator<T> alloc;
  static int tag;
};

// The address of this static member is only used as a key in
// CommonLinkerContext::instances. Its value does not matter.
template <class T> int SpecificAlloc<T>::tag = 0;

// Creates the arena on-demand on the first call; or returns it, if it was
// already created.
template <typename T>
inline llvm::SpecificBumpPtrAllocator<T> &getSpecificAllocSingleton() {
  SpecificAllocBase *instance = SpecificAllocBase::getOrCreate(
      &SpecificAlloc<T>::tag, sizeof(SpecificAlloc<T>),
      alignof(SpecificAlloc<T>), SpecificAlloc<T>::create);
  return ((SpecificAlloc<T> *)instance)->alloc;
}

// Creates new instances of T off a (almost) contiguous arena/object pool. The
// instances are destroyed whenever lldMain() goes out of scope.
template <typename T, typename... U> T *make(U &&... args) {
  return new (getSpecificAllocSingleton<T>().Allocate())
      T(std::forward<U>(args)...);
}

template <typename T>
inline llvm::SpecificBumpPtrAllocator<T> &
getSpecificAllocSingletonThreadLocal() {
  thread_local SpecificAlloc<T> instance;
  return instance.alloc;
}

// Create a new instance of T off a thread-local SpecificAlloc, used by code
// like parallel input section initialization. The use cases assume that the
// return value outlives the containing parallelForEach (if exists), which is
// currently guaranteed: when parallelForEach returns, the threads allocating
// the TLS are not destroyed.
//
// Note: Some ports (e.g. ELF) have lots of global states which are currently
// infeasible to remove, and context() just adds overhead with no benefit. The
// allocation performance is of higher importance, so we simply use thread_local
// allocators instead of doing context indirection and pthread_getspecific.
template <typename T, typename... U> T *makeThreadLocal(U &&...args) {
  return new (getSpecificAllocSingletonThreadLocal<T>().Allocate())
      T(std::forward<U>(args)...);
}

template <typename T> T *makeThreadLocalN(size_t n) {
  return new (getSpecificAllocSingletonThreadLocal<T>().Allocate(n)) T[n];
}

} // namespace lld

#endif
