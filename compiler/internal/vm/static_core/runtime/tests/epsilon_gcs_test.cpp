/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>

#include "runtime/include/thread_scopes.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/mem/gc/epsilon-g1/epsilon-g1.h"

#include "test_utils.h"

namespace ark::mem {

// NOLINTBEGIN(readability-magic-numbers)
class EpsilonGCTest : public testing::Test {
public:
    explicit EpsilonGCTest() : EpsilonGCTest(CreateDefaultOptions()) {}

    explicit EpsilonGCTest(const RuntimeOptions &options)
    {
        Runtime::Create(options);
    }

    ~EpsilonGCTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(EpsilonGCTest);
    NO_MOVE_SEMANTIC(EpsilonGCTest);

    static RuntimeOptions CreateDefaultOptions(GCType gcType = GCType::EPSILON_GC)
    {
        constexpr size_t HEAP_SIZE_LIMIT_FOR_TEST = 16_MB;
        RuntimeOptions options;
        switch (gcType) {
            case GCType::EPSILON_GC:
                options.SetGcType("epsilon");
                break;
            case GCType::EPSILON_G1_GC:
                options.SetGcType("epsilon-g1");
                options.SetGcWorkersCount(0);
                options.SetG1PromotionRegionAliveRate(100U);
                break;
            default:
                UNREACHABLE();
        }
        options.SetHeapSizeLimit(HEAP_SIZE_LIMIT_FOR_TEST);
        options.SetLoadRuntimes({"core"});
        options.SetRunGcInPlace(true);
        options.SetCompilerEnableJit(false);
        options.SetGcTriggerType("debug-never");
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        return options;
    }

    template <typename T>
    T *GetAllocator()
    {
        static_assert(std::is_same<T, ObjectAllocatorNoGen<>>::value || std::is_same<T, ObjectAllocatorG1<>>::value);
        Runtime *runtime = Runtime::GetCurrent();
        GC *gc = runtime->GetPandaVM()->GetGC();
        return static_cast<T *>(gc->GetObjectAllocator());
    }

    template <typename T>
    std::vector<ObjectHeader *> AllocObjectsForTest()
    {
        std::vector<ObjectHeader *> objVector;

        objVector.emplace_back(ObjectAllocator::AllocArray(GetAllocator<T>()->GetRegularObjectMaxSize() * 0.8F,
                                                           ClassRoot::ARRAY_U8, false));
        objVector.emplace_back(
            ObjectAllocator::AllocArray(GetAllocator<T>()->GetLargeObjectMaxSize(), ClassRoot::ARRAY_U8, false));
        objVector.emplace_back(ObjectAllocator::AllocArray(GetAllocator<T>()->GetRegularObjectMaxSize() * 0.5F,
                                                           ClassRoot::ARRAY_U8, true));
        objVector.emplace_back(ObjectAllocator::AllocString(GetAllocator<T>()->GetRegularObjectMaxSize() * 0.8F));
        objVector.emplace_back(ObjectAllocator::AllocString(GetAllocator<T>()->GetLargeObjectMaxSize()));

        return objVector;
    }

    static constexpr size_t NUM_OF_ELEMS_CHECKED = 5;
};

class EpsilonG1GCTest : public EpsilonGCTest {
public:
    explicit EpsilonG1GCTest() : EpsilonGCTest(CreateDefaultOptions(GCType::EPSILON_G1_GC)) {}

    NO_COPY_SEMANTIC(EpsilonG1GCTest);
    NO_MOVE_SEMANTIC(EpsilonG1GCTest);

    ~EpsilonG1GCTest() override = default;

    static constexpr size_t GetHumongousStringLength()
    {
        // Total string size will be G1_REGION_SIZE + sizeof(String).
        // It is enough to make it humongous.
        return G1_REGION_SIZE;
    }

