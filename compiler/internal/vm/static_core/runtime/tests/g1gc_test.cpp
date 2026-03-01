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

#include <gtest/gtest.h>
#include <array>

#include "runtime/include/object_header.h"
#include "runtime/mem/tlab.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/class_linker.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/mem/vm_handle.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/include/coretypes/array.h"
#include "runtime/include/coretypes/line_string.h"
#include "runtime/mem/gc/card_table.h"
#include "runtime/mem/gc/g1/g1-allocator.h"
#include "runtime/mem/rem_set-inl.h"
#include "runtime/mem/region_space.h"
#include "runtime/mem/object_helpers.h"
#include "runtime/mem/gc/g1/g1-gc.h"
#include "runtime/mem/gc/gc_barrier_set.h"

#include "test_utils.h"

namespace ark::mem {

class G1GCTest : public testing::Test {
public:
    explicit G1GCTest() : G1GCTest(CreateDefaultOptions()) {}

    explicit G1GCTest(const RuntimeOptions &options)
    {
        Runtime::Create(options);
    }

    ~G1GCTest() override
    {
        Runtime::Destroy();
    }

    NO_COPY_SEMANTIC(G1GCTest);
    NO_MOVE_SEMANTIC(G1GCTest);

    static RuntimeOptions CreateDefaultOptions()
    {
        RuntimeOptions options;
        options.SetLoadRuntimes({"core"});
        options.SetGcType("g1-gc");
        options.SetRunGcInPlace(true);
        options.SetCompilerEnableJit(false);
        options.SetGcWorkersCount(0);
        options.SetAdaptiveTlabSize(false);
        // NOLINTNEXTLINE(readability-magic-numbers)
        options.SetG1PromotionRegionAliveRate(100U);
        options.SetGcTriggerType("debug-never");
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetExplicitConcurrentGcEnabled(false);
        options.SetG1NumberOfTenuredRegionsAtMixedCollection(2U);
        return options;
    }

    static constexpr size_t GetHumongousStringLength()
    {
        // Total string size will be DEFAULT_REGION_SIZE + sizeof(String).
        // It is enough to make it humongous.
        return DEFAULT_REGION_SIZE;
    }

    static constexpr size_t StringLengthFitIntoRegion(size_t numRegions)
    {
        return numRegions * DEFAULT_REGION_SIZE - sizeof(coretypes::String) - Region::HeadSize();
    }

    static size_t GetHumongousArrayLength(ClassRoot classRoot)
    {
        Runtime *runtime = Runtime::GetCurrent();
        LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        auto *arrayClass = runtime->GetClassLinker()->GetExtension(ctx)->GetClassRoot(classRoot);
        EXPECT_TRUE(arrayClass->IsArrayClass());
        if (!arrayClass->IsArrayClass()) {
            return 0;
        }
        // Total array size will be DEFAULT_REGION_SIZE * elem_size + sizeof(Array).
        // It is enough to make it humongous.
        size_t elemSize = arrayClass->GetComponentSize();
        ASSERT(elemSize != 0);
        return DEFAULT_REGION_SIZE / elemSize + 1;
    }

    ObjectAllocatorG1<> *GetAllocator()
    {
        Runtime *runtime = Runtime::GetCurrent();
        GC *gc = runtime->GetPandaVM()->GetGC();
        return static_cast<ObjectAllocatorG1<> *>(gc->GetObjectAllocator());
    }

    void ProcessDirtyCards(G1GC<PandaAssemblyLanguageConfig> *gc)
    {
        gc->EndConcurrentScopeRoutine();
        gc->ProcessDirtyCards();
        gc->StartConcurrentScopeRoutine();
    }

    template <class T>
    size_t GetYoungRegionsCount(G1GC<T> *gc)
    {
        return gc->GetG1ObjectAllocator()->GetYoungRegions().size();
    }

    auto GetMemStats(G1GC<PandaAssemblyLanguageConfig> *gc)
    {
        return gc->memStats_;
    }

    template <class T>
    size_t GetPromotedRegions(G1GC<T> *gc)
    {
        return gc->analytics_.GetPromotedRegions();
    }

    template <class T>
    size_t GetLiveObjects(G1GC<T> *gc)
    {
        return gc->analytics_.liveObjects_;
    }

    template <class T>
    size_t GetEvacuatedBytes(G1GC<T> *gc)
    {
        return gc->analytics_.GetEvacuatedBytes();
    }
};

class RemSetChecker : public GCListener {
public:
    explicit RemSetChecker(GC *gc, ObjectHeader *obj, ObjectHeader *ref)
        : gc_(static_cast<G1GC<PandaAssemblyLanguageConfig> *>(gc)),
          obj_(MTManagedThread::GetCurrent(), obj),
          ref_(MTManagedThread::GetCurrent(), ref)
    {
    }

    void GCPhaseStarted([[maybe_unused]] GCPhase phase) override {}

    void GCPhaseFinished(GCPhase phase) override
    {
        if (phase == GCPhase::GC_PHASE_MARK_YOUNG) {
            // We don't do it in phase started because refs from remset will be collected at marking stage
            Check();
        }
        if (phase == GCPhase::GC_PHASE_COLLECT_YOUNG_AND_MOVE) {
            // remset is not fully updated during mixed collection
            gc_->ProcessDirtyCards();
            Check();
        }
    }

private:
    void Check()
    {
        RemSet<> *remset = ObjectToRegion(ref_.GetPtr())->GetRemSet();
        ASSERT_NE(nullptr, remset);
        bool hasObject = false;
        ObjectHeader *object = obj_.GetPtr();
        remset->IterateOverObjects([object, &hasObject](ObjectHeader *obj) { hasObject |= object == obj; });
        // remset is not fully updated during mixed collection, check set of dirty objects
        ASSERT_TRUE(hasObject || gc_->HasRefFromRemset(object));
    }

private:
    G1GC<PandaAssemblyLanguageConfig> *gc_;
    VMHandle<ObjectHeader> obj_;
    VMHandle<ObjectHeader> ref_;
};

TEST_F(G1GCTest, TestAddrToRegion)
{
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    size_t humongousLen = GetHumongousArrayLength(ClassRoot::ARRAY_U8);
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    VMHandle<ObjectHeader> young(thread, ObjectAllocator::AllocArray(0, ClassRoot::ARRAY_U8, false));
    ASSERT_NE(nullptr, young.GetPtr());
    VMHandle<ObjectHeader> nonmovable(thread, ObjectAllocator::AllocArray(0, ClassRoot::ARRAY_U8, true));
    ASSERT_NE(nullptr, nonmovable.GetPtr());
    VMHandle<ObjectHeader> humongous(thread, ObjectAllocator::AllocArray(humongousLen, ClassRoot::ARRAY_U8, false));
    ASSERT_NE(nullptr, humongous.GetPtr());

    Region *youngRegion = ObjectToRegion(young.GetPtr());
    ASSERT_NE(nullptr, youngRegion);
    ASSERT_EQ(youngRegion, AddrToRegion(young.GetPtr()));
    bool hasYoungObj = false;
    youngRegion->IterateOverObjects(
        [&hasYoungObj, &young](ObjectHeader *obj) { hasYoungObj |= obj == young.GetPtr(); });
    ASSERT_TRUE(hasYoungObj);

    Region *nonmovableRegion = ObjectToRegion(nonmovable.GetPtr());
    ASSERT_NE(nullptr, nonmovableRegion);
    ASSERT_EQ(nonmovableRegion, AddrToRegion(nonmovable.GetPtr()));
    ASSERT_TRUE(nonmovableRegion->GetLiveBitmap()->Test(nonmovable.GetPtr()));

    Region *humongousRegion = ObjectToRegion(humongous.GetPtr());
    ASSERT_NE(nullptr, humongousRegion);
    ASSERT_EQ(humongousRegion, AddrToRegion(humongous.GetPtr()));
    ASSERT_EQ(humongousRegion, AddrToRegion(ToVoidPtr(ToUintPtr(humongous.GetPtr()) + DEFAULT_REGION_SIZE)));
    bool hasHumongousObj = false;
    humongousRegion->IterateOverObjects(
        [&hasHumongousObj, &humongous](ObjectHeader *obj) { hasHumongousObj |= obj == humongous.GetPtr(); });
    ASSERT_TRUE(hasHumongousObj);
}

TEST_F(G1GCTest, TestAllocHumongousArray)
{
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    ObjectHeader *obj =
        ObjectAllocator::AllocArray(GetHumongousArrayLength(ClassRoot::ARRAY_U8), ClassRoot::ARRAY_U8, false);
    ASSERT_TRUE(ObjectToRegion(obj)->HasFlag(RegionFlag::IS_LARGE_OBJECT));
}

TEST_F(G1GCTest, NonMovable2YoungRef)
{
    Runtime *runtime = Runtime::GetCurrent();
    LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();

    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    static constexpr size_t ARRAY_LENGTH = 100;
    coretypes::Array *nonMovableObj = nullptr;
    uintptr_t prevYoungAddr = 0;
    Class *klass = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)
                       ->GetClass(ctx.GetStringArrayClassDescriptor());
    ASSERT_NE(klass, nullptr);
    nonMovableObj = coretypes::Array::Create(klass, ARRAY_LENGTH, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    coretypes::String *youngObj = coretypes::String::CreateEmptyString(ctx, runtime->GetPandaVM());
    nonMovableObj->Set(0, youngObj);
    prevYoungAddr = ToUintPtr(youngObj);
    VMHandle<coretypes::Array> nonMovableObjPtr(thread, nonMovableObj);

    // Trigger GC
    RemSetChecker listener(gc, nonMovableObj, nonMovableObj->Get<ObjectHeader *>(0));
    gc->AddListener(&listener);

    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }

    auto youngObj2 = static_cast<coretypes::String *>(nonMovableObjPtr->Get<ObjectHeader *>(0));
    // Check GC has moved the young obj
    ASSERT_NE(prevYoungAddr, ToUintPtr(youngObj2));
    // Check young object is accessible
    ASSERT_EQ(0, youngObj2->GetLength());
}

TEST_F(G1GCTest, Humongous2YoungRef)
{
    Runtime *runtime = Runtime::GetCurrent();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    uintptr_t prevYoungAddr = 0;
    size_t arrayLength = GetHumongousArrayLength(ClassRoot::ARRAY_STRING);
    VMHandle<coretypes::Array> humongousObj(thread,
                                            ObjectAllocator::AllocArray(arrayLength, ClassRoot::ARRAY_STRING, false));
    ObjectHeader *youngObj = ObjectAllocator::AllocObjectInYoung();
    humongousObj->Set(0, youngObj);
    prevYoungAddr = ToUintPtr(youngObj);

    // Trigger GC
    RemSetChecker listener(gc, humongousObj.GetPtr(), humongousObj->Get<ObjectHeader *>(0));
    gc->AddListener(&listener);

    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }

    youngObj = static_cast<ObjectHeader *>(humongousObj->Get<ObjectHeader *>(0));
    // Check GC has moved the young obj
    ASSERT_NE(prevYoungAddr, ToUintPtr(youngObj));
    // Check the young object is accessible
    ASSERT_NE(nullptr, youngObj->ClassAddr<Class>());
}

TEST_F(G1GCTest, CheckRemsetAfterPostBarrier)
{
    Runtime *runtime = Runtime::GetCurrent();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    auto ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto arrayClass = runtime->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::ARRAY_STRING);
    size_t elemSize = arrayClass->GetComponentSize();
    size_t arraySize = CardTable::GetCardSize();
    // NOLINTNEXTLINE(clang-analyzer-core.DivideZero)
    size_t arrayLength = arraySize / elemSize + 1;
    // OBJECT_NUM needs to be large enough to ensure that the thread's g1PostBarrierRingBuffer is fully filled
    // CC-OFFNXT(G.NAM.03-CPP) project code style
    constexpr size_t OBJECT_NUM = 1024 * 12 + 1;
    std::vector<VMHandle<coretypes::Array>> arrays;
    for (size_t i = 0; i < OBJECT_NUM; i++) {
        arrays.emplace_back(thread, ObjectAllocator::AllocArray(arrayLength, ClassRoot::ARRAY_STRING, false));
    }

    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }

    for (auto &array : arrays) {
        ASSERT_TRUE(ObjectToRegion(array.GetPtr())->HasFlag(IS_OLD));
    }

    std::vector<VMHandle<coretypes::String>> strings;
    std::vector<void *> stringOrigPtrs;
    for (auto &array : arrays) {
        // NOLINTNEXTLINE(readability-magic-numbers)
        auto str = ObjectAllocator::AllocString(10);
        strings.emplace_back(thread, str);
        stringOrigPtrs.push_back(str);
        array->Set(0, str);  // create dirty card
    }

    // With high probability update_remset_worker could not process all dirty cards before GC
    // so GC drains them from update_remset_worker and handles separately
    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }

    // dirty cards corresponding to dirty_regions_objects should be reenqueued
    ProcessDirtyCards(static_cast<G1GC<PandaAssemblyLanguageConfig> *>(gc));
    for (size_t i = 0; i < OBJECT_NUM; i++) {
        auto &array = arrays[i];
        auto &str = strings[i];
        bool found = false;
        ObjectToRegion(str.GetPtr())->GetRemSet()->IterateOverObjects([&found, &array](ObjectHeader *obj) {
            if (obj == array.GetPtr()) {
                found = true;
            }
        });
        ASSERT_TRUE(found);
    }
}

