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

#include "common_components/common/type_def.h"
#include "common_components/heap/allocator/allocator.h"
#include "common_components/heap/allocator/region_desc.h"
#include "common_components/heap/allocator/region_manager.h"
#include "common_components/heap/allocator/regional_heap.h"
#include "common_components/heap/allocator/regional_heap.cpp"
#include "common_components/heap/collector/collector_resources.h"
#include "common_components/heap/space/nonmovable_space.h"
#include "common_components/heap/space/readonly_space.h"
#include "common_components/tests/test_helper.h"
#include "common_interfaces/base_runtime.h"

using namespace common;

namespace common::test {
class RegionalHeapTest : public common::test::BaseTestWithScope {
protected:
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
        holder_ = ThreadHolder::CreateAndRegisterNewThreadHolder(nullptr);
        scope_ = new ThreadHolder::TryBindMutatorScope(holder_);
    }

    void TearDown() override
    {
        if (scope_ != nullptr) {
            delete scope_;
            scope_ = nullptr;
        }
    }

    ThreadHolder *holder_ {nullptr};
    ThreadHolder::TryBindMutatorScope *scope_ {nullptr};
};

HWTEST_F_L0(RegionalHeapTest, FeedHungryBuffers2)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_FIX);
    ThreadLocal::SetThreadType(ThreadType::ARK_PROCESSOR);
    AllocationBuffer* buffer1 = new (std::nothrow) AllocationBuffer();
    AllocationBuffer* buffer2 = new (std::nothrow) AllocationBuffer();
    RegionDesc* region = RegionDesc::InitRegion(0, 1, RegionDesc::UnitRole::LARGE_SIZED_UNITS);
    region->InitFreeUnits();
    buffer1->SetPreparedRegion(region);
    buffer2->SetPreparedRegion(region);

    Heap::GetHeap().GetAllocator().AddHungryBuffer(*buffer1);
    Heap::GetHeap().GetAllocator().AddHungryBuffer(*buffer2);
    Heap::GetHeap().GetAllocator().FeedHungryBuffers();
    EXPECT_NE(buffer2->GetPreparedRegion(), nullptr);
    delete buffer1;
    delete buffer2;
}

HWTEST_F_L0(RegionalHeapTest, FeedHungryBuffers3)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_FIX);
    Heap::GetHeap().GetAllocator().FeedHungryBuffers();
    AllocBufferManager::HungryBuffers hungryBuffers;
    Heap::GetHeap().GetAllocator().SwapHungryBuffers(hungryBuffers);
    EXPECT_EQ(hungryBuffers.size(), 0);
}

HWTEST_F_L0(RegionalHeapTest, AllocRegion_PhaseEnum)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_ENUM);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t addr = theAllocator.AllocOldRegion();
    ASSERT_NE(addr, 0);
    RegionDesc* region = RegionDesc::GetAliveRegionDescAt(addr);
    EXPECT_EQ(region->GetMarkingLine(), region->GetRegionStart());
    EXPECT_EQ(region->GetCopyLine(), std::numeric_limits<uintptr_t>::max());
}

HWTEST_F_L0(RegionalHeapTest, AllocRegion_PhaseMark)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_MARK);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t addr = theAllocator.AllocOldRegion();
    ASSERT_NE(addr, 0);
    RegionDesc* region = RegionDesc::GetAliveRegionDescAt(addr);
    EXPECT_EQ(region->GetMarkingLine(), region->GetRegionStart());
    EXPECT_EQ(region->GetCopyLine(), std::numeric_limits<uintptr_t>::max());
}

HWTEST_F_L0(RegionalHeapTest, AllocRegion_PhaseRemarkStab)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_REMARK_SATB);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t addr = theAllocator.AllocOldRegion();
    ASSERT_NE(addr, 0);
    RegionDesc* region = RegionDesc::GetAliveRegionDescAt(addr);
    EXPECT_EQ(region->GetMarkingLine(), region->GetRegionStart());
    EXPECT_EQ(region->GetCopyLine(), std::numeric_limits<uintptr_t>::max());
}

