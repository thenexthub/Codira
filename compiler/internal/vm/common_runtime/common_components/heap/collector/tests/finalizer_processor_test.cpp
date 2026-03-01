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

#include "common_components/heap/collector/finalizer_processor.h"
#include "common_components/heap/collector/collector_proxy.h"
#include "common_components/heap/heap_manager.h"
#include "common_components/tests/test_helper.h"

using namespace common;

namespace common::test {
const uint32_t TWO_SECONDS = 2;
const uint32_t HUNDRED_MILLISECONDS = 100;
constexpr uint64_t TAG_BOOLEAN = 0x04ULL;

class FinalizerProcessorTest : public common::test::BaseTestWithScope {
protected:
    static void SetUpTestCase()
    {
        BaseRuntime::GetInstance()->InitFromDynamic();
    }

    static void TearDownTestCase() {}

    void SetUp() override
    {
        MutatorManager::Instance().CreateRuntimeMutator(ThreadType::ARK_PROCESSOR);
    }

    void TearDown() override
    {
        MutatorManager::Instance().DestroyRuntimeMutator(ThreadType::ARK_PROCESSOR);
    }
};

HWTEST_F_L0(FinalizerProcessorTest, RegisterFinalizer_TEST1)
{
    FinalizerProcessor finalizerProcessor;
    HeapAddress addr = common::HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject *obj = reinterpret_cast<BaseObject*>(addr | TAG_BOOLEAN);
    new (obj) BaseObject(); // Construct BaseObject
    finalizerProcessor.RegisterFinalizer(obj);
    bool flag = common::RegionalHeap::IsMarkedObject(obj);
    EXPECT_FALSE(flag);
}

HWTEST_F_L0(FinalizerProcessorTest, EnqueueFinalizables_TEST1)
{
    FinalizerProcessor finalizerProcessor;
    HeapAddress addr = common::HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject *obj = reinterpret_cast<BaseObject*>(addr | TAG_BOOLEAN);
    new (obj) BaseObject(); // Construct BaseObject
    finalizerProcessor.RegisterFinalizer(obj);
    std::function<bool(BaseObject*)> finalizable = [](BaseObject* obj) {
        return !common::RegionalHeap::IsMarkedObject(obj);
    };
    finalizerProcessor.EnqueueFinalizables(finalizable, 1);
    bool flag = finalizable(obj);
    EXPECT_TRUE(flag);
}

HWTEST_F_L0(FinalizerProcessorTest, EnqueueFinalizables_TEST2)
{
    FinalizerProcessor finalizerProcessor;
    HeapAddress addr = common::HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject *obj = reinterpret_cast<BaseObject*>(addr | TAG_BOOLEAN);
    new (obj) BaseObject();
    finalizerProcessor.RegisterFinalizer(obj);
    RootVisitor visitor = [](ObjectRef&) {
        return;
    };
    std::function<bool(BaseObject*)> finalizable = [this](BaseObject* obj) {
        return common::RegionalHeap::IsMarkedObject(obj);
    };
    auto before = finalizerProcessor.VisitFinalizers(visitor);
    finalizerProcessor.EnqueueFinalizables(finalizable, 1);
    auto after = finalizerProcessor.VisitFinalizers(visitor);
    bool flag = finalizable(obj);
    EXPECT_FALSE(flag);
    EXPECT_EQ(before, after);
}

HWTEST_F_L0(FinalizerProcessorTest, EnqueueFinalizables_TEST3)
{
    FinalizerProcessor finalizerProcessor;
    RootVisitor visitor = [](ObjectRef&) {
        return;
    };
    std::function<bool(BaseObject*)> finalizable = [this](BaseObject* obj) {
        return common::RegionalHeap::IsMarkedObject(obj);
    };
    auto num1 = finalizerProcessor.VisitFinalizers(visitor);
    finalizerProcessor.EnqueueFinalizables(finalizable, 0);
    EXPECT_EQ(num1, 0);
    auto num2 = finalizerProcessor.VisitFinalizers(visitor);
    finalizerProcessor.EnqueueFinalizables(finalizable, 1);
    EXPECT_EQ(num2, 0);
    HeapAddress addr = common::HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject *obj = reinterpret_cast<BaseObject*>(addr | TAG_BOOLEAN);
    new (obj) BaseObject();
    finalizerProcessor.RegisterFinalizer(obj);
    auto num3 = finalizerProcessor.VisitFinalizers(visitor);
    finalizerProcessor.EnqueueFinalizables(finalizable, 0);
    bool flag = finalizable(obj);
    EXPECT_NE(num3, 0);
    EXPECT_FALSE(flag);
}

HWTEST_F_L0(FinalizerProcessorTest, Run_TEST1)
{
    FinalizerProcessor finalizerProcessor;
    HeapAddress addr = common::HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject *obj = reinterpret_cast<BaseObject*>(addr | TAG_BOOLEAN);
    new (obj) BaseObject();
    AllocationBuffer* buffer = new (std::nothrow) AllocationBuffer();
    RegionDesc* region = RegionDesc::GetRegionDescAt(addr);
    buffer->SetPreparedRegion(region);
    Heap::GetHeap().GetAllocator().AddHungryBuffer(*buffer);
    finalizerProcessor.RegisterFinalizer(obj);
    finalizerProcessor.Start();
    std::thread notifier([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(HUNDRED_MILLISECONDS));
        finalizerProcessor.NotifyToFeedAllocBuffers();
        std::this_thread::sleep_for(std::chrono::milliseconds(HUNDRED_MILLISECONDS));
        finalizerProcessor.NotifyToReclaimGarbage();
    });
    notifier.join();
    std::this_thread::sleep_for(std::chrono::seconds(TWO_SECONDS));
    finalizerProcessor.Stop();
    EXPECT_NE(buffer->GetPreparedRegion(), nullptr);
    delete buffer;
}
} // namespace common::test