TEST_F(G1GCTest, TestRegionInvalidationDuringFullGC)
{
    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    auto oldString = VMHandle<ObjectHeader>(thread, ObjectAllocator::AllocString(8U));
    gc->WaitForGCInManaged(GCTask(GCTaskCause::YOUNG_GC_CAUSE));
    ASSERT_TRUE(ObjectToRegion(oldString.GetPtr())->HasFlag(IS_OLD));
    VMHandle<coretypes::Array> arr(thread, ObjectAllocator::AllocArray(1U, ClassRoot::ARRAY_STRING, false));
    ASSERT_NE(ObjectToRegion(oldString.GetPtr()), ObjectToRegion(arr.GetPtr()));
    arr->Set(0, oldString.GetPtr());
    gc->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
    ASSERT_EQ(ObjectToRegion(oldString.GetPtr())->GetRemSet()->Size(), 0);
}

TEST_F(G1GCTest, TestCollectTenured)
{
    Runtime *runtime = Runtime::GetCurrent();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> hs(thread);

    VMHandle<coretypes::Array> humongous;
    VMHandle<coretypes::Array> nonmovable;
    ObjectHeader *obj;
    uintptr_t objAddr;

    humongous =
        VMHandle<coretypes::Array>(thread, ObjectAllocator::AllocArray(GetHumongousArrayLength(ClassRoot::ARRAY_STRING),
                                                                       ClassRoot::ARRAY_STRING, false));
    nonmovable = VMHandle<coretypes::Array>(thread, ObjectAllocator::AllocArray(1, ClassRoot::ARRAY_STRING, true));
    obj = ObjectAllocator::AllocObjectInYoung();
    humongous->Set(0, obj);
    nonmovable->Set(0, obj);
    objAddr = ToUintPtr(obj);

    RemSetChecker listener1(gc, humongous.GetPtr(), obj);
    RemSetChecker listener2(gc, nonmovable.GetPtr(), obj);
    gc->AddListener(&listener1);
    gc->AddListener(&listener2);
    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }
    // Check the obj obj was propagated to tenured
    obj = humongous->Get<ObjectHeader *>(0);
    ASSERT_NE(objAddr, ToUintPtr(obj));
    ASSERT_TRUE(ObjectToRegion(obj)->HasFlag(RegionFlag::IS_OLD));

    objAddr = ToUintPtr(obj);
    {
        ScopedNativeCodeThread sn(thread);
        GCTask task1(GCTaskCause::EXPLICIT_CAUSE);  // run full GC to collect all regions
        task1.Run(*gc);
    }

    // Check the tenured obj was propagated to another tenured region
    obj = humongous->Get<ObjectHeader *>(0);
    ASSERT_NE(objAddr, ToUintPtr(obj));
    ASSERT_TRUE(ObjectToRegion(obj)->HasFlag(RegionFlag::IS_OLD));

    // Check the objet is accessible
    ASSERT_NE(nullptr, obj->ClassAddr<Class>());
}

// test that we don't have remset from humongous space after we reclaim humongous object
TEST_F(G1GCTest, CheckRemsetToHumongousAfterReclaimHumongousObject)
{
    LanguageContext ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    MTManagedThread *thread = MTManagedThread::GetCurrent();

    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scopeForYoungObj(thread);

    // 1MB array
    static constexpr size_t HUMONGOUS_ARRAY_LENGTH = 262144LU;
    static constexpr size_t YOUNG_ARRAY_LENGTH = ((DEFAULT_REGION_SIZE - Region::HeadSize()) / 4U) - 16U;

    auto *gc = Runtime::GetCurrent()->GetPandaVM()->GetGC();
    auto regionPred = []([[maybe_unused]] Region *r) { return true; };

    Class *klass = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)
                       ->GetClass(ctx.GetStringArrayClassDescriptor());
    ASSERT_NE(klass, nullptr);

    auto *youngArr = coretypes::Array::Create(klass, YOUNG_ARRAY_LENGTH);
    ASSERT_NE(youngArr, nullptr);
    ASSERT_NE(ObjectToRegion(youngArr), nullptr);

    VMHandle<coretypes::Array> youngObjPtr(thread, youngArr);
    GCTask task(GCTaskCause::EXPLICIT_CAUSE);
    {
        [[maybe_unused]] HandleScope<ObjectHeader *> scopeForHumongousObj(thread);

        auto *humongousObj = coretypes::Array::Create(klass, HUMONGOUS_ARRAY_LENGTH);
        ASSERT_NE(humongousObj, nullptr);
        // add humongous object to our remset
        humongousObj->Set(0, youngObjPtr.GetPtr());

        ASSERT_EQ(gc->GetType(), GCType::G1_GC);
        {
            VMHandle<coretypes::Array> humongousObjPtr(thread, humongousObj);
            {
                ScopedNativeCodeThread sn(thread);
                task.Run(*gc);
            }

            auto *arrayRegion = ObjectToRegion(youngObjPtr.GetPtr());
            PandaVector<Region *> regions;
            arrayRegion->GetRemSet()->Iterate(
                regionPred, [&regions](Region *r, [[maybe_unused]] const MemRange &range) { regions.push_back(r); });
            ASSERT_EQ(1U, regions.size());  // we have reference only from 1 humongous space
            ASSERT_TRUE(regions[0]->HasFlag(IS_LARGE_OBJECT));
            ASSERT_EQ(SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT,
                      PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(regions[0]));
        }
    }
    /*
     * humongous object is dead now
     * need one fake GC because we marked humongous in concurrent in the first GC before we removed Scoped, need to
     * unmark it
     */
    {
        ScopedNativeCodeThread sn(thread);
        task.Run(*gc);
        task.Run(*gc);  // humongous object should be reclaimed
    }

    auto *arrayRegion = ObjectToRegion(youngObjPtr.GetPtr());
    PandaVector<Region *> regions;
    arrayRegion->GetRemSet()->Iterate(
        regionPred, [&regions](Region *r, [[maybe_unused]] const MemRange &range) { regions.push_back(r); });
    ASSERT_EQ(0U, regions.size());  // we have no references from the humongous space
}

class NewObjectsListener : public GCListener {
public:
    void GCPhaseStarted(GCPhase phase) override
    {
        if (phase != GCPhase::GC_PHASE_MARK) {
            return;
        }
        MTManagedThread *thread = MTManagedThread::GetCurrent();
        ScopedManagedCodeThread s(thread);

        // Allocate quite large object to make allocator to create a separate region
        // NOLINTNEXTLINE(readability-magic-numbers)
        size_t nonmovableLen = 9 * DEFAULT_REGION_SIZE / 10;
        ObjectHeader *dummy = ObjectAllocator::AllocArray(nonmovableLen, ClassRoot::ARRAY_U8, true);
        Region *dummyRegion = ObjectToRegion(dummy);
        EXPECT_TRUE(dummyRegion->HasFlag(RegionFlag::IS_NONMOVABLE));
        nonmovable_ =
            VMHandle<ObjectHeader>(thread, ObjectAllocator::AllocArray(nonmovableLen, ClassRoot::ARRAY_U8, true));
        Region *nonmovableRegion = ObjectToRegion(nonmovable_.GetPtr());
        EXPECT_TRUE(nonmovableRegion->HasFlag(RegionFlag::IS_NONMOVABLE));
        EXPECT_NE(nonmovableRegion, dummyRegion);
        nonmovableMarkBitmapAddr_ = ToUintPtr(nonmovableRegion->GetMarkBitmap());

        size_t humongousLen = G1GCTest::GetHumongousArrayLength(ClassRoot::ARRAY_U8);
        humongous_ =
            VMHandle<ObjectHeader>(thread, ObjectAllocator::AllocArray(humongousLen, ClassRoot::ARRAY_U8, false));
        Region *humongousRegion = ObjectToRegion(humongous_.GetPtr());
        humongousMarkBitmapAddr_ = ToUintPtr(humongousRegion->GetMarkBitmap());
    }

    ObjectHeader *GetNonMovable()
    {
        ASSERT(nonmovable_.GetPtr() != nullptr);
        return nonmovable_.GetPtr();
    }

    uintptr_t GetNonMovableMarkBitmapAddr()
    {
        return nonmovableMarkBitmapAddr_;
    }

    ObjectHeader *GetHumongous()
    {
        return humongous_.GetPtr();
    }

    uintptr_t GetHumongousMarkBitmapAddr()
    {
        return humongousMarkBitmapAddr_;
    }

private:
    VMHandle<ObjectHeader> nonmovable_;
    uintptr_t nonmovableMarkBitmapAddr_ {};
    VMHandle<ObjectHeader> humongous_;
    uintptr_t humongousMarkBitmapAddr_ {};
};

// Test the new objects created during concurrent marking are alive
TEST_F(G1GCTest, TestNewObjectsSATB)
{
    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    NewObjectsListener listener;
    gc->AddListener(&listener);

    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);  // threshold cause should trigger concurrent marking
        task.Run(*gc);
    }
    // nullptr means we cannot allocate an object or concurrent phase wasn't triggered or
    // the listener wasn't called.
    ASSERT_NE(nullptr, listener.GetNonMovable());
    ASSERT_NE(nullptr, listener.GetHumongous());

    // Check the objects are alive
    Region *nonmovableRegion = ObjectToRegion(listener.GetNonMovable());
    ASSERT_NE(nullptr, nonmovableRegion->GetLiveBitmap());
    ASSERT_TRUE(nonmovableRegion->GetLiveBitmap()->Test(listener.GetNonMovable()));
    ASSERT_FALSE(listener.GetNonMovable()->IsMarkedForGC());  // mark should be done using mark bitmap
    Region *humongousRegion = ObjectToRegion(listener.GetHumongous());
    ASSERT_NE(nullptr, humongousRegion->GetLiveBitmap());
    ASSERT_TRUE(humongousRegion->GetLiveBitmap()->Test(listener.GetHumongous()));
    ASSERT_FALSE(listener.GetHumongous()->IsMarkedForGC());  // mark should be done using mark bitmap
}

class CollectionSetChecker : public GCListener {
public:
    explicit CollectionSetChecker(ObjectAllocatorG1<> *allocator) : allocator_(allocator) {}

    void SetExpectedRegions(const std::initializer_list<Region *> &expectedRegions)
    {
        expectedRegions_ = expectedRegions;
    }

    void GCPhaseStarted(GCPhase phase) override
    {
        if (phase == GCPhase::GC_PHASE_MARK_YOUNG) {
            EXPECT_EQ(expectedRegions_, GetCollectionSet());
            expectedRegions_.clear();
        }
    }

private:
    PandaSet<Region *> GetCollectionSet()
    {
        PandaSet<Region *> collectionSet;
        for (Region *region : allocator_->GetAllRegions()) {
            if (region->HasFlag(RegionFlag::IS_COLLECTION_SET)) {
                collectionSet.insert(region);
            }
        }
        return collectionSet;
    }

private:
    ObjectAllocatorG1<> *allocator_;
    PandaSet<Region *> expectedRegions_;
};

