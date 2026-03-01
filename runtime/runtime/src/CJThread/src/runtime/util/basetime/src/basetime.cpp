/*
 * Copyright (c) NeXTHub Corporation. All Rights Reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Author: Tunjay Akbarli
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Please contact NeXTHub Corporation, 651 N Broad St, Suite 201,
 * Middletown, DE 19709, New Castle County, USA.
 */


#include <ctime>
#include <cstdint>
#include "basetime.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Number of nanoseconds equal to 1 second */
const unsigned long long SECOND_TO_NANO_NUMBER = 1000000000;
#define CPU_SHIFTS_STEP (32)

/* Obtains the current time in nanoseconds. */
unsigned long long CurrentNanotimeGet(void)
{
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    return (time.tv_sec * SECOND_TO_NANO_NUMBER + time.tv_nsec);
}

/* Obtain the current time, expressed in nanoseconds */
#if (MRT_HARDWARE_PLATFORM == MRT_ARM && VOS_WORDSIZE == 64)
unsigned long long CurrentCPUTicks(void)
{
    unsigned long long ticks = 0;
    asm volatile("mrs %0, cntvct_el0" : "=r"(ticks));
    return ticks;
}
#elif (MRT_HARDWARE_PLATFORM == MRT_ARM && VOS_WORDSIZE == 32)
unsigned long long CurrentCPUTicks(void)
{
    uint32_t ticks_low = 0;
    uint32_t ticks_high = 0;
    asm volatile(
        "mrrc p15, 1, %0, %1, c14" : "=r" (ticks_low), "=r" (ticks_high)
    );
    return (static_cast<unsigned long long>(ticks_high) << 32) | ticks_low;
}
#elif  (MRT_HARDWARE_PLATFORM == MRT_X86 || MRT_HARDWARE_PLATFORM == MRT_WINDOWS_X86)
unsigned long long CurrentCPUTicks(void)
{
    unsigned long long retHigh;
    unsigned long long retLow;
    asm volatile("rdtsc" : "=d" (retHigh), "=a" (retLow));
    return ((retHigh << CPU_SHIFTS_STEP) | (retLow));
}
#endif

#ifdef __cplusplus
}
#endif
