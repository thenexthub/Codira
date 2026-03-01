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
#include "libarkbase/utils/regmask.h"

#include <gtest/gtest.h>
#include <bitset>

namespace ark::test {

using BitsetType = std::bitset<RegMask::Size()>;

void CompareWithBitset(RegMask mask, BitsetType base)
{
    ASSERT_EQ(mask.Count(), base.count());
    if (base.any()) {
        ASSERT_EQ(mask.GetMinRegister(), Ctz(base.to_ulong()));
        ASSERT_EQ(mask.GetMaxRegister(), base.size() - Clz(static_cast<RegMask::ValueType>(base.to_ulong())) - 1L);
    }
    ASSERT_EQ(mask.Size(), base.size());
    ASSERT_EQ(mask.Any(), base.any());
    ASSERT_EQ(mask.None(), base.none());
    for (size_t i = 0; i < base.size(); i++) {
        ASSERT_EQ(mask.Test(i), base.test(i));
        ASSERT_EQ(mask[i], base[i]);
    }
}

void TestRegMask(RegMask::ValueType value)
{
    RegMask mask(value);
    BitsetType base(value);
    CompareWithBitset(mask, base);
    for (size_t i = 0; i < base.size(); i++) {
        mask.Set(i);
        base.set(i);
        CompareWithBitset(mask, base);
        mask.Reset(i);
        base.reset(i);
        CompareWithBitset(mask, base);
    }
    mask.Set();
    base.set();
    CompareWithBitset(mask, base);
    mask.Reset();
    base.reset();
    CompareWithBitset(mask, base);
}

void TestDistance(RegMask mask, size_t bit, size_t bitsBefore, size_t bitsAfter)
{
    ASSERT_EQ(mask.GetDistanceFromTail(bit), bitsBefore);
    ASSERT_EQ(mask.GetDistanceFromHead(bit), bitsAfter);
}

TEST(RegMask, Base)
{
    // NOLINTBEGIN(readability-magic-numbers)
    TestRegMask(MakeMask(0U, 3U, 2U, 17U, 25U, 31U));
    TestRegMask(MakeMask(1U, 4U, 8U, 3U, 24U, 28U, 30U));
    TestRegMask(MakeMaskByExcluding(32U, 0U));
    TestRegMask(MakeMaskByExcluding(32U, 31U));
    TestRegMask(MakeMaskByExcluding(32U, 0U, 31U));
    TestRegMask(MakeMaskByExcluding(32U, 0U, 15U, 31U));
    TestRegMask(0U);
    TestRegMask(~0U);

    RegMask mask(MakeMask(0U, 2U, 3U, 17U, 25U, 31U));
    TestDistance(mask, 0U, 0U, 5U);
    TestDistance(mask, 1U, 1U, 5U);
    TestDistance(mask, 2U, 1U, 4U);
    TestDistance(mask, 3U, 2U, 3U);
    TestDistance(mask, 4U, 3U, 3U);
    TestDistance(mask, 17U, 3U, 2U);
    TestDistance(mask, 18U, 4U, 2U);
    TestDistance(mask, 31U, 5U, 0U);
    // NOLINTEND(readability-magic-numbers)
}

}  // namespace ark::test