TEST_F(G1GCTest, TestGetCollectibleRegionsHasAllYoungRegions)
{
    // The object will occupy more than half of region.
    // So expect the allocator allocates a separate young region for each object.
    size_t youngLen = DEFAULT_REGION_SIZE / 2 + sizeof(coretypes::Array);

    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    ObjectAllocatorG1<> *allocator = GetAllocator();
    MTManagedThread *thread = MTManagedThread::GetCurrent();

    CollectionSetChecker checker(allocator);
    gc->AddListener(&checker);
    {
        ScopedManagedCodeThread s(thread);
        [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
        VMHandle<ObjectHeader> young1;
        VMHandle<ObjectHeader> young2;
        VMHandle<ObjectHeader> young3;

        young1 = VMHandle<ObjectHeader>(thread, ObjectAllocator::AllocArray(youngLen, ClassRoot::ARRAY_U8, false));
        young2 = VMHandle<ObjectHeader>(thread, ObjectAllocator::AllocArray(youngLen, ClassRoot::ARRAY_U8, false));
        young3 = VMHandle<ObjectHeader>(thread, ObjectAllocator::AllocArray(youngLen, ClassRoot::ARRAY_U8, false));

        Region *yregion1 = ObjectToRegion(young1.GetPtr());
        Region *yregion2 = ObjectToRegion(young2.GetPtr());
        Region *yregion3 = ObjectToRegion(young3.GetPtr());
        // Check all 3 objects are in different regions
        ASSERT_NE(yregion1, yregion2);
        ASSERT_NE(yregion2, yregion3);
        ASSERT_NE(yregion1, yregion3);
        checker.SetExpectedRegions({yregion1, yregion2, yregion3});
    }
    GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
    task.Run(*gc);
}

TEST_F(G1GCTest, TestGetCollectibleRegionsHasAllRegionsInCaseOfFull)
{
    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    ObjectAllocatorG1<> *allocator = GetAllocator();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    VMHandle<ObjectHeader> young;
    VMHandle<ObjectHeader> tenured;
    VMHandle<ObjectHeader> humongous;
    tenured = VMHandle<ObjectHeader>(thread, ObjectAllocator::AllocObjectInYoung());

    {
        ScopedNativeCodeThread sn(thread);
        // Propogate young to tenured
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }

    young = VMHandle<ObjectHeader>(thread, ObjectAllocator::AllocObjectInYoung());
    humongous = VMHandle<ObjectHeader>(
        thread, ObjectAllocator::AllocArray(GetHumongousArrayLength(ClassRoot::ARRAY_U8), ClassRoot::ARRAY_U8, false));

    Region *yregion = ObjectToRegion(young.GetPtr());
    [[maybe_unused]] Region *tregion = ObjectToRegion(tenured.GetPtr());
    [[maybe_unused]] Region *hregion = ObjectToRegion(humongous.GetPtr());

    CollectionSetChecker checker(allocator);
    gc->AddListener(&checker);
    // Even thou it's full, currently we split it into two parts, the 1st one is young-only collection.
    // And the tenured collection part doesn't use GC_PHASE_MARK_YOUNG
    checker.SetExpectedRegions({yregion});
    {
        ScopedNativeCodeThread sn(thread);
        GCTask task1(GCTaskCause::EXPLICIT_CAUSE);
        task1.Run(*gc);
    }
}

TEST_F(G1GCTest, TestMixedCollections)
{
    uint32_t garbageRate = Runtime::GetOptions().GetG1RegionGarbageRateThreshold();
    // The object will occupy more than half of region.
    // So expect the allocator allocates a separate young region for each object.
    static constexpr size_t ARRAY_SIZE = 4;
    // NOLINTNEXTLINE(readability-magic-numbers)
    size_t bigLen = garbageRate * DEFAULT_REGION_SIZE / 100 + sizeof(coretypes::String);
    // NOLINTNEXTLINE(readability-magic-numbers)
    size_t bigLen1 = (garbageRate + 1) * DEFAULT_REGION_SIZE / 100 + sizeof(coretypes::String);
    // NOLINTNEXTLINE(readability-magic-numbers)
    size_t bigLen2 = (garbageRate + 2) * DEFAULT_REGION_SIZE / 100 + sizeof(coretypes::String);
    size_t smallLen = DEFAULT_REGION_SIZE / 2 + sizeof(coretypes::String);
    std::array<size_t, ARRAY_SIZE> lenthsArray {bigLen, bigLen1, bigLen2, smallLen};
    size_t miniObjLen = Runtime::GetOptions().GetInitTlabSize() + 1;  // To allocate not in TLAB

    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    ObjectAllocatorG1<> *allocator = GetAllocator();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    VMHandle<coretypes::Array> smallObjectHolder;
    VMHandle<coretypes::Array> bigObjectHolder;
    VMHandle<ObjectHeader> young;

    // Allocate objects of different sizes.
    // Allocate mini object after each of them for prevent clearing after concurrent
    // Mixed regions should be choosen according to the largest garbage.
    bigObjectHolder =
        VMHandle<coretypes::Array>(thread, ObjectAllocator::AllocArray(4U, ClassRoot::ARRAY_STRING, false));
    smallObjectHolder =
        VMHandle<coretypes::Array>(thread, ObjectAllocator::AllocArray(4U, ClassRoot::ARRAY_STRING, false));
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        bigObjectHolder->Set(i, ObjectAllocator::AllocString(lenthsArray[i]));
        smallObjectHolder->Set(i, ObjectAllocator::AllocString(miniObjLen));
        Region *firstRegion = ObjectToRegion(bigObjectHolder->Get<ObjectHeader *>(i));
        Region *secondRegion = ObjectToRegion(smallObjectHolder->Get<ObjectHeader *>(i));
        ASSERT_TRUE(firstRegion->HasFlag(RegionFlag::IS_EDEN));
        ASSERT_TRUE(secondRegion->HasFlag(RegionFlag::IS_EDEN));
        ASSERT_TRUE(firstRegion == secondRegion);
    }

    {
        ScopedNativeCodeThread sn(thread);
        // Propogate young objects -> tenured
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }
    // GC doesn't include current tenured region to the collection set.
    // Now we don't know which tenured region is current.
    // So propagate one big young object to tenured to make the latter current.
    VMHandle<ObjectHeader> current;
    current = VMHandle<ObjectHeader>(thread, ObjectAllocator::AllocArray(smallLen, ClassRoot::ARRAY_U8, false));

    // Propogate 'current' object -> tenured and prepare for mixed GC
    // Release 'big1', 'big2' and 'small' objects to make them garbage
    Region *region0 = ObjectToRegion(bigObjectHolder->Get<ObjectHeader *>(0));
    Region *region1 = ObjectToRegion(bigObjectHolder->Get<ObjectHeader *>(1));
    Region *region2 = ObjectToRegion(bigObjectHolder->Get<ObjectHeader *>(2));
    Region *region3 = ObjectToRegion(bigObjectHolder->Get<ObjectHeader *>(3));
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        bigObjectHolder->Set(i, static_cast<ObjectHeader *>(nullptr));
    }
    {
        ScopedNativeCodeThread sn(thread);
        GCTask task1(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);
        task1.Run(*gc);
    }

    // Now the region with 'current' is current and it will not be included into the collection set.

    young = VMHandle<ObjectHeader>(thread, ObjectAllocator::AllocObjectInYoung());

    Region *yregion = ObjectToRegion(young.GetPtr());
    CollectionSetChecker checker(allocator);
    gc->AddListener(&checker);
    checker.SetExpectedRegions({region1, region2, yregion});
    {
        ScopedNativeCodeThread sn(thread);
        GCTask task2(GCTaskCause::YOUNG_GC_CAUSE);  // should run mixed GC
        task2.Run(*gc);
    }

    // Run GC one more time because we still have garbage regions.
    // Check we collect them.
    young = VMHandle<ObjectHeader>(thread, ObjectAllocator::AllocObjectInYoung());
    yregion = ObjectToRegion(young.GetPtr());
    checker.SetExpectedRegions({region0, yregion, region3});
    {
        ScopedNativeCodeThread sn(thread);
        GCTask task3(GCTaskCause::YOUNG_GC_CAUSE);  // should run mixed GC
        task3.Run(*gc);
    }
}

TEST_F(G1GCTest, TestHandlePendingCards)
{
    auto thread = MTManagedThread::GetCurrent();
    auto runtime = Runtime::GetCurrent();
    auto ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto arrayClass = runtime->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::ARRAY_STRING);
    auto gc = runtime->GetPandaVM()->GetGC();
    size_t elemSize = arrayClass->GetComponentSize();
    size_t arraySize = DEFAULT_REGION_SIZE / 2;
    // NOLINTNEXTLINE(clang-analyzer-core.DivideZero)
    size_t arrayLength = arraySize / elemSize + 1;
    ScopedManagedCodeThread s(thread);
    HandleScope<ObjectHeader *> scope(thread);

    constexpr size_t REGION_NUM = 16;
    std::vector<VMHandle<coretypes::Array>> arrays;

    for (size_t i = 0; i < REGION_NUM; i++) {
        arrays.emplace_back(thread, ObjectAllocator::AllocArray(arrayLength, ClassRoot::ARRAY_STRING, false));
    }

    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }

    for (auto &array : arrays) {
        ASSERT_TRUE(ObjectToRegion(array.GetPtr())->HasFlag(IS_OLD));
    }

    std::vector<VMHandle<coretypes::String>> strings;
    std::vector<void *> stringOrigPtrs;

    for (auto &array : arrays) {
        auto str = ObjectAllocator::AllocString(StringLengthFitIntoRegion(1));
        strings.emplace_back(thread, str);
        stringOrigPtrs.push_back(str);
        array->Set(0, str);  // create dirty card
    }

    // With high probability update_remset_worker could not process all dirty cards before GC
    // so GC drains them from update_remset_worker and handles separately
    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }

    for (size_t i = 0; i < REGION_NUM; i++) {
        auto &array = arrays[i];
        auto &str = strings[i];
        auto strOrigPtr = stringOrigPtrs[i];
        ASSERT_NE(strOrigPtr, str.GetPtr());                     // string was moved
        ASSERT_EQ(array->Get<ObjectHeader *>(0), str.GetPtr());  // refs were correctly updated
    }

    // dirty cards corresponding to dirty_regions_objects should be reenqueued
    ProcessDirtyCards(static_cast<G1GC<PandaAssemblyLanguageConfig> *>(gc));
    for (size_t i = 0; i < REGION_NUM; i++) {
        auto &array = arrays[i];
        auto &str = strings[i];
        bool found = false;
        ObjectToRegion(str.GetPtr())->GetRemSet()->IterateOverObjects([&found, &array](ObjectHeader *obj) {
            if (obj == array.GetPtr()) {
                found = true;
            }
        });
        ASSERT_TRUE(found);
    }
}

class G1GCPromotionTest : public G1GCTest {
public:
    G1GCPromotionTest() : G1GCTest(CreateOptions()) {}

    static RuntimeOptions CreateOptions()
    {
        RuntimeOptions options = CreateDefaultOptions();
        // NOLINTNEXTLINE(readability-magic-numbers)
        options.SetG1PromotionRegionAliveRate(PROMOTE_RATE);
        return options;
    }

    static constexpr size_t PROMOTE_RATE = 50;
};

class G1GCPromotionTestNoSinglePass : public G1GCTest {
public:
    G1GCPromotionTestNoSinglePass() : G1GCTest(CreateOptions()) {}

    static RuntimeOptions CreateOptions()
    {
        RuntimeOptions options = CreateDefaultOptions();
        // NOLINTNEXTLINE(readability-magic-numbers)
        options.SetG1PromotionRegionAliveRate(PROMOTE_RATE);
        options.SetG1SinglePassCompactionEnabled(false);
        return options;
    }

    static constexpr size_t PROMOTE_RATE = 50;
};

// CC-OFFNXT(huge_method[C++], G.FUN.01-CPP, G.FUD.05) solid logic
TEST_F(G1GCPromotionTestNoSinglePass, TestCorrectPromotionYoungRegion)
{
    // We will create a humongous object with a links to two young regions
    // and check promotion workflow
    static constexpr size_t HUMONGOUS_STRING_LEN = G1GCPromotionTest::GetHumongousStringLength();
    // Consume more than 50% of region size
    static constexpr size_t FIRST_YOUNG_REGION_ALIVE_OBJECTS_COUNT =
        DEFAULT_REGION_SIZE / sizeof(coretypes::String) * 2U / 3U + 1;
    // Consume less than 50% of region size
    static constexpr size_t SECOND_YOUNG_REGION_ALIVE_OBJECTS_COUNT = 1;
    ASSERT(FIRST_YOUNG_REGION_ALIVE_OBJECTS_COUNT <= HUMONGOUS_STRING_LEN);
    ASSERT((FIRST_YOUNG_REGION_ALIVE_OBJECTS_COUNT * sizeof(coretypes::String) * 100U / DEFAULT_REGION_SIZE) >
           G1GCPromotionTest::PROMOTE_RATE);
    ASSERT((SECOND_YOUNG_REGION_ALIVE_OBJECTS_COUNT * sizeof(coretypes::String) * 100U / DEFAULT_REGION_SIZE) <
           G1GCPromotionTest::PROMOTE_RATE);

    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();

    // Run Full GC to compact all existed young regions:
    GCTask task0(GCTaskCause::EXPLICIT_CAUSE);
    task0.Run(*gc);

    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    VMHandle<coretypes::Array> firstHolder;
    VMHandle<coretypes::Array> secondHolder;
    VMHandle<ObjectHeader> young;
    std::array<ObjectHeader *, FIRST_YOUNG_REGION_ALIVE_OBJECTS_COUNT> firstRegionObjectLinks {};
    std::array<ObjectHeader *, SECOND_YOUNG_REGION_ALIVE_OBJECTS_COUNT> secondRegionObjectLinks {};
    // Check Promotion for young region:

    firstHolder = VMHandle<coretypes::Array>(
        thread, ObjectAllocator::AllocArray(HUMONGOUS_STRING_LEN, ClassRoot::ARRAY_STRING, false));
    Region *firstRegion = ObjectToRegion(ObjectAllocator::AllocObjectInYoung());
    ASSERT_TRUE(firstRegion->HasFlag(RegionFlag::IS_EDEN));
    for (size_t i = 0; i < FIRST_YOUNG_REGION_ALIVE_OBJECTS_COUNT; i++) {
        firstRegionObjectLinks[i] = ObjectAllocator::AllocObjectInYoung();
        ASSERT_TRUE(firstRegionObjectLinks[i] != nullptr);
        firstHolder->Set(i, firstRegionObjectLinks[i]);
        ASSERT_TRUE(ObjectToRegion(firstRegionObjectLinks[i]) == firstRegion);
    }

    {
        ScopedNativeCodeThread sn(thread);
        // Promote young objects in one region -> tenured
        GCTask task1(GCTaskCause::YOUNG_GC_CAUSE);
        task1.Run(*gc);
    }
    // Check that we didn't change the links for young objects from the first region:
    for (size_t i = 0; i < FIRST_YOUNG_REGION_ALIVE_OBJECTS_COUNT; i++) {
        ASSERT_EQ(firstRegionObjectLinks[i], firstHolder->Get<ObjectHeader *>(i));
        ASSERT_TRUE(ObjectToRegion(firstHolder->Get<ObjectHeader *>(i))->HasFlag(RegionFlag::IS_OLD));
        ASSERT_FALSE(ObjectToRegion(firstHolder->Get<ObjectHeader *>(i))->HasFlag(RegionFlag::IS_PROMOTED));
    }

    secondHolder = VMHandle<coretypes::Array>(
        thread, ObjectAllocator::AllocArray(HUMONGOUS_STRING_LEN, ClassRoot::ARRAY_STRING, false));
    Region *secondRegion = ObjectToRegion(ObjectAllocator::AllocObjectInYoung());
    ASSERT_TRUE(secondRegion->HasFlag(RegionFlag::IS_EDEN));
    for (size_t i = 0; i < SECOND_YOUNG_REGION_ALIVE_OBJECTS_COUNT; i++) {
        secondRegionObjectLinks[i] = ObjectAllocator::AllocObjectInYoung();
        ASSERT_TRUE(secondRegionObjectLinks[i] != nullptr);
        secondHolder->Set(i, secondRegionObjectLinks[i]);
        ASSERT_TRUE(ObjectToRegion(secondRegionObjectLinks[i]) == secondRegion);
    }

    {
        ScopedNativeCodeThread sn(thread);
        // Compact young objects in one region -> tenured
        GCTask task2(GCTaskCause::YOUNG_GC_CAUSE);
        task2.Run(*gc);
    }
    // Check that we changed the links for young objects from the second region:
    for (size_t i = 0; i < SECOND_YOUNG_REGION_ALIVE_OBJECTS_COUNT; i++) {
        ASSERT_NE(secondRegionObjectLinks[i], secondHolder->Get<ObjectHeader *>(i));
        ASSERT_TRUE(ObjectToRegion(secondHolder->Get<ObjectHeader *>(i))->HasFlag(RegionFlag::IS_OLD));
        ASSERT_FALSE(ObjectToRegion(secondHolder->Get<ObjectHeader *>(i))->HasFlag(RegionFlag::IS_PROMOTED));
    }

    {
        ScopedNativeCodeThread sn(thread);
        // Run Full GC to compact all tenured regions:
        GCTask task3(GCTaskCause::EXPLICIT_CAUSE);
        task3.Run(*gc);
    }
    // Now we should have updated links in the humongous object to first region objects:
    for (size_t i = 0; i < FIRST_YOUNG_REGION_ALIVE_OBJECTS_COUNT; i++) {
        ASSERT_NE(firstRegionObjectLinks[i], firstHolder->Get<ObjectHeader *>(i));
        ASSERT_TRUE(ObjectToRegion(firstHolder->Get<ObjectHeader *>(i))->HasFlag(RegionFlag::IS_OLD));
        ASSERT_FALSE(ObjectToRegion(firstHolder->Get<ObjectHeader *>(i))->HasFlag(RegionFlag::IS_PROMOTED));
    }
}