HWTEST_F_L0(RegionalHeapTest, AllocRegion_PhasePostMark)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_POST_MARK);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t addr = theAllocator.AllocOldRegion();
    ASSERT_NE(addr, 0);
    RegionDesc* region = RegionDesc::GetAliveRegionDescAt(addr);
    EXPECT_EQ(region->GetMarkingLine(), region->GetRegionStart());
    EXPECT_EQ(region->GetCopyLine(), std::numeric_limits<uintptr_t>::max());
}

HWTEST_F_L0(RegionalHeapTest, AllocRegion_PhasePrecopy)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_PRECOPY);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t addr = theAllocator.AllocOldRegion();
    ASSERT_NE(addr, 0);
    RegionDesc* region = RegionDesc::GetAliveRegionDescAt(addr);
    EXPECT_EQ(region->GetCopyLine(), region->GetRegionStart());
}

HWTEST_F_L0(RegionalHeapTest, AllocRegion_PhaseCopy)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_COPY);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t addr = theAllocator.AllocOldRegion();
    ASSERT_NE(addr, 0);
    RegionDesc* region = RegionDesc::GetAliveRegionDescAt(addr);
    EXPECT_EQ(region->GetCopyLine(), region->GetRegionStart());
}

HWTEST_F_L0(RegionalHeapTest, AllocRegion_PhaseFix)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_FIX);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t addr = theAllocator.AllocOldRegion();
    ASSERT_NE(addr, 0);
    RegionDesc* region = RegionDesc::GetAliveRegionDescAt(addr);
    EXPECT_EQ(region->GetCopyLine(), region->GetRegionStart());
}

HWTEST_F_L0(RegionalHeapTest, AllocRegion_PhaseUndef)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_UNDEF);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t addr = theAllocator.AllocOldRegion();
    ASSERT_NE(addr, 0);
    RegionDesc* region = RegionDesc::GetAliveRegionDescAt(addr);
    EXPECT_EQ(region->GetCopyLine(), std::numeric_limits<uintptr_t>::max());
}

HWTEST_F_L0(RegionalHeapTest, AllocateNonMovableRegion_PhaseEnum)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_ENUM);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t addr = theAllocator.AllocateNonMovableRegion();
    ASSERT_NE(addr, 0);
    RegionDesc* region = RegionDesc::GetAliveRegionDescAt(addr);
    EXPECT_EQ(region->GetMarkingLine(), region->GetRegionStart());
    EXPECT_EQ(region->GetCopyLine(), std::numeric_limits<uintptr_t>::max());
}

HWTEST_F_L0(RegionalHeapTest, AllocateNonMovableRegion_PhaseMark)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_MARK);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t addr = theAllocator.AllocateNonMovableRegion();
    ASSERT_NE(addr, 0);
    RegionDesc* region = RegionDesc::GetAliveRegionDescAt(addr);
    EXPECT_EQ(region->GetMarkingLine(), region->GetRegionStart());
    EXPECT_EQ(region->GetCopyLine(), std::numeric_limits<uintptr_t>::max());
}

HWTEST_F_L0(RegionalHeapTest, AllocateNonMovableRegion_PhaseRemarkStab)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_REMARK_SATB);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t addr = theAllocator.AllocateNonMovableRegion();
    ASSERT_NE(addr, 0);
    RegionDesc* region = RegionDesc::GetAliveRegionDescAt(addr);
    EXPECT_EQ(region->GetMarkingLine(), region->GetRegionStart());
    EXPECT_EQ(region->GetCopyLine(), std::numeric_limits<uintptr_t>::max());
}

HWTEST_F_L0(RegionalHeapTest, AllocateNonMovableRegion_PhasePostMark)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_POST_MARK);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t addr = theAllocator.AllocateNonMovableRegion();
    ASSERT_NE(addr, 0);
    RegionDesc* region = RegionDesc::GetAliveRegionDescAt(addr);
    EXPECT_EQ(region->GetMarkingLine(), region->GetRegionStart());
    EXPECT_EQ(region->GetCopyLine(), std::numeric_limits<uintptr_t>::max());
}

