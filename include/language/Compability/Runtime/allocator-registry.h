//===-- language/Compability-rt/runtime/allocator-registry.h -----------*- C++ -*-===//
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

#ifndef FLANG_RT_RUNTIME_ALLOCATOR_REGISTRY_H_
#define FLANG_RT_RUNTIME_ALLOCATOR_REGISTRY_H_

#include "language/Compability/Common/api-attrs.h"
#include "language/Compability/Runtime/allocator-registry-consts.h"
#include <cstdint>
#include <cstdlib>
#include <vector>

#define MAX_ALLOCATOR 7 // 3 bits are reserved in the descriptor.

namespace language::Compability::runtime {

using AllocFct = void *(*)(std::size_t, std::int64_t *);
using FreeFct = void (*)(void *);

typedef struct Allocator_t {
  AllocFct alloc{nullptr};
  FreeFct free{nullptr};
} Allocator_t;

static RT_API_ATTRS void *MallocWrapper(
    std::size_t size, [[maybe_unused]] std::int64_t *) {
  return std::malloc(size);
}
#ifdef RT_DEVICE_COMPILATION
static RT_API_ATTRS void FreeWrapper(void *p) { return std::free(p); }
#endif

struct AllocatorRegistry {
#ifdef RT_DEVICE_COMPILATION
  RT_API_ATTRS constexpr AllocatorRegistry()
      : allocators{{&MallocWrapper, &FreeWrapper}} {}
#else
  constexpr AllocatorRegistry() {
    allocators[kDefaultAllocator] = {&MallocWrapper, &std::free};
  };
#endif
  RT_API_ATTRS void Register(int, Allocator_t);
  RT_API_ATTRS AllocFct GetAllocator(int pos);
  RT_API_ATTRS FreeFct GetDeallocator(int pos);

  Allocator_t allocators[MAX_ALLOCATOR];
};

RT_OFFLOAD_VAR_GROUP_BEGIN
extern RT_VAR_ATTRS AllocatorRegistry allocatorRegistry;
RT_OFFLOAD_VAR_GROUP_END

} // namespace language::Compability::runtime

#endif // FLANG_RT_RUNTIME_ALLOCATOR_REGISTRY_H_