TEST_F(G1GCPromotionTest, TestFullCollectionSetPromotionObjects)
{
    // The object will occupy more than half of region.
    // So expect the allocator allocates a separate young region for each object.
    // NOLINTNEXTLINE(readability-identifier-naming)
    static constexpr size_t youngLength = DEFAULT_REGION_SIZE / 2 + sizeof(coretypes::Array);

    ObjectAllocatorG1<> *allocator = GetAllocator();
    Runtime *runtime = Runtime::GetCurrent();
    // Setting FastGC flag = true - means that G1-GC should
    // only promote all young regions without marking
    GC *gc = runtime->GetPandaVM()->GetGC();
    gc->SetFastGCFlag(true);
    ASSERT_TRUE(gc->GetFastGCFlag());

    // Run Full GC to compact all existed young regions:
    GCTask task0(GCTaskCause::OOM_CAUSE);
    task0.Run(*gc);

    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    CollectionSetChecker checker(allocator);
    gc->AddListener(&checker);
    {
        // Allocating arrays to young regions without saving any references to them
        coretypes::Array *young1 = ObjectAllocator::AllocArray(youngLength, ClassRoot::ARRAY_U8, false);
        coretypes::Array *young2 = ObjectAllocator::AllocArray(youngLength, ClassRoot::ARRAY_U8, false);
        coretypes::Array *young3 = ObjectAllocator::AllocArray(youngLength, ClassRoot::ARRAY_U8, false);
        Region *yregion1 = ObjectToRegion(young1);
        Region *yregion2 = ObjectToRegion(young2);
        Region *yregion3 = ObjectToRegion(young3);
        ASSERT_TRUE(yregion1->HasFlag(RegionFlag::IS_EDEN));
        ASSERT_TRUE(yregion2->HasFlag(RegionFlag::IS_EDEN));
        ASSERT_TRUE(yregion3->HasFlag(RegionFlag::IS_EDEN));
        // Check all 3 objects are in different regions
        ASSERT_NE(yregion1, yregion2);
        ASSERT_NE(yregion2, yregion3);
        ASSERT_NE(yregion1, yregion3);
        checker.SetExpectedRegions({yregion1, yregion2, yregion3});
    }
    auto regions = allocator->GetAllRegions();
    size_t aliveBytesSum = 0;
    for (auto *region : regions) {
        aliveBytesSum += region->GetAllocatedBytes();
    }
    {
        ScopedNativeCodeThread sn(thread);
        // Promote array's regions to tenured
        GCTask task(GCTaskCause::MIXED);
        task.Run(*gc);
    }
    // Even when there are no references to the arrays
    // they must not be deleted
    regions = allocator->GetAllRegions();
    size_t aliveBytesSumToCheck = 0;
    for (auto *region : regions) {
        aliveBytesSumToCheck += region->GetAllocatedBytes();
    }
    ASSERT_EQ(aliveBytesSum, aliveBytesSumToCheck);
}

static coretypes::String *CreateStringFromNumber(size_t value)
{
    Runtime *runtime = Runtime::GetCurrent();
    LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto s = std::to_string(value);
    coretypes::String *strObj = coretypes::String::CreateFromMUtf8(
        utf::CStringAsMutf8(s.data()), s.size(), s.size(), true, ctx, Runtime::GetCurrent()->GetPandaVM(), true, false);

    auto youngRegion = ObjectToRegion(strObj);
    EXPECT_TRUE(youngRegion->HasFlag(RegionFlag::IS_EDEN));
    return strObj;
}

TEST_F(G1GCPromotionTest, TestFullCollectionSetPromotionRemsets)
{
    // Compute array length so it will fill almost whole young region
    Runtime *runtime = Runtime::GetCurrent();
    LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto *arrayClass = runtime->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::ARRAY_STRING);
    // NOLINTNEXTLINE(clang-analyzer-core.DivideZero, readability-magic-numbers)
    size_t youngLength = ((DEFAULT_REGION_SIZE - sizeof(coretypes::Array)) / arrayClass->GetComponentSize()) - 32;

    // Setting FastGC flag = true - means that G1-GC should only promote all young regions
    auto *gc = static_cast<G1GC<PandaAssemblyLanguageConfig> *>(runtime->GetPandaVM()->GetGC());
    gc->SetFastGCFlag(true);
    ASSERT_TRUE(gc->GetFastGCFlag());

    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    auto runGC = [thread, gc](GCTaskCause reason) {
        ScopedNativeCodeThread sn(thread);
        GCTask task0(reason);
        task0.Run(*gc);
    };
    // Run Full GC to compact all existed young regions
    runGC(GCTaskCause::OOM_CAUSE);

    // To test remsets correctness - create array which occupies whole young region and fill it with strings which
    // located in another young regions
    auto youngHolder =
        VMHandle<coretypes::Array>(thread, ObjectAllocator::AllocArray(youngLength, ClassRoot::ARRAY_STRING, false));
    auto youngRegion = ObjectToRegion(youngHolder.GetPtr());
    ASSERT_TRUE(youngRegion->HasFlag(RegionFlag::IS_EDEN));
    for (size_t idx = 0; idx < youngLength; ++idx) {
        youngHolder->Set(thread, idx, CreateStringFromNumber(idx));
    }
    auto youngRegionsCount = GetYoungRegionsCount(gc);

    auto verifyArray = [&youngHolder, youngLength, thread]() {
        for (size_t idx = 0; idx < youngLength; ++idx) {
            auto elem = static_cast<coretypes::String *>(youngHolder->Get<ObjectHeader *>(thread, idx));
            std::string elemStr(utf::Mutf8AsCString(elem->GetDataMUtf8()), elem->GetUtf8Length());
            ASSERT_EQ(elemStr, std::to_string(idx));
        }
    };

    ASSERT_EQ(GetPromotedRegions(gc), 0);
    ASSERT_EQ(GetLiveObjects(gc), 0);
    ASSERT_EQ(GetEvacuatedBytes(gc), 0);
    ASSERT_EQ(GetMemStats(gc).GetCountMovedYoung(), 0);

    runGC(GCTaskCause::MIXED);

    ASSERT_EQ(GetPromotedRegions(gc), youngRegionsCount);
    ASSERT_EQ(GetLiveObjects(gc), youngLength + 1);
    ASSERT_EQ(GetEvacuatedBytes(gc), 0);
    ASSERT_EQ(GetMemStats(gc).GetCountMovedYoung(), youngLength + 1);
    verifyArray();

    // Trigger GC several times to ensure remsets were built correctly
    gc->SetFastGCFlag(false);
    for (size_t iter = 0; iter < 4U; iter++) {
        runGC(GCTaskCause::OOM_CAUSE);
        verifyArray();
    }
}

TEST_F(G1GCPromotionTest, TestFullCollectionSetPromotionBitmaps)
{
    // Lets check LiveBitmaps after collection
    // NOLINTNEXTLINE(readability-identifier-naming)
    static constexpr size_t youngLength = DEFAULT_REGION_SIZE / 2 + sizeof(coretypes::Array);
    ObjectAllocatorG1<> *allocator = GetAllocator();
    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    gc->SetFastGCFlag(true);
    ASSERT_TRUE(gc->GetFastGCFlag());

    GCTask task0(GCTaskCause::OOM_CAUSE);
    task0.Run(*gc);

    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    VMHandle<coretypes::Array> firstHolder =
        VMHandle<coretypes::Array>(thread, ObjectAllocator::AllocArray(youngLength, ClassRoot::ARRAY_U8, false));
    VMHandle<coretypes::Array> secondHolder =
        VMHandle<coretypes::Array>(thread, ObjectAllocator::AllocArray(youngLength, ClassRoot::ARRAY_U8, false));
    VMHandle<coretypes::Array> thirdHolder =
        VMHandle<coretypes::Array>(thread, ObjectAllocator::AllocArray(youngLength, ClassRoot::ARRAY_U8, false));
    CollectionSetChecker listener(allocator);
    gc->AddListener(&listener);
    {
        Region *yregion1 = ObjectToRegion(firstHolder.GetPtr());
        Region *yregion2 = ObjectToRegion(secondHolder.GetPtr());
        Region *yregion3 = ObjectToRegion(thirdHolder.GetPtr());
        ASSERT_TRUE(yregion1->HasFlag(RegionFlag::IS_EDEN));
        ASSERT_TRUE(yregion2->HasFlag(RegionFlag::IS_EDEN));
        ASSERT_TRUE(yregion3->HasFlag(RegionFlag::IS_EDEN));
        // Check all 3 objects are in different regions
        ASSERT_NE(yregion1, yregion2);
        ASSERT_NE(yregion2, yregion3);
        ASSERT_NE(yregion1, yregion3);
        listener.SetExpectedRegions({yregion1, yregion2, yregion3});
    }
    {
        ScopedNativeCodeThread sn(thread);
        // Promote array's regions to tenured
        GCTask task(GCTaskCause::MIXED);
        task.Run(*gc);
    }
    Region *region1 = ObjectToRegion(firstHolder.GetPtr());
    Region *region2 = ObjectToRegion(secondHolder.GetPtr());
    Region *region3 = ObjectToRegion(thirdHolder.GetPtr());
    ASSERT_TRUE(region1->HasFlag(RegionFlag::IS_OLD));
    ASSERT_TRUE(region2->HasFlag(RegionFlag::IS_OLD));
    ASSERT_TRUE(region3->HasFlag(RegionFlag::IS_OLD));
    ASSERT_TRUE(region1->GetLiveBitmap()->Test(firstHolder.GetPtr()));
    ASSERT_TRUE(region2->GetLiveBitmap()->Test(secondHolder.GetPtr()));
    ASSERT_TRUE(region3->GetLiveBitmap()->Test(thirdHolder.GetPtr()));
}

TEST_F(G1GCPromotionTest, TestFullCollectionSetPromotionReferences)
{
    // Lets check LiveBitmaps after collection
    // NOLINTNEXTLINE(readability-identifier-naming)
    static constexpr size_t youngLength = DEFAULT_REGION_SIZE / 2 + sizeof(coretypes::Array);
    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    gc->SetFastGCFlag(true);
    ASSERT_TRUE(gc->GetFastGCFlag());

    GCTask task0(GCTaskCause::OOM_CAUSE);
    task0.Run(*gc);

    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    auto *obj = ObjectAllocator::AllocObjectInYoung();
    VMHandle<coretypes::Array> youngHolder =
        VMHandle<coretypes::Array>(thread, ObjectAllocator::AllocArray(youngLength, ClassRoot::ARRAY_U8, false));
    VMHandle<coretypes::Array> hugeHolder =
        VMHandle<coretypes::Array>(thread, ObjectAllocator::AllocArray(GetHumongousArrayLength(ClassRoot::ARRAY_STRING),
                                                                       ClassRoot::ARRAY_STRING, false));
    youngHolder->Set(0, obj);
    hugeHolder->Set(0, obj);
    RemSetChecker listener(gc, hugeHolder.GetPtr(), obj);
    gc->AddListener(&listener);
    {
        Region *yregion = ObjectToRegion(youngHolder.GetPtr());
        ASSERT_TRUE(yregion->HasFlag(RegionFlag::IS_EDEN));

        ASSERT_TRUE(ObjectToRegion(obj)->HasFlag(RegionFlag::IS_EDEN));
        Region *hregion = ObjectToRegion(hugeHolder.GetPtr());
        ASSERT_TRUE(hregion->HasFlag(RegionFlag::IS_OLD));
        ASSERT_TRUE(hregion->HasFlag(RegionFlag::IS_LARGE_OBJECT));
    }
    {
        ScopedNativeCodeThread sn(thread);
        // Promote array's regions to tenured
        GCTask task(GCTaskCause::MIXED);
        task.Run(*gc);
    }
    Region *region1 = ObjectToRegion(youngHolder.GetPtr());
    ASSERT_TRUE(region1->HasFlag(RegionFlag::IS_OLD));
    ASSERT_TRUE(region1->GetLiveBitmap()->Test(youngHolder.GetPtr()));

    // Check update reference from objects which were moved while garbage collection
    obj = youngHolder->Get<ObjectHeader *>(0);
    ASSERT_TRUE(ObjectToRegion(obj)->HasFlag(RegionFlag::IS_OLD));
    ASSERT_FALSE(ObjectToRegion(obj)->HasFlag(RegionFlag::IS_PROMOTED));
    // Check that the object is accessible
    ASSERT_NE(obj->ClassAddr<Class>(), nullptr);

    // Check update references from objects which are not part of collection set
    obj = hugeHolder->Get<ObjectHeader *>(0);
    ASSERT_TRUE(ObjectToRegion(obj)->HasFlag(RegionFlag::IS_OLD));
    ASSERT_FALSE(ObjectToRegion(obj)->HasFlag(RegionFlag::IS_PROMOTED));
    // Check that the object is accessible
    ASSERT_NE(obj->ClassAddr<Class>(), nullptr);
}

