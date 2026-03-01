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

#include "common_components/heap/ark_collector/ark_collector.h"
#include "common_components/heap/verification.cpp"
#include "common_components/heap/heap_manager.h"
#include "common_components/tests/test_helper.h"
#include "objects/base_object_operator.h"

using namespace common;
namespace common::test {
class TestBaseObjectOperator : public common::BaseObjectOperatorInterfaces {
public:
    bool IsValidObject([[maybe_unused]] const BaseObject *object) const override { return enbaleValidObject_; }
    void ForEachRefField(const BaseObject *object, const common::RefFieldVisitor &visitor) const override {}
    size_t ForEachRefFieldAndGetSize(
        const BaseObject *object,
        const common::RefFieldVisitor &visitor) const override
    {
        return 0;
    }
    size_t GetSize(const BaseObject *object) const override{ return size_; }
    BaseObject *GetForwardingPointer(const BaseObject *object) const override { return nullptr; }
    void SetForwardingPointerAfterExclusive(BaseObject *object, BaseObject *fwdPtr) override {}
    void SetValidObject(bool value) { enbaleValidObject_ = value; }
    void SetSize(size_t size) { size_ = size; }
    void ForEachRefFieldSkipReferent(const BaseObject *object, const RefFieldVisitor &visitor) const override {}
    void IterateXRef(const BaseObject *object, const RefFieldVisitor &visitor) const override {}
private:
    bool enbaleValidObject_ = false;
    size_t size_ = 0;
};
class VerificationTest : public common::test::BaseTestWithScope {
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
        MutatorManager::Instance().CreateRuntimeMutator(ThreadType::GC_THREAD);
    }

    void TearDown() override
    {
        MutatorManager::Instance().DestroyRuntimeMutator(ThreadType::GC_THREAD);
    }
};

HWTEST_F_L0(VerificationTest, GetObjectInfoTest)
{
    BaseObject* obj = nullptr;
    std::string result = GetObjectInfo(obj);

    EXPECT_NE(result.find("address: 0x0"), std::string::npos);
    EXPECT_NE(result.find("Skip: nullptr"), std::string::npos);
    EXPECT_NE(result.find("Skip: Object is not in heap range"), std::string::npos);
}

HWTEST_F_L0(VerificationTest, GetObjectInfoTest2)
{
    BaseObject obj;
    std::string result = GetObjectInfo(&obj);
    EXPECT_NE(result.find("address: 0x"), std::string::npos);
    EXPECT_NE(result.find("Skip: Object is not in heap range"), std::string::npos);
}

HWTEST_F_L0(VerificationTest, GetRefInfoTest)
{
    BaseObject oldObj;
    RefField<false> oldField(&oldObj);
    MAddress oldAddress = oldField.GetFieldValue();
    std::string result = GetRefInfo(oldField);
    EXPECT_NE(result.find("address: 0x"), std::string::npos);
    EXPECT_NE(result.find("Skip: Object is not in heap range"), std::string::npos);
}

HWTEST_F_L0(VerificationTest, VerifyRefImplTest2)
{
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t addr = theAllocator.AllocOldRegion();
    ASSERT_NE(addr, 0U);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RegionDesc* region = RegionDesc::GetRegionDescAt(reinterpret_cast<uintptr_t>(obj));
    ASSERT_NE(region, nullptr);
    region->SetRegionType(RegionDesc::RegionType::FROM_REGION);
    RefField<false> field(obj);

    auto refObj = field.GetTargetObject();

    AfterForwardVisitor visitor;
    visitor.VerifyRefImpl(obj, field);
    ASSERT_FALSE(RegionalHeap::IsMarkedObject(refObj));
    ASSERT_FALSE(RegionalHeap::IsResurrectedObject(refObj));
}

