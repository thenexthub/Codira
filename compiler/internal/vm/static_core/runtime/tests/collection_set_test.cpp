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

#include <iterator>
#include <gtest/gtest.h>

#include "libarkbase/mem/mem.h"
#include "runtime/mem/gc/g1/collection_set.h"
#include "runtime/mem/internal_allocator-inl.h"
#include "runtime/mem/mem_stats_default.h"
#include "runtime/mem/mem_stats_additional_info.h"

namespace ark::mem {

class CollectionSetTest : public testing::Test {
public:
    CollectionSetTest()
    {
        static constexpr size_t MEMORY_POOL_SIZE = 16_MB;
        MemConfig::Initialize(0, MEMORY_POOL_SIZE, 0, 0, 0, 0);
        PoolManager::Initialize();
        memStats_ = new mem::MemStatsType();
        allocator_ = new InternalAllocatorT<mem::InternalAllocatorConfig::PANDA_ALLOCATORS>(memStats_);
        // mem::InternalAllocatorPtr allocator_ptr(allocator_);
        mem::InternalAllocator<>::InitInternalAllocatorFromRuntime(static_cast<Allocator *>(allocator_));
    }

    ~CollectionSetTest() override
    {
        delete allocator_;
        PoolManager::Finalize();
        ark::mem::MemConfig::Finalize();
        delete memStats_;
        InternalAllocator<>::ClearInternalAllocatorFromRuntime();
    }