class PromotionRemSetChecker : public GCListener {
public:
    PromotionRemSetChecker(VMHandle<coretypes::Array> *array, VMHandle<coretypes::String> *string)
        : array_(array), string_(string)
    {
    }

    void GCPhaseStarted(GCPhase phase) override
    {
        if (phase != GCPhase::GC_PHASE_MARK_YOUNG) {
            return;
        }
        // Before marking young all remsets must by actual
        CheckRemSets();
    }

    bool CheckRemSets()
    {
        Region *refRegion = ObjectToRegion(string_->GetPtr());
        found_ = false;
        refRegion->GetRemSet()->IterateOverObjects([this](ObjectHeader *obj) {
            if (obj == array_->GetPtr()) {
                found_ = true;
            }
        });
        return found_;
    }

    bool IsFound() const
    {
        return found_;
    }

private:
    VMHandle<coretypes::Array> *array_;
    VMHandle<coretypes::String> *string_;
    bool found_ = false;
};

TEST_F(G1GCPromotionTestNoSinglePass, TestPromotedRegionHasValidRemSets)
{
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    Runtime *runtime = Runtime::GetCurrent();
    LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    GC *gc = runtime->GetPandaVM()->GetGC();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    VMHandle<coretypes::String> string(thread, ObjectAllocator::AllocString(1));
    ASSERT_TRUE(ObjectToRegion(string.GetPtr())->IsYoung());
    {
        ScopedNativeCodeThread sn(thread);
        // Move string to tenured
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }
    ASSERT_TRUE(ObjectToRegion(string.GetPtr())->HasFlag(IS_OLD));

    // Allocate an array which ocuppies more than half region.
    // This array will be promoted.
    auto *arrayClass = runtime->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::ARRAY_STRING);
    size_t elemSize = arrayClass->GetComponentSize();
    size_t arraySize = DEFAULT_REGION_SIZE / 2;
    size_t arrayLength = arraySize / elemSize + 1;
    VMHandle<coretypes::Array> array(thread, ObjectAllocator::AllocArray(arrayLength, ClassRoot::ARRAY_STRING, false));
    ASSERT_FALSE(array->IsForwarded());
    Region *arrayRegion = ObjectToRegion(array.GetPtr());
    ASSERT_TRUE(arrayRegion->IsYoung());
    array->Set(0, string.GetPtr());

    PromotionRemSetChecker listener(&array, &string);
    gc->AddListener(&listener);
    {
        ScopedNativeCodeThread sn(thread);
        // Promote array's regions to tenured
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
        ASSERT_FALSE(listener.IsFound());
    }
    // Check the array was promoted.
    ASSERT_TRUE(arrayRegion == ObjectToRegion(array.GetPtr()));

    // remset is not fully updated during mixed collection
    ProcessDirtyCards(static_cast<G1GC<PandaAssemblyLanguageConfig> *>(gc));
    // Check remsets
    ASSERT_TRUE(listener.CheckRemSets());
}

class InterruptGCListener : public GCListener {
public:
    explicit InterruptGCListener(VMHandle<coretypes::Array> *array) : array_(array) {}

    void GCPhaseStarted(GCPhase phase) override
    {
        if (phase != GCPhase::GC_PHASE_MARK) {
            return;
        }
        GC *gc = Runtime::GetCurrent()->GetPandaVM()->GetGC();
        {
            ScopedManagedCodeThread s(ManagedThread::GetCurrent());
            // Allocate an object to add it into SATB buffer
            ObjectAllocator::AllocObjectInYoung();
        }
        // Set interrupt flag
        gc->OnWaitForIdleFail();
    }

    void GCPhaseFinished(GCPhase phase) override
    {
        if (phase != GCPhase::GC_PHASE_MARK) {
            return;
        }
        Region *region = ObjectToRegion((*array_)->Get<ObjectHeader *>(0));
        // Check the object array[0] is not marked
        EXPECT_FALSE(region->GetMarkBitmap()->Test((*array_)->Get<ObjectHeader *>(0)));
        // Check GC haven't calculated live bytes for the region
        EXPECT_EQ(0, region->GetLiveBytes());
        // Check GC has cleared SATB buffer
        MTManagedThread *thread = MTManagedThread::GetCurrent();
        EXPECT_NE(nullptr, thread->GetPreBuff());
        EXPECT_EQ(0, thread->GetPreBuff()->size());
    }

private:
    VMHandle<coretypes::Array> *array_;
};

TEST_F(G1GCTest, TestInterruptConcurrentMarking)
{
    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();

    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<coretypes::Array> array;

    array = VMHandle<coretypes::Array>(thread, ObjectAllocator::AllocArray(1, ClassRoot::ARRAY_STRING, false));
    array->Set(0, ObjectAllocator::AllocString(1));

    {
        ScopedNativeCodeThread sn(thread);
        // Propogate young objects -> tenured
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);

        // Clear live bytes to check that concurrent marking will not calculate them
        Region *region = ObjectToRegion(array->Get<ObjectHeader *>(0));
        ASSERT_TRUE(region != nullptr);
        region->SetLiveBytes(0);

        InterruptGCListener listener(&array);
        gc->AddListener(&listener);
        // Trigger concurrent marking
        GCTask task1(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);
        task1.Run(*gc);
    }
}

class NullRefListener : public GCListener {
public:
    explicit NullRefListener(VMHandle<coretypes::Array> *array) : array_(array) {}

    void GCPhaseStarted(GCPhase phase) override
    {
        if (phase != GCPhase::GC_PHASE_MARK) {
            return;
        }
        (*array_)->Set(0, static_cast<ObjectHeader *>(nullptr));
    }

private:
    VMHandle<coretypes::Array> *array_;
};

TEST_F(G1GCTest, TestGarbageBytesCalculation)
{
    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);

    VMHandle<coretypes::Array> array;

    // Allocate objects of different sizes.
    // Mixed regions should be choosen according to the largest garbage.
    // Allocate an array of length 2. 2 because the array's size must be 8 bytes aligned
    array = VMHandle<coretypes::Array>(thread, ObjectAllocator::AllocArray(2, ClassRoot::ARRAY_STRING, false));
    ASSERT_TRUE(ObjectToRegion(array.GetPtr())->HasFlag(RegionFlag::IS_EDEN));
    // The same for string. The instance size must be 8-bytes aligned.
    array->Set(0, ObjectAllocator::AllocString(8U));
    array->Set(1, ObjectAllocator::AllocString(8U));
    ASSERT_TRUE(ObjectToRegion(array->Get<ObjectHeader *>(0))->HasFlag(RegionFlag::IS_EDEN));

    size_t arraySize = GetObjectSize(array.GetPtr());
    size_t strSize = GetObjectSize(array->Get<ObjectHeader *>(0));

    {
        ScopedNativeCodeThread sn(thread);
        // Propogate young objects -> tenured
        GCTask task(GCTaskCause::YOUNG_GC_CAUSE);
        task.Run(*gc);
    }
    // check the array and the string are in the same tenured region
    ASSERT_EQ(ObjectToRegion(array.GetPtr()), ObjectToRegion(array->Get<ObjectHeader *>(0)));
    ASSERT_TRUE(ObjectToRegion(array.GetPtr())->HasFlag(RegionFlag::IS_OLD));

    ObjectAllocator::AllocObjectInYoung();
    array->Set(1, static_cast<ObjectHeader *>(nullptr));

    NullRefListener listener(&array);
    gc->AddListener(&listener);
    {
        ScopedNativeCodeThread sn(thread);
        // Prepare for mixed GC, start concurrent marking and calculate garbage for regions
        GCTask task2(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);
        task2.Run(*gc);
    }

    Region *region = ObjectToRegion(array.GetPtr());
    ASSERT_EQ(arraySize + strSize, region->GetLiveBytes());
    ASSERT_EQ(strSize, region->GetGarbageBytes());
}

TEST_F(G1GCTest, NonMovableClearingDuringConcurrentPhaseTest)
{
    Runtime *runtime = Runtime::GetCurrent();
    LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto objAllocator = Runtime::GetCurrent()->GetPandaVM()->GetGC()->GetObjectAllocator();
    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();

    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    // NOLINTBEGIN(readability-magic-numbers)
    size_t arrayLength = GetHumongousArrayLength(ClassRoot::ARRAY_STRING) - 50;
    coretypes::Array *firstNonMovableObj = nullptr;
    coretypes::Array *secondNonMovableObj = nullptr;
    uintptr_t prevYoungAddr = 0;

    Class *klass = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)
                       ->GetClass(ctx.GetStringArrayClassDescriptor());
    ASSERT_NE(klass, nullptr);
    firstNonMovableObj = coretypes::Array::Create(klass, arrayLength, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    secondNonMovableObj = coretypes::Array::Create(klass, arrayLength, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    ASSERT_EQ(true, ObjectToRegion(firstNonMovableObj)->HasFlag(RegionFlag::IS_NONMOVABLE));
    ASSERT_EQ(true, ObjectToRegion(secondNonMovableObj)->HasFlag(RegionFlag::IS_NONMOVABLE));
    coretypes::String *youngObj = coretypes::String::CreateEmptyString(ctx, runtime->GetPandaVM());
    firstNonMovableObj->Set(0, youngObj);
    prevYoungAddr = ToUintPtr(youngObj);

    VMHandle<coretypes::Array> secondNonMovableObjPtr(thread, secondNonMovableObj);

    {
        [[maybe_unused]] HandleScope<ObjectHeader *> firstScope(thread);
        VMHandle<coretypes::Array> firstNonMovableObjPtr(thread, firstNonMovableObj);
        {
            ScopedNativeCodeThread sn(thread);
            GCTask task(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);
            task.Run(*gc);
        }

        auto youngObj2 = static_cast<coretypes::String *>(firstNonMovableObjPtr->Get<ObjectHeader *>(0));
        // Check GC has moved the young obj
        ASSERT_NE(prevYoungAddr, ToUintPtr(youngObj2));
        // Check young object is accessible
        ASSERT_EQ(0, youngObj2->GetLength());
    }

    // Check that all objects are alive
    ASSERT_EQ(true, objAllocator->ContainObject(firstNonMovableObj));
    ASSERT_EQ(true, objAllocator->ContainObject(secondNonMovableObj));
    ASSERT_EQ(true, objAllocator->IsLive(firstNonMovableObj));
    ASSERT_EQ(true, objAllocator->IsLive(secondNonMovableObj));
    // Check that the first object is accessible
    bool foundFirstObject = false;
    objAllocator->IterateOverObjects([&firstNonMovableObj, &foundFirstObject](ObjectHeader *object) {
        if (firstNonMovableObj == object) {
            foundFirstObject = true;
        }
    });
    ASSERT_EQ(true, foundFirstObject);

    // So, try to remove the first non movable object:
    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);
        task.Run(*gc);
    }

    // Check that the second object is still alive
    ASSERT_EQ(true, objAllocator->ContainObject(secondNonMovableObj));
    ASSERT_EQ(true, objAllocator->IsLive(secondNonMovableObj));
    // Check that the first object is dead
    objAllocator->IterateOverObjects(
        [&firstNonMovableObj](ObjectHeader *object) { ASSERT_NE(firstNonMovableObj, object); });
}

