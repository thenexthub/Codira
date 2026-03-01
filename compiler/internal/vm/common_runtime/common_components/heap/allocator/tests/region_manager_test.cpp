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

#include "common_components/heap/allocator/region_manager.h"
#include "common_components/heap/collector/marking_collector.h"
#include "common_components/heap/heap.cpp"
#include "common_components/common_runtime/base_runtime_param.h"
#include "common_components/heap/heap_manager.h"
#include "common_components/tests/test_helper.h"
#include <cstdint>

using namespace common;

namespace common::test {
const size_t SIZE_SIXTEEN = 16;
const size_t SIZE_MAX_TEST = 1024;
const size_t SIZE_HALF_MAX_TEST = 512;
const uint32_t NUM_TEN = 10;
const uint32_t NUM_TWO = 2;
const uint32_t NUM_THREE = 3;
const uint32_t NUM_FIVE = 5;

class RegionManagerTest : public common::test::BaseTestWithScope {
protected:
    void* regionMemory_;
    size_t totalUnits_ = SIZE_MAX_TEST;
    size_t heapSize_;
    Mutator* mutator_ = nullptr;

    static void SetUpTestCase()
    {
        BaseRuntime::GetInstance()->InitFromDynamic();
    }

    static void TearDownTestCase()
    {
        BaseRuntime::GetInstance()->FiniFromDynamic();
    }

    void SetUp() override
    {
        heapSize_ = totalUnits_ * RegionDesc::UNIT_SIZE;
        size_t allocSize = heapSize_ + totalUnits_ * sizeof(RegionDesc);
        regionMemory_ = malloc(allocSize);
        ASSERT_NE(regionMemory_, nullptr);
        uintptr_t unitInfoStart = reinterpret_cast<uintptr_t>(regionMemory_);
        size_t metadataSize = totalUnits_ * sizeof(RegionDesc);
        uintptr_t heapStartAddress = unitInfoStart + metadataSize;
        RegionDesc::Initialize(totalUnits_, unitInfoStart, heapStartAddress);
        mutator_ = Mutator::NewMutator();
        ASSERT_NE(mutator_, nullptr);
        mutator_->InitTid();
        ThreadLocal::SetMutator(mutator_);
    }

