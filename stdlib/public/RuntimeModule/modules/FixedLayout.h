//===--- FixedLayout.h - Types whose layout must be fixed -------*- C++ -*-===//
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
//  Defines types whose in-memory layout must be fixed, which therefore have
//  to be defined using C code.
//
//===----------------------------------------------------------------------===//

#ifndef LANGUAGE_BACKTRACING_FIXED_LAYOUT_H
#define LANGUAGE_BACKTRACING_FIXED_LAYOUT_H

#include <stdint.h>

#ifdef __cplusplus
namespace language {
namespace runtime {
namespace backtrace {
#endif

struct x86_64_gprs {
  uint64_t _r[16];
  uint64_t rflags;
  uint16_t cs, fs, gs, _pad0;
  uint64_t rip;
  uint64_t valid;
};

struct i386_gprs {
  uint32_t _r[8];
  uint32_t eflags;
  uint16_t segreg[6];
  uint32_t eip;
  uint32_t valid;
};

struct arm64_gprs {
  uint64_t _x[32];
  uint64_t pc;
  uint64_t valid;
};

struct arm_gprs {
  uint32_t _r[16];
  uint32_t valid;
};

#ifdef __cplusplus
} // namespace backtrace
} // namespace runtime
} // namespace language
#endif

#endif // LANGUAGE_BACKTRACING_FIXED_LAYOUT_H