TEST_F(G1GCTest, HumongousClearingDuringConcurrentPhaseTest)
{
    Runtime *runtime = Runtime::GetCurrent();
    LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
    auto objAllocator = Runtime::GetCurrent()->GetPandaVM()->GetGC()->GetObjectAllocator();
    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    MTManagedThread *thread = MTManagedThread::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();

    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    size_t arrayLength = GetHumongousArrayLength(ClassRoot::ARRAY_STRING);
    coretypes::Array *firstHumongousObj = nullptr;
    coretypes::Array *secondHumongousObj = nullptr;
    uintptr_t prevYoungAddr = 0;

    Class *klass = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)
                       ->GetClass(ctx.GetStringArrayClassDescriptor());
    ASSERT_NE(klass, nullptr);
    firstHumongousObj = coretypes::Array::Create(klass, arrayLength, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    secondHumongousObj = coretypes::Array::Create(klass, arrayLength, SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
    ASSERT_EQ(true, ObjectToRegion(firstHumongousObj)->HasFlag(RegionFlag::IS_LARGE_OBJECT));
    ASSERT_EQ(true, ObjectToRegion(secondHumongousObj)->HasFlag(RegionFlag::IS_LARGE_OBJECT));
    coretypes::String *youngObj = coretypes::String::CreateEmptyString(ctx, runtime->GetPandaVM());
    firstHumongousObj->Set(0, youngObj);
    prevYoungAddr = ToUintPtr(youngObj);

    VMHandle<coretypes::Array> secondHumongousObjPtr(thread, secondHumongousObj);

    {
        HandleScope<ObjectHeader *> firstScope(thread);
        VMHandle<coretypes::Array> firstHumongousObjPtr(thread, firstHumongousObj);
        {
            ScopedNativeCodeThread sn(thread);
            GCTask task(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);
            task.Run(*gc);
        }

        auto youngObj2 = static_cast<coretypes::String *>(firstHumongousObjPtr->Get<ObjectHeader *>(0));
        // Check GC has moved the young obj
        ASSERT_NE(prevYoungAddr, ToUintPtr(youngObj2));
        // Check young object is accessible
        ASSERT_EQ(0, youngObj2->GetLength());
    }

    // Check that all objects are alive
    ASSERT_EQ(true, objAllocator->ContainObject(firstHumongousObj));
    ASSERT_EQ(true, objAllocator->ContainObject(secondHumongousObj));
    ASSERT_EQ(true, objAllocator->IsLive(firstHumongousObj));
    ASSERT_EQ(true, objAllocator->IsLive(secondHumongousObj));
    // Check that the first object is accessible
    bool foundFirstObject = false;
    objAllocator->IterateOverObjects([&firstHumongousObj, &foundFirstObject](ObjectHeader *object) {
        if (firstHumongousObj == object) {
            foundFirstObject = true;
        }
    });
    ASSERT_EQ(true, foundFirstObject);

    {
        ScopedNativeCodeThread sn(thread);
        // So, try to remove the first non movable object:
        GCTask task(GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE);
        task.Run(*gc);
    }

    // Check that the second object is still alive
    ASSERT_EQ(true, objAllocator->ContainObject(secondHumongousObj));
    ASSERT_EQ(true, objAllocator->IsLive(secondHumongousObj));
    // Check that the first object is dead
    objAllocator->IterateOverObjects(
        [&firstHumongousObj](ObjectHeader *object) { ASSERT_NE(firstHumongousObj, object); });
}

class G1FullGCTest : public G1GCTest {
public:
    explicit G1FullGCTest(uint32_t fullGcRegionFragmentationRate = 0)
        : G1GCTest(CreateOptions(fullGcRegionFragmentationRate))
    {
    }

    static RuntimeOptions CreateOptions(uint32_t fullGcRegionFragmentationRate)
    {
        RuntimeOptions options = CreateDefaultOptions();
        options.SetInitYoungSpaceSize(YOUNG_SIZE);
        options.SetYoungSpaceSize(YOUNG_SIZE);
        options.SetHeapSizeLimit(HEAP_SIZE);
        options.SetG1FullGcRegionFragmentationRate(fullGcRegionFragmentationRate);
        return options;
    }

    static constexpr size_t NumYoungRegions()
    {
        return YOUNG_SIZE / DEFAULT_REGION_SIZE;
    }

    static constexpr size_t NumRegions()
    {
        // Region count without reserved region for Full GC
        return HEAP_SIZE / DEFAULT_REGION_SIZE - 1U;
    }

    size_t RefArrayLengthFitIntoRegion(size_t numRegions)
    {
        Runtime *runtime = Runtime::GetCurrent();
        LanguageContext ctx = runtime->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        auto *klass = runtime->GetClassLinker()->GetExtension(ctx)->GetClassRoot(ClassRoot::ARRAY_STRING);
        size_t elemSize = klass->GetComponentSize();
        // NOLINTNEXTLINE(clang-analyzer-core.DivideZero)
        return (numRegions * DEFAULT_REGION_SIZE - sizeof(coretypes::Array) - Region::HeadSize()) / elemSize;
    }

    void FillHeap(size_t numRegions, VMHandle<coretypes::Array> &holder, size_t startIndex)
    {
        constexpr size_t STRING_LENGTH = StringLengthFitIntoRegion(1);
        EXPECT_LE(numRegions, holder->GetLength());
        for (size_t i = 0; i < numRegions; ++i) {
            ObjectHeader *obj = ObjectAllocator::AllocString(STRING_LENGTH);
            EXPECT_NE(nullptr, obj);
            holder->Set(startIndex + i, obj);
        }
    }

    static constexpr size_t YOUNG_SIZE = 1_MB;
    static constexpr size_t HEAP_SIZE = 4_MB;
    static constexpr size_t NUM_NONMOVABLE_REGIONS_FOR_RUNTIME = 1;
};

TEST_F(G1FullGCTest, TestFullGCCollectsNonRegularObjects)
{
    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    ObjectHeader *humongousObj = ObjectAllocator::AllocString(GetHumongousStringLength());
    ObjectHeader *nonmovableObj = AllocNonMovableObject();

    {
        ScopedNativeCodeThread sn(thread);
        GCTask task(GCTaskCause::EXPLICIT_CAUSE);
        task.Run(*gc);
    }

    PandaVector<Region *> nonregularRegions = GetAllocator()->GetNonRegularRegions();
    for (Region *region : nonregularRegions) {
        if (region->HasFlag(IS_LARGE_OBJECT)) {
            ASSERT_NE(humongousObj, region->GetLargeObject());
        } else if (region->HasFlag(IS_NONMOVABLE)) {
            if (region->Begin() <= ToUintPtr(nonmovableObj) && ToUintPtr(nonmovableObj) < region->End()) {
                ASSERT_FALSE(region->GetLiveBitmap()->Test(nonmovableObj));
            }
        } else {
            FAIL() << "Unknown region type";
        }
    }
}

TEST_F(G1FullGCTest, TestFullGCFreeHumongousBeforeTenuredCollection)
{
    constexpr size_t NUM_REGIONS_FOR_HUMONGOUS = 4;

    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<coretypes::Array> holder(thread, ObjectAllocator::AllocArray(NumRegions(), ClassRoot::ARRAY_STRING, true));
    coretypes::String *humongousObj =
        ObjectAllocator::AllocString(StringLengthFitIntoRegion(NUM_REGIONS_FOR_HUMONGOUS));
    holder->Set(0, humongousObj);
    size_t numFreeRegions =
        NumRegions() - NumYoungRegions() - NUM_NONMOVABLE_REGIONS_FOR_RUNTIME - NUM_REGIONS_FOR_HUMONGOUS;
    FillHeap(numFreeRegions, holder, 1);  // occupy 4 tenured regions and 3 young regions
    // move 3 young regions to tenured space.
    gc->WaitForGCInManaged(GCTask(GCTaskCause::YOUNG_GC_CAUSE));
    // now tenured space is full. Fill young space.
    FillHeap(NumYoungRegions(), holder, numFreeRegions + 1);
    // At this point we have filled 4 tenured regions and 4 young regions and 3 free tenured regions.
    // We cannot do young GC because there are not enough free regions in tenured to move 4 young regions.
    // Check we are OOM
    ASSERT_EQ(nullptr, ObjectAllocator::AllocObjectInYoung());
    // Forget humongous_obj
    holder->Set(0, static_cast<ObjectHeader *>(nullptr));
    gc->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
    // We should have 2 free regions but during allocation we reserve 1 region for full GC.
    // So we can allocate only one region.
    ASSERT_NE(nullptr, ObjectAllocator::AllocObjectInYoung());
}

TEST_F(G1FullGCTest, TestRemSetsAndYoungCardsAfterFailedFullGC)
{
    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    CardTable *cardTable = gc->GetCardTable();
    ASSERT_NE(nullptr, cardTable);
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<coretypes::Array> holder(thread, ObjectAllocator::AllocArray(NumRegions(), ClassRoot::ARRAY_STRING, true));
    size_t numFreeRegions = NumRegions() - NumYoungRegions() - NUM_NONMOVABLE_REGIONS_FOR_RUNTIME + 1U;
    FillHeap(numFreeRegions, holder, 0);  // occupy 8 tenured regions and 3 young regions
    VMHandle<coretypes::Array> youngArray(
        thread, ObjectAllocator::AllocArray(RefArrayLengthFitIntoRegion(1), ClassRoot::ARRAY_STRING, false));
    ASSERT(ObjectToRegion(youngArray.GetPtr())->IsEden());
    youngArray->Set(0, holder->Get<ObjectHeader *>(0));
    uintptr_t tenuredAddrBeforeGc = ToUintPtr(holder->Get<ObjectHeader *>(0));
    // Trigger FullGC by allocating an object in full young. It should fail because there is no tenured space to move 4
    // young regions.
    ASSERT_EQ(nullptr, ObjectAllocator::AllocObjectInYoung());
    uintptr_t tenuredAddrAfterGc = ToUintPtr(holder->Get<ObjectHeader *>(0));
    // Check GC moved tenured regions
    ASSERT_NE(tenuredAddrBeforeGc, tenuredAddrAfterGc);
    // Check FullGC updates refs in young correctly in case it cannot collect young.
    ASSERT_EQ(holder->Get<ObjectHeader *>(0), youngArray->Get<ObjectHeader *>(0));
    // Check remsets.
    Region *youngRegion = ObjectToRegion(youngArray.GetPtr());
    ASSERT_TRUE(youngRegion->IsEden());
    Region *tenuredRegion = ObjectToRegion(holder->Get<ObjectHeader *>(0));
    ASSERT_TRUE(tenuredRegion->HasFlag(IS_OLD));
    bool hasObject = false;
    tenuredRegion->GetRemSet()->IterateOverObjects(
        [&hasObject, &youngArray](ObjectHeader *obj) { hasObject |= obj == youngArray.GetPtr(); });
    ASSERT_FALSE(hasObject);
    // Check young cards
    ASSERT_EQ(NumYoungRegions(), GetAllocator()->GetYoungRegions().size());
    for (Region *region : GetAllocator()->GetYoungRegions()) {
        uintptr_t begin = ToUintPtr(region);
        uintptr_t end = region->End();
        while (begin < end) {
            ASSERT_TRUE(cardTable->GetCardPtr(begin)->IsYoung());
            begin += CardTable::GetCardSize();
        }
    }
}

TEST_F(G1FullGCTest, TestFullGCGenericFlow)
{
    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    CardTable *cardTable = gc->GetCardTable();
    ASSERT_NE(nullptr, cardTable);
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<coretypes::Array> holder(thread, ObjectAllocator::AllocArray(NumRegions(), ClassRoot::ARRAY_STRING, true));
    size_t numFreeRegions = NumRegions() - NumYoungRegions() - NUM_NONMOVABLE_REGIONS_FOR_RUNTIME + 1U;
    FillHeap(numFreeRegions, holder, 0);  // occupy 8 tenured regions and 3 young regions
    VMHandle<coretypes::Array> youngArray(
        thread, ObjectAllocator::AllocArray(RefArrayLengthFitIntoRegion(1), ClassRoot::ARRAY_STRING, false));
    ASSERT(ObjectToRegion(youngArray.GetPtr())->IsEden());
    youngArray->Set(0, holder->Get<ObjectHeader *>(0));
    // Check we are OOM
    ASSERT_EQ(nullptr, ObjectAllocator::AllocObjectInYoung());
    uintptr_t tenuredAddrBeforeGc = ToUintPtr(holder->Get<ObjectHeader *>(0));
    // Forget two tenured regions
    holder->Set(1U, static_cast<ObjectHeader *>(nullptr));
    holder->Set(2U, static_cast<ObjectHeader *>(nullptr));
    // Now there should be enough space in tenured to move young
    gc->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
    ASSERT_NE(nullptr, ObjectAllocator::AllocObjectInYoung());
    uintptr_t tenuredAddrAfterGc = ToUintPtr(holder->Get<ObjectHeader *>(0));
    // Check GC moved tenured regions
    ASSERT_NE(tenuredAddrBeforeGc, tenuredAddrAfterGc);
    // Check FullGC updates refs in young correctly in case it cannot collect young.
    ASSERT_EQ(holder->Get<ObjectHeader *>(0), youngArray->Get<ObjectHeader *>(0));
}

TEST_F(G1FullGCTest, TestFullGCResetTenuredRegions)
{
    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    CardTable *cardTable = gc->GetCardTable();
    ASSERT_NE(nullptr, cardTable);
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<coretypes::Array> holder(thread, ObjectAllocator::AllocArray(NumRegions(), ClassRoot::ARRAY_STRING, true));
    // Fill almost all regions (3 tenured regions will be free)
    size_t numFreeRegions = NumRegions() - NUM_NONMOVABLE_REGIONS_FOR_RUNTIME;
    size_t numFilledRegions = RoundDown(numFreeRegions, NumYoungRegions());
    FillHeap(numFilledRegions, holder, 0);
    // Foreget all objects
    for (size_t i = 0; i < numFilledRegions; ++i) {
        holder->Set(i, static_cast<ObjectHeader *>(nullptr));
    }
    gc->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
    // Fill almost all regions (3 tenured regions will be free)
    // We should be able to allocate all objects because FullGC should reset old tenured regions.
    FillHeap(numFilledRegions, holder, 0);
}

template <uint32_t REGION_FRAGMENTATION_RATE>
class G1FullGCWithRegionFragmentationRate : public G1FullGCTest {
public:
    G1FullGCWithRegionFragmentationRate() : G1FullGCTest(REGION_FRAGMENTATION_RATE) {}
};

class FullGcRegionFragmentationRateOptionNever : public G1FullGCWithRegionFragmentationRate<100U> {};

TEST_F(FullGcRegionFragmentationRateOptionNever, TestG1FullGcRegionFragmentationRateOptionNever)
{
    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    CardTable *cardTable = gc->GetCardTable();
    ASSERT_NE(nullptr, cardTable);
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<coretypes::Array> holder(thread, ObjectAllocator::AllocArray(NumRegions(), ClassRoot::ARRAY_STRING, true));
    // Fill one region
    FillHeap(1, holder, 0);
    // Save ref to young object
    auto object = holder->Get<ObjectHeader *>(0);
    gc->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
    // Check that we moved this young object
    ASSERT_NE(holder->Get<ObjectHeader *>(0), object);
    // Save ref to tenured object
    object = holder->Get<ObjectHeader *>(0);
    gc->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
    // Check that we don't move this object
    ASSERT_EQ(holder->Get<ObjectHeader *>(0), object);
}

class FullGcRegionFragmentationRateOptionAlways : public G1FullGCWithRegionFragmentationRate<0> {};

TEST_F(FullGcRegionFragmentationRateOptionAlways, TestG1FullGcRegionFragmentationRateOptionAlways)
{
    Runtime *runtime = Runtime::GetCurrent();
    GC *gc = runtime->GetPandaVM()->GetGC();
    CardTable *cardTable = gc->GetCardTable();
    ASSERT_NE(nullptr, cardTable);
    ManagedThread *thread = ManagedThread::GetCurrent();
    ScopedManagedCodeThread s(thread);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
    VMHandle<coretypes::Array> holder(thread, ObjectAllocator::AllocArray(NumRegions(), ClassRoot::ARRAY_STRING, true));
    // Fill one region
    FillHeap(1, holder, 0);
    // Save ref to young object
    auto object = holder->Get<ObjectHeader *>(0);
    gc->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
    // Check that we moved this young object
    ASSERT_NE(holder->Get<ObjectHeader *>(0), object);
    // Save ref to tenured object
    object = holder->Get<ObjectHeader *>(0);
    gc->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
    // Check that we moved this object
    ASSERT_NE(holder->Get<ObjectHeader *>(0), object);
}

class G1FullGCOOMTest : public G1GCTest {
public:
    G1FullGCOOMTest() : G1GCTest(CreateOOMOptions())
    {
        thread_ = MTManagedThread::GetCurrent();
        ASSERT(thread_ != nullptr);
        thread_->ManagedCodeBegin();
    }

    NO_COPY_SEMANTIC(G1FullGCOOMTest);
    NO_MOVE_SEMANTIC(G1FullGCOOMTest);

    static RuntimeOptions CreateOOMOptions()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetCompilerEnableJit(false);
        options.SetShouldInitializeIntrinsics(false);
        // GC options
        constexpr size_t HEAP_SIZE_LIMIT_TEST = 16_MB;
        options.SetRunGcInPlace(true);
        options.SetGcType("g1-gc");
        options.SetHeapSizeLimit(HEAP_SIZE_LIMIT_TEST);
        options.SetGcTriggerType("debug-never");
        options.SetG1NumberOfTenuredRegionsAtMixedCollection(0);
        options.SetGcWorkersCount(0);
        return options;
    }

    ~G1FullGCOOMTest() override
    {
        thread_->ManagedCodeEnd();
    }

protected:
    MTManagedThread *thread_;  // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(G1FullGCOOMTest, AllocateBy1Region)
{
    constexpr size_t OBJECT_SIZE = AlignUp(static_cast<size_t>(DEFAULT_REGION_SIZE * 0.8F), DEFAULT_ALIGNMENT_IN_BYTES);
    {
        [[maybe_unused]] HandleScope<ark::ObjectHeader *> scope(thread_);
        auto *g1Allocator =
            static_cast<ObjectAllocatorG1<> *>(Runtime::GetCurrent()->GetPandaVM()->GetGC()->GetObjectAllocator());
        // Fill tenured space by garbage
        do {
            VMHandle<ObjectHeader> handle(thread_, ObjectAllocator::AllocString(OBJECT_SIZE));
            ASSERT_NE(handle.GetPtr(), nullptr) << "Must be correctly allocated object in non-full heap";
            // Move new object to tenured
            Runtime::GetCurrent()->GetPandaVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::YOUNG_GC_CAUSE));
        } while (g1Allocator->HaveTenuredSize(2U));
        ASSERT_TRUE(g1Allocator->HaveTenuredSize(1));
        // Allocate one young region
        VMHandle<ObjectHeader> handle1(thread_, ObjectAllocator::AllocString(OBJECT_SIZE));
        ASSERT_NE(handle1.GetPtr(), nullptr) << "Must be correctly allocated object. Heap has a lot of garbage";
        // Try to move alone young region to last tenured region
        Runtime::GetCurrent()->GetPandaVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::YOUNG_GC_CAUSE));
        // Fully fill young space
        while (g1Allocator->GetHeapSpace()->GetCurrentFreeYoungSize() > 0) {
            auto *youngObj = ObjectAllocator::AllocString(OBJECT_SIZE);
            ASSERT_NE(youngObj, nullptr) << "Must allocate in free young space";
        }
    }
    ASSERT_NE(ObjectAllocator::AllocString(OBJECT_SIZE), nullptr)
        << "We must correctly allocate object in non-full heap";
}