    static constexpr size_t YOUNG_OBJECT_SIZE =
        AlignUp(static_cast<size_t>(G1_REGION_SIZE * 0.8F), DEFAULT_ALIGNMENT_IN_BYTES);
};

TEST_F(EpsilonGCTest, TestObjectsAllocation)
{
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    ASSERT_EQ(Runtime::GetCurrent()->GetPandaVM()->GetGC()->GetType(), GCType::EPSILON_GC);

    // Allocation of objects of different sizes in movable/non-movable spaces and checking that they are accessible
    std::vector<ObjectHeader *> allocatedObjects = AllocObjectsForTest<ObjectAllocatorNoGen<>>();
    for (size_t i = 0; i < allocatedObjects.size(); ++i) {
        ASSERT_NE(allocatedObjects[i], nullptr);
        if (i < 3U) {  // First 3 elements are coretypes::Array
            ASSERT_NE(static_cast<coretypes::Array *>(allocatedObjects[i])->GetLength(), 0);
        } else {
            ASSERT_NE(static_cast<coretypes::String *>(allocatedObjects[i])->GetLength(), 0);
        }
    }
}

TEST_F(EpsilonGCTest, TestOOMAndGCTriggering)
{
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    GC *gc = Runtime::GetCurrent()->GetPandaVM()->GetGC();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> hs(thread);
    VMHandle<coretypes::Array> nonmovable = VMHandle<coretypes::Array>(
        thread, ObjectAllocator::AllocArray(NUM_OF_ELEMS_CHECKED, ClassRoot::ARRAY_STRING, true));
    std::vector<VMHandle<coretypes::String>> strings;
    coretypes::String *objString;

    // Alloc objects until OOM
    // First NUM_OF_ELEMS_CHECKED objects are added to nonmovable array to check their addresses after triggered GC
    do {
        objString =
            ObjectAllocator::AllocString(GetAllocator<ObjectAllocatorNoGen<>>()->GetRegularObjectMaxSize() * 0.8F);
        if (strings.size() < NUM_OF_ELEMS_CHECKED) {
            strings.emplace_back(thread, objString);
            size_t lastElemIndx = strings.size() - 1;
            nonmovable->Set<ObjectHeader *>(lastElemIndx, strings[lastElemIndx].GetPtr());
        }
    } while (objString != nullptr);
    VMHandle<coretypes::String> objAfterOom = VMHandle<coretypes::String>(
        thread, ObjectAllocator::AllocString(GetAllocator<ObjectAllocatorNoGen<>>()->GetRegularObjectMaxSize() * 0.8F));
    ASSERT_EQ(objAfterOom.GetPtr(), nullptr) << "Expected OOM";
    ASSERT_EQ(strings.size(), NUM_OF_ELEMS_CHECKED);

    // Checking if the addresses are correct before triggering GC
    for (size_t i = 0; i < strings.size(); ++i) {
        ASSERT_EQ(strings[i].GetPtr(), nonmovable->Get<ObjectHeader *>(i));
    }

    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::OOM_CAUSE);
        task.Run(*gc);
    }
    // Checking if the addresses are correct after triggering GC
    for (size_t i = 0; i < strings.size(); ++i) {
        ASSERT_EQ(strings[i].GetPtr(), nonmovable->Get<ObjectHeader *>(i));
    }

    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::EXPLICIT_CAUSE);
        task.Run(*gc);
    }
    // Checking if the addresses are correct after triggering GC
    for (size_t i = 0; i < strings.size(); ++i) {
        ASSERT_EQ(strings[i].GetPtr(), nonmovable->Get<ObjectHeader *>(i));
    }

    // Trying to alloc after triggering GC
    VMHandle<coretypes::String> objAfterTriggeredGc = VMHandle<coretypes::String>(
        thread, ObjectAllocator::AllocString(GetAllocator<ObjectAllocatorNoGen<>>()->GetRegularObjectMaxSize() * 0.8F));
    ASSERT_EQ(objAfterOom.GetPtr(), nullptr) << "Expected OOM";
}

TEST_F(EpsilonG1GCTest, TestObjectsAllocation)
{
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    ASSERT_EQ(Runtime::GetCurrent()->GetPandaVM()->GetGC()->GetType(), GCType::EPSILON_G1_GC);

    // Allocation of objects of different sizes in movable/non-movable spaces and checking that they are accessible and
    // are in correct regions
    std::vector<ObjectHeader *> allocatedObjects = AllocObjectsForTest<ObjectAllocatorG1<>>();
    ASSERT_EQ(allocatedObjects.size(), NUM_OF_ELEMS_CHECKED);
    for (auto &allocatedObject : allocatedObjects) {
        ASSERT_NE(allocatedObject, nullptr);
    }

    ASSERT_TRUE(ObjectToRegion(allocatedObjects[0U])->HasFlag(RegionFlag::IS_EDEN));
    ASSERT_FALSE(ObjectToRegion(allocatedObjects[0U])->HasFlag(RegionFlag::IS_NONMOVABLE));
    ASSERT_TRUE(ObjectToRegion(allocatedObjects[1U])->HasFlag(RegionFlag::IS_LARGE_OBJECT));
    ASSERT_TRUE(ObjectToRegion(allocatedObjects[2U])->HasFlag(RegionFlag::IS_NONMOVABLE));
    ASSERT_TRUE(ObjectToRegion(allocatedObjects[3U])->HasFlag(RegionFlag::IS_EDEN));
    ASSERT_TRUE(ObjectToRegion(allocatedObjects[4U])->HasFlag(RegionFlag::IS_LARGE_OBJECT));
}

