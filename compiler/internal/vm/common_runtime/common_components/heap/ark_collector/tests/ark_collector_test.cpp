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

#include "common_components/tests/test_helper.h"

#include "common_components/heap/ark_collector/ark_collector.h"
#include "common_components/heap/ark_collector/ark_collector.cpp"
#include "common_components/heap/collector/collector_proxy.h"
#include "common_components/heap/heap_manager.h"
#include "common_components/heap/allocator/region_desc.h"
#include "common_components/mutator/mutator_manager-inl.h"
#include "common_interfaces/objects/base_object.h"
#include "gtest/gtest.h"
#include <cstddef>

using namespace common;

namespace common::test {
using SuspensionType = MutatorBase::SuspensionType;
class ArkCollectorTest : public common::test::BaseTestWithScope {
protected:
    static void SetUpTestCase()
    {
        BaseRuntime::GetInstance()->InitFromDynamic();
    }

    static void TearDownTestCase()
    {}

    void SetUp() override
    {
        MutatorManager::Instance().CreateRuntimeMutator(ThreadType::ARK_PROCESSOR);
    }

    void TearDown() override
    {
        MutatorManager::Instance().DestroyRuntimeMutator(ThreadType::ARK_PROCESSOR);
    }
};

std::unique_ptr<ArkCollector> GetArkCollector()
{
    CollectorResources &resources = Heap::GetHeap().GetCollectorResources();
    Allocator &allocator = Heap::GetHeap().GetAllocator();

    return std::make_unique<ArkCollector>(allocator, resources);
}

HWTEST_F_L0(ArkCollectorTest, IsUnmovableFromObjectTest0)
{
    std::unique_ptr<ArkCollector> arkCollector = GetArkCollector();
    ASSERT_TRUE(arkCollector != nullptr);

    BaseObject *obj = nullptr;
    EXPECT_FALSE(arkCollector->IsUnmovableFromObject(obj));
}

HWTEST_F_L0(ArkCollectorTest, IsUnmovableFromObjectTest1)
{
    std::unique_ptr<ArkCollector> arkCollector = GetArkCollector();
    ASSERT_TRUE(arkCollector != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject *obj = reinterpret_cast<BaseObject *>(addr);

    new (obj) BaseObject();

    EXPECT_FALSE(arkCollector->IsUnmovableFromObject(obj));
}

HWTEST_F_L0(ArkCollectorTest, IsUnmovableFromObjectTest2)
{
    std::unique_ptr<ArkCollector> arkCollector = GetArkCollector();
    ASSERT_TRUE(arkCollector != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::NONMOVABLE_OBJECT, true);
    BaseObject *obj = reinterpret_cast<BaseObject *>(addr);

    new (obj) BaseObject();

    RegionDesc *region = RegionDesc::GetAliveRegionDescAt(addr);

    bool isMarked = region->GetOrAllocResurrectBitmap()->MarkBits(0);
    region->SetResurrectedRegionFlag(1);
    region->SetRegionType(RegionDesc::RegionType::EXEMPTED_FROM_REGION);

    EXPECT_FALSE(isMarked);

    EXPECT_TRUE(arkCollector->IsUnmovableFromObject(obj));
}

HWTEST_F_L0(ArkCollectorTest, ForwardUpdateRawRefTest0)
{
    std::unique_ptr<ArkCollector> arkCollector = GetArkCollector();
    ASSERT_TRUE(arkCollector != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject *obj = reinterpret_cast<BaseObject *>(addr);

    new (obj) BaseObject();

    common::ObjectRef root = {obj};

    BaseObject *oldObj = arkCollector->ForwardUpdateRawRef(root);
    EXPECT_EQ(oldObj, obj);
}

void FlipTest()
{
    MutatorManager &mutatorManager = MutatorManager::Instance();
    ThreadHolder::CreateAndRegisterNewThreadHolder(nullptr);
    bool stwCallbackExecuted = false;
    auto stwTest = [&mutatorManager, &stwCallbackExecuted]() {
        EXPECT_TRUE(mutatorManager.WorldStopped());
        stwCallbackExecuted = true;
    };
    FlipFunction mutatorTest = [&mutatorManager, &stwCallbackExecuted](Mutator &mutator) {
        EXPECT_TRUE(mutator.HasSuspensionRequest(SuspensionType::SUSPENSION_FOR_RUNNING_CALLBACK));
        EXPECT_FALSE(mutatorManager.WorldStopped());
        EXPECT_TRUE(stwCallbackExecuted);
    };
    STWParam param;
    param.stwReason = "flip-test";
    mutatorManager.FlipMutators(param, stwTest, &mutatorTest);
}

HWTEST_F_L0(ArkCollectorTest, FlipTest)
{
    std::thread t1(FlipTest);
    t1.join();
}

HWTEST_F_L0(ArkCollectorTest, IsUnmovableFromObject_ReturnsFalseForNullptr)
{
    std::unique_ptr<ArkCollector> arkCollector = GetArkCollector();
    ASSERT_TRUE(arkCollector != nullptr);

    BaseObject* obj = nullptr;
    EXPECT_FALSE(arkCollector->IsUnmovableFromObject(obj));
}

class TestableArkCollector : public ArkCollector {
public:
    using ArkCollector::ForwardObject;

    explicit TestableArkCollector(Allocator& allocator, CollectorResources& resources)
        : ArkCollector(allocator, resources), currentGCPhase_(GCPhase::GC_PHASE_COPY) {}

    void SetCurrentGCPhaseForTest(GCPhase phase)
    {
        currentGCPhase_ = phase;
    }

    GCPhase GetCurrentGCPhaseForTest() const
    {
        return currentGCPhase_;
    }

private:
    GCPhase currentGCPhase_;
};


class DummyObject : public BaseObject {
public:
    const common::TypeInfo* GetTypeInfo() const { return nullptr; }
    size_t GetSize() const { return sizeof(DummyObject); }

    void SetClass(uintptr_t cls)
    {
        stateWord_.StoreStateWord(static_cast<StateWordType>(cls));
    }

private:
    class BaseStateWord {
    public:
        using StateWordType = uint64_t;

        void StoreStateWord(StateWordType word)
        {
            stateWord_ = word;
        }

        StateWordType LoadStateWord() const
        {
            return stateWord_;
        }

    private:
        StateWordType stateWord_{0};
    };

    BaseStateWord stateWord_;
};

class TestBaseObjectOperator : public common::BaseObjectOperatorInterfaces {
public:
    bool IsValidObject([[maybe_unused]] const BaseObject *object) const override { return true; }
    void ForEachRefField(const BaseObject *object, const common::RefFieldVisitor &visitor) const override {}
    size_t GetSize(const BaseObject *object) const override{ return size_; }
    size_t ForEachRefFieldAndGetSize(
        const BaseObject *object,
        const common::RefFieldVisitor &visitor) const override
    {
        return 0;
    }
    BaseObject *GetForwardingPointer(const BaseObject *object) const override
    {
        return fwdPtr_;
    }
    void SetForwardingPointerAfterExclusive(BaseObject *object, BaseObject *fwdPtr) override
    {
        fwdPtr_ = fwdPtr;
    }
    void SetSize(size_t size) { size_ = size; }
    void ForEachRefFieldSkipReferent(const BaseObject *object, const RefFieldVisitor &visitor) const override {}
    void IterateXRef(const BaseObject *object, const RefFieldVisitor &visitor) const override {}
private:
    size_t size_ = 0;
    BaseObject *fwdPtr_ = nullptr;
};
HWTEST_F_L0(ArkCollectorTest, ForwardObject_WithUnmovedObject_ReturnsSameAddress)
{
    std::unique_ptr<ArkCollector> arkCollector = GetArkCollector();
    ASSERT_TRUE(arkCollector != nullptr);

    TestableArkCollector* testableCollector = reinterpret_cast<TestableArkCollector*>(arkCollector.get());

    testableCollector->SetCurrentGCPhaseForTest(GCPhase::GC_PHASE_COPY);
    EXPECT_EQ(testableCollector->GetCurrentGCPhaseForTest(), GCPhase::GC_PHASE_COPY);
}

HWTEST_F_L0(ArkCollectorTest, MarkingRefField_TEST1)
{
    constexpr uint64_t TAG_BITS_SHIFT = 48;
    constexpr uint64_t TAG_MARK = 0xFFFFULL << TAG_BITS_SHIFT;
    constexpr uint64_t TAG_SPECIAL = 0x02ULL;
    constexpr uint64_t TAG_BOOLEAN = 0x04ULL;
    constexpr uint64_t TAG_HEAP_OBJECT_MASK = TAG_MARK | TAG_SPECIAL | TAG_BOOLEAN;
    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject *obj = reinterpret_cast<BaseObject *>(addr | TAG_HEAP_OBJECT_MASK);
    RefField<> field(obj);

    GlobalMarkStack globalMarkStack;
    ParallelMarkingMonitor monitor(0, 0);
    ParallelLocalMarkStack markStack(&globalMarkStack, &monitor);
    WeakStack weakStack;
    MarkingRefField(nullptr, field, markStack, weakStack, GCReason::GC_REASON_YOUNG);
    EXPECT_FALSE(Heap::IsTaggedObject(field.GetFieldValue()));
}

HWTEST_F_L0(ArkCollectorTest, MarkingRefField_TEST2)
{
    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    RegionDesc* region = RegionDesc::GetRegionDescAt(addr);
    region->SetRegionType(RegionDesc::RegionType::ALIVE_REGION_FIRST);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RefField<false> field(obj);

    GlobalMarkStack globalMarkStack;
    ParallelMarkingMonitor monitor(0, 0);
    ParallelLocalMarkStack markStack(&globalMarkStack, &monitor);
    WeakStack weakStack;
    MarkingRefField(nullptr, field, markStack, weakStack, GCReason::GC_REASON_APPSPAWN);
    EXPECT_FALSE(region->IsInOldSpace());
    BaseObject *temp;
    while (markStack.Pop(&temp)) {}
}

HWTEST_F_L0(ArkCollectorTest, MarkingRefField_TEST3)
{
    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    RegionDesc* region = RegionDesc::GetRegionDescAt(addr);
    region->SetRegionType(RegionDesc::RegionType::OLD_REGION);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RefField<false> field(obj);

    GlobalMarkStack globalMarkStack;
    ParallelMarkingMonitor monitor(0, 0);
    ParallelLocalMarkStack markStack(&globalMarkStack, &monitor);
    WeakStack weakStack;
    MarkingRefField(nullptr, field, markStack, weakStack, GCReason::GC_REASON_APPSPAWN);
    EXPECT_TRUE(region->IsInOldSpace());
    BaseObject *temp;
    while (markStack.Pop(&temp)) {}
}

HWTEST_F_L0(ArkCollectorTest, MarkingRefField_TEST4)
{
    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    RegionDesc* region = RegionDesc::GetRegionDescAt(addr);
    region->SetRegionType(RegionDesc::RegionType::ALIVE_REGION_FIRST);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RefField<false> field(obj);

    GlobalMarkStack globalMarkStack;
    ParallelMarkingMonitor monitor(0, 0);
    ParallelLocalMarkStack markStack(&globalMarkStack, &monitor);
    WeakStack weakStack;
    MarkingRefField(nullptr, field, markStack, weakStack, GCReason::GC_REASON_YOUNG);
    EXPECT_FALSE(region->IsInOldSpace());
    BaseObject *temp;
    while (markStack.Pop(&temp)) {}
}

HWTEST_F_L0(ArkCollectorTest, MarkingRefField_TEST5)
{
    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    RegionDesc* region = RegionDesc::GetRegionDescAt(addr);
    region->SetRegionType(RegionDesc::RegionType::OLD_REGION);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RefField<false> field(obj);

    GlobalMarkStack globalMarkStack;
    ParallelMarkingMonitor monitor(0, 0);
    ParallelLocalMarkStack markStack(&globalMarkStack, &monitor);
    WeakStack weakStack;
    MarkingRefField(nullptr, field, markStack, weakStack, GCReason::GC_REASON_YOUNG);
    EXPECT_TRUE(region->IsInOldSpace());
}

HWTEST_F_L0(ArkCollectorTest, MarkingRefField_TEST6)
{
    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    RegionDesc* region = RegionDesc::GetRegionDescAt(addr);
    region->SetRegionAllocPtr(addr - 1);
    region->SetMarkingLine();
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RefField<false> field(obj);

    GlobalMarkStack globalMarkStack;
    ParallelMarkingMonitor monitor(0, 0);
    ParallelLocalMarkStack markStack(&globalMarkStack, &monitor);
    MarkingRefField(obj, obj, field, markStack, region);
    EXPECT_TRUE(region->IsNewObjectSinceMarking(obj));
}
class TestCreateMarkingArkCollector : public MarkingCollector {
public:
    using MarkingCollector::SetGCReason;
    explicit TestCreateMarkingArkCollector(Allocator& allocator, CollectorResources& resources)
        : MarkingCollector(allocator, resources) {}
    BaseObject* ForwardObject(BaseObject*) override { return nullptr; }
    bool IsFromObject(BaseObject*) const override { return false; }
    bool IsUnmovableFromObject(BaseObject*) const override { return false; }
    BaseObject* FindToVersion(BaseObject* obj) const override { return nullptr; }
    bool TryUpdateRefField(BaseObject*, RefField<>&, BaseObject*&) const override { return false; }
    bool TryForwardRefField(BaseObject*, RefField<>&, BaseObject*&) const override { return false; }
    bool TryUntagRefField(BaseObject*, RefField<>&, BaseObject*&) const override { return false; }
    RefField<> GetAndTryTagRefField(BaseObject*) const override { return RefField<>(nullptr); }
    bool IsOldPointer(RefField<>&) const override { return false; }
    void AddRawPointerObject(BaseObject*) override {}
    void RemoveRawPointerObject(BaseObject*) override {}
    bool MarkObject(BaseObject* obj) const override { return false; }
    BaseObject *CopyObjectAfterExclusive(BaseObject *obj) override
    {
        return nullptr;
    }
    void DoGarbageCollection() override {}
    bool IsCurrentPointer(RefField<>&) const override { return false; }
    MarkingRefFieldVisitor CreateMarkingObjectRefFieldsVisitor(ParallelLocalMarkStack &workStack,
                                                               WeakStack &weakStack) override
    {
        return MarkingRefFieldVisitor();
    }
};

HWTEST_F_L0(ArkCollectorTest, CreateMarkingObjectRefFieldsVisitor_TEST1)
{
    std::unique_ptr<ArkCollector> arkCollector = GetArkCollector();
    ASSERT_TRUE(arkCollector != nullptr);

    GlobalMarkStack globalMarkStack;
    ParallelMarkingMonitor monitor(0, 0);
    ParallelLocalMarkStack markStack(&globalMarkStack, &monitor);
    WeakStack weakStack;
    TestCreateMarkingArkCollector* collector = reinterpret_cast<TestCreateMarkingArkCollector*>(arkCollector.get());
    collector->SetGCReason(GCReason::GC_REASON_YOUNG);
    auto visitor = arkCollector->CreateMarkingObjectRefFieldsVisitor(markStack, weakStack);
    EXPECT_TRUE(visitor.GetRefFieldVisitor() != nullptr);
}

HWTEST_F_L0(ArkCollectorTest, FixRefField_TEST1)
{
    std::unique_ptr<ArkCollector> arkCollector = GetArkCollector();
    ASSERT_TRUE(arkCollector != nullptr);

    constexpr uint64_t TAG_BITS_SHIFT = 48;
    constexpr uint64_t TAG_MARK = 0xFFFFULL << TAG_BITS_SHIFT;
    constexpr uint64_t TAG_SPECIAL = 0x02ULL;
    constexpr uint64_t TAG_BOOLEAN = 0x04ULL;
    constexpr uint64_t TAG_HEAP_OBJECT_MASK = TAG_MARK | TAG_SPECIAL | TAG_BOOLEAN;
    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    BaseObject *obj = reinterpret_cast<BaseObject *>(addr | TAG_HEAP_OBJECT_MASK);
    RefField<> field(obj);
    arkCollector->FixRefField(obj, field);
    EXPECT_FALSE(Heap::IsTaggedObject(field.GetFieldValue()));
}

HWTEST_F_L0(ArkCollectorTest, FixRefField_TEST2)
{
    std::unique_ptr<ArkCollector> arkCollector = GetArkCollector();
    ASSERT_TRUE(arkCollector != nullptr);

    BaseObject* obj = reinterpret_cast<BaseObject*>(0);
    RefField<> field(obj);
    arkCollector->FixRefField(obj, field);
    EXPECT_FALSE(Heap::IsHeapAddress(obj));
}

HWTEST_F_L0(ArkCollectorTest, FixRefField_RefNotInFrom)
{
    // refRegion Not In IsFrom Branch
    std::unique_ptr<ArkCollector> arkCollector = GetArkCollector();
    ASSERT_TRUE(arkCollector != nullptr);
    HeapAddress addr = HeapManager::Allocate(8, AllocType::MOVEABLE_OBJECT);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RegionDesc *refRegion =  RegionDesc::GetRegionDescAt(
    reinterpret_cast<uintptr_t>(obj));
    refRegion->SetRegionType(RegionDesc::RegionType::TO_REGION);
    RefField<> field(obj);
    arkCollector->FixRefField(obj, field);
    // Because it is in TO_REGION, it will not be moved, so the results are consistent
    EXPECT_EQ((uint64_t)field.GetFieldValue(), (uint64_t)obj);
}

HWTEST_F_L0(ArkCollectorTest, FixRefField_RefInFrom)
{
    // refRegion In IsFrom Branch
    std::unique_ptr<ArkCollector> arkCollector = GetArkCollector();
    ASSERT_TRUE(arkCollector != nullptr);
    TestBaseObjectOperator operatorImpl;
    BaseObject::RegisterDynamic(&operatorImpl);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject) * 3, AllocType::MOVEABLE_OBJECT);
    HeapAddress fwdAddr = HeapManager::Allocate(sizeof(BaseObject) * 3, AllocType::MOVEABLE_OBJECT);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    BaseObject* fwdObj = reinterpret_cast<BaseObject*>(fwdAddr);
    RegionDesc *refRegion =  RegionDesc::GetRegionDescAt(
    reinterpret_cast<uintptr_t>(obj));
    refRegion->SetRegionType(RegionDesc::RegionType::FROM_REGION);
    obj->SetForwardState(BaseStateWord::ForwardState::FORWARDING);
    obj->SetForwardingPointerAfterExclusive(fwdObj);
    RefField<> field(obj);
    arkCollector->FixRefField(obj, field);
    // Because it is in FromSpace, it will be forwarded to a new pointer
    EXPECT_EQ((uint64_t)field.GetFieldValue(), (uint64_t)fwdObj);
}

HWTEST_F_L0(ArkCollectorTest, FixRefField_RefInRecent1)
{
    // refRegion InRecent
    std::unique_ptr<ArkCollector> arkCollector = GetArkCollector();
    ASSERT_TRUE(arkCollector != nullptr);
    HeapAddress addr = HeapManager::Allocate(8, AllocType::MOVEABLE_OBJECT);
    BaseObject* ref = reinterpret_cast<BaseObject*>(addr);
    RegionDesc *refRegion =  RegionDesc::GetRegionDescAt(
    reinterpret_cast<uintptr_t>(ref));
    refRegion->SetRegionType(RegionDesc::RegionType::RECENT_FULL_REGION);
    RefField<> field(ref);

    //  && objRegion InRecent
    HeapAddress addr2 = HeapManager::Allocate(8, AllocType::MOVEABLE_OLD_OBJECT);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr2);
    RegionDesc *objRegion =  RegionDesc::GetRegionDescAt(
    reinterpret_cast<uintptr_t>(obj));
    objRegion->SetRegionType(RegionDesc::RegionType::RECENT_FULL_REGION);
    objRegion->ClearRSet();
    arkCollector->FixRefField(obj, field);
    // refRegion in RecentSpace will not be forwarded (fwd),
    // because it belongs to the Recent generation
    // so the Remembered Set (Rset) will not be updated, resulting in false
    EXPECT_EQ((uint64_t)field.GetFieldValue(), (uint64_t)ref);
    EXPECT_EQ(objRegion->MarkRSetCardTable(obj), false);
}

