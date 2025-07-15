//===- MemAlloc.h - Memory allocation functions -----------------*- C++ -*-===//
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
/// \file
///
/// This file defines counterparts of C library allocation functions defined in
/// the namespace 'std'. The new allocation functions crash on allocation
/// failure instead of returning null pointer.
///
//===----------------------------------------------------------------------===//

#ifndef TOOLCHAIN_SUPPORT_MEMALLOC_H
#define TOOLCHAIN_SUPPORT_MEMALLOC_H

#include "toolchain/Support/Compiler.h"
#include "toolchain/Support/ErrorHandling.h"
#include <cstdlib>

inline namespace __language { inline namespace __runtime {
namespace toolchain {

TOOLCHAIN_ATTRIBUTE_RETURNS_NONNULL inline void *safe_malloc(size_t Sz) {
  void *Result = std::malloc(Sz);
  if (Result == nullptr) {
    // It is implementation-defined whether allocation occurs if the space
    // requested is zero (ISO/IEC 9899:2018 7.22.3). Retry, requesting
    // non-zero, if the space requested was zero.
    if (Sz == 0)
      return safe_malloc(1);
    report_bad_alloc_error("Allocation failed");
  }
  return Result;
}

TOOLCHAIN_ATTRIBUTE_RETURNS_NONNULL inline void *safe_calloc(size_t Count,
                                                        size_t Sz) {
  void *Result = std::calloc(Count, Sz);
  if (Result == nullptr) {
    // It is implementation-defined whether allocation occurs if the space
    // requested is zero (ISO/IEC 9899:2018 7.22.3). Retry, requesting
    // non-zero, if the space requested was zero.
    if (Count == 0 || Sz == 0)
      return safe_malloc(1);
    report_bad_alloc_error("Allocation failed");
  }
  return Result;
}

TOOLCHAIN_ATTRIBUTE_RETURNS_NONNULL inline void *safe_realloc(void *Ptr, size_t Sz) {
  void *Result = std::realloc(Ptr, Sz);
  if (Result == nullptr) {
    // It is implementation-defined whether allocation occurs if the space
    // requested is zero (ISO/IEC 9899:2018 7.22.3). Retry, requesting
    // non-zero, if the space requested was zero.
    if (Sz == 0)
      return safe_malloc(1);
    report_bad_alloc_error("Allocation failed");
  }
  return Result;
}

/// Allocate a buffer of memory with the given size and alignment.
///
/// When the compiler supports aligned operator new, this will use it to to
/// handle even over-aligned allocations.
///
/// However, this doesn't make any attempt to leverage the fancier techniques
/// like posix_memalign due to portability. It is mostly intended to allow
/// compatibility with platforms that, after aligned allocation was added, use
/// reduced default alignment.
TOOLCHAIN_ATTRIBUTE_RETURNS_NONNULL TOOLCHAIN_ATTRIBUTE_RETURNS_NOALIAS void *
allocate_buffer(size_t Size, size_t Alignment);

/// Deallocate a buffer of memory with the given size and alignment.
///
/// If supported, this will used the sized delete operator. Also if supported,
/// this will pass the alignment to the delete operator.
///
/// The pointer must have been allocated with the corresponding new operator,
/// most likely using the above helper.
void deallocate_buffer(void *Ptr, size_t Size, size_t Alignment);

} // namespace toolchain
}} // namespace language::runtime

#endif
