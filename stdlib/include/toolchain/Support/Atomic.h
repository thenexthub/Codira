//===- toolchain/Support/Atomic.h - Atomic Operations -----------------*- C++ -*-===//
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
// This file declares the toolchain::sys atomic operations.
//
// DO NOT USE IN NEW CODE!
//
// New code should always rely on the std::atomic facilities in C++11.
//
//===----------------------------------------------------------------------===//

#ifndef TOOLCHAIN_SUPPORT_ATOMIC_H
#define TOOLCHAIN_SUPPORT_ATOMIC_H

#include <stdint.h>

// Windows will at times define MemoryFence.
#ifdef MemoryFence
#undef MemoryFence
#endif

inline namespace __language { inline namespace __runtime {
namespace toolchain {
  namespace sys {
    void MemoryFence();

#ifdef _MSC_VER
    typedef long cas_flag;
#else
    typedef uint32_t cas_flag;
#endif
    cas_flag CompareAndSwap(volatile cas_flag* ptr,
                            cas_flag new_value,
                            cas_flag old_value);
  }
}
}} // namespace language::runtime

#endif