    void TearDown() override
    {
        if (mutator_) {
            delete mutator_;
            mutator_ = nullptr;
            ThreadLocal::SetMutator(mutator_);
        }
        if (regionMemory_) {
            free(regionMemory_);
            regionMemory_ = nullptr;
        }
    }
};

HWTEST_F_L0(RegionManagerTest, VisitLiveObjectsUntilFalse_LiveByteCountZero)
{
    size_t unitIdx = 0;
    size_t nUnit = 1;
    RegionDesc* region = RegionDesc::InitRegion(unitIdx, nUnit, RegionDesc::UnitRole::SMALL_SIZED_UNITS);
    region->AddLiveByteCount(0);

    bool callbackCalled = false;
    bool result = region->VisitLiveObjectsUntilFalse([&callbackCalled](BaseObject* obj) {
        callbackCalled = true;
        return true;
    });
    EXPECT_TRUE(result);
    EXPECT_FALSE(callbackCalled);
}

HWTEST_F_L0(RegionManagerTest, VisitLiveObjectsUntilFalse_IsSmallRegion)
{
    size_t unitIdx = 0;
    size_t nUnit = 1;
    RegionDesc* region = RegionDesc::InitRegion(unitIdx, nUnit, RegionDesc::UnitRole::SMALL_SIZED_UNITS);
    ASSERT_NE(region, nullptr);
    region->AddLiveByteCount(SIZE_SIXTEEN);

    bool callbackCalled = false;
    bool result = region->VisitLiveObjectsUntilFalse(
        [&callbackCalled](BaseObject* obj) {
            callbackCalled = true;
            return true;
        });
    EXPECT_FALSE(callbackCalled);
    EXPECT_TRUE(result);
}

HWTEST_F_L0(RegionManagerTest, VisitLiveObjectsUntilFalse_IsLargeRegion)
{
    size_t unitIdx = 0;
    size_t nUnit = 4;
    RegionDesc* region = RegionDesc::InitRegion(unitIdx, nUnit, RegionDesc::UnitRole::LARGE_SIZED_UNITS);
    ASSERT_NE(region, nullptr);
    region->AddLiveByteCount(SIZE_SIXTEEN);
    bool callbackCalled = false;

    bool result = region->VisitLiveObjectsUntilFalse(
        [&callbackCalled](BaseObject* obj) {
            callbackCalled = true;
            return true;
        });
    EXPECT_TRUE(callbackCalled);
    EXPECT_TRUE(result);
}

HWTEST_F_L0(RegionManagerTest, VisitLiveObjectsUntilFalse)
{
    size_t unitIdx = 0;
    size_t nUnit = 1;
    RegionDesc* region = RegionDesc::InitRegion(unitIdx, nUnit, RegionDesc::UnitRole::SMALL_SIZED_UNITS);
    ASSERT_NE(region, nullptr);
    region->AddLiveByteCount(SIZE_SIXTEEN);
    bool callbackCalled = false;

    bool result = region->VisitLiveObjectsUntilFalse(
        [&callbackCalled](BaseObject* obj) {
            callbackCalled = true;
            return true;
        });
    EXPECT_FALSE(callbackCalled);
    EXPECT_TRUE(result);
}
HWTEST_F_L0(RegionManagerTest, VisitAllObjectsBeforeFix1)
{
    size_t unitIdx = 0;
    size_t nUnit = 4;
    RegionDesc* region = RegionDesc::InitRegion(unitIdx, nUnit, RegionDesc::UnitRole::LARGE_SIZED_UNITS);
    ASSERT_NE(region, nullptr);

    uintptr_t start = region->GetRegionStart();
    region->SetRegionAllocPtr(start + SIZE_SIXTEEN);
    bool callbackCalled = false;
    region->VisitAllObjectsBeforeCopy([&](BaseObject* obj) {
        callbackCalled = true;
        EXPECT_EQ(obj, reinterpret_cast<BaseObject*>(region->GetRegionStart()));
    });
    EXPECT_TRUE(callbackCalled);
}

HWTEST_F_L0(RegionManagerTest, VisitAllObjectsBeforeFix2)
{
    size_t unitIdx = 0;
    size_t nUnit = 1;
    RegionDesc* region = RegionDesc::InitRegion(unitIdx, nUnit, RegionDesc::UnitRole::SMALL_SIZED_UNITS);
    RegionDesc::InitFreeRegion(unitIdx, nUnit);
    ASSERT_NE(region, nullptr);

    uintptr_t start = region->GetRegionStart();
    region->SetRegionAllocPtr(start + SIZE_SIXTEEN);
    bool callbackCalled = false;
    region->VisitAllObjectsBeforeCopy([&](BaseObject* obj) {
        callbackCalled = true;
        EXPECT_EQ(obj, reinterpret_cast<BaseObject*>(region->GetRegionStart()));
    });
    EXPECT_FALSE(callbackCalled);
}

HWTEST_F_L0(RegionManagerTest, VisitAllObjectsBeforeFix3)
{
    size_t unitIdx = 0;
    size_t nUnit = 4;

    RegionDesc* region = RegionDesc::InitRegion(unitIdx, nUnit, RegionDesc::UnitRole::LARGE_SIZED_UNITS);
    ASSERT_NE(region, nullptr);

    bool callbackCalled = false;
    region->VisitAllObjectsBeforeCopy([&](BaseObject* obj) {
        callbackCalled = true;
        EXPECT_EQ(obj, reinterpret_cast<BaseObject*>(region->GetRegionStart()));
    });
    EXPECT_FALSE(callbackCalled);
}

HWTEST_F_L0(RegionManagerTest, VisitAllObjectsBeforeFix4)
{
    size_t unitIdx = 0;
    size_t nUnit = 1;

    RegionDesc* region = RegionDesc::InitRegion(unitIdx, nUnit, RegionDesc::UnitRole::SMALL_SIZED_UNITS);
    ASSERT_NE(region, nullptr);

    bool callbackCalled = false;
    region->VisitAllObjectsBeforeCopy([&](BaseObject* obj) {
        callbackCalled = true;
        EXPECT_EQ(obj, reinterpret_cast<BaseObject*>(region->GetRegionStart()));
    });
    EXPECT_FALSE(callbackCalled);
}

HWTEST_F_L0(RegionManagerTest, PrependRegionLockedTest)
{
    RegionList list("list");
    list.PrependRegionLocked(nullptr, RegionDesc::RegionType::FREE_REGION);
    EXPECT_EQ(list.GetHeadRegion(), nullptr);
}

HWTEST_F_L0(RegionManagerTest, ReleaseGarbageRegions)
{
    size_t targetCachedSize = 0;
    RegionManager manager;
    FreeRegionManager freeRegionManager(manager);
    freeRegionManager.Initialize(NUM_TEN);
    freeRegionManager.AddGarbageUnits(0, 1);
    freeRegionManager.AddGarbageUnits(NUM_TWO, NUM_TWO);
    freeRegionManager.AddGarbageUnits(NUM_FIVE, NUM_THREE);
    size_t released = freeRegionManager.ReleaseGarbageRegions(targetCachedSize);
    EXPECT_GT(released, 0);
}

HWTEST_F_L0(RegionManagerTest, ReclaimRegion1)
{
    size_t huge_page = (2048 * KB) / getpagesize();
    size_t nUnit = 1;
    size_t unitIdx = 0;
    RegionManager manager;
    manager.Initialize(SIZE_MAX_TEST, reinterpret_cast<uintptr_t>(regionMemory_));
    RegionDesc* region = RegionDesc::InitRegion(unitIdx, nUnit,
        RegionDesc::UnitRole::SMALL_SIZED_UNITS);
    ASSERT_NE(region, nullptr);
    EXPECT_FALSE(region->IsLargeRegion());
    manager.ReclaimRegion(region);
    EXPECT_GT(manager.GetDirtyUnitCount(), 0);
}

HWTEST_F_L0(RegionManagerTest, ReclaimRegion2)
{
    size_t huge_page = (2048 * KB) / getpagesize();
    size_t nUnit = huge_page;
    size_t unitIdx = 0;
    RegionManager manager;
    manager.Initialize(SIZE_MAX_TEST, reinterpret_cast<uintptr_t>(regionMemory_));
    RegionDesc* region = RegionDesc::InitRegion(unitIdx, nUnit,
        RegionDesc::UnitRole::LARGE_SIZED_UNITS);
    ASSERT_NE(region, nullptr);
    manager.ReclaimRegion(region);
    EXPECT_GT(manager.GetDirtyUnitCount(), 0);
}

HWTEST_F_L0(RegionManagerTest, ReleaseRegion)
{
    size_t huge_page = (2048 * KB) / getpagesize();
    size_t nUnit = 1;
    size_t unitIdx = 0;
    RegionManager manager;
    manager.Initialize(SIZE_MAX_TEST, reinterpret_cast<uintptr_t>(regionMemory_));
    RegionDesc* region = RegionDesc::InitRegion(unitIdx, nUnit,
        RegionDesc::UnitRole::LARGE_SIZED_UNITS);
    ASSERT_NE(region, nullptr);
    auto ret = manager.ReleaseRegion(region);
    EXPECT_EQ(ret, region->GetRegionSize());
}

HWTEST_F_L0(RegionManagerTest, TakeRegion1)
{
    ASSERT_NE(mutator_, nullptr);
    mutator_->SetMutatorPhase(GCPhase::GC_PHASE_ENUM);
    RegionManager manager;
    manager.Initialize(SIZE_MAX_TEST, reinterpret_cast<uintptr_t>(regionMemory_));
    size_t nUnit = 4;
    RegionDesc* garbageRegion = RegionDesc::InitRegion(SIZE_HALF_MAX_TEST, nUnit,
        RegionDesc::UnitRole::LARGE_SIZED_UNITS);
    auto size = manager.CollectRegion(garbageRegion);
    RegionDesc* region = manager.TakeRegion(1, RegionDesc::UnitRole::SMALL_SIZED_UNITS, false, false);
    EXPECT_GT(manager.GetDirtyUnitCount(), 0);
}

HWTEST_F_L0(RegionManagerTest, TakeRegion2)
{
    ASSERT_NE(mutator_, nullptr);
    mutator_->SetMutatorPhase(GCPhase::GC_PHASE_ENUM);
    RegionManager manager;
    size_t nUnit = 1;
    manager.Initialize(SIZE_MAX_TEST, reinterpret_cast<uintptr_t>(regionMemory_));
    RegionDesc* garbageRegion = RegionDesc::InitRegion(SIZE_HALF_MAX_TEST, nUnit,
        RegionDesc::UnitRole::LARGE_SIZED_UNITS);
    auto size = manager.CollectRegion(garbageRegion);
    RegionDesc* region = manager.TakeRegion(16, RegionDesc::UnitRole::LARGE_SIZED_UNITS, true, false);
    EXPECT_NE(region, nullptr);
}

HWTEST_F_L0(RegionManagerTest, VisitRememberSetTest)
{
    size_t totalUnits = 1024;
    size_t heapSize = totalUnits * RegionDesc::UNIT_SIZE;

    void* regionMemory = malloc(heapSize + 4096);
    ASSERT_NE(regionMemory, nullptr);

    uintptr_t heapStartAddress = reinterpret_cast<uintptr_t>(regionMemory);
    uintptr_t regionInfoAddr = heapStartAddress + 4096;

    RegionManager manager;
    manager.Initialize(totalUnits, regionInfoAddr);

    size_t unitIdx = 0;
    size_t nUnit = 4;
    RegionDesc* region = RegionDesc::InitRegion(unitIdx, nUnit, RegionDesc::UnitRole::LARGE_SIZED_UNITS);
    ASSERT_NE(region, nullptr);

    int callbackCount = 0;
    region->VisitRememberSet([&](BaseObject* obj) {
        callbackCount++;
    });

    EXPECT_GE(callbackCount, 0);
    free(regionMemory);
}

HWTEST_F_L0(RegionManagerTest, VisitRememberSetBeforeCopyTest)
{
    size_t totalUnits = 1024;
    size_t heapSize = totalUnits * RegionDesc::UNIT_SIZE;

    void* regionMemory = malloc(heapSize + 4096);
    ASSERT_NE(regionMemory, nullptr);

    uintptr_t heapStartAddress = reinterpret_cast<uintptr_t>(regionMemory);
    uintptr_t regionInfoAddr = heapStartAddress + 4096;

    RegionManager manager;
    manager.Initialize(totalUnits, regionInfoAddr);

    size_t unitIdx = 0;
    size_t nUnit = 4;
    RegionDesc* region = RegionDesc::InitRegion(unitIdx, nUnit, RegionDesc::UnitRole::LARGE_SIZED_UNITS);
    ASSERT_NE(region, nullptr);

    int callbackCount = 0;
    region->VisitRememberSetBeforeCopy([&](BaseObject* obj) {
        callbackCount++;
    });

    EXPECT_GE(callbackCount, 0);
    free(regionMemory);
}

HWTEST_F_L0(RegionManagerTest, VisitRememberSetBeforeMarkingTest)
{
    size_t totalUnits = 1024;
    size_t heapSize = totalUnits * RegionDesc::UNIT_SIZE;

    void* regionMemory = malloc(heapSize + 4096);
    ASSERT_NE(regionMemory, nullptr);

    uintptr_t heapStartAddress = reinterpret_cast<uintptr_t>(regionMemory);
    uintptr_t regionInfoAddr = heapStartAddress + 4096;

    RegionManager manager;
    manager.Initialize(totalUnits, regionInfoAddr);

    size_t unitIdx = 0;
    size_t nUnit = 4;
    RegionDesc* region = RegionDesc::InitRegion(unitIdx, nUnit, RegionDesc::UnitRole::LARGE_SIZED_UNITS);
    ASSERT_NE(region, nullptr);

    int callbackCount = 0;
    region->VisitRememberSetBeforeMarking([&](BaseObject* obj) {
        callbackCount++;
    });

    EXPECT_GE(callbackCount, 0);
    free(regionMemory);
}

}