TEST_F(G1FullGCOOMTest, PinUnpinObject)
{
    constexpr size_t OBJECT_SIZE = AlignUp(static_cast<size_t>(DEFAULT_REGION_SIZE * 0.8F), DEFAULT_ALIGNMENT_IN_BYTES);
    auto *g1Allocator =
        static_cast<ObjectAllocatorG1<> *>(Runtime::GetCurrent()->GetPandaVM()->GetGC()->GetObjectAllocator());
    {
        [[maybe_unused]] HandleScope<ark::ObjectHeader *> scope(thread_);
        // Fill tenured space by garbage
        do {
            VMHandle<ObjectHeader> handle(thread_, ObjectAllocator::AllocString(OBJECT_SIZE));
            ASSERT_NE(handle.GetPtr(), nullptr) << "Must be correctly allocated object in non-full heap";
            // Move new object to tenured
            Runtime::GetCurrent()->GetPandaVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::YOUNG_GC_CAUSE));
        } while (g1Allocator->HaveTenuredSize(2U));
        ASSERT_TRUE(g1Allocator->HaveTenuredSize(1));
        // Allocate one young region
        VMHandle<ObjectHeader> handle1(thread_, ObjectAllocator::AllocString(OBJECT_SIZE));
        ASSERT_NE(handle1.GetPtr(), nullptr) << "Must be correctly allocated object in young";
        ASSERT_TRUE(ObjectToRegion(handle1.GetPtr())->IsYoung());
        // Pin object in young region
        g1Allocator->PinObject(handle1.GetPtr());
        // Try to move young region to last tenured region
        Runtime::GetCurrent()->GetPandaVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::YOUNG_GC_CAUSE));
        ASSERT_TRUE(ObjectToRegion(handle1.GetPtr())->HasFlag(RegionFlag::IS_OLD));
        // Just allocate one object in young
        auto *youngObj = ObjectAllocator::AllocString(OBJECT_SIZE);
        ASSERT_NE(youngObj, nullptr) << "Must allocate in free young space";
        // Run Full GC
        Runtime::GetCurrent()->GetPandaVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::OOM_CAUSE));
        // Check "pinned" region
        ASSERT_TRUE(ObjectToRegion(handle1.GetPtr())->HasFlag(RegionFlag::IS_OLD));
        ASSERT_TRUE(ObjectToRegion(handle1.GetPtr())->HasPinnedObjects());
        // Unpin object
        g1Allocator->UnpinObject(handle1.GetPtr());
        // Check "unpinned" region
        ASSERT_TRUE(ObjectToRegion(handle1.GetPtr())->HasFlag(RegionFlag::IS_OLD));
        ASSERT_FALSE(ObjectToRegion(handle1.GetPtr())->HasPinnedObjects());
    }
    // Run yet FullGC after unpinning
    Runtime::GetCurrent()->GetPandaVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::OOM_CAUSE));
    g1Allocator->IterateOverObjects([](ObjectHeader *obj) {
        ASSERT_FALSE(ObjectToRegion(obj)->HasPinnedObjects()) << "Along pinned object was unpinned before GC";
    });
}