HWTEST_F_L0(RegionalHeapTest, AllocateNonMovableRegion_PhasePrecopy)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_PRECOPY);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t addr = theAllocator.AllocateNonMovableRegion();
    ASSERT_NE(addr, 0);
    RegionDesc* region = RegionDesc::GetAliveRegionDescAt(addr);
    EXPECT_EQ(region->GetCopyLine(), region->GetRegionStart());
}

HWTEST_F_L0(RegionalHeapTest, AllocateNonMovableRegion_PhaseCopy)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_COPY);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t addr = theAllocator.AllocateNonMovableRegion();
    ASSERT_NE(addr, 0);
    RegionDesc* region = RegionDesc::GetAliveRegionDescAt(addr);
    EXPECT_EQ(region->GetCopyLine(), region->GetRegionStart());
}

HWTEST_F_L0(RegionalHeapTest, AllocateNonMovableRegion_PhaseFix)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_FIX);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t addr = theAllocator.AllocateNonMovableRegion();
    ASSERT_NE(addr, 0);
    RegionDesc* region = RegionDesc::GetAliveRegionDescAt(addr);
    EXPECT_EQ(region->GetCopyLine(), region->GetRegionStart());
}

HWTEST_F_L0(RegionalHeapTest, AllocateNonMovableRegion_PhaseUndef)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_UNDEF);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t addr = theAllocator.AllocateNonMovableRegion();
    ASSERT_NE(addr, 0);
    RegionDesc* region = RegionDesc::GetAliveRegionDescAt(addr);
    EXPECT_EQ(region->GetCopyLine(), std::numeric_limits<uintptr_t>::max());
}

HWTEST_F_L0(RegionalHeapTest, AllocateThreadLocalRegion2)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_FIX);
    ThreadLocal::SetThreadType(ThreadType::ARK_PROCESSOR);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    RegionDesc* region = theAllocator.AllocateThreadLocalRegion<AllocBufferType::OLD>(false);
    EXPECT_EQ(region->GetCopyLine(), region->GetRegionStart());
}

HWTEST_F_L0(RegionalHeapTest, AllocateThreadLocalRegion3)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_COPY);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    ThreadLocal::SetThreadType(ThreadType::ARK_PROCESSOR);
    RegionDesc* region = theAllocator.AllocateThreadLocalRegion<AllocBufferType::OLD>(false);
    EXPECT_EQ(region->GetCopyLine(), region->GetRegionStart());
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_PRECOPY);
    RegionDesc* region1 = theAllocator.AllocateThreadLocalRegion<AllocBufferType::OLD>(false);
    EXPECT_EQ(region1->GetCopyLine(), region1->GetRegionStart());
}

HWTEST_F_L0(RegionalHeapTest, AllocateThreadLocalRegion4)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_ENUM);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    ThreadLocal::SetThreadType(ThreadType::ARK_PROCESSOR);
    RegionDesc* region = theAllocator.AllocateThreadLocalRegion<AllocBufferType::OLD>(false);
    EXPECT_EQ(region->GetMarkingLine(), region->GetRegionStart());
    EXPECT_EQ(region->GetCopyLine(), std::numeric_limits<uintptr_t>::max());

    mutator->SetMutatorPhase(GCPhase::GC_PHASE_MARK);
    RegionDesc* region2 = theAllocator.AllocateThreadLocalRegion<AllocBufferType::OLD>(false);
    EXPECT_EQ(region2->GetMarkingLine(), region2->GetRegionStart());
    EXPECT_EQ(region2->GetCopyLine(), std::numeric_limits<uintptr_t>::max());

    mutator->SetMutatorPhase(GCPhase::GC_PHASE_REMARK_SATB);
    RegionDesc* region3 = theAllocator.AllocateThreadLocalRegion<AllocBufferType::OLD>(false);
    EXPECT_EQ(region3->GetMarkingLine(), region3->GetRegionStart());
    EXPECT_EQ(region3->GetCopyLine(), std::numeric_limits<uintptr_t>::max());

    mutator->SetMutatorPhase(GCPhase::GC_PHASE_POST_MARK);
    RegionDesc* region4 = theAllocator.AllocateThreadLocalRegion<AllocBufferType::OLD>(false);
    EXPECT_EQ(region4->GetMarkingLine(), region4->GetRegionStart());
    EXPECT_EQ(region4->GetCopyLine(), std::numeric_limits<uintptr_t>::max());
}

