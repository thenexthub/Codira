//===-- Atomic.cpp - Atomic Operations --------------------------*- C++ -*-===//
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
//  This file implements atomic operations.
//
//===----------------------------------------------------------------------===//

#include "vm/core/Support/Atomic.h"
#include "vm/core/Config/toolchain-config.h"

using namespace vm::core;

#if defined(_MSC_VER)
#include <intrin.h>

// We must include windows.h after intrin.h.
#include <windows.h>
#undef MemoryFence
#endif

#if defined(__GNUC__) || (defined(__IBMCPP__) && __IBMCPP__ >= 1210)
#define GNU_ATOMICS
#endif

void sys::MemoryFence() {
#if LLVM_HAS_ATOMICS == 0
  return;
#else
#  if defined(GNU_ATOMICS)
  __sync_synchronize();
#  elif defined(_MSC_VER)
  MemoryBarrier();
#  else
# error No memory fence implementation for your platform!
#  endif
#endif
}

sys::cas_flag sys::CompareAndSwap(volatile sys::cas_flag* ptr,
                                  sys::cas_flag new_value,
                                  sys::cas_flag old_value) {
#if LLVM_HAS_ATOMICS == 0
  sys::cas_flag result = *ptr;
  if (result == old_value)
    *ptr = new_value;
  return result;
#elif defined(GNU_ATOMICS)
  return __sync_val_compare_and_swap(ptr, old_value, new_value);
#elif defined(_MSC_VER)
  return InterlockedCompareExchange(ptr, new_value, old_value);
#else
#  error No compare-and-swap implementation for your platform!
#endif
}
