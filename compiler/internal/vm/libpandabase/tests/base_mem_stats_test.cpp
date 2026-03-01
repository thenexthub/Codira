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

#include "mem/code_allocator.h"
#include "mem/pool_manager.h"
#include "mem/base_mem_stats.h"

#include "gtest/gtest.h"

namespace panda {

class BaseMemStatsTest : public testing::Test {
protected:
    void SetUp() override
    {
        panda::mem::MemConfig::Initialize(128_MB, 64_MB, 64_MB, 32_MB);
        PoolManager::Initialize();
    }

    void TearDown() override
    {
        PoolManager::Finalize();
        panda::mem::MemConfig::Finalize();
    }
};

HWTEST_F(BaseMemStatsTest, CodeStatistic, testing::ext::TestSize.Level0)
{
    static constexpr size_t N = 100;
    BaseMemStats stats;
    size_t sum = 0;
    for (size_t i = 1; i < N; i++) {
        sum += i;
        stats.RecordAllocateRaw(i, SpaceType::SPACE_TYPE_CODE);
    }
    stats.RecordFreeRaw(N, SpaceType::SPACE_TYPE_CODE);
    ASSERT_EQ(sum, stats.GetAllocated(SpaceType::SPACE_TYPE_CODE));
    ASSERT_EQ(N, stats.GetFreed(SpaceType::SPACE_TYPE_CODE));
    ASSERT_EQ(sum - N, stats.GetFootprint(SpaceType::SPACE_TYPE_CODE));
}

HWTEST_F(BaseMemStatsTest, AllocationsOverAllocator, testing::ext::TestSize.Level0)
{
    BaseMemStats stats;
    void *tmp;
    CodeAllocator ca(&stats);

    uint8_t buff1[] = {0xCC};
    uint8_t buff2[] = {0xCC, 0xCC, 0xCC};
    size_t size1 = sizeof(buff1);
    size_t size2 = sizeof(buff2);

    tmp = ca.AllocateCode(size1, static_cast<void *>(&buff1[0]));
    tmp = ca.AllocateCode(sizeof(buff2), static_cast<void *>(&buff2[0]));

    ASSERT_EQ(size1 + size2, stats.GetAllocated(SpaceType::SPACE_TYPE_CODE));
    ASSERT_EQ(0U, stats.GetFreed(SpaceType::SPACE_TYPE_CODE));
    ASSERT_EQ(size1 + size2, stats.GetFootprint(SpaceType::SPACE_TYPE_CODE));

    stats.RecordFreeRaw(size2, SpaceType::SPACE_TYPE_CODE);

    ASSERT_EQ(sizeof(buff1) + sizeof(buff2), stats.GetAllocated(SpaceType::SPACE_TYPE_CODE));
    ASSERT_EQ(sizeof(buff2), stats.GetFreed(SpaceType::SPACE_TYPE_CODE));
    ASSERT_EQ(sizeof(buff1), stats.GetFootprint(SpaceType::SPACE_TYPE_CODE));
}

HWTEST_F(BaseMemStatsTest, HeapAllocateTest, testing::ext::TestSize.Level0)
{
    BaseMemStats stats;
    stats.RecordAllocateRaw(1U, SpaceType::SPACE_TYPE_CODE);
    ASSERT_EQ(stats.GetAllocated(SpaceType::SPACE_TYPE_CODE), 1U);

    stats.RecordAllocateRaw(2U, SpaceType::SPACE_TYPE_INTERNAL);
    ASSERT_EQ(stats.GetAllocated(SpaceType::SPACE_TYPE_INTERNAL), 2U);

    stats.RecordFreeRaw(4U, SpaceType::SPACE_TYPE_CODE);
    ASSERT_EQ(stats.GetFreed(SpaceType::SPACE_TYPE_CODE), 4U);

    stats.RecordFreeRaw(5U, SpaceType::SPACE_TYPE_INTERNAL);
    ASSERT_EQ(stats.GetFreed(SpaceType::SPACE_TYPE_INTERNAL), 5U);

    ASSERT_EQ(0U, stats.GetAllocatedHeap());
    ASSERT_EQ(0U, stats.GetFreedHeap());
}

}  // namespace panda