HWTEST_F_L0(RegionalHeapTest, CopyRegion)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_ENUM);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t addr = theAllocator.AllocOldRegion();
    ASSERT_NE(addr, 0);
    RegionDesc* region = RegionDesc::GetRegionDescAt(addr);
    region->SetRegionType(RegionDesc::RegionType::FROM_REGION);
    ASSERT(region->IsFromRegion());
    theAllocator.CopyRegion(region);
    EXPECT_EQ(theAllocator.FromSpaceSize(), 0);
}

HWTEST_F_L0(RegionalHeapTest, AllocateThreadLocalRegion1_NotGcThread_EntersElseBranch)
{
    auto& heapAllocator = Heap::GetHeap().GetAllocator();
    RegionalHeap& regionalHeap = reinterpret_cast<RegionalHeap&>(heapAllocator);

    Mutator::GetMutator()->SetMutatorPhase(GCPhase::GC_PHASE_ENUM);

    auto* region = regionalHeap.AllocateThreadLocalRegion<AllocBufferType::OLD>(false);
    EXPECT_NE(region, nullptr);
}

HWTEST_F_L0(RegionalHeapTest, AllocateThreadLocalRegion2_NotGcThread_EntersElseBranch)
{
    auto& heapAllocator = Heap::GetHeap().GetAllocator();
    RegionalHeap& regionalHeap = reinterpret_cast<RegionalHeap&>(heapAllocator);

    Mutator::GetMutator()->SetMutatorPhase(GCPhase::GC_PHASE_PRECOPY);

    auto* region = regionalHeap.AllocateThreadLocalRegion<AllocBufferType::OLD>(false);
    EXPECT_NE(region, nullptr);
}

HWTEST_F_L0(RegionalHeapTest, AllocateThreadLocalRegion3_NotGcThread_EntersElseBranch)
{
    auto& heapAllocator = Heap::GetHeap().GetAllocator();
    RegionalHeap& regionalHeap = reinterpret_cast<RegionalHeap&>(heapAllocator);

    Mutator::GetMutator()->SetMutatorPhase(GCPhase::GC_PHASE_FIX);

    auto* region = regionalHeap.AllocateThreadLocalRegion<AllocBufferType::OLD>(false);
    EXPECT_NE(region, nullptr);
}

HWTEST_F_L0(RegionalHeapTest, Allocate_ValidSize_ReturnsNonNull)
{
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t addr = theAllocator.AllocOldRegion();
    ASSERT_NE(addr, 0);

    RegionDesc* region = RegionDesc::GetRegionDescAt(addr);
    region->InitFreeUnits();
    region->SetRegionType(RegionDesc::RegionType::THREAD_LOCAL_REGION);
    Mutator::GetMutator()->SetMutatorPhase(GCPhase::GC_PHASE_UNDEF);
    Heap::GetHeap().EnableGC(true);
    Heap::GetHeap().GetCollectorResources().SetGcStarted(false);

    uintptr_t result = theAllocator.Allocate(16, AllocType::NONMOVABLE_OBJECT);
    EXPECT_NE(result, 0u);
}

