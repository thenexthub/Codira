//===---------- Coro.h - ABI structures for coroutines ----------*- C++ -*-===//
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
// Codira ABI describing the coroutine ABI.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_ABI_CORO_H
#define LANGUAGE_ABI_CORO_H

#include "language/Basic/FlagSet.h"
#include <cstddef>
#include <cstdint>

namespace language {

enum class CoroAllocatorKind : uint8_t {
  // stacksave/stackrestore
  Stack = 0,
  // language_task_alloc/language_task_dealloc_through
  Async = 1,
  // malloc/free
  Malloc = 2,
};

class CoroAllocatorFlags : public FlagSet<uint32_t> {
public:
  // clang-format off
  enum {
    Kind           = 0,
    Kind_width     = 8,

    ShouldDeallocateImmediately = 8,

    // 24 reserved bits
  };
  // clang-format on

  constexpr explicit CoroAllocatorFlags(uint32_t bits) : FlagSet(bits) {}
  CoroAllocatorFlags(CoroAllocatorKind kind) { setKind(kind); }
  constexpr CoroAllocatorFlags() {}

  FLAGSET_DEFINE_FIELD_ACCESSORS(Kind, Kind_width, CoroAllocatorKind, getKind,
                                 setKind)
  FLAGSET_DEFINE_FLAG_ACCESSORS(ShouldDeallocateImmediately,
                                shouldDeallocateImmediately,
                                setShouldDeallocateImmediately)
};

using CoroAllocation = void *;
using CoroAllocateFn = CoroAllocation (*)(size_t);
using CoroDealllocateFn = void (*)(CoroAllocation);

struct CoroAllocator {
  CoroAllocatorFlags flags;
  CoroAllocateFn allocate;
  CoroDealllocateFn deallocate;

  /// Whether the allocator should deallocate memory on calls to
  /// language_coro_dealloc.
  bool shouldDeallocateImmediately() {
    // Currently, only the "mallocator" should immediately deallocate in
    // language_coro_dealloc.  It must because the ABI does not provide a means for
    // the callee to return its allocations to the caller.
    //
    // Async allocations can be deferred until the first non-coroutine caller
    // from where language_task_dealloc_through can be called and passed the
    // caller-allocated fixed-per-callee-sized-frame.
    // Stack allocations just pop the stack.
    return flags.shouldDeallocateImmediately();
  }
};

} // end namespace language

#endif
