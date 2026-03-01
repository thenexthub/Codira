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

#include "libarkbase/utils/arch.h"
#include "gtest/gtest.h"
#include "libarkbase/utils/utils.h"
#include <array>

extern "C" int IrtocTestAddValues(int64_t, int64_t);
extern "C" int IrtocTestIncMaxValue(uint64_t, uint64_t);
extern "C" int IrtocTestIncMaxValueLabels(uint64_t, uint64_t);
extern "C" unsigned IrtocTestSeqLabels(uint64_t);
extern "C" uint64_t IrtocTestCfg(void *, uint64_t);
extern "C" uint64_t IrtocTestCfgLabels(void *, uint64_t);
extern "C" size_t IrtocTestLabels(size_t);
extern "C" size_t IrtocTestReturnBeforeLabel(size_t);

using TestCfgFunc = uint64_t (*)(void *, uint64_t);

namespace ark::test {

TEST(Irtoc, AddValues)
{
    ASSERT_EQ(IrtocTestAddValues(1L, 2L), 3_I);
    ASSERT_EQ(IrtocTestAddValues(-1L, -2L), -3_I);
}

TEST(Irtoc, IncMaxValue)
{
    ASSERT_EQ(IrtocTestIncMaxValue(10U, 9U), 11_I);
    ASSERT_EQ(IrtocTestIncMaxValue(4U, 8U), 9_I);
    ASSERT_EQ(IrtocTestIncMaxValue(4U, 4U), 5_I);
}

TEST(Irtoc, IncMaxValueLabels)
{
    ASSERT_EQ(IrtocTestIncMaxValueLabels(10U, 9U), 11_I);
    ASSERT_EQ(IrtocTestIncMaxValueLabels(4U, 8U), 9_I);
    ASSERT_EQ(IrtocTestIncMaxValueLabels(4U, 4U), 5_I);
}

template <size_t N>
uint64_t ModifyArrayForLoopTest(std::array<uint64_t, N> *data)
{
    uint64_t res = 0;
    for (size_t i = 0; i < data->size(); i++) {
        if ((i % 2U) == 0) {
            if (((*data)[i] % 2U) == 0) {
                (*data)[i] += 2U;
                res += 2U;
            } else {
                (*data)[i] += 1;
                res += 1;
            }
        } else {
            (*data)[i] -= 1;
            res -= 1;
        }
    }
    return res;
}

void TestLoop(TestCfgFunc func)
{
    std::array<uint64_t, 8U> buf {};
    for (size_t i = 0; i < buf.size(); i++) {
        buf[i] = i + i % 3U;
    }
    std::array<uint64_t, 8U> bufExpected = buf;
    auto expected = ModifyArrayForLoopTest(&bufExpected);
    uint64_t res = func(static_cast<void *>(buf.data()), buf.size());
    ASSERT_EQ(expected, res);
    ASSERT_EQ(bufExpected, buf);
}

TEST(Irtoc, Loop)
{
    if constexpr (RUNTIME_ARCH == Arch::AARCH32) {
        GTEST_SKIP();
    }
    TestLoop(IrtocTestCfg);
    TestLoop(IrtocTestCfgLabels);
}

TEST(Irtoc, SeqLabels)
{
    if constexpr (RUNTIME_ARCH == Arch::AARCH32) {
        GTEST_SKIP();
    }
    EXPECT_EQ(IrtocTestSeqLabels(0U), 1U);
    EXPECT_EQ(IrtocTestSeqLabels(5U), 6U);
    EXPECT_EQ(IrtocTestSeqLabels(10U), 11U);
    EXPECT_EQ(IrtocTestSeqLabels(11U), 13U);
    EXPECT_EQ(IrtocTestSeqLabels(55U), 57U);
    EXPECT_EQ(IrtocTestSeqLabels(100U), 102U);
    EXPECT_EQ(IrtocTestSeqLabels(101U), 104U);
    EXPECT_EQ(IrtocTestSeqLabels(1010U), 1013U);
    EXPECT_EQ(IrtocTestSeqLabels(54545U), 54548U);
}

extern "C" int IrtocTestRelocations(int);
extern "C" int TestCall(int n)
{
    return n + 2U;
}

TEST(Irtoc, Relocations)
{
    ASSERT_EQ(IrtocTestRelocations(1_I), 5_I);
    ASSERT_EQ(IrtocTestRelocations(2_I), 10_I);
    ASSERT_EQ(IrtocTestRelocations(3_I), 17_I);
}

extern "C" size_t IncrementInt(size_t n)
{
    return n + 1;
}

extern "C" double IncrementFloat(double n)
{
    return n + 1;
}

// CC-OFFNXT(G.FUN.01) solid logic
extern "C" size_t IrtocTestRelocations2(size_t a0, size_t a1, double f0, size_t a2, size_t a3, size_t a4, double f1,
                                        double f2, size_t a5, size_t a6, double f3, size_t a7, size_t a8, size_t a9,
                                        double f4);

TEST(Irtoc, RelocationsParams)
{
    if constexpr (RUNTIME_ARCH == Arch::AARCH32) {
        GTEST_SKIP();
    }
    ASSERT_EQ(IrtocTestRelocations2(0U, 1U, 2.0F, 3U, 4U, 5U, 6.0F, 7.0F, 8U, 9U, 10.0F, 11U, 12U, 13U, 14.0F), 120U);
}

TEST(Irtoc, Labels)
{
    EXPECT_EQ(IrtocTestLabels(0U), 0U);
    EXPECT_EQ(IrtocTestLabels(1U), 1U);
    EXPECT_EQ(IrtocTestLabels(2U), 3U);
    EXPECT_EQ(IrtocTestLabels(3U), 6U);
    EXPECT_EQ(IrtocTestLabels(4U), 10U);
    EXPECT_EQ(IrtocTestLabels(5U), 15U);
    EXPECT_EQ(IrtocTestLabels(100U), 5050U);
}

TEST(Irtoc, ReturnBeforeLabel)
{
    EXPECT_EQ(IrtocTestReturnBeforeLabel(42U), 2U);
    EXPECT_EQ(IrtocTestReturnBeforeLabel(146U), 1U);
}

}  // namespace ark::test
