/**
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

#include "util/addr_map.h"
#include "util/range.h"

#include "util/tests/verifier_test.h"

#include <gtest/gtest.h>

namespace ark::verifier::test {

// NOLINTBEGIN(readability-magic-numbers)

TEST_F(VerifierTest, AddrMap)
{
    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    char mem[123U] = {};
    AddrMap amap1 {&mem[0], &mem[122U]};
    AddrMap amap2 {&mem[0], &mem[122U]};
    amap1.Mark(&mem[50U], &mem[60U]);
    EXPECT_TRUE(amap1.HasMark(&mem[50U]));
    EXPECT_TRUE(amap1.HasMark(&mem[60U]));
    EXPECT_FALSE(amap1.HasMark(&mem[49U]));
    EXPECT_FALSE(amap1.HasMark(&mem[61U]));
    amap2.Mark(&mem[70U], &mem[90U]);
    EXPECT_FALSE(amap1.HasCommonMarks(amap2));
    amap2.Mark(&mem[60U]);
    char *ptr;
    EXPECT_TRUE(amap1.GetFirstCommonMark(amap2, &ptr));
    EXPECT_EQ(ptr, &mem[60U]);

    PandaVector<const char *> ptrs;
    amap1.Clear();
    amap1.Mark(&mem[48U]);
    amap1.Mark(&mem[61U]);
    amap1.Mark(&mem[50U]);
    amap1.Mark(&mem[60U]);
    amap1.EnumerateMarksInScope<const char *>(&mem[49U], &mem[60U], [&ptrs](const char *addr) {
        ptrs.push_back(addr);
        return true;
    });

    EXPECT_EQ(ptrs.size(), 1);
    EXPECT_EQ(ptrs[0], reinterpret_cast<const char *>(&mem[50U]));
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::verifier::test