TEST_F(EpsilonG1GCTest, TestOOM)
{
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> hs(thread);
    auto objectAllocatorG1 = thread->GetVM()->GetGC()->GetObjectAllocator();
    VMHandle<coretypes::String> objString;
    VMHandle<coretypes::String> objStringHuge;

    while (reinterpret_cast<GenerationalSpaces *>(objectAllocatorG1->GetHeapSpace())->GetCurrentFreeYoungSize() >=
           YOUNG_OBJECT_SIZE) {
        objString = VMHandle<coretypes::String>(thread, ObjectAllocator::AllocString(YOUNG_OBJECT_SIZE));
        ASSERT_NE(objString.GetPtr(), nullptr) << "Cannot allocate an object in young space when heap is not full";
    }
    objString = VMHandle<coretypes::String>(thread, ObjectAllocator::AllocString(YOUNG_OBJECT_SIZE));
    ASSERT_EQ(objString.GetPtr(), nullptr) << "Expected OOM in young space";

    size_t hugeStringSize = GetHumongousStringLength();
    size_t newRegionForHugeStringSize =
        Region::RegionSize(AlignUp(hugeStringSize, GetAlignmentInBytes(DEFAULT_ALIGNMENT)), G1_REGION_SIZE);
    while (reinterpret_cast<GenerationalSpaces *>(objectAllocatorG1->GetHeapSpace())->GetCurrentFreeTenuredSize() >
           newRegionForHugeStringSize) {
        objStringHuge = VMHandle<coretypes::String>(thread, ObjectAllocator::AllocString(hugeStringSize));
        ASSERT_NE(objStringHuge.GetPtr(), nullptr)
            << "Cannot allocate an object in tenured space when heap is not full";
    }
    objStringHuge = VMHandle<coretypes::String>(thread, ObjectAllocator::AllocString(hugeStringSize));
    ASSERT_EQ(objStringHuge.GetPtr(), nullptr) << "Expected OOM in tenured space";
}

TEST_F(EpsilonG1GCTest, TestGCTriggering)
{
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    GC *gc = Runtime::GetCurrent()->GetPandaVM()->GetGC();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> hs(thread);
    ASSERT_EQ(gc->GetType(), GCType::EPSILON_G1_GC);

    VMHandle<coretypes::Array> nonmovable;
    ObjectHeader *obj;
    ObjectHeader *hugeObj;
    uintptr_t objAddr;
    uintptr_t hugeObjAddr;

    nonmovable = VMHandle<coretypes::Array>(thread, ObjectAllocator::AllocArray(2U, ClassRoot::ARRAY_STRING, true));
    obj = ObjectAllocator::AllocObjectInYoung();
    hugeObj = ObjectAllocator::AllocString(GetHumongousStringLength());
    nonmovable->Set(0, obj);
    nonmovable->Set(1, hugeObj);
    objAddr = ToUintPtr(obj);
    hugeObjAddr = ToUintPtr(hugeObj);
    ASSERT_TRUE(ObjectToRegion(obj)->HasFlag(RegionFlag::IS_EDEN));
    ASSERT_TRUE(ObjectToRegion(hugeObj)->HasFlag(RegionFlag::IS_OLD));
    ASSERT_TRUE(ObjectToRegion(hugeObj)->HasFlag(RegionFlag::IS_LARGE_OBJECT));

    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }
    // Check obj was not propagated to tenured and huge_obj was not moved to any other region
    obj = nonmovable->Get<ObjectHeader *>(0);
    hugeObj = nonmovable->Get<ObjectHeader *>(1);
    ASSERT_EQ(objAddr, ToUintPtr(obj));
    ASSERT_EQ(hugeObjAddr, ToUintPtr(hugeObj));
    ASSERT_TRUE(ObjectToRegion(obj)->HasFlag(RegionFlag::IS_EDEN));
    ASSERT_TRUE(ObjectToRegion(hugeObj)->HasFlag(RegionFlag::IS_OLD));
    ASSERT_TRUE(ObjectToRegion(hugeObj)->HasFlag(RegionFlag::IS_LARGE_OBJECT));

    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::EXPLICIT_CAUSE);  // run full GC
        task.Run(*gc);
    }
    // Check obj was not propagated to tenured and huge_obj was not propagated to any other region after full gc
    obj = nonmovable->Get<ObjectHeader *>(0);
    hugeObj = nonmovable->Get<ObjectHeader *>(1);
    ASSERT_EQ(objAddr, ToUintPtr(obj));
    ASSERT_EQ(hugeObjAddr, ToUintPtr(hugeObj));
    ASSERT_TRUE(ObjectToRegion(obj)->HasFlag(RegionFlag::IS_EDEN));
    ASSERT_TRUE(ObjectToRegion(hugeObj)->HasFlag(RegionFlag::IS_OLD));
    ASSERT_TRUE(ObjectToRegion(hugeObj)->HasFlag(RegionFlag::IS_LARGE_OBJECT));

    // Check the objects are accessible
    ASSERT_NE(nullptr, obj->ClassAddr<Class>());
    ASSERT_NE(nullptr, hugeObj->ClassAddr<Class>());
}
// NOLINTEND(readability-magic-numbers)

}  // namespace ark::mem