HWTEST_F_L0(ArkCollectorTest, FixRefField_RefInRecent2)
{
    // refRegion InRecent
    std::unique_ptr<ArkCollector> arkCollector = GetArkCollector();
    ASSERT_TRUE(arkCollector != nullptr);
    HeapAddress addr = HeapManager::Allocate(8, AllocType::MOVEABLE_OBJECT);
    BaseObject* ref = reinterpret_cast<BaseObject*>(addr);
    RegionDesc *refRegion =  RegionDesc::GetRegionDescAt(
    reinterpret_cast<uintptr_t>(ref));
    refRegion->SetRegionType(RegionDesc::RegionType::RECENT_FULL_REGION);
    RefField<> field(ref);

    //  && !objRegion InRecent
    HeapAddress addr2 = HeapManager::Allocate(8, AllocType::MOVEABLE_OLD_OBJECT);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr2);
    RegionDesc *objRegion =  RegionDesc::GetRegionDescAt(
    reinterpret_cast<uintptr_t>(obj));
    objRegion->SetRegionType(RegionDesc::RegionType::TO_REGION);
    // && !objRegion->MarkRSetCardTable(obj)
    objRegion->ClearRSet();
    arkCollector->FixRefField(obj, field);
    EXPECT_EQ((uint64_t)field.GetFieldValue(), (uint64_t)ref);
    EXPECT_EQ(objRegion->MarkRSetCardTable(obj), true);
}

