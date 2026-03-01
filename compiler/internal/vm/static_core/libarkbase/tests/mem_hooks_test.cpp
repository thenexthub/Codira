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

#include <gtest/gtest.h>
#include <iostream>

#include "libarkbase/os/mem_hooks.h"

namespace ark {

void MallocFunc()
{
    char *ptr = static_cast<char *>(malloc(5_MB));  // NOLINT(cppcoreguidelines-no-malloc)
    ptr[0U] = 'a';                                  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    ptr[1U] = '\0';                                 // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    std::cout << ptr << std::endl;                  // NOLINT(clang-analyzer-unix.Malloc)
}

TEST(MemHooksTest, SimpleDeathTest)
{
#ifdef PANDA_USE_MEMORY_HOOKS
    os::mem_hooks::PandaHooks::Enable();
    ASSERT_TRUE(os::mem_hooks::PandaHooks::IsActive());
#ifndef NDEBUG
    // Allocate too much memory via malloc in MallocFunc, so test must die
    ASSERT_DEATH(MallocFunc(), "");
#endif
    os::mem_hooks::PandaHooks::Disable();
    ASSERT_FALSE(os::mem_hooks::PandaHooks::IsActive());
#endif  // PANDA_USE_MEMORY_HOOKS
}

}  // namespace ark
