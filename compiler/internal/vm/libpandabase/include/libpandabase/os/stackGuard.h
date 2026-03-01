/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef LIBPANDABASE_OS_STACKGUARD_H
#define LIBPANDABASE_OS_STACKGUARD_H

#include <cstdint>

#include "macros.h"

namespace panda {
// Returns the current stack top
//
// GetCurrentFrameAddress() should not be inlined, because it works on stack
// frames if it were inlined into a function with a huge stack frame it would
// return an address significantly above the actual current stack position.
[[maybe_unused]] NO_INLINE static uintptr_t GetCurrentFrameAddress()
{
#if defined(__clang__) || defined(__GNUC__)
// Linux\OHOS\MAC OS
    return reinterpret_cast<uintptr_t>(__builtin_frame_address(0));
#elif defined(_MSC_VER)
// Windows
    return reinterpret_cast<uintptr_t>(_AddressOfReturnAddress());
#else
    return 0ULL;
#endif
}

class StackGuard {
public:
    inline void Initialize()
    {
        uintptr_t initialFrame = GetCurrentFrameAddress();
    #if defined(PANDA_TARGET_UNIX)
    // Linux\OHOS\MAC OS. The stack space is limited to 96% of the default stack space
        stackLimit_ = initialFrame - PANDA_STACK_SIZE;
    #elif defined(PANDA_TARGET_WINDOWS)
    // Windows platform. The stack space is limited to 96% of the default stack space
        stackLimit_ = initialFrame - PANDA_STACK_SIZE;
    #else
        stackLimit_ = 0;
    #endif
        overflow_ = false;
    }

    bool CheckOverflow();
    uintptr_t StackLimit()
    {
        return stackLimit_;
    }

private:
    uintptr_t stackLimit_ {0};
    bool overflow_ {false};
};
}  // namespace panda
#endif  // LIBPANDABASE_OS_STACKGUARD_H
