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


#ifndef MRT_GLOBALS_H
#define MRT_GLOBALS_H

#include <cstddef>

#include "Base/Log.h"

namespace MapleRuntime {
// Time Factors
constexpr uint64_t TIME_FACTOR = 1000LL;
constexpr uint64_t SECOND_TO_MILLI_SECOND = TIME_FACTOR;
constexpr uint64_t SECOND_TO_MICRO_SECOND = TIME_FACTOR * TIME_FACTOR;
constexpr uint64_t SECOND_TO_NANO_SECOND = TIME_FACTOR * TIME_FACTOR * TIME_FACTOR;
constexpr uint64_t MILLI_SECOND_TO_MICRO_SECOND = TIME_FACTOR;
constexpr uint64_t MICRO_SECOND_TO_NANO_SECOND = TIME_FACTOR;
constexpr uint64_t MILLI_SECOND_TO_NANO_SECOND = TIME_FACTOR * TIME_FACTOR;

constexpr size_t KB = 1024;
constexpr size_t MB = KB * KB;
constexpr size_t GB = KB * KB * KB;

// System default page size in linux
extern const size_t MRT_PAGE_SIZE;

// The Array Struct size
constexpr size_t ARRAY_STRUCT_SIZE = 3;

template<typename T>
struct Identity {
    using type = T;
};

// For rounding integers.
// Note: Omit the `n` from T type deduction, deduce only from the `x` argument.
template<typename T>
constexpr bool IsPowerOfTwo(T x)
{
    static_assert(std::is_integral<T>::value, "T must be integral");
    bool ret = false;
    if (x != 0) {
        ret = (x & (x - 1)) == 0;
    }
    return ret;
}

template<typename T>
T RoundDown(T x, typename Identity<T>::type n)
{
    DCHECK(IsPowerOfTwo(n));
    return (x & -n);
}

template<typename T>
constexpr T RoundUp(T x, typename std::remove_reference<T>::type n)
{
    return RoundDown(x + n - 1, n);
}

template<typename T>
inline T* AlignUp(T* x, uintptr_t n)
{
    return reinterpret_cast<T*>(RoundUp(reinterpret_cast<uintptr_t>(x), n));
}

template<class T>
constexpr T AlignUp(T size, T alignment)
{
    return ((size + alignment - 1) & ~static_cast<T>(alignment - 1));
}

template<class T>
constexpr T AlignDown(T size, T alignment)
{
    return (size & ~static_cast<T>(alignment - 1));
}
} // namespace MapleRuntime

#endif // MRT_GLOBALS_H
