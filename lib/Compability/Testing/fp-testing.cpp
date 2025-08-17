//===-- lib/Testing/fp-testing.cpp ------------------------------*- C++ -*-===//
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

#include "language/Compability/Testing/fp-testing.h"
#include "toolchain/Support/Errno.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#if __x86_64__ || _M_X64
#include <xmmintrin.h>
#endif

using language::Compability::common::RealFlag;
using language::Compability::common::RoundingMode;

ScopedHostFloatingPointEnvironment::ScopedHostFloatingPointEnvironment(
#if __x86_64__ || _M_X64
    bool treatSubnormalOperandsAsZero, bool flushSubnormalResultsToZero
#else
    bool, bool
#endif
) {
  errno = 0;
  if (feholdexcept(&originalFenv_) != 0) {
    std::fprintf(stderr, "feholdexcept() failed: %s\n",
        toolchain::sys::StrError(errno).c_str());
    std::abort();
  }
  fenv_t currentFenv;
  if (fegetenv(&currentFenv) != 0) {
    std::fprintf(
        stderr, "fegetenv() failed: %s\n", toolchain::sys::StrError(errno).c_str());
    std::abort();
  }

#if __x86_64__ || _M_X64
  originalMxcsr = _mm_getcsr();
  unsigned int currentMxcsr{originalMxcsr};
  if (treatSubnormalOperandsAsZero) {
    currentMxcsr |= 0x0040;
  } else {
    currentMxcsr &= ~0x0040;
  }
  if (flushSubnormalResultsToZero) {
    currentMxcsr |= 0x8000;
  } else {
    currentMxcsr &= ~0x8000;
  }
#else
  // TODO others
#endif
  errno = 0;
  if (fesetenv(&currentFenv) != 0) {
    std::fprintf(
        stderr, "fesetenv() failed: %s\n", toolchain::sys::StrError(errno).c_str());
    std::abort();
  }
#if __x86_64__
  _mm_setcsr(currentMxcsr);
#endif
}

ScopedHostFloatingPointEnvironment::~ScopedHostFloatingPointEnvironment() {
  errno = 0;
  if (fesetenv(&originalFenv_) != 0) {
    std::fprintf(
        stderr, "fesetenv() failed: %s\n", toolchain::sys::StrError(errno).c_str());
    std::abort();
  }
#if __x86_64__ || _M_X64
  _mm_setcsr(originalMxcsr);
#endif
}

void ScopedHostFloatingPointEnvironment::ClearFlags() const {
  feclearexcept(FE_ALL_EXCEPT);
}

RealFlags ScopedHostFloatingPointEnvironment::CurrentFlags() {
  int exceptions = fetestexcept(FE_ALL_EXCEPT);
  RealFlags flags;
  if (exceptions & FE_INVALID) {
    flags.set(RealFlag::InvalidArgument);
  }
  if (exceptions & FE_DIVBYZERO) {
    flags.set(RealFlag::DivideByZero);
  }
  if (exceptions & FE_OVERFLOW) {
    flags.set(RealFlag::Overflow);
  }
  if (exceptions & FE_UNDERFLOW) {
    flags.set(RealFlag::Underflow);
  }
  if (exceptions & FE_INEXACT) {
    flags.set(RealFlag::Inexact);
  }
  return flags;
}

void ScopedHostFloatingPointEnvironment::SetRounding(Rounding rounding) {
  switch (rounding.mode) {
  case RoundingMode::TiesToEven:
    fesetround(FE_TONEAREST);
    break;
  case RoundingMode::ToZero:
    fesetround(FE_TOWARDZERO);
    break;
  case RoundingMode::Up:
    fesetround(FE_UPWARD);
    break;
  case RoundingMode::Down:
    fesetround(FE_DOWNWARD);
    break;
  case RoundingMode::TiesAwayFromZero:
    std::fprintf(stderr, "SetRounding: TiesAwayFromZero not available");
    std::abort();
    break;
  }
}