HWTEST_F_L0(RegionalHeapTest, FeedHungryBuffers_ShouldProvideValidRegions)
{
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t addr = theAllocator.AllocOldRegion();
    ASSERT_NE(addr, 0);

    AllocationBuffer* buffer1 = new (std::nothrow) AllocationBuffer();
    AllocationBuffer* buffer2 = new (std::nothrow) AllocationBuffer();
    ASSERT_NE(buffer1, nullptr);
    ASSERT_NE(buffer2, nullptr);

    RegionDesc* region = RegionDesc::GetRegionDescAt(addr);
    ASSERT_NE(region, nullptr);
    region->InitFreeUnits();
    region->SetRegionType(RegionDesc::RegionType::THREAD_LOCAL_REGION);

    buffer1->SetPreparedRegion(region);
    buffer2->SetPreparedRegion(region);
    Heap::GetHeap().GetAllocator().AddHungryBuffer(*buffer1);
    Heap::GetHeap().GetAllocator().AddHungryBuffer(*buffer2);

    Mutator::GetMutator()->SetMutatorPhase(GCPhase::GC_PHASE_FIX);

    Heap::GetHeap().GetAllocator().FeedHungryBuffers();

    EXPECT_NE(buffer2->GetPreparedRegion(), nullptr);
    delete buffer1;
    delete buffer2;
}

HWTEST_F_L0(RegionalHeapTest, AllocationBuffer_AllocateRawPointerObject_ValidSize_ReturnsNonNull)
{
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t addr = theAllocator.AllocOldRegion();
    ASSERT_NE(addr, 0);

    RegionDesc* region = RegionDesc::GetRegionDescAt(addr);
    ASSERT_NE(region, nullptr);
    region->InitFreeUnits();
    region->SetRegionType(RegionDesc::RegionType::THREAD_LOCAL_REGION);

    AllocationBuffer* buffer = new (std::nothrow) AllocationBuffer();
    ASSERT_NE(buffer, nullptr);
    buffer->SetPreparedRegion(region);

    Mutator::GetMutator()->SetMutatorPhase(GCPhase::GC_PHASE_UNDEF);
    Heap::GetHeap().EnableGC(true);
    Heap::GetHeap().GetCollectorResources().SetGcStarted(false);

    uintptr_t result = theAllocator.Allocate(16, AllocType::NONMOVABLE_OBJECT);
    EXPECT_NE(result, 0u);
    delete buffer;
}

HWTEST_F_L0(RegionalHeapTest, AllocNonMovableObject)
{
    auto* mutator = common::Mutator::GetMutator();
    RegionManager regionManager;
    NonMovableSpace nonMovableSpace(regionManager);
    EXPECT_EQ(nonMovableSpace.Alloc(sizeof(RegionDesc), false), 0);
}

HWTEST_F_L0(RegionalHeapTest, AllocReadOnly1)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_ENUM);
    ThreadLocal::SetThreadType(ThreadType::ARK_PROCESSOR);
    RegionManager regionManager;
    ReadOnlySpace readOnlySpace(regionManager);
    uintptr_t ret = readOnlySpace.Alloc(sizeof(RegionDesc), false);
    EXPECT_EQ(ret, 0);
}

HWTEST_F_L0(RegionalHeapTest, AllocReadOnly2)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_MARK);
    ThreadLocal::SetThreadType(ThreadType::ARK_PROCESSOR);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t ret = theAllocator.Allocate(sizeof(RegionDesc), AllocType::READ_ONLY_OBJECT);
    EXPECT_NE(ret, 0);
}

HWTEST_F_L0(RegionalHeapTest, AllocReadOnly3)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_POST_MARK);
    ThreadLocal::SetThreadType(ThreadType::ARK_PROCESSOR);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t ret = theAllocator.Allocate(sizeof(RegionDesc), AllocType::READ_ONLY_OBJECT);
    EXPECT_NE(ret, 0);
}

HWTEST_F_L0(RegionalHeapTest, AllocReadOnly4)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_PRECOPY);
    ThreadLocal::SetThreadType(ThreadType::ARK_PROCESSOR);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t ret = theAllocator.Allocate(sizeof(RegionDesc), AllocType::READ_ONLY_OBJECT);
    EXPECT_NE(ret, 0);
}