static void PinUnpinTest(SpaceType requestedSpaceType, size_t objectSize = 1_KB)
{
    ASSERT_TRUE(IsHeapSpace(requestedSpaceType));
    Runtime::Create(G1FullGCOOMTest::CreateOOMOptions());
    auto *thread = MTManagedThread::GetCurrent();
    ASSERT_NE(thread, nullptr);
    thread->ManagedCodeBegin();
    auto *g1Allocator =
        static_cast<ObjectAllocatorG1<> *>(Runtime::GetCurrent()->GetPandaVM()->GetGC()->GetObjectAllocator());
    {
        [[maybe_unused]] HandleScope<ark::ObjectHeader *> scope(thread);
        constexpr size_t OBJ_ELEMENT_SIZE = 64;
        auto *addressBeforeGc =
            ObjectAllocator::AllocArray(objectSize / OBJ_ELEMENT_SIZE, ClassRoot::ARRAY_I64,
                                        requestedSpaceType == SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
        ASSERT_NE(addressBeforeGc, nullptr);
        VMHandle<ObjectHeader> handle(thread, addressBeforeGc);
        SpaceType objSpaceType =
            PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(static_cast<void *>(handle.GetPtr()));
        ASSERT_EQ(objSpaceType, requestedSpaceType);
        g1Allocator->PinObject(handle.GetPtr());
        // Run GC - try to move objects
        Runtime::GetCurrent()->GetPandaVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
        ASSERT_EQ(addressBeforeGc, handle.GetPtr()) << "Pinned object must not moved";
        g1Allocator->UnpinObject(handle.GetPtr());
        ASSERT_FALSE(ObjectToRegion(handle.GetPtr())->HasPinnedObjects());
    }
    thread->ManagedCodeEnd();
    Runtime::Destroy();
}

TEST(G1GCPinnigTest, PinUnpinRegularObjectTest)
{
    PinUnpinTest(SpaceType::SPACE_TYPE_OBJECT);
}

TEST(G1GCPinnigTest, PinUnpinHumongousObjectTest)
{
    constexpr size_t HUMONGOUS_OBJECT_FOR_PINNING_SIZE = 4_MB;
    PinUnpinTest(SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT, HUMONGOUS_OBJECT_FOR_PINNING_SIZE);
}

TEST(G1GCPinnigTest, PinUnpinNonMovableObjectTest)
{
    PinUnpinTest(SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
}

class G1GCPromotePinnedRegionTest : public G1GCTest {
public:
    G1GCPromotePinnedRegionTest() : G1GCTest(CreateOptions())
    {
        thread_ = MTManagedThread::GetCurrent();
        ASSERT(thread_ != nullptr);
        thread_->ManagedCodeBegin();
    }

    NO_COPY_SEMANTIC(G1GCPromotePinnedRegionTest);
    NO_MOVE_SEMANTIC(G1GCPromotePinnedRegionTest);

    static RuntimeOptions CreateOptions()
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetCompilerEnableJit(false);
        options.SetShouldInitializeIntrinsics(false);
        // GC options
        constexpr size_t HEAP_SIZE_LIMIT_TEST = 16_MB;
        options.SetRunGcInPlace(true);
        options.SetGcType("g1-gc");
        options.SetHeapSizeLimit(HEAP_SIZE_LIMIT_TEST);
        options.SetGcTriggerType("debug-never");
        options.SetG1NumberOfTenuredRegionsAtMixedCollection(0);
        return options;
    }

    ~G1GCPromotePinnedRegionTest() override
    {
        thread_->ManagedCodeEnd();
    }

protected:
    MTManagedThread *thread_;  // NOLINT(misc-non-private-member-variables-in-classes)
};

TEST_F(G1GCPromotePinnedRegionTest, CompactingRegularStringObjectAndPromoteToMixedTLABRegionAndUnpin)
{
    auto *g1Allocator =
        static_cast<ObjectAllocatorG1<> *>(Runtime::GetCurrent()->GetPandaVM()->GetGC()->GetObjectAllocator());
    {
        [[maybe_unused]] HandleScope<ark::ObjectHeader *> scope(thread_);
        constexpr size_t OBJECT_SIZE = 1_KB;
        auto *addressBeforeGc = ObjectAllocator::AllocString(OBJECT_SIZE);
        ASSERT_NE(addressBeforeGc, nullptr);
        VMHandle<ObjectHeader> handle1(thread_, addressBeforeGc);
        g1Allocator->PinObject(handle1.GetPtr());
        // Run GC - promote pinned region
        Runtime::GetCurrent()->GetPandaVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
        ASSERT_TRUE(ObjectToRegion(handle1.GetPtr())->IsMixedTLAB());
        ASSERT_EQ(addressBeforeGc, handle1.GetPtr()) << "Pinned object must not moved";
        g1Allocator->UnpinObject(handle1.GetPtr());
        Runtime::GetCurrent()->GetPandaVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
    }
}

TEST_F(G1GCPromotePinnedRegionTest, PromoteTLABRegionToMixedTLABAndTestIsInAllocRangeMethod)
{
    auto *g1Allocator =
        static_cast<ObjectAllocatorG1<> *>(Runtime::GetCurrent()->GetPandaVM()->GetGC()->GetObjectAllocator());
    [[maybe_unused]] HandleScope<ark::ObjectHeader *> scope(thread_);
    constexpr size_t OBJECT_SIZE = 1_KB;
    auto *addressBeforeGc = ObjectAllocator::AllocString(OBJECT_SIZE);
    auto *addressBeforeGc2 = ObjectAllocator::AllocString(OBJECT_SIZE);
    ASSERT_NE(addressBeforeGc, nullptr);
    ASSERT_NE(addressBeforeGc2, nullptr);
    VMHandle<ObjectHeader> handle1(thread_, addressBeforeGc);
    VMHandle<ObjectHeader> handle2(thread_, addressBeforeGc2);
    g1Allocator->PinObject(handle1.GetPtr());
    g1Allocator->PinObject(handle2.GetPtr());
    // Run GC - promote pinned region
    Runtime::GetCurrent()->GetPandaVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
    ASSERT_TRUE(ObjectToRegion(handle1.GetPtr())->IsMixedTLAB());
    ASSERT_EQ(addressBeforeGc, handle1.GetPtr()) << "Pinned object must not moved";
    ASSERT_TRUE(ObjectToRegion(handle2.GetPtr())->IsMixedTLAB());
    ASSERT_EQ(addressBeforeGc2, handle2.GetPtr()) << "Pinned object must not moved";
    ASSERT_TRUE(ObjectToRegion(handle1.GetPtr())->IsInAllocRange(handle1.GetPtr()));
    auto *pinnedObj = ObjectAllocator::AllocString(OBJECT_SIZE, true);  // Pinned allocation
    ASSERT_TRUE(ObjectToRegion(handle1.GetPtr())->IsInAllocRange(pinnedObj));
    g1Allocator->UnpinObject(handle1.GetPtr());
    Runtime::GetCurrent()->GetPandaVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
}

class G1AllocatePinnedObjectTest : public G1GCPromotePinnedRegionTest {};

TEST_F(G1AllocatePinnedObjectTest, AllocatePinnedRegularArrayAndCreatedNewPinnedRegion)
{
    [[maybe_unused]] HandleScope<ark::ObjectHeader *> scope(thread_);
    constexpr size_t OBJ_ELEMENT_SIZE = 64;
    size_t objectSize = 1_KB;
    auto *arrayObj = ObjectAllocator::AllocArray(objectSize / OBJ_ELEMENT_SIZE, ClassRoot::ARRAY_I64, false, true);
    ASSERT_NE(arrayObj, nullptr);
    VMHandle<ObjectHeader> handle(thread_, arrayObj);
    ASSERT_TRUE(ObjectToRegion(handle.GetPtr())->HasPinnedObjects());
    ASSERT_TRUE(ObjectToRegion(handle.GetPtr())->HasFlag(RegionFlag::IS_OLD));
}

TEST_F(G1AllocatePinnedObjectTest, AllocatePinnedRegularStringAndCreatedNewPinnedRegion)
{
    auto *g1Allocator =
        static_cast<ObjectAllocatorG1<> *>(Runtime::GetCurrent()->GetPandaVM()->GetGC()->GetObjectAllocator());
    [[maybe_unused]] HandleScope<ark::ObjectHeader *> scope(thread_);
    size_t objectSize = 128_KB;
    auto *arrayObj = ObjectAllocator::AllocString(objectSize, true);
    ASSERT_NE(arrayObj, nullptr);
    VMHandle<ObjectHeader> handle(thread_, arrayObj);
    ASSERT_TRUE(ObjectToRegion(handle.GetPtr())->HasPinnedObjects());
    ASSERT_TRUE(ObjectToRegion(handle.GetPtr())->HasFlag(RegionFlag::IS_OLD));
    g1Allocator->UnpinObject(handle.GetPtr());
    size_t objectSize2 = 32_KB;
    size_t sizeRegionBefore = ObjectToRegion(handle.GetPtr())->GetAllocatedBytes();
    auto *arrayObj2 = ObjectAllocator::AllocString(objectSize2, true);
    size_t sizeRegionAfter = ObjectToRegion(handle.GetPtr())->GetAllocatedBytes();
    ASSERT_NE(arrayObj2, nullptr);
    ASSERT_EQ(sizeRegionBefore, sizeRegionAfter);
}

TEST_F(G1AllocatePinnedObjectTest, AllocatePinnedRegularStringToNewPinnedRegion)
{
    [[maybe_unused]] HandleScope<ark::ObjectHeader *> scope(thread_);
    size_t objectSize = 128_KB;
    auto *strObj1 = ObjectAllocator::AllocString(objectSize, true);
    ASSERT_NE(strObj1, nullptr);
    VMHandle<ObjectHeader> handle(thread_, strObj1);
    ASSERT_TRUE(ObjectToRegion(handle.GetPtr())->HasPinnedObjects());
    ASSERT_TRUE(ObjectToRegion(handle.GetPtr())->HasFlag(RegionFlag::IS_OLD));
    size_t sizeRegionBefore = ObjectToRegion(handle.GetPtr())->GetAllocatedBytes();
    // Create new pinned object
    size_t objectSize2 = 196_KB;
    auto *strObj2 = ObjectAllocator::AllocString(objectSize2, true);
    ASSERT_NE(strObj2, nullptr);
    VMHandle<ObjectHeader> handle1(thread_, strObj2);
    ASSERT_TRUE(ObjectToRegion(handle1.GetPtr())->HasPinnedObjects());
    ASSERT_TRUE(ObjectToRegion(handle1.GetPtr())->HasFlag(RegionFlag::IS_OLD));
    size_t sizeRegionAfter = ObjectToRegion(handle.GetPtr())->GetAllocatedBytes();
    ASSERT_EQ(sizeRegionBefore, sizeRegionAfter);
    ASSERT_NE(ObjectToRegion(handle.GetPtr()), ObjectToRegion(handle1.GetPtr()));
}

TEST_F(G1AllocatePinnedObjectTest, AllocatePinnedRegularStringToExistPinnedRegion)
{
    [[maybe_unused]] HandleScope<ark::ObjectHeader *> scope(thread_);
    size_t objectSize = 32_KB;
    auto *strObj1 = ObjectAllocator::AllocString(objectSize, true);
    ASSERT_NE(strObj1, nullptr);
    VMHandle<ObjectHeader> handle(thread_, strObj1);
    ASSERT_TRUE(ObjectToRegion(handle.GetPtr())->HasPinnedObjects());
    ASSERT_TRUE(ObjectToRegion(handle.GetPtr())->HasFlag(RegionFlag::IS_OLD));
    size_t sizeRegionBefore = ObjectToRegion(handle.GetPtr())->GetAllocatedBytes();
    // Create new pinned object
    size_t objectSize2 = 32_KB;
    auto *strObj2 = ObjectAllocator::AllocString(objectSize2, true);
    ASSERT_NE(strObj2, nullptr);
    VMHandle<ObjectHeader> handle1(thread_, strObj2);
    ASSERT_TRUE(ObjectToRegion(handle1.GetPtr())->HasPinnedObjects());
    ASSERT_TRUE(ObjectToRegion(handle1.GetPtr())->HasFlag(RegionFlag::IS_OLD));
    size_t sizeRegionAfter = ObjectToRegion(handle.GetPtr())->GetAllocatedBytes();
    ASSERT_NE(sizeRegionBefore, sizeRegionAfter);
}

TEST_F(G1AllocatePinnedObjectTest, AllocateRegularStringToExistPinnedRegion)
{
    auto *g1Allocator =
        static_cast<ObjectAllocatorG1<> *>(Runtime::GetCurrent()->GetPandaVM()->GetGC()->GetObjectAllocator());
    [[maybe_unused]] HandleScope<ark::ObjectHeader *> scope(thread_);
    constexpr size_t OBJECT_SIZE =
        AlignUp(static_cast<size_t>(DEFAULT_REGION_SIZE * 0.08F), DEFAULT_ALIGNMENT_IN_BYTES);
    auto *addressBeforeGc = ObjectAllocator::AllocString(OBJECT_SIZE);
    ASSERT_NE(addressBeforeGc, nullptr);
    VMHandle<ObjectHeader> handle1(thread_, addressBeforeGc);
    g1Allocator->PinObject(handle1.GetPtr());
    size_t sizeRegionBefore = ObjectToRegion(handle1.GetPtr())->GetAllocatedBytes();
    // Run GC - promote pinned region
    Runtime::GetCurrent()->GetPandaVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));

    ASSERT_EQ(addressBeforeGc, handle1.GetPtr()) << "Pinned object must not moved";
    auto *pinnedObj = ObjectAllocator::AllocString(OBJECT_SIZE, true);  // Pinned allocation
    VMHandle<ObjectHeader> handle3(thread_, pinnedObj);
    size_t sizeRegionAfter = ObjectToRegion(handle1.GetPtr())->GetAllocatedBytes();
    ASSERT_EQ(ObjectToRegion(handle3.GetPtr()), ObjectToRegion(handle1.GetPtr()));
    ASSERT_TRUE(ObjectToRegion(handle3.GetPtr())->HasPinnedObjects());
    ASSERT_TRUE(ObjectToRegion(handle3.GetPtr())->HasFlag(RegionFlag::IS_OLD));
    ASSERT_NE(sizeRegionBefore, sizeRegionAfter);
}

TEST_F(G1AllocatePinnedObjectTest, AllocateRegularArray)
{
    auto *g1Allocator =
        static_cast<ObjectAllocatorG1<> *>(Runtime::GetCurrent()->GetPandaVM()->GetGC()->GetObjectAllocator());
    [[maybe_unused]] HandleScope<ark::ObjectHeader *> scope(thread_);
    constexpr size_t OBJECT_SIZE =
        AlignUp(static_cast<size_t>(DEFAULT_REGION_SIZE * 0.08F), DEFAULT_ALIGNMENT_IN_BYTES);
    auto *addressBeforeGc = ObjectAllocator::AllocString(OBJECT_SIZE);
    ASSERT_NE(addressBeforeGc, nullptr);
    VMHandle<ObjectHeader> handle1(thread_, addressBeforeGc);
    g1Allocator->PinObject(handle1.GetPtr());
    size_t sizeRegionBefore = ObjectToRegion(handle1.GetPtr())->GetAllocatedBytes();
    // Run GC - promote pinned region
    Runtime::GetCurrent()->GetPandaVM()->GetGC()->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));

    ASSERT_EQ(addressBeforeGc, handle1.GetPtr()) << "Pinned object must not moved";
    constexpr size_t OBJ_ELEMENT_SIZE = 64;
    size_t objectSize = 1_KB;
    auto *arrayObj = ObjectAllocator::AllocArray(objectSize / OBJ_ELEMENT_SIZE, ClassRoot::ARRAY_I64, false, true);
    ASSERT_NE(arrayObj, nullptr);
    VMHandle<ObjectHeader> handle3(thread_, arrayObj);
    size_t sizeRegionAfter = ObjectToRegion(handle1.GetPtr())->GetAllocatedBytes();
    ASSERT_EQ(ObjectToRegion(handle3.GetPtr()), ObjectToRegion(handle1.GetPtr()));
    ASSERT_TRUE(ObjectToRegion(handle3.GetPtr())->HasPinnedObjects());
    ASSERT_NE(sizeRegionBefore, sizeRegionAfter);
}

static void AllocatePinnedObjectTest(SpaceType requestedSpaceType, size_t objectSize = 1_KB)
{
    ASSERT_TRUE(IsHeapSpace(requestedSpaceType));
    Runtime::Create(G1AllocatePinnedObjectTest::CreateOptions());
    auto *thread = MTManagedThread::GetCurrent();
    ASSERT_NE(thread, nullptr);
    thread->ManagedCodeBegin();
    auto *g1Allocator =
        static_cast<ObjectAllocatorG1<> *>(Runtime::GetCurrent()->GetPandaVM()->GetGC()->GetObjectAllocator());
    {
        [[maybe_unused]] HandleScope<ark::ObjectHeader *> scope(thread);
        constexpr size_t OBJ_ELEMENT_SIZE = 64;
        auto *addressBeforeGc =
            ObjectAllocator::AllocArray(objectSize / OBJ_ELEMENT_SIZE, ClassRoot::ARRAY_I64,
                                        requestedSpaceType == SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT, true);
        ASSERT_NE(addressBeforeGc, nullptr);
        VMHandle<ObjectHeader> handle(thread, addressBeforeGc);
        SpaceType objSpaceType =
            PoolManager::GetMmapMemPool()->GetSpaceTypeForAddr(static_cast<void *>(handle.GetPtr()));
        ASSERT_EQ(objSpaceType, requestedSpaceType);
        ASSERT_EQ(addressBeforeGc, handle.GetPtr());
        if (requestedSpaceType == SpaceType::SPACE_TYPE_OBJECT) {
            ASSERT_TRUE(ObjectToRegion(handle.GetPtr())->HasPinnedObjects());
        }
        g1Allocator->UnpinObject(handle.GetPtr());
        ASSERT_FALSE(ObjectToRegion(handle.GetPtr())->HasPinnedObjects());
    }
    thread->ManagedCodeEnd();
    Runtime::Destroy();
}

TEST(G1AllocateDifferentSpaceTypePinnedObjectTest, AllocatePinnedRegularObjectTest)
{
    AllocatePinnedObjectTest(SpaceType::SPACE_TYPE_OBJECT);
}

TEST(G1AllocateDifferentSpaceTypePinnedObjectTest, AllocatePinnedHumongousObjectTest)
{
    constexpr size_t HUMONGOUS_OBJECT_FOR_PINNING_SIZE = 4_MB;
    AllocatePinnedObjectTest(SpaceType::SPACE_TYPE_HUMONGOUS_OBJECT, HUMONGOUS_OBJECT_FOR_PINNING_SIZE);
}

TEST(G1AllocateDifferentSpaceTypePinnedObjectTest, AllocatePinnedNonMovableObjectTest)
{
    AllocatePinnedObjectTest(SpaceType::SPACE_TYPE_NON_MOVABLE_OBJECT);
}

// NOLINTEND(readability-magic-numbers)

}  // namespace ark::mem
