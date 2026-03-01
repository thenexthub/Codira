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

#include <cstdint>
#include <cstdlib>

#include "common_components/base/mem_utils.h"
#include "common_components/tests/test_helper.h"

using namespace common;
namespace common::test {
class MemUtilsTest : public common::test::BaseTestWithScope {
};

HWTEST_F_L0(MemUtilsTest, CopyZeroBytes)
{
    char dest[100] = {};
    const char* src = "hello world";
    MemoryCopy(reinterpret_cast<uintptr_t>(dest), 0,
               reinterpret_cast<uintptr_t>(src), strlen(src) + 1);
    EXPECT_EQ(dest[0], '\0');
}

HWTEST_F_L0(MemUtilsTest, CopyTwoChunks)
{
    constexpr size_t totalSize = 100;
    char dest[totalSize] = {};
    char src[totalSize] = {};
    for (size_t i = 0; i < totalSize; ++i) {
        src[i] = static_cast<char>('A' + (i % 26));
    }

    MemoryCopy(reinterpret_cast<uintptr_t>(dest), totalSize,
               reinterpret_cast<uintptr_t>(src), totalSize);

    EXPECT_EQ(memcmp(dest, src, totalSize), 0);
}
} // namespace common::test