class TestStaticObject : public BaseObjectOperatorInterfaces {
public:
    size_t GetSize(const BaseObject *object) const override { return 0; }
    bool IsValidObject(const BaseObject *object) const override { return false; }
    void ForEachRefField(const BaseObject *object, const RefFieldVisitor &visitor) const override {}
    size_t ForEachRefFieldAndGetSize(
        const BaseObject *object,
        const common::RefFieldVisitor &visitor) const override
    {
        return 0;
    }
    void SetForwardingPointerAfterExclusive(BaseObject *object, BaseObject *fwdPtr) override {}
    BaseObject *GetForwardingPointer(const BaseObject *object) const override
    {
        return const_cast<BaseObject*>(object);
    }
    void ForEachRefFieldSkipReferent(const BaseObject *object, const RefFieldVisitor &visitor) const override {}
    void IterateXRef(const BaseObject *object, const RefFieldVisitor &visitor) const override {}
};

HWTEST_F_L0(ArkCollectorTest, ForwardUpdateRawRef_TEST1)
{
    std::unique_ptr<ArkCollector> arkCollector = GetArkCollector();
    ASSERT_TRUE(arkCollector != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    RegionDesc* region = RegionDesc::GetRegionDescAt(addr);
    region->SetRegionType(RegionDesc::RegionType::FROM_REGION);
    Mutator::GetMutator()->SetMutatorPhase(GCPhase::GC_PHASE_PRECOPY);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    EXPECT_FALSE(obj->IsForwarded());
    obj->SetForwardState(BaseStateWord::ForwardState::FORWARDED);
    obj->SetLanguageType(LanguageType::STATIC);
    EXPECT_TRUE(obj->IsForwarded());
    TestStaticObject staticObject;
    obj->RegisterStatic(&staticObject);

    alignas(RefField<>) char rootBuffer[sizeof(RefField<>)] = {0};
    ObjectRef& root = *new (rootBuffer) ObjectRef();
    root.object = obj;
    auto ret = arkCollector->ForwardUpdateRawRef(root);
    EXPECT_TRUE(arkCollector->IsFromObject(obj));
    EXPECT_EQ(ret, obj);
}

class TestForwardNullObject : public TestStaticObject {
public:
    BaseObject *GetForwardingPointer(const BaseObject *object) const override { return nullptr; }
};

HWTEST_F_L0(ArkCollectorTest, ForwardObject_TEST1)
{
    std::unique_ptr<ArkCollector> arkCollector = GetArkCollector();
    ASSERT_TRUE(arkCollector != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    RegionDesc* region = RegionDesc::GetRegionDescAt(addr);
    region->SetRegionType(RegionDesc::RegionType::FROM_REGION);
    Mutator::GetMutator()->SetMutatorPhase(GCPhase::GC_PHASE_PRECOPY);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    EXPECT_FALSE(obj->IsForwarded());
    obj->SetForwardState(BaseStateWord::ForwardState::FORWARDED);
    obj->SetLanguageType(LanguageType::STATIC);
    EXPECT_TRUE(obj->IsForwarded());
    TestForwardNullObject staticObject;
    obj->RegisterStatic(&staticObject);

    auto ret = arkCollector->ForwardObject(obj);
    EXPECT_EQ(ret, obj);
}

HWTEST_F_L0(ArkCollectorTest, CopyObjectImpl_TEST1)
{
    std::unique_ptr<ArkCollector> arkCollector = GetArkCollector();
    ASSERT_TRUE(arkCollector != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    RegionDesc* region = RegionDesc::GetRegionDescAt(addr);
    region->SetRegionType(RegionDesc::RegionType::FROM_REGION);
    Mutator::GetMutator()->SetMutatorPhase(GCPhase::GC_PHASE_PRECOPY);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    EXPECT_FALSE(obj->IsForwarded());
    obj->SetForwardState(BaseStateWord::ForwardState::FORWARDED);
    obj->SetLanguageType(LanguageType::STATIC);
    EXPECT_TRUE(obj->IsForwarded());
    TestForwardNullObject staticObject;
    obj->RegisterStatic(&staticObject);

    auto ret = arkCollector->CopyObjectImpl(obj);
    EXPECT_TRUE(ret == nullptr);
}

HWTEST_F_L0(ArkCollectorTest, TryUpdateRefFieldImpl_TEST1)
{
    std::unique_ptr<ArkCollector> arkCollector = GetArkCollector();
    ASSERT_TRUE(arkCollector != nullptr);

    RefField<> field(nullptr);
    BaseObject* obj = nullptr;
    bool ret = arkCollector->TryUpdateRefField(nullptr, field, obj);
    EXPECT_FALSE(ret);
}

HWTEST_F_L0(ArkCollectorTest, TryUpdateRefFieldImpl_TEST2)
{
    std::unique_ptr<ArkCollector> arkCollector = GetArkCollector();
    ASSERT_TRUE(arkCollector != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    RegionDesc* region = RegionDesc::GetRegionDescAt(addr);
    region->SetRegionType(RegionDesc::RegionType::FROM_REGION);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    RefField<false> field(obj);
    bool ret = arkCollector->TryUpdateRefField(nullptr, field, obj);
    EXPECT_FALSE(ret);
}

HWTEST_F_L0(ArkCollectorTest, TryUpdateRefFieldImpl_TEST3)
{
    std::unique_ptr<ArkCollector> arkCollector = GetArkCollector();
    ASSERT_TRUE(arkCollector != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    RegionDesc* region = RegionDesc::GetRegionDescAt(addr);
    region->SetRegionType(RegionDesc::RegionType::FROM_REGION);
    Mutator::GetMutator()->SetMutatorPhase(GCPhase::GC_PHASE_PRECOPY);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    EXPECT_FALSE(obj->IsForwarded());
    obj->SetForwardState(BaseStateWord::ForwardState::FORWARDED);
    obj->SetLanguageType(LanguageType::STATIC);
    EXPECT_TRUE(obj->IsForwarded());
    TestForwardNullObject staticObject;
    obj->RegisterStatic(&staticObject);

    RefField<false> field(obj);
    bool ret = arkCollector->TryForwardRefField(nullptr, field, obj);
    EXPECT_FALSE(ret);
}

HWTEST_F_L0(ArkCollectorTest, TryUpdateRefFieldImpl_TEST4)
{
    std::unique_ptr<ArkCollector> arkCollector = GetArkCollector();
    ASSERT_TRUE(arkCollector != nullptr);

    HeapAddress addr = HeapManager::Allocate(sizeof(BaseObject), AllocType::MOVEABLE_OBJECT, true);
    RegionDesc* region = RegionDesc::GetRegionDescAt(addr);
    region->SetRegionType(RegionDesc::RegionType::FROM_REGION);
    Mutator::GetMutator()->SetMutatorPhase(GCPhase::GC_PHASE_PRECOPY);
    BaseObject* obj = reinterpret_cast<BaseObject*>(addr);
    EXPECT_FALSE(obj->IsForwarded());
    obj->SetForwardState(BaseStateWord::ForwardState::FORWARDED);
    obj->SetLanguageType(LanguageType::STATIC);
    EXPECT_TRUE(obj->IsForwarded());
    TestStaticObject staticObject;
    obj->RegisterStatic(&staticObject);

    RefField<false> field(obj);
    bool ret = arkCollector->TryForwardRefField(nullptr, field, obj);
    EXPECT_TRUE(ret);
}
}  // namespace common::test
