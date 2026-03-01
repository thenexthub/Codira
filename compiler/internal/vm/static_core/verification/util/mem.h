/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef PANDA_VERIFICATION_UTIL_MEM_H
#define PANDA_VERIFICATION_UTIL_MEM_H

#include <cstdint>
#include "libarkbase/macros.h"

namespace ark::verifier {

/* We are using the fact that on every operating system some of the process's virtual memory space
   is unavailable for allocation -- for example, it may be reserved for kernel memory.

   For Linux:
       https://linux-kernel-labs.github.io/refs/heads/master/lectures/address-space.html

       Linux is using a split address space for 32 bit systems, although in the past there
       were options for supporting 4/4s split or dedicated kernel address space (on those
       architecture that supports it, e.g. x86). Linux always uses split address space for 64 bit systems.

       [For 32-bit Linux, the split is usually 3/1, i.e. 0x00000000-0xc0000000 is user space,
       0xc0000000-0xffffffff is kernel space]

   For Windows: https://learn.microsoft.com/en-us/windows-hardware/drivers/gettingstarted/virtual-address-spaces

       For a 32-bit process, the virtual address space is usually the 2-gigabyte range 0x00000000
       through 0x7FFFFFFF.
       For a 64-bit process on 64-bit Windows, the virtual address space is the 128-terabyte range
       0x000'00000000 through 0x7FFF'FFFFFFFF.
*/

/* Works on Linux and Windows. Might need adjustment for other OSes */

size_t constexpr POINTER_CHECK_SHIFT = sizeof(uintptr_t) * 8 - 4;
// NOLINTNEXTLINE(hicpp-signed-bitwise)
uintptr_t constexpr POINTER_CHECK_MASK = 0xfL << POINTER_CHECK_SHIFT;
uintptr_t constexpr NOT_POINTER = POINTER_CHECK_MASK;

size_t constexpr TAG_SHIFT = sizeof(uintptr_t) * 8 - 8;
// NOLINTNEXTLINE(hicpp-signed-bitwise)
uintptr_t constexpr TAG_MASK = 0xfL << TAG_SHIFT;

uintptr_t constexpr PAYLOAD_MASK = ~(POINTER_CHECK_MASK | TAG_MASK);

ALWAYS_INLINE inline bool IsNotPointer(uintptr_t x)
{
    return (x & POINTER_CHECK_MASK) == NOT_POINTER;
}

ALWAYS_INLINE inline bool IsPointer(uintptr_t x)
{
    return !IsNotPointer(x);
}

ALWAYS_INLINE inline int GetTag(uintptr_t x)
{
    ASSERT(IsNotPointer(x));
    return (x & TAG_MASK) >> TAG_SHIFT;
}

ALWAYS_INLINE inline uintptr_t GetPayload(uintptr_t x)
{
    ASSERT(IsNotPointer(x));
    return x & PAYLOAD_MASK;
}

ALWAYS_INLINE inline uintptr_t GetPointer(uintptr_t x)
{
    ASSERT(IsPointer(x));
    return x;
}

ALWAYS_INLINE inline uintptr_t ConstructWithTag(int tag, uintptr_t v)
{
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    ASSERT(tag < (1 << 4U));
    ASSERT(v < (1UL << TAG_SHIFT));
    return NOT_POINTER | (static_cast<uintptr_t>(tag) << TAG_SHIFT) | v;
}

}  // namespace ark::verifier

#endif /* PANDA_VERIFICATION_UTIL_MEM_H */
