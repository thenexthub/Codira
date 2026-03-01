//===-- PerfTests.cpp -----------------------------------------------------===//
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

#ifdef __x86_64__

#include "Perf.h"

#include "llvm/Support/Error.h"

#include "gtest/gtest.h"

#include <chrono>
#include <cstdint>

using namespace lldb_private;
using namespace process_linux;
using namespace llvm;

/// Helper function to read current TSC value.
///
/// This code is based on llvm/xray.
static Expected<uint64_t> readTsc() {

  unsigned int eax, ebx, ecx, edx;

  // We check whether rdtscp support is enabled. According to the x86_64 manual,
  // level should be set at 0x80000001, and we should have a look at bit 27 in
  // EDX. That's 0x8000000 (or 1u << 27).
  __asm__ __volatile__("cpuid"
                       : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                       : "0"(0x80000001));
  if (!(edx & (1u << 27))) {
    return createStringError(inconvertibleErrorCode(),
                             "Missing rdtscp support.");
  }

  unsigned cpu;
  unsigned long rax, rdx;

  __asm__ __volatile__("rdtscp\n" : "=a"(rax), "=d"(rdx), "=c"(cpu)::);

  return (rdx << 32) + rax;
}

// Test TSC to walltime conversion based on perf conversion values.
TEST(Perf, TscConversion) {
  // This test works by first reading the TSC value directly before
  // and after sleeping, then converting these values to nanoseconds, and
  // finally ensuring the difference is approximately equal to the sleep time.
  //
  // There will be slight overhead associated with the sleep call, so it isn't
  // reasonable to expect the difference to be exactly equal to the sleep time.

  const int SLEEP_SECS = 1;
  std::chrono::nanoseconds SLEEP_NANOS{std::chrono::seconds(SLEEP_SECS)};

  Expected<LinuxPerfZeroTscConversion> params =
      LoadPerfTscConversionParameters();

  // Skip the test if the conversion parameters aren't available.
  if (!params)
    GTEST_SKIP() << toString(params.takeError());

  Expected<uint64_t> tsc_before_sleep = readTsc();
  sleep(SLEEP_SECS);
  Expected<uint64_t> tsc_after_sleep = readTsc();

  // Skip the test if we are unable to read the TSC value.
  if (!tsc_before_sleep)
    GTEST_SKIP() << toString(tsc_before_sleep.takeError());
  if (!tsc_after_sleep)
    GTEST_SKIP() << toString(tsc_after_sleep.takeError());

  uint64_t converted_tsc_diff =
      params->ToNanos(*tsc_after_sleep) - params->ToNanos(*tsc_before_sleep);

  std::chrono::microseconds acceptable_overhead(500);

  ASSERT_GE(converted_tsc_diff, static_cast<uint64_t>(SLEEP_NANOS.count()));
  ASSERT_LT(converted_tsc_diff,
            static_cast<uint64_t>((SLEEP_NANOS + acceptable_overhead).count()));
}

#endif // __x86_64__
