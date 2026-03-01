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
#include "utils/regmask.h"

#include <gtest/gtest.h>
#include <bitset>

namespace panda::test {

using BitsetType = std::bitset<RegMask::Size()>;

void CompareWithBitset(RegMask mask, BitsetType base)
{
    ASSERT_EQ(mask.Count(), base.count());
    if (base.any()) {
        ASSERT_EQ(mask.GetMinRegister(), static_cast<uint32_t>(Ctz(base.to_ulong())));
        ASSERT_EQ(mask.GetMaxRegister(), base.size() - Clz(static_cast<RegMask::ValueType>(base.to_ulong())) - 1);
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
        mask.set(i);
        base.set(i);
        CompareWithBitset(mask, base);
        mask.reset(i);
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

void TestDistance(RegMask mask, size_t bit, size_t bits_before, size_t bits_after)
{
    ASSERT_EQ(mask.GetDistanceFromTail(bit), bits_before);
    ASSERT_EQ(mask.GetDistanceFromHead(bit), bits_after);
}

HWTEST(RegMask, Base, testing::ext::TestSize.Level0)
{
    TestRegMask(MakeMask(0, 3, 2, 17, 25, 31));
    TestRegMask(MakeMask(1, 4, 8, 3, 24, 28, 30));
    TestRegMask(MakeMaskByExcluding(32, 0));
    TestRegMask(MakeMaskByExcluding(32, 31));
    TestRegMask(MakeMaskByExcluding(32, 0, 31));
    TestRegMask(MakeMaskByExcluding(32, 0, 15, 31));
    TestRegMask(0U);
    TestRegMask(~0U);

    RegMask mask(MakeMask(0, 2, 3, 17, 25, 31));
    TestDistance(mask, 0, 0, 5);
    TestDistance(mask, 1, 1, 5);
    TestDistance(mask, 2, 1, 4);
    TestDistance(mask, 3, 2, 3);
    TestDistance(mask, 4, 3, 3);
    TestDistance(mask, 17, 3, 2);
    TestDistance(mask, 18, 4, 2);
    TestDistance(mask, 31, 5, 0);
}

HWTEST(RegMask, SetAndDumpTest, testing::ext::TestSize.Level0)
{
    RegMask mask(MakeMask(1, 2, 3));
    ASSERT_EQ(mask.GetValue(), 14U);

    mask.Set(1U, false);
    ASSERT_EQ(mask.GetValue(), 12U);

    mask.Set(5U, true);
    ASSERT_EQ(mask.GetValue(), 44U);
}

}  // namespace panda::test