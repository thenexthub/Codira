//===- MemAlloc.h - Memory allocation functions -----------------*- C++ -*-===//
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
/// \file
///
/// This file defines counterparts of C library allocation functions defined in
/// the namespace 'std'. The new allocation functions crash on allocation
/// failure instead of returning null pointer.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_MEMALLOC_H
#define LLVM_SUPPORT_MEMALLOC_H

#include <IndexStoreDB_LLVMSupport/toolchain_Support_Compiler.h>
#include <IndexStoreDB_LLVMSupport/toolchain_Support_ErrorHandling.h>
#include <cstdlib>

namespace toolchain {

LLVM_ATTRIBUTE_RETURNS_NONNULL inline void *safe_malloc(size_t Sz) {
  void *Result = std::malloc(Sz);
  if (Result == nullptr)
    report_bad_alloc_error("Allocation failed");
  return Result;
}

LLVM_ATTRIBUTE_RETURNS_NONNULL inline void *safe_calloc(size_t Count,
                                                        size_t Sz) {
  void *Result = std::calloc(Count, Sz);
  if (Result == nullptr)
    report_bad_alloc_error("Allocation failed");
  return Result;
}

LLVM_ATTRIBUTE_RETURNS_NONNULL inline void *safe_realloc(void *Ptr, size_t Sz) {
  void *Result = std::realloc(Ptr, Sz);
  if (Result == nullptr)
    report_bad_alloc_error("Allocation failed");
  return Result;
}

}
#endif