HWTEST_F_L0(RegionalHeapTest, AllocReadOnly5)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_COPY);
    ThreadLocal::SetThreadType(ThreadType::ARK_PROCESSOR);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t ret = theAllocator.Allocate(sizeof(RegionDesc), AllocType::READ_ONLY_OBJECT);
    EXPECT_NE(ret, 0);
}

HWTEST_F_L0(RegionalHeapTest, AllocReadOnly6)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_FIX);
    ThreadLocal::SetThreadType(ThreadType::ARK_PROCESSOR);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t ret = theAllocator.Allocate(sizeof(RegionDesc), AllocType::READ_ONLY_OBJECT);
    EXPECT_NE(ret, 0);
}

HWTEST_F_L0(RegionalHeapTest, AllocReadOnly7)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_UNDEF);
    ThreadLocal::SetThreadType(ThreadType::ARK_PROCESSOR);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t ret = theAllocator.Allocate(sizeof(RegionDesc), AllocType::READ_ONLY_OBJECT);
    EXPECT_NE(ret, 0);
}

HWTEST_F_L0(RegionalHeapTest, AllocateNoGC1)
{
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    size_t size = 32;
    AllocType allocType = AllocType::MOVEABLE_OLD_OBJECT;
    HeapAddress addr = theAllocator.AllocateNoGC(size, allocType);
    EXPECT_NE(addr, 0);
}

HWTEST_F_L0(RegionalHeapTest, AllocateNoGC2)
{
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    size_t size = 16;
    AllocType allocType = AllocType::MOVEABLE_OBJECT;
    HeapAddress addr = theAllocator.AllocateNoGC(size, allocType);
    EXPECT_NE(addr, 0);
}

HWTEST_F_L0(RegionalHeapTest, AllocateNoGC3)
{
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    AllocType allocType = AllocType::MOVEABLE_OBJECT;

    uintptr_t regionAddr = theAllocator.AllocOldRegion();
    ASSERT_NE(regionAddr, 0);
    RegionDesc* region = RegionDesc::GetRegionDescAt(regionAddr);
    ASSERT_NE(region, nullptr);
    region->InitFreeUnits();
    region->SetRegionType(RegionDesc::RegionType::THREAD_LOCAL_REGION);

    HeapAddress addr = theAllocator.AllocateNoGC(1, allocType);
    EXPECT_NE(addr, 0);

    size_t maxSize = region->GetRegionSize() - Allocator::HEADER_SIZE;
    addr = theAllocator.AllocateNoGC(maxSize, allocType);
    EXPECT_NE(addr, 0);

    addr = theAllocator.AllocateNoGC(maxSize + 1, allocType);
    EXPECT_EQ(addr, 0);

    maxSize = 1024 * 1024 * 1024;
    addr = theAllocator.AllocateNoGC(maxSize, allocType);
    EXPECT_EQ(addr, 0);
}

HWTEST_F_L0(RegionalHeapTest, AllocateNoGC4)
{
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    size_t size = 16;
    AllocType allocType = AllocType::NONMOVABLE_OBJECT;

    uintptr_t regionAddr = theAllocator.AllocateNonMovableRegion();
    EXPECT_NE(regionAddr, 0);

    HeapAddress addr = theAllocator.AllocateNoGC(size, allocType);
    EXPECT_NE(addr, 0);
}

HWTEST_F_L0(RegionalHeapTest, MarkJitFortMemInstalled)
{
    auto* mutator = common::Mutator::GetMutator();
    mutator->SetMutatorPhase(GCPhase::GC_PHASE_MARK);
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    
    BaseObject* obj =
        reinterpret_cast<BaseObject*>(theAllocator.Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT));
    ASSERT_NE(obj, nullptr);

    RegionDesc* region = RegionDesc::GetAliveRegionDescAt(reinterpret_cast<uintptr_t>(obj));
    ASSERT_NE(region, nullptr);

    Heap::GetHeap().SetGCPhase(GCPhase::GC_PHASE_IDLE);
    theAllocator.MarkJitFortMemInstalled(nullptr, obj);
    EXPECT_FALSE(region->IsJitFortAwaitInstallFlag());
}
}