HWTEST_F_L0(VerificationTest, VerifyRefImplTest3)
{
    RegionalHeap& theAllocator = reinterpret_cast<RegionalHeap&>(Heap::GetHeap().GetAllocator());
    uintptr_t addr = theAllocator.AllocOldRegion();
    ASSERT_NE(addr, 0U);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RegionDesc* region = RegionDesc::GetRegionDescAt(reinterpret_cast<uintptr_t>(obj));
    ASSERT_NE(region, nullptr);
    region->SetRegionType(RegionDesc::RegionType::FULL_POLYSIZE_NONMOVABLE_REGION);
    RefField<false> field(obj);

    auto refObj = field.GetTargetObject();

    ReadBarrierSetter visitor;
    visitor.VerifyRefImpl(nullptr, field);
    visitor.VerifyRefImpl(obj, field);
    EXPECT_EQ(RegionDesc::RegionType::FULL_POLYSIZE_NONMOVABLE_REGION,
        RegionDesc::GetRegionDescAt(reinterpret_cast<MAddress>(field.GetTargetObject()))->GetRegionType());

    region->SetRegionType(RegionDesc::RegionType::RECENT_POLYSIZE_NONMOVABLE_REGION);
    visitor.VerifyRefImpl(obj, field);
    EXPECT_EQ(RegionDesc::RegionType::RECENT_POLYSIZE_NONMOVABLE_REGION,
        RegionDesc::GetRegionDescAt(reinterpret_cast<MAddress>(field.GetTargetObject()))->GetRegionType());

    region->SetRegionType(RegionDesc::RegionType::MONOSIZE_NONMOVABLE_REGION);
    visitor.VerifyRefImpl(obj, field);
    EXPECT_EQ(RegionDesc::RegionType::MONOSIZE_NONMOVABLE_REGION,
        RegionDesc::GetRegionDescAt(reinterpret_cast<MAddress>(field.GetTargetObject()))->GetRegionType());

    region->SetRegionType(RegionDesc::RegionType::FULL_MONOSIZE_NONMOVABLE_REGION);
    visitor.VerifyRefImpl(obj, field);
    EXPECT_EQ(RegionDesc::RegionType::FULL_MONOSIZE_NONMOVABLE_REGION,
        RegionDesc::GetRegionDescAt(reinterpret_cast<MAddress>(field.GetTargetObject()))->GetRegionType());

    region->SetRegionType(RegionDesc::RegionType::READ_ONLY_REGION);
    auto oldRefValue = field.GetFieldValue();
    visitor.VerifyRefImpl(obj, field);
    auto newRefValue = field.GetFieldValue();
    EXPECT_NE(oldRefValue, newRefValue);
}

std::unique_ptr<ArkCollector> GetArkCollector()
{
    CollectorResources &resources = Heap::GetHeap().GetCollectorResources();
    Allocator &allocator = Heap::GetHeap().GetAllocator();

    return std::make_unique<ArkCollector>(allocator, resources);
}

HWTEST_F_L0(VerificationTest, VerifyAfterMarkTest1)
{
    Heap::GetHeap().SetGCPhase(GCPhase::GC_PHASE_POST_MARK);
    std::unique_ptr<ArkCollector> arkCollector = GetArkCollector();
    ASSERT_TRUE(arkCollector != nullptr);
    WVerify verify;
    verify.VerifyAfterMark(*arkCollector);
    ASSERT_FALSE(MutatorManager::Instance().WorldStopped());
}

HWTEST_F_L0(VerificationTest, VerifyAfterForwardTest1)
{
    Heap::GetHeap().SetGCPhase(GCPhase::GC_PHASE_COPY);
    std::unique_ptr<ArkCollector> arkCollector = GetArkCollector();
    ASSERT_TRUE(arkCollector != nullptr);
    WVerify verify;
    verify.VerifyAfterForward(*arkCollector);
    ASSERT_FALSE(MutatorManager::Instance().WorldStopped());
}

HWTEST_F_L0(VerificationTest, VerifyAfterFixTest1)
{
    Heap::GetHeap().SetGCPhase(GCPhase::GC_PHASE_FIX);
    std::unique_ptr<ArkCollector> arkCollector = GetArkCollector();
    ASSERT_TRUE(arkCollector != nullptr);
    WVerify verify;
    verify.VerifyAfterFix(*arkCollector);
    ASSERT_FALSE(MutatorManager::Instance().WorldStopped());
}

HWTEST_F_L0(VerificationTest, EnableReadBarrierDFXTest1)
{
    std::unique_ptr<ArkCollector> arkCollector = GetArkCollector();
    ASSERT_TRUE(arkCollector != nullptr);
    WVerify verify;
    verify.EnableReadBarrierDFX(*arkCollector);
    ASSERT_FALSE(MutatorManager::Instance().WorldStopped());
}

HWTEST_F_L0(VerificationTest, DisableReadBarrierDFXTest1)
{
    std::unique_ptr<ArkCollector> arkCollector = GetArkCollector();
    ASSERT_TRUE(arkCollector != nullptr);
    WVerify verify;
    verify.DisableReadBarrierDFX(*arkCollector);
    ASSERT_FALSE(MutatorManager::Instance().WorldStopped());
}