    NO_COPY_SEMANTIC(CollectionSetTest);
    NO_MOVE_SEMANTIC(CollectionSetTest);

private:
    MemStatsType *memStats_;
    InternalAllocatorT<mem::InternalAllocatorConfig::PANDA_ALLOCATORS> *allocator_;
};

// NOLINTBEGIN(readability-magic-numbers)

TEST_F(CollectionSetTest, TestCtor)
{
    Region youngRegion(nullptr, 0x0, 0x1000);
    youngRegion.AddFlag(RegionFlag::IS_EDEN);
    PandaVector<Region *> youngRegions = {&youngRegion};

    CollectionSet cs(std::move(youngRegions));

    ASSERT_EQ(1U, cs.size());
    auto young = cs.Young();
    auto tenured = cs.Tenured();
    auto humongous = cs.Humongous();
    ASSERT_EQ(1U, std::distance(young.begin(), young.end()));
    ASSERT_EQ(&youngRegion, *young.begin());
    ASSERT_EQ(0U, std::distance(tenured.begin(), tenured.end()));
    ASSERT_EQ(0U, std::distance(humongous.begin(), humongous.end()));
}

TEST_F(CollectionSetTest, TestAddTenuredRegion)
{
    Region youngRegion(nullptr, 0x0, 0x1000);
    youngRegion.AddFlag(RegionFlag::IS_EDEN);
    PandaVector<Region *> youngRegions = {&youngRegion};
    Region tenuredRegion(nullptr, 0x1000, 0x2000);
    tenuredRegion.AddFlag(RegionFlag::IS_OLD);

    CollectionSet cs(std::move(youngRegions));
    cs.AddRegion(&tenuredRegion);

    ASSERT_EQ(2U, cs.size());
    auto young = cs.Young();
    auto tenured = cs.Tenured();
    auto humongous = cs.Humongous();
    ASSERT_EQ(1U, std::distance(young.begin(), young.end()));
    ASSERT_EQ(&youngRegion, *young.begin());
    ASSERT_EQ(1U, std::distance(tenured.begin(), tenured.end()));
    ASSERT_EQ(&tenuredRegion, *tenured.begin());
    ASSERT_EQ(0U, std::distance(humongous.begin(), humongous.end()));
}

TEST_F(CollectionSetTest, TestAddHumongousRegion)
{
    Region youngRegion(nullptr, 0x0, 0x1000);
    youngRegion.AddFlag(RegionFlag::IS_EDEN);
    PandaVector<Region *> youngRegions = {&youngRegion};
    Region humongousRegion(nullptr, 0x1000, 0x2000);
    humongousRegion.AddFlag(RegionFlag::IS_OLD);
    humongousRegion.AddFlag(RegionFlag::IS_LARGE_OBJECT);

    CollectionSet cs(std::move(youngRegions));
    cs.AddRegion(&humongousRegion);

    ASSERT_EQ(2U, cs.size());
    auto young = cs.Young();
    auto tenured = cs.Tenured();
    auto humongous = cs.Humongous();
    ASSERT_EQ(1U, std::distance(young.begin(), young.end()));
    ASSERT_EQ(&youngRegion, *young.begin());
    ASSERT_EQ(0U, std::distance(tenured.begin(), tenured.end()));
    ASSERT_EQ(1U, std::distance(humongous.begin(), humongous.end()));
    ASSERT_EQ(&humongousRegion, *humongous.begin());
}

TEST_F(CollectionSetTest, TestAddDifferentRegions)
{
    Region youngRegion(nullptr, 0x0, 0x1000);
    youngRegion.AddFlag(RegionFlag::IS_EDEN);
    PandaVector<Region *> youngRegions = {&youngRegion};
    Region tenured1Region(nullptr, 0x1000, 0x2000);
    tenured1Region.AddFlag(RegionFlag::IS_OLD);
    Region tenured2Region(nullptr, 0x1000, 0x2000);
    tenured2Region.AddFlag(RegionFlag::IS_OLD);
    Region humongous1Region(nullptr, 0x2000, 0x3000);
    humongous1Region.AddFlag(RegionFlag::IS_OLD);
    humongous1Region.AddFlag(RegionFlag::IS_LARGE_OBJECT);
    Region humongous2Region(nullptr, 0x2000, 0x3000);
    humongous2Region.AddFlag(RegionFlag::IS_OLD);
    humongous2Region.AddFlag(RegionFlag::IS_LARGE_OBJECT);

    CollectionSet cs(std::move(youngRegions));
    cs.AddRegion(&humongous1Region);
    cs.AddRegion(&tenured1Region);
    cs.AddRegion(&humongous2Region);
    cs.AddRegion(&tenured2Region);

    ASSERT_EQ(5U, cs.size());
    auto young = cs.Young();
    auto tenured = cs.Tenured();
    auto humongous = cs.Humongous();
    ASSERT_EQ(1U, std::distance(young.begin(), young.end()));
    ASSERT_EQ(&youngRegion, *young.begin());
    ASSERT_EQ(2U, std::distance(tenured.begin(), tenured.end()));
    ASSERT_EQ(2U, std::distance(humongous.begin(), humongous.end()));
    ASSERT_EQ(5U, std::distance(cs.begin(), cs.end()));
    auto it = cs.begin();
    // one young region
    ASSERT_TRUE((*it)->HasFlag(RegionFlag::IS_EDEN));
    // two tenured regions
    ++it;
    ASSERT_TRUE((*it)->HasFlag(RegionFlag::IS_OLD));
    ASSERT_FALSE((*it)->HasFlag(RegionFlag::IS_LARGE_OBJECT));
    ++it;
    ASSERT_TRUE((*it)->HasFlag(RegionFlag::IS_OLD));
    ASSERT_FALSE((*it)->HasFlag(RegionFlag::IS_LARGE_OBJECT));
    // two humongous regions
    ++it;
    ASSERT_TRUE((*it)->HasFlag(RegionFlag::IS_OLD));
    ASSERT_TRUE((*it)->HasFlag(RegionFlag::IS_LARGE_OBJECT));
    ++it;
    ASSERT_TRUE((*it)->HasFlag(RegionFlag::IS_OLD));
    ASSERT_TRUE((*it)->HasFlag(RegionFlag::IS_LARGE_OBJECT));
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::mem
