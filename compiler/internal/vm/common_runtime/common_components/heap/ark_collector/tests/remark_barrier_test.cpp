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

#include "common_components/heap/ark_collector/remark_barrier.h"
#include "common_components/heap/ark_collector/tests/mock_barrier_collector.h"
#include "common_components/mutator/mutator_manager.h"
#include "common_components/tests/test_helper.h"
#include "common_components/heap/heap_manager.h"
#include "common_interfaces/base_runtime.h"

using namespace common;

namespace common::test {
class RemarkBarrierTest : public BaseTestWithScope {
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

HWTEST_F_L0(RemarkBarrierTest, ReadRefField_TEST1)
{
    MockCollector collector;
    auto remarkBarrier = std::make_unique<RemarkBarrier>(collector);
    ASSERT_TRUE(remarkBarrier != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RefField<false> field(obj);

    BaseObject *resultObj = remarkBarrier->ReadRefField(obj, field);
    EXPECT_EQ(resultObj, obj);
}

HWTEST_F_L0(RemarkBarrierTest, ReadRefField_TEST2)
{
    MockCollector collector;
    auto remarkBarrier = std::make_unique<RemarkBarrier>(collector);
    ASSERT_TRUE(remarkBarrier != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RefField<false> field(obj);

    BaseObject *resultObj = remarkBarrier->ReadRefField(nullptr, field);
    EXPECT_EQ(resultObj, obj);
}

HWTEST_F_L0(RemarkBarrierTest, ReadStaticRef_TEST1)
{
    MockCollector collector;
    auto remarkBarrier = std::make_unique<RemarkBarrier>(collector);
    ASSERT_TRUE(remarkBarrier != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RefField<false> field(obj);

    BaseObject *resultObj = remarkBarrier->ReadStaticRef(field);
    EXPECT_EQ(resultObj, obj);
}

HWTEST_F_L0(RemarkBarrierTest, ReadStringTableStaticRef_TEST1)
{
    MockCollector collector;
    auto remarkBarrier = std::make_unique<RemarkBarrier>(collector);
    ASSERT_TRUE(remarkBarrier != nullptr);

    RefField<false> field(nullptr);

    BaseObject* resultObj = remarkBarrier->ReadStringTableStaticRef(field);
    ASSERT_TRUE(resultObj == nullptr);
}

HWTEST_F_L0(RemarkBarrierTest, ReadStringTableStaticRef_TEST2)
{
    MockCollector collector;
    auto remarkBarrier = std::make_unique<RemarkBarrier>(collector);
    ASSERT_TRUE(remarkBarrier != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RegionDesc *regionInfo = RegionDesc::GetRegionDescAt(addr);
    regionInfo->SetRegionAllocPtr(addr - 1);
    regionInfo->SetMarkingLine();
    RefField<false> field(obj);

    BaseObject* resultObj = remarkBarrier->ReadStringTableStaticRef(field);
    ASSERT_TRUE(resultObj != nullptr);
}

HWTEST_F_L0(RemarkBarrierTest, ReadStringTableStaticRef_TEST3)
{
    MockCollector collector;
    auto remarkBarrier = std::make_unique<RemarkBarrier>(collector);
    ASSERT_TRUE(remarkBarrier != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RegionDesc *regionInfo = RegionDesc::GetRegionDescAt(addr);
    regionInfo->SetRegionType(RegionDesc::RegionType::ALIVE_REGION_FIRST);
    RefField<false> field(obj);

    BaseObject* resultObj = remarkBarrier->ReadStringTableStaticRef(field);
    ASSERT_TRUE(resultObj == nullptr);
}

HWTEST_F_L0(RemarkBarrierTest, ReadStringTableStaticRef_TEST4)
{
    MockCollector collector;
    auto remarkBarrier = std::make_unique<RemarkBarrier>(collector);
    ASSERT_TRUE(remarkBarrier != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RefField<false> field(obj);

    Heap::GetHeap().SetGCReason(GC_REASON_YOUNG);
    BaseObject* resultObj = remarkBarrier->ReadStringTableStaticRef(field);
    ASSERT_TRUE(resultObj != nullptr);
}

HWTEST_F_L0(RemarkBarrierTest, ReadStruct_TEST1)
{
    MockCollector collector;
    auto remarkBarrier = std::make_unique<RemarkBarrier>(collector);
    ASSERT_TRUE(remarkBarrier != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    HeapAddress src = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* srcObj = reinterpret_cast<BaseObject*>(src);
    srcObj->SetForwardState(BaseStateWord::ForwardState::FORWARDING);
    HeapAddress dst = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* dstObj = reinterpret_cast<BaseObject*>(dst);
    dstObj->SetForwardState(BaseStateWord::ForwardState::FORWARDED);
    EXPECT_NE(dstObj->IsForwarding(), srcObj->IsForwarding());
    remarkBarrier->ReadStruct(dst, obj, src, sizeof(BaseObject));
    EXPECT_EQ(dstObj->IsForwarding(), srcObj->IsForwarding());
}

HWTEST_F_L0(RemarkBarrierTest, WriteRefField_TEST1)
{
    MockCollector collector;
    auto remarkBarrier = std::make_unique<RemarkBarrier>(collector);
    ASSERT_TRUE(remarkBarrier != nullptr);

    HeapAddress oldAddr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* oldObj = reinterpret_cast<BaseObject*>(oldAddr);
    constexpr size_t oldSize = 100;
    oldObj->SetSizeForwarded(oldSize);
    EXPECT_EQ(oldObj->GetSizeForwarded(), oldSize);

    RefField<false> field(oldObj);
    BaseObject *target = field.GetTargetObject();
    EXPECT_TRUE(target != nullptr);

    HeapAddress newAddr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* newObj = reinterpret_cast<BaseObject*>(newAddr);
    constexpr size_t newSize = 200;
    newObj->SetSizeForwarded(newSize);
    EXPECT_EQ(newObj->GetSizeForwarded(), newSize);

    remarkBarrier->WriteRefField(oldObj, field, newObj);

    MAddress newAddress = field.GetFieldValue();
    MAddress expectedAddress = RefField<>(newObj).GetFieldValue();
    EXPECT_EQ(newAddress, expectedAddress);
}

HWTEST_F_L0(RemarkBarrierTest, WriteRefField_TEST2)
{
    MockCollector collector;
    auto remarkBarrier = std::make_unique<RemarkBarrier>(collector);
    ASSERT_TRUE(remarkBarrier != nullptr);

    HeapAddress oldAddr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* oldObj = reinterpret_cast<BaseObject*>(oldAddr);
    constexpr size_t oldSize = 100;
    oldObj->SetSizeForwarded(oldSize);
    EXPECT_EQ(oldObj->GetSizeForwarded(), oldSize);

    RefField<false> field(MAddress(0));
    BaseObject *target = field.GetTargetObject();
    EXPECT_TRUE(target == nullptr);

    HeapAddress newAddr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* newObj = reinterpret_cast<BaseObject*>(newAddr);
    constexpr size_t newSize = 200;
    newObj->SetSizeForwarded(newSize);
    EXPECT_EQ(newObj->GetSizeForwarded(), newSize);

    remarkBarrier->WriteRefField(oldObj, field, newObj);

    MAddress newAddress = field.GetFieldValue();
    MAddress expectedAddress = RefField<>(newObj).GetFieldValue();
    EXPECT_EQ(newAddress, expectedAddress);
}

HWTEST_F_L0(RemarkBarrierTest, WriteBarrier_TEST1)
{
    MockCollector collector;
    auto remarkBarrier = std::make_unique<RemarkBarrier>(collector);
    ASSERT_TRUE(remarkBarrier != nullptr);

#ifndef ARK_USE_SATB_BARRIER
    constexpr uint64_t TAG_BITS_SHIFT = 48;
    constexpr uint64_t TAG_MARK = 0xFFFFULL << TAG_BITS_SHIFT;
    constexpr uint64_t TAG_SPECIAL = 0x02ULL;
    constexpr uint64_t TAG_BOOLEAN = 0x04ULL;
    constexpr uint64_t TAG_HEAP_OBJECT_MASK = TAG_MARK | TAG_SPECIAL | TAG_BOOLEAN;

    RefField<> field(MAddress(0));
    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    addr |= TAG_HEAP_OBJECT_MASK;
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    remarkBarrier->WriteBarrier(nullptr, obj, field, obj);
    EXPECT_TRUE(obj != nullptr);
#endif
}

HWTEST_F_L0(RemarkBarrierTest, WriteBarrier_TEST2)
{
    MockCollector collector;
    auto remarkBarrier = std::make_unique<RemarkBarrier>(collector);
    ASSERT_TRUE(remarkBarrier != nullptr);

#ifdef ARK_USE_SATB_BARRIER
    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RefField<false> normalField(obj);
    remarkBarrier->WriteBarrier(nullptr, obj, normalField, obj);
    EXPECT_TRUE(obj != nullptr);

    BaseObject weakObj;
    RefField<false> weakField(MAddress(0));
    remarkBarrier->WriteBarrier(nullptr, &weakObj, weakField, &weakObj);
    EXPECT_TRUE(weakObj != nullptr);

    BaseObject nonTaggedObj;
    RefField<false> nonTaggedField(&nonTaggedObj);
    remarkBarrier->WriteBarrier(nullptr, nullptr, nonTaggedField, &nonTaggedObj);
    EXPECT_TRUE(nonTaggedObj != nullptr);
#endif
}

HWTEST_F_L0(RemarkBarrierTest, WriteBarrier_TEST3)
{
    MockCollector collector;
    auto remarkBarrier = std::make_unique<RemarkBarrier>(collector);
    ASSERT_TRUE(remarkBarrier != nullptr);

#ifndef ARK_USE_SATB_BARRIER
    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RefField<> field(obj);
    remarkBarrier->WriteBarrier(nullptr, obj, field, obj);
    EXPECT_TRUE(obj != nullptr);
#endif
}

HWTEST_F_L0(RemarkBarrierTest, WriteBarrier_TEST4)
{
    MockCollector collector;
    auto remarkBarrier = std::make_unique<RemarkBarrier>(collector);
    ASSERT_TRUE(remarkBarrier != nullptr);

#ifndef ARK_USE_SATB_BARRIER
    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RefField<> field(obj);
    remarkBarrier->WriteBarrier(Mutator::GetMutator(), obj, field, obj);
    EXPECT_TRUE(obj != nullptr);
#endif
}

HWTEST_F_L0(RemarkBarrierTest, WriteStruct_TEST1)
{
    MockCollector collector;
    auto remarkBarrier = std::make_unique<RemarkBarrier>(collector);
    ASSERT_TRUE(remarkBarrier != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    HeapAddress src = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* srcObj = reinterpret_cast<BaseObject*>(src);
    srcObj->SetForwardState(BaseStateWord::ForwardState::FORWARDING);
    HeapAddress dst = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* dstObj = reinterpret_cast<BaseObject*>(dst);
    dstObj->SetForwardState(BaseStateWord::ForwardState::FORWARDED);
    EXPECT_NE(dstObj->IsForwarding(), srcObj->IsForwarding());
    remarkBarrier->WriteStruct(obj, dst, sizeof(BaseObject), src, sizeof(BaseObject));
    EXPECT_EQ(dstObj->IsForwarding(), srcObj->IsForwarding());
}

HWTEST_F_L0(RemarkBarrierTest, WriteStruct_TEST2)
{
    MockCollector collector;
    auto remarkBarrier = std::make_unique<RemarkBarrier>(collector);
    ASSERT_TRUE(remarkBarrier != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    HeapAddress src = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* srcObj = reinterpret_cast<BaseObject*>(src);
    srcObj->SetForwardState(BaseStateWord::ForwardState::FORWARDING);
    HeapAddress dst = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* dstObj = reinterpret_cast<BaseObject*>(dst);
    dstObj->SetForwardState(BaseStateWord::ForwardState::FORWARDED);
    EXPECT_NE(dstObj->IsForwarding(), srcObj->IsForwarding());

    auto mutator = ThreadLocal::GetMutator();
    ThreadLocal::SetMutator(nullptr);
    remarkBarrier->WriteStruct(obj, dst, sizeof(BaseObject), src, sizeof(BaseObject));
    ThreadLocal::SetMutator(mutator);
    EXPECT_EQ(dstObj->IsForwarding(), srcObj->IsForwarding());
}

HWTEST_F_L0(RemarkBarrierTest, AtomicReadRefField_TEST1)
{
    MockCollector collector;
    auto remarkBarrier = std::make_unique<RemarkBarrier>(collector);
    ASSERT_TRUE(remarkBarrier != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    constexpr size_t size = 100;
    obj->SetSizeForwarded(size);
    EXPECT_EQ(obj->GetSizeForwarded(), size);
    RefField<true> field(obj);

    BaseObject *resultObj = nullptr;
    resultObj = remarkBarrier->AtomicReadRefField(obj, field, std::memory_order_seq_cst);
    ASSERT_TRUE(resultObj != nullptr);
}

HWTEST_F_L0(RemarkBarrierTest, AtomicWriteRefField_TEST1)
{
    MockCollector collector;
    auto remarkBarrier = std::make_unique<RemarkBarrier>(collector);
    ASSERT_TRUE(remarkBarrier != nullptr);

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

    remarkBarrier->AtomicWriteRefField(oldObj, oldField, newObj, std::memory_order_relaxed);
    EXPECT_EQ(oldField.GetFieldValue(), neWAddress);
}

HWTEST_F_L0(RemarkBarrierTest, AtomicWriteRefField_TEST2)
{
    MockCollector collector;
    auto remarkBarrier = std::make_unique<RemarkBarrier>(collector);
    ASSERT_TRUE(remarkBarrier != nullptr);

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

    remarkBarrier->AtomicWriteRefField(nullptr, oldField, newObj, std::memory_order_relaxed);
    EXPECT_EQ(oldField.GetFieldValue(), neWAddress);
}

HWTEST_F_L0(RemarkBarrierTest, AtomicSwapRefField_TEST1)
{
    MockCollector collector;
    auto remarkBarrier = std::make_unique<RemarkBarrier>(collector);
    ASSERT_TRUE(remarkBarrier != nullptr);

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

    BaseObject *resultObj = nullptr;
    resultObj = remarkBarrier->AtomicSwapRefField(oldObj, oldField, newObj, std::memory_order_relaxed);
    EXPECT_EQ(oldField.GetFieldValue(), newField.GetFieldValue());
}

HWTEST_F_L0(RemarkBarrierTest, CompareAndSwapRefField_TEST1)
{
    MockCollector collector;
    auto remarkBarrier = std::make_unique<RemarkBarrier>(collector);
    ASSERT_TRUE(remarkBarrier != nullptr);

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

    bool result = remarkBarrier->CompareAndSwapRefField(
        oldObj, oldField, oldObj, newObj, std::memory_order_seq_cst, std::memory_order_seq_cst);
    ASSERT_TRUE(result);
}

HWTEST_F_L0(RemarkBarrierTest, CompareAndSwapRefField_TEST2)
{
    MockCollector collector;
    auto remarkBarrier = std::make_unique<RemarkBarrier>(collector);
    ASSERT_TRUE(remarkBarrier != nullptr);

    HeapAddress oldAddr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject* oldObj = reinterpret_cast<BaseObject*>(oldAddr);
    constexpr size_t oldSize = 100;
    oldObj->SetSizeForwarded(oldSize);
    EXPECT_EQ(oldObj->GetSizeForwarded(), oldSize);
    RefField<true> oldField(oldObj);

    bool result = remarkBarrier->CompareAndSwapRefField(
        oldObj, oldField, oldObj, oldObj, std::memory_order_seq_cst, std::memory_order_seq_cst);
    ASSERT_TRUE(result);
}

HWTEST_F_L0(RemarkBarrierTest, CompareAndSwapRefField_TEST3)
{
    MockCollector collector;
    auto remarkBarrier = std::make_unique<RemarkBarrier>(collector);
    ASSERT_TRUE(remarkBarrier != nullptr);

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

    bool result = remarkBarrier->CompareAndSwapRefField(
        oldObj, newField, oldObj, newObj, std::memory_order_seq_cst, std::memory_order_seq_cst);
    ASSERT_FALSE(result);
}
}  // namespace common::test
