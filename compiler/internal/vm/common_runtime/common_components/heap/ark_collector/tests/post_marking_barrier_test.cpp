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

#include "common_components/heap/ark_collector/post_marking_barrier.h"
#include "common_components/heap/ark_collector/tests/mock_barrier_collector.h"
#include "common_components/mutator/mutator_manager.h"
#include "common_components/tests/test_helper.h"
#include "common_components/heap/heap_manager.h"
#include "common_interfaces/base_runtime.h"

using namespace common;

namespace common::test {
class PostMarkingBarrierTest : public ::testing::Test {
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

HWTEST_F_L0(PostMarkingBarrierTest, ReadRefField_TEST1)
{
    MockCollector collector;
    auto postMarkingBarrier = std::make_unique<PostMarkingBarrier>(collector);
    ASSERT_TRUE(postMarkingBarrier != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RefField<false> field(obj);

    BaseObject *resultObj = postMarkingBarrier->ReadRefField(obj, field);
    ASSERT_TRUE(resultObj != nullptr);
    EXPECT_EQ(resultObj, obj);
}

HWTEST_F_L0(PostMarkingBarrierTest, ReadRefField_TEST2)
{
    MockCollector collector;
    auto postMarkingBarrier = std::make_unique<PostMarkingBarrier>(collector);
    ASSERT_TRUE(postMarkingBarrier != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RefField<false> field(obj);

    BaseObject *resultObj = postMarkingBarrier->ReadRefField(nullptr, field);
    ASSERT_TRUE(resultObj != nullptr);
    EXPECT_EQ(resultObj, obj);
}

HWTEST_F_L0(PostMarkingBarrierTest, ReadStaticRef_TEST1)
{
    MockCollector collector;
    auto postMarkingBarrier = std::make_unique<PostMarkingBarrier>(collector);
    ASSERT_TRUE(postMarkingBarrier != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RefField<false> field(obj);

    BaseObject *resultObj = postMarkingBarrier->ReadStaticRef(field);
    ASSERT_TRUE(resultObj != nullptr);
    EXPECT_EQ(resultObj, obj);
}

HWTEST_F_L0(PostMarkingBarrierTest, ReadStringTableStaticRef_TEST1)
{
    MockCollector collector;
    auto postMarkingBarrier = std::make_unique<PostMarkingBarrier>(collector);
    ASSERT_TRUE(postMarkingBarrier != nullptr);

    RefField<false> field(nullptr);

    BaseObject* resultObj = postMarkingBarrier->ReadStringTableStaticRef(field);
    ASSERT_TRUE(resultObj == nullptr);
}

HWTEST_F_L0(PostMarkingBarrierTest, ReadStringTableStaticRef_TEST2)
{
    MockCollector collector;
    auto postMarkingBarrier = std::make_unique<PostMarkingBarrier>(collector);
    ASSERT_TRUE(postMarkingBarrier != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RegionDesc *regionInfo = RegionDesc::GetRegionDescAt(addr);
    regionInfo->SetRegionAllocPtr(addr - 1);
    regionInfo->SetMarkingLine();
    RefField<false> field(obj);

    BaseObject* resultObj = postMarkingBarrier->ReadStringTableStaticRef(field);
    ASSERT_TRUE(resultObj != nullptr);
}

HWTEST_F_L0(PostMarkingBarrierTest, ReadStringTableStaticRef_TEST3)
{
    MockCollector collector;
    auto postMarkingBarrier = std::make_unique<PostMarkingBarrier>(collector);
    ASSERT_TRUE(postMarkingBarrier != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RegionDesc *regionInfo = RegionDesc::GetRegionDescAt(addr);
    regionInfo->SetRegionType(RegionDesc::RegionType::ALIVE_REGION_FIRST);
    RefField<false> field(obj);

    BaseObject* resultObj = postMarkingBarrier->ReadStringTableStaticRef(field);
    ASSERT_TRUE(resultObj == nullptr);
}

HWTEST_F_L0(PostMarkingBarrierTest, ReadStringTableStaticRef_TEST4)
{
    MockCollector collector;
    auto postMarkingBarrier = std::make_unique<PostMarkingBarrier>(collector);
    ASSERT_TRUE(postMarkingBarrier != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RefField<false> field(obj);

    Heap::GetHeap().SetGCReason(GC_REASON_YOUNG);
    BaseObject* resultObj = postMarkingBarrier->ReadStringTableStaticRef(field);
    ASSERT_TRUE(resultObj != nullptr);
}

HWTEST_F_L0(PostMarkingBarrierTest, WriteRefField_TEST1)
{
    MockCollector collector;
    auto postMarkingBarrier = std::make_unique<PostMarkingBarrier>(collector);
    ASSERT_TRUE(postMarkingBarrier != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* oldObj = reinterpret_cast<BaseObject*>(addr);
    RefField<false> field(oldObj);
    MAddress oldAddr = field.GetFieldValue();

    HeapAddress newObjAddr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* newObj = reinterpret_cast<BaseObject*>(newObjAddr);
    postMarkingBarrier->WriteRefField(oldObj, field, newObj);
    MAddress newAddr = field.GetFieldValue();

    EXPECT_NE(oldAddr, newAddr);
    EXPECT_EQ(newAddr, reinterpret_cast<MAddress>(newObj));
}

HWTEST_F_L0(PostMarkingBarrierTest, WriteBarrier_TEST1)
{
    MockCollector collector;
    auto postMarkingBarrier = std::make_unique<PostMarkingBarrier>(collector);
    ASSERT_TRUE(postMarkingBarrier != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RefField<> field(obj);
    postMarkingBarrier->WriteBarrier(nullptr, obj, field, obj);
    EXPECT_TRUE(obj != nullptr);
}

HWTEST_F_L0(PostMarkingBarrierTest, WriteBarrier_TEST2)
{
    MockCollector collector;
    auto postMarkingBarrier = std::make_unique<PostMarkingBarrier>(collector);
    ASSERT_TRUE(postMarkingBarrier != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RefField<> field(obj);
    Heap::GetHeap().SetGCReason(GC_REASON_YOUNG);
    postMarkingBarrier->WriteBarrier(nullptr, obj, field, obj);
    EXPECT_TRUE(obj != nullptr);
}

HWTEST_F_L0(PostMarkingBarrierTest, ReadStruct_TEST1)
{
    MockCollector collector;
    auto postMarkingBarrier = std::make_unique<PostMarkingBarrier>(collector);
    ASSERT_TRUE(postMarkingBarrier != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    HeapAddress src = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* srcObj = reinterpret_cast<BaseObject*>(src);
    srcObj->SetForwardState(BaseStateWord::ForwardState::FORWARDING);
    HeapAddress dst = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* dstObj = reinterpret_cast<BaseObject*>(dst);
    dstObj->SetForwardState(BaseStateWord::ForwardState::FORWARDED);
    EXPECT_NE(dstObj->IsForwarding(), srcObj->IsForwarding());
    postMarkingBarrier->ReadStruct(dst, obj, src, sizeof(BaseObject));
    EXPECT_EQ(dstObj->IsForwarding(), srcObj->IsForwarding());
}

HWTEST_F_L0(PostMarkingBarrierTest, WriteStruct_TEST1)
{
    MockCollector collector;
    auto postMarkingBarrier = std::make_unique<PostMarkingBarrier>(collector);
    ASSERT_TRUE(postMarkingBarrier != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    HeapAddress src = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* srcObj = reinterpret_cast<BaseObject*>(src);
    srcObj->SetForwardState(BaseStateWord::ForwardState::FORWARDING);
    HeapAddress dst = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* dstObj = reinterpret_cast<BaseObject*>(dst);
    dstObj->SetForwardState(BaseStateWord::ForwardState::FORWARDED);
    EXPECT_NE(dstObj->IsForwarding(), srcObj->IsForwarding());
    postMarkingBarrier->WriteStruct(obj, dst, sizeof(BaseObject), src, sizeof(BaseObject));
    EXPECT_EQ(dstObj->IsForwarding(), srcObj->IsForwarding());
}

HWTEST_F_L0(PostMarkingBarrierTest, AtomicReadRefField_TEST1)
{
    MockCollector collector;
    auto postMarkingBarrier = std::make_unique<PostMarkingBarrier>(collector);
    ASSERT_TRUE(postMarkingBarrier != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RefField<true> field(obj);

    BaseObject* result = postMarkingBarrier->AtomicReadRefField(obj, field, std::memory_order_seq_cst);
    EXPECT_EQ(result, obj);
}

HWTEST_F_L0(PostMarkingBarrierTest, AtomicWriteRefField_TEST1)
{
    MockCollector collector;
    auto postMarkingBarrier = std::make_unique<PostMarkingBarrier>(collector);
    ASSERT_TRUE(postMarkingBarrier != nullptr);

    HeapAddress oldAddr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* oldObj = reinterpret_cast<BaseObject*>(oldAddr);
    constexpr size_t oldSize = 100;
    oldObj->SetSizeForwarded(oldSize);
    EXPECT_EQ(oldObj->GetSizeForwarded(), oldSize);
    RefField<true> oldField(oldObj);
    MAddress oldAddress = oldField.GetFieldValue();

    HeapAddress newAddr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* newObj = reinterpret_cast<BaseObject*>(newAddr);
    constexpr size_t newSize = 200;
    newObj->SetSizeForwarded(newSize);
    EXPECT_EQ(newObj->GetSizeForwarded(), newSize);

    postMarkingBarrier->AtomicWriteRefField(oldObj, oldField, newObj, std::memory_order_relaxed);

    EXPECT_NE(oldField.GetFieldValue(), oldAddress);
    EXPECT_EQ(oldField.GetFieldValue(), reinterpret_cast<MAddress>(newObj));
}

HWTEST_F_L0(PostMarkingBarrierTest, AtomicWriteRefField_TEST2)
{
    MockCollector collector;
    auto postMarkingBarrier = std::make_unique<PostMarkingBarrier>(collector);
    ASSERT_TRUE(postMarkingBarrier != nullptr);

    HeapAddress oldAddr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* oldObj = reinterpret_cast<BaseObject*>(oldAddr);
    constexpr size_t oldSize = 100;
    oldObj->SetSizeForwarded(oldSize);
    EXPECT_EQ(oldObj->GetSizeForwarded(), oldSize);
    RefField<true> oldField(oldObj);
    MAddress oldAddress = oldField.GetFieldValue();

    HeapAddress newAddr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* newObj = reinterpret_cast<BaseObject*>(newAddr);
    constexpr size_t newSize = 200;
    newObj->SetSizeForwarded(newSize);
    EXPECT_EQ(newObj->GetSizeForwarded(), newSize);

    postMarkingBarrier->AtomicWriteRefField(nullptr, oldField, newObj, std::memory_order_relaxed);

    EXPECT_NE(oldField.GetFieldValue(), oldAddress);
    EXPECT_EQ(oldField.GetFieldValue(), reinterpret_cast<MAddress>(newObj));
}

HWTEST_F_L0(PostMarkingBarrierTest, AtomicSwapRefField_TEST1)
{
    MockCollector collector;
    auto postMarkingBarrier = std::make_unique<PostMarkingBarrier>(collector);
    ASSERT_TRUE(postMarkingBarrier != nullptr);

    HeapAddress oldAddr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* oldObj = reinterpret_cast<BaseObject*>(oldAddr);
    RefField<true> field(oldObj);
    HeapAddress newAddr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* newObj = reinterpret_cast<BaseObject*>(newAddr);
    BaseObject* result = postMarkingBarrier->AtomicSwapRefField(
        oldObj, field, newObj, std::memory_order_relaxed);
    EXPECT_EQ(result, oldObj);
    EXPECT_EQ(field.GetFieldValue(), reinterpret_cast<MAddress>(newObj));
}

HWTEST_F_L0(PostMarkingBarrierTest, CompareAndSwapRefField_TEST1)
{
    MockCollector collector;
    auto postMarkingBarrier = std::make_unique<PostMarkingBarrier>(collector);
    ASSERT_TRUE(postMarkingBarrier != nullptr);

    HeapAddress oldAddr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* oldObj = reinterpret_cast<BaseObject*>(oldAddr);
    constexpr size_t oldSize = 100;
    oldObj->SetSizeForwarded(oldSize);
    EXPECT_EQ(oldObj->GetSizeForwarded(), oldSize);
    RefField<true> oldField(oldObj);
    MAddress oldAddress = oldField.GetFieldValue();

    HeapAddress newAddr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* newObj = reinterpret_cast<BaseObject*>(newAddr);
    constexpr size_t newSize = 200;
    newObj->SetSizeForwarded(newSize);
    EXPECT_EQ(newObj->GetSizeForwarded(), newSize);
    RefField<true> newField(newObj);
    MAddress neWAddress = newField.GetFieldValue();
    EXPECT_NE(oldAddress, neWAddress);

    bool result = postMarkingBarrier->CompareAndSwapRefField(
        oldObj, oldField, oldObj, newObj, std::memory_order_seq_cst, std::memory_order_seq_cst);
    ASSERT_TRUE(result);
    EXPECT_EQ(oldField.GetFieldValue(), neWAddress);
}

HWTEST_F_L0(PostMarkingBarrierTest, CompareAndSwapRefField_TEST2)
{
    MockCollector collector;
    auto postMarkingBarrier = std::make_unique<PostMarkingBarrier>(collector);
    ASSERT_TRUE(postMarkingBarrier != nullptr);

    HeapAddress oldAddr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* oldObj = reinterpret_cast<BaseObject*>(oldAddr);
    constexpr size_t oldSize = 100;
    oldObj->SetSizeForwarded(oldSize);
    EXPECT_EQ(oldObj->GetSizeForwarded(), oldSize);
    RefField<true> oldField(oldObj);

    MAddress initialAddress = oldField.GetFieldValue();

    bool result = postMarkingBarrier->CompareAndSwapRefField(
        oldObj, oldField, oldObj, oldObj, std::memory_order_seq_cst, std::memory_order_seq_cst);
    ASSERT_TRUE(result);
    EXPECT_EQ(oldField.GetFieldValue(), initialAddress);
}

HWTEST_F_L0(PostMarkingBarrierTest, CompareAndSwapRefField_TEST3)
{
    MockCollector collector;
    auto postMarkingBarrier = std::make_unique<PostMarkingBarrier>(collector);
    ASSERT_TRUE(postMarkingBarrier != nullptr);

    HeapAddress oldAddr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* oldObj = reinterpret_cast<BaseObject*>(oldAddr);
    constexpr size_t oldSize = 100;
    oldObj->SetSizeForwarded(oldSize);
    EXPECT_EQ(oldObj->GetSizeForwarded(), oldSize);
    RefField<true> oldField(oldObj);
    MAddress oldAddress = oldField.GetFieldValue();

    HeapAddress newAddr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* newObj = reinterpret_cast<BaseObject*>(newAddr);
    constexpr size_t newSize = 200;
    newObj->SetSizeForwarded(newSize);
    EXPECT_EQ(newObj->GetSizeForwarded(), newSize);
    RefField<true> newField(newObj);
    MAddress neWAddress = newField.GetFieldValue();
    EXPECT_NE(oldAddress, neWAddress);

    bool result = postMarkingBarrier->CompareAndSwapRefField(oldObj, newField, oldObj, newObj,
        std::memory_order_seq_cst, std::memory_order_seq_cst);
    ASSERT_FALSE(result);
}

HWTEST_F_L0(PostMarkingBarrierTest, CopyStructArray_TEST1)
{
    MockCollector collector;
    auto postMarkingBarrier = std::make_unique<PostMarkingBarrier>(collector);
    ASSERT_TRUE(postMarkingBarrier != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    HeapAddress src = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* srcObj = reinterpret_cast<BaseObject*>(src);
    srcObj->SetForwardState(BaseStateWord::ForwardState::FORWARDING);
    HeapAddress dst = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* dstObj = reinterpret_cast<BaseObject*>(dst);
    dstObj->SetForwardState(BaseStateWord::ForwardState::FORWARDED);
    EXPECT_NE(dstObj->IsForwarding(), srcObj->IsForwarding());
    postMarkingBarrier->CopyStructArray(obj, dst, sizeof(BaseObject), obj, src, sizeof(BaseObject));
    EXPECT_EQ(dstObj->IsForwarding(), srcObj->IsForwarding());
}
}  // namespace common::test
