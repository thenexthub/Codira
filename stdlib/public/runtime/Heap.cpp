//===--- Heap.cpp - Codira Language Heap Logic -----------------------------===//
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
// Implementations of the Codira heap
//
//===----------------------------------------------------------------------===//

#include "language/Runtime/HeapObject.h"
#include "language/Runtime/Heap.h"
#include "Private.h"
#include "language/Runtime/Debug.h"
#include "language/shims/RuntimeShims.h"
#include <algorithm>
#include <stdlib.h>
#include <string.h>
#if defined(__APPLE__) && LANGUAGE_STDLIB_HAS_DARWIN_LIBMALLOC
#include "language/Basic/Lazy.h"
#include <malloc/malloc.h>
#endif

using namespace language;

#if defined(__APPLE__)
/// On Apple platforms, \c malloc() is always 16-byte aligned.
static constexpr size_t MALLOC_ALIGN_MASK = 15;

#elif defined(__linux__) || defined(_WIN32) || defined(__wasi__)
/// On Linux and Windows, \c malloc() returns 16-byte aligned pointers on 64-bit
/// and 8-byte aligned pointers on 32-bit.
/// On wasi-libc, pointers are 16-byte aligned even though 32-bit for SIMD access.
#if defined(__LP64) || defined(_WIN64) || defined(__wasi__)
static constexpr size_t MALLOC_ALIGN_MASK = 15;
#else
static constexpr size_t MALLOC_ALIGN_MASK = 7;
#endif

#else
/// This platform's \c malloc() constraints are unknown, so fall back to a value
/// derived from \c std::max_align_t that will be sufficient, but is not
/// necessarily optimal.
///
/// The C and C++ standards defined \c max_align_t as a type whose alignment is
/// at least that of every scalar type. It is the lower bound for the alignment
/// of any pointer returned from \c malloc().
static constexpr size_t MALLOC_ALIGN_MASK = alignof(std::max_align_t) - 1;
#endif

// This assert ensures that manually allocated memory always uses the
// AlignedAlloc path. The stdlib will use "default" alignment for any user
// requested alignment less than or equal to _language_MinAllocationAlignment. The
// runtime must ensure that any alignment > _language_MinAllocationAlignment also
// uses the "aligned" deallocation path.
static_assert(_language_MinAllocationAlignment > MALLOC_ALIGN_MASK,
              "Codira's default alignment must exceed platform malloc mask.");

// When alignMask == ~(size_t(0)), allocation uses the "default"
// _language_MinAllocationAlignment. This is different than calling language_slowAlloc
// with `alignMask == _language_MinAllocationAlignment - 1` because it forces
// the use of AlignedAlloc. This allows manually allocated to memory to always
// be deallocated with AlignedFree without knowledge of its original allocation
// alignment.
static size_t computeAlignment(size_t alignMask) {
  return (alignMask == ~(size_t(0))) ? _language_MinAllocationAlignment
                                     : alignMask + 1;
}

// For alignMask > (_minAllocationAlignment-1)
// i.e. alignment == 0 || alignment > _minAllocationAlignment:
//   The runtime must use AlignedAlloc, and the standard library must
//   deallocate using an alignment that meets the same condition.
//
// For alignMask <= (_minAllocationAlignment-1)
// i.e. 0 < alignment <= _minAllocationAlignment:
//   The runtime may use either malloc or AlignedAlloc, and the standard library
//   must deallocate using an identical alignment.
void *language::language_slowAlloc(size_t size, size_t alignMask) {
  void *p;
  // This check also forces "default" alignment to use AlignedAlloc.
  if (alignMask <= MALLOC_ALIGN_MASK) {
    p = malloc(size);
  } else {
    size_t alignment = computeAlignment(alignMask);
    p = AlignedAlloc(size, alignment);
  }
  if (!p) language::language_abortAllocationFailure(size, alignMask);
  return p;
}

void *language::language_slowAllocTyped(size_t size, size_t alignMask,
                                  MallocTypeId typeId) {
#if LANGUAGE_STDLIB_HAS_MALLOC_TYPE
  if (__builtin_available(macOS 15, iOS 17, tvOS 17, watchOS 10, *)) {
    void *p;
    // This check also forces "default" alignment to use malloc_memalign().
    if (alignMask <= MALLOC_ALIGN_MASK) {
      p = malloc_type_malloc(size, typeId);
    } else {
      size_t alignment = computeAlignment(alignMask);

      // Do not use malloc_type_aligned_alloc() here, because we want this
      // to work if `size` is not an integer multiple of `alignment`, which
      // was a requirement of the latter in C11 (but not C17 and later).
      int err = malloc_type_posix_memalign(&p, alignment, size, typeId);
      if (err != 0)
        p = nullptr;
    }
    if (!p) language::language_abortAllocationFailure(size, alignMask);
    return p;
  }
#endif
  return language_slowAlloc(size, alignMask);
}

void *language::language_coroFrameAlloc(size_t size,
                                  MallocTypeId typeId) {
#if LANGUAGE_STDLIB_HAS_MALLOC_TYPE
  if (__builtin_available(macOS 15, iOS 17, tvOS 17, watchOS 10, *)) {
    void *p = malloc_type_malloc(size, typeId);
    if (!p) language::language_abortAllocationFailure(size, 0);
    return p;
  }
#endif
  return malloc(size);
}

// Unknown alignment is specified by passing alignMask == ~(size_t(0)), forcing
// the AlignedFree deallocation path for unknown alignment. The memory
// deallocated with unknown alignment must have been allocated with either
// "default" alignment, or alignment > _language_MinAllocationAlignment, to
// guarantee that it was allocated with AlignedAlloc.
//
// The standard library assumes the following behavior:
//
// For alignMask > (_minAllocationAlignment-1)
// i.e. alignment == 0 || alignment > _minAllocationAlignment:
//   The runtime must use AlignedFree.
//
// For alignMask <= (_minAllocationAlignment-1)
// i.e. 0 < alignment <= _minAllocationAlignment:
//   The runtime may use either `free` or AlignedFree as long as it is
//   consistent with allocation with the same alignment.
static void language_slowDeallocImpl(void *ptr, size_t alignMask) {
  if (alignMask <= MALLOC_ALIGN_MASK) {
    free(ptr);
  } else {
    AlignedFree(ptr);
  }
}

void language::language_slowDealloc(void *ptr, size_t bytes, size_t alignMask) {
  language_slowDeallocImpl(ptr, alignMask);
}

void language::language_clearSensitive(void *ptr, size_t bytes) {
  // TODO: use memset_s if available
  // Though, it shouldn't make too much difference because the optimizer cannot remove
  // the following memset without inlining this library function.
  memset(ptr, 0, bytes);
}
