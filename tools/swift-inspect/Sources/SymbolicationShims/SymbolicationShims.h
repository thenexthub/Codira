//===----------------------------------------------------------------------===//
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

#if defined(__APPLE__)

#include <mach/vm_param.h>
#include <stdint.h>
#include <ptrauth.h>

struct CSTypeRef {
  uintptr_t a, b;
};

struct Range {
  uintptr_t location, length;
};

static inline uintptr_t GetPtrauthMask(void) {
#if __has_feature(ptrauth_calls)
  return (uintptr_t)ptrauth_strip((void*)0x0007ffffffffffff, 0);
#elif __arm64__ && __LP64__
  // Mask all bits above the top of MACH_VM_MAX_ADDRESS, which will
  // match the above ptrauth_strip.
  return (uintptr_t)~0ull >> __builtin_clzll(MACH_VM_MAX_ADDRESS);
#else
  return (uintptr_t)~0ull;
#endif
}

#endif