HWTEST_F_L0(VerificationTest, GetObjectInfoTest3)
{
    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::NONMOVABLE_OBJECT, true);
    BaseObject *obj = reinterpret_cast<BaseObject *>(addr);
    std::string result = GetObjectInfo(obj);
    EXPECT_NE(result.find("address: 0x"), std::string::npos);
    EXPECT_NE(result.find("Type: 0x"), std::string::npos);
    EXPECT_NE(result.find("Base: 0x"), std::string::npos);
    EXPECT_NE(result.find("Start: 0x"), std::string::npos);
    EXPECT_NE(result.find("End: 0x"), std::string::npos);
    EXPECT_NE(result.find("AllocPtr: 0x"), std::string::npos);
    EXPECT_NE(result.find("MarkingLine: 0x"), std::string::npos);
    EXPECT_NE(result.find("CopyLine: 0x"), std::string::npos);
}

HWTEST_F_L0(VerificationTest, GetRefInfoTest2)
{
    RefField<false> field(nullptr);
    uintptr_t taggedValue = 0x04;
    field.SetFieldValue(static_cast<MAddress>(taggedValue));
    std::string result = GetRefInfo(field);
    EXPECT_NE(result.find("> Raw memory:"), std::string::npos);
    EXPECT_NE(result.find("Skip: primitive"), std::string::npos);
}

HWTEST_F_L0(VerificationTest, VerifyRefImplTest)
{
    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::NONMOVABLE_OBJECT, true);
    BaseObject *obj = reinterpret_cast<BaseObject *>(addr);
    RefField<false> oldField(obj);
    TestBaseObjectOperator operatorImpl;
    BaseObject::RegisterDynamic(&operatorImpl);
    operatorImpl.SetValidObject(true);
    Heap::GetHeap().SetGCReason(GCReason::GC_REASON_YOUNG);
    operatorImpl.SetSize(BaseObject::BaseObjectSize());
    AfterMarkVisitor visitor;
    visitor.VerifyRefImpl(nullptr, oldField);
    ASSERT_TRUE(Heap::GetHeap().GetGCReason() == GCReason::GC_REASON_YOUNG);
    ASSERT_TRUE(Heap::IsTaggedObject(oldField.GetFieldValue()));

    AfterMarkVisitor<false> visitor1;
    visitor1.VerifyRefImpl(nullptr, oldField);
    ASSERT_TRUE(Heap::IsTaggedObject(oldField.GetFieldValue()));
}

HWTEST_F_L0(VerificationTest, VerifyRefImplTest1)
{
    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::NONMOVABLE_OBJECT, true);
    BaseObject *obj = reinterpret_cast<BaseObject *>(addr);
    RefField<false> oldField(obj);
    TestBaseObjectOperator operatorImpl;
    BaseObject::RegisterDynamic(&operatorImpl);
    operatorImpl.SetValidObject(true);
    Heap::GetHeap().SetGCReason(GCReason::GC_REASON_YOUNG);
    operatorImpl.SetSize(BaseObject::BaseObjectSize());
    AfterMarkVisitor visitor;
    visitor.VerifyRefImpl(obj, oldField);
    ASSERT_TRUE(Heap::GetHeap().GetGCReason() == GCReason::GC_REASON_YOUNG);
    ASSERT_TRUE(Heap::IsTaggedObject(oldField.GetFieldValue()));
}

static BaseObject* testObj = nullptr;
static void CustomVisitRoot(const RefFieldVisitor& visitorFunc)
{
    RefField<> field(testObj);
    visitorFunc(field);
}
HWTEST_F_L0(VerificationTest, IterateRemarked_VerifyAllRefs)
{
    RegionalHeap regionalHeap;
    VerifyIterator verify(regionalHeap);
    AfterForwardVisitor visitor;
    std::unordered_set<BaseObject*> markSet;
    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::NONMOVABLE_OBJECT, true);
    testObj = reinterpret_cast<BaseObject*>(addr);
    markSet.insert(testObj);

    verify.IterateRemarked<CustomVisitRoot>(visitor, markSet, true);
    verify.IterateRemarked<CustomVisitRoot>(visitor, markSet, false);
    EXPECT_EQ(markSet.size(), 1);
    EXPECT_TRUE(markSet.find(testObj) != markSet.end());
}
} // namespace common::test

