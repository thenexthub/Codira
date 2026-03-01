/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "compiler/optimizer/code_generator/fast_divisor.h"
#include "compiler/optimizer/code_generator/type_info.h"

namespace ark::compiler {
// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct SignedResult {
    explicit SignedResult(int64_t aMagic, int64_t aShift) : magic(aMagic), shift(aShift) {}

    int64_t magic {0L};
    int64_t shift {0L};
};

struct UnsignedResult {
    explicit UnsignedResult(uint64_t aMagic, uint64_t aShift, bool anAdd) : magic(aMagic), shift(aShift), add(anAdd) {}

    uint64_t magic {0U};
    uint64_t shift {0U};
    bool add {false};
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

// NOLINTBEGIN(readability-magic-numbers)
TEST(FastDivisor, Signed32)
{
    auto testFastDivisor = [](int32_t divisor, SignedResult res) {
        FastConstSignedDivisor fastDiv(divisor, WORD_SIZE);
        EXPECT_EQ(res.magic, fastDiv.GetMagic());
        EXPECT_EQ(res.shift, fastDiv.GetShift());
    };

    testFastDivisor(3L, SignedResult(0x0000000055555556LL, 0L));
    testFastDivisor(5L, SignedResult(0x0000000066666667LL, 1L));
    testFastDivisor(6L, SignedResult(0x000000002AAAAAABLL, 0L));
    testFastDivisor(7L, SignedResult(0xFFFFFFFF92492493LL, 2L));
    testFastDivisor(9L, SignedResult(0x0000000038E38E39LL, 1L));
    testFastDivisor(10L, SignedResult(0x0000000066666667LL, 2L));
    testFastDivisor(11L, SignedResult(0x000000002E8BA2E9LL, 1L));
    testFastDivisor(12L, SignedResult(0x000000002AAAAAABLL, 1L));
    testFastDivisor(25L, SignedResult(0x0000000051EB851FLL, 3L));
    testFastDivisor(125L, SignedResult(0x0000000010624DD3LL, 3L));
    testFastDivisor(625L, SignedResult(0x0000000068DB8BADLL, 8L));
}

TEST(FastDivisor, Signed64)
{
    auto testFastDivisor = [](int64_t divisor, SignedResult res) {
        FastConstSignedDivisor fastDiv(divisor, DOUBLE_WORD_SIZE);
        EXPECT_EQ(res.magic, fastDiv.GetMagic());
        EXPECT_EQ(res.shift, fastDiv.GetShift());
    };

    testFastDivisor(3L, SignedResult(0x5555555555555556LL, 0L));
    testFastDivisor(5L, SignedResult(0x6666666666666667LL, 1L));
    testFastDivisor(6L, SignedResult(0x2AAAAAAAAAAAAAABLL, 0L));
    testFastDivisor(7L, SignedResult(0x4924924924924925LL, 1L));
    testFastDivisor(9L, SignedResult(0x1C71C71C71C71C72LL, 0L));
    testFastDivisor(10L, SignedResult(0x6666666666666667LL, 2L));
    testFastDivisor(11L, SignedResult(0x2E8BA2E8BA2E8BA3LL, 1L));
    testFastDivisor(12L, SignedResult(0x2AAAAAAAAAAAAAABLL, 1L));
    testFastDivisor(25L, SignedResult(0xA3D70A3D70A3D70BLL, 4L));
    testFastDivisor(125L, SignedResult(0x20C49BA5E353F7CFLL, 4L));
    testFastDivisor(625L, SignedResult(0x346DC5D63886594BLL, 7L));
}

TEST(FastDivisor, Unsigned32)
{
    auto testFastDivisor = [](uint32_t divisor, UnsignedResult res) {
        FastConstUnsignedDivisor fastDiv(divisor, WORD_SIZE);
        EXPECT_EQ(res.magic, fastDiv.GetMagic());
        EXPECT_EQ(res.shift, fastDiv.GetShift());
        EXPECT_EQ(res.add, fastDiv.GetAdd());
    };

    testFastDivisor(3U, UnsignedResult(0xAAAAAAABU, 1U, false));
    testFastDivisor(5U, UnsignedResult(0xCCCCCCCDU, 2U, false));
    testFastDivisor(6U, UnsignedResult(0xAAAAAAABU, 2U, false));
    testFastDivisor(7U, UnsignedResult(0x24924925U, 3U, true));
    testFastDivisor(9U, UnsignedResult(0x38E38E39U, 1U, false));
    testFastDivisor(10U, UnsignedResult(0xCCCCCCCDU, 3U, false));
    testFastDivisor(11U, UnsignedResult(0xBA2E8BA3U, 3U, false));
    testFastDivisor(12U, UnsignedResult(0xAAAAAAABU, 3U, false));
    testFastDivisor(25U, UnsignedResult(0x51EB851FU, 3U, false));
    testFastDivisor(125U, UnsignedResult(0x10624DD3U, 3U, false));
    testFastDivisor(625U, UnsignedResult(0xD1B71759U, 9U, false));
}

TEST(FastDivisor, Unsigned64)
{
    auto testFastDivisor = [](uint64_t divisor, UnsignedResult res) {
        FastConstUnsignedDivisor fastDiv(divisor, DOUBLE_WORD_SIZE);
        EXPECT_EQ(res.magic, fastDiv.GetMagic());
        EXPECT_EQ(res.shift, fastDiv.GetShift());
        EXPECT_EQ(res.add, fastDiv.GetAdd());
    };

    testFastDivisor(3U, UnsignedResult(0xAAAAAAAAAAAAAAABULL, 1U, false));
    testFastDivisor(5U, UnsignedResult(0xCCCCCCCCCCCCCCCDULL, 2U, false));
    testFastDivisor(6U, UnsignedResult(0xAAAAAAAAAAAAAAABULL, 2U, false));
    testFastDivisor(7U, UnsignedResult(0x2492492492492493ULL, 3U, true));
    testFastDivisor(9U, UnsignedResult(0xE38E38E38E38E38FULL, 3U, false));
    testFastDivisor(10U, UnsignedResult(0xCCCCCCCCCCCCCCCDULL, 3U, false));
    testFastDivisor(11U, UnsignedResult(0x2E8BA2E8BA2E8BA3ULL, 1U, false));
    testFastDivisor(12U, UnsignedResult(0xAAAAAAAAAAAAAAABULL, 3U, false));
    testFastDivisor(25U, UnsignedResult(0x47AE147AE147AE15ULL, 5U, true));
    testFastDivisor(125U, UnsignedResult(0x0624DD2F1A9FBE77ULL, 7U, true));
    testFastDivisor(625U, UnsignedResult(0x346DC5D63886594BULL, 7U, false));
}
// NOLINTEND(readability-magic-numbers)
}  // namespace ark::compiler
