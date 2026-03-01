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
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <limits>
#include <thread>

#include "gtest/gtest.h"
#include "iostream"
#include "libarkbase/utils/utils.h"
#include "runtime/include/coretypes/line_string.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/handle_scope-inl.h"
#include "runtime/mem/gc/g1/g1-allocator.h"
#include "runtime/mem/gc/generational-gc-base.h"
#include "runtime/mem/malloc-proxy-allocator-inl.h"
#include "runtime/mem/mem_stats.h"
#include "runtime/mem/mem_stats_default.h"
#include "runtime/mem/runslots_allocator-inl.h"

namespace ark::mem::test {

class G1GCFullGCTest : public testing::Test {
public:
    using ObjVec = PandaVector<ObjectHeader *>;
    using HanVec = PandaVector<VMHandle<ObjectHeader *> *>;
    static constexpr size_t ROOT_MAX_SIZE = 100000U;

    static constexpr GCTaskCause MIXED_G1_GC_CAUSE = GCTaskCause::YOUNG_GC_CAUSE;
    static constexpr GCTaskCause FULL_GC_CAUSE = GCTaskCause::EXPLICIT_CAUSE;

    enum class TargetSpace { YOUNG, TENURED, LARGE, HUMONG };

    class GCCounter : public GCListener {
    public:
        void GCStarted([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSize) override
        {
            count_++;
        }

        void GCFinished([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSizeBeforeGc,
                        [[maybe_unused]] size_t heapSize) override
        {
        }

    private:
        int count_ = 0;
    };

    template <typename F>
    class GCHook : public GCListener {
    public:
        explicit GCHook(F hookArg) : hook_(hookArg) {};

        void GCStarted([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSize) override {}

        void GCFinished([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSizeBeforeGc,
                        [[maybe_unused]] size_t heapSize) override
        {
            hook_();
        }

    private:
        F hook_;
    };

    void SetupRuntime(const std::string &gcTypeParam)
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetUseTlabForAllocations(false);
        options.SetGcType(gcTypeParam);
        options.SetGcTriggerType("debug");
        options.SetGcDebugTriggerStart(std::numeric_limits<int>::max());
        options.SetCompilerEnableJit(false);
        constexpr size_t HEAP_SIZE = 33554432U;
        options.SetHeapSizeLimit(HEAP_SIZE);
        constexpr size_t YOUNG_SIZE = 4194304U;
        options.SetYoungSpaceSize(YOUNG_SIZE);
        options.SetExplicitConcurrentGcEnabled(false);
        [[maybe_unused]] bool success = Runtime::Create(options);
        ASSERT(success);

        thread = ark::MTManagedThread::GetCurrent();
        gcType = Runtime::GetGCType(options, plugins::RuntimeTypeToLang(Runtime::GetRuntimeType()));
        [[maybe_unused]] auto gcLocal = thread->GetVM()->GetGC();
        ASSERT(gcLocal->GetType() == ark::mem::GCTypeFromString(gcTypeParam));
        ASSERT(gcLocal->IsGenerational());
        thread->ManagedCodeBegin();
    }

    void ResetRuntime()
    {
        DeleteHandles();
        internalAllocator->Delete(gccnt);
        thread->ManagedCodeEnd();
        bool success = Runtime::Destroy();
        ASSERT_TRUE(success) << "Cannot destroy Runtime";
    }

    template <typename F, size_t MULTI>
    ObjVec MakeAllocations(size_t minSize, size_t maxSize, size_t count, size_t *allocated, size_t *requested,
                           F spaceChecker, bool checkOomInTenured = false);

    void InitRoot();
    void MakeObjectsAlive(const ObjVec &objects, int every = 1);
    void MakeObjectsPermAlive(const ObjVec &objects, int every = 1);
    void MakeObjectsGarbage(size_t startIdx, size_t afterEndIdx, int every = 1);
    void DumpHandles();
    void DumpAliveObjects();
    void DeleteHandles();
    bool IsInYoung(uintptr_t addr);

    template <class LanguageConfig>
    void PrepareTest();

    void TearDown() override {}

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    ark::MTManagedThread *thread {};
    GCType gcType {};

    LanguageContext ctx {nullptr};
    ObjectAllocatorBase *objectAllocator {};
    mem::InternalAllocatorPtr internalAllocator;
    PandaVM *vm {};
    GC *gc {};
    std::vector<HanVec> handles;
    MemStatsType *ms {};
    GCStats *gcMs {};
    coretypes::Array *root = nullptr;
    size_t rootSize = 0;
    GCCounter *gccnt {};
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

template <typename F, size_t MULTI>
G1GCFullGCTest::ObjVec G1GCFullGCTest::MakeAllocations(size_t minSize, size_t maxSize, size_t count, size_t *allocated,
                                                       size_t *requested, [[maybe_unused]] F spaceChecker,
                                                       bool checkOomInTenured)
{
    ASSERT(minSize <= maxSize);
    *allocated = 0;
    *requested = 0;
    // Create array of object templates based on count and max size
    PandaVector<PandaString> objTemplates(count);
    size_t objSize = sizeof(coretypes::String) + minSize;
    for (size_t i = 0; i < count; ++i) {
        PandaString simpleString;
        simpleString.resize(objSize - sizeof(coretypes::String));
        objTemplates[i] = std::move(simpleString);
        objSize += (maxSize / count + i);  // +i to mess with the alignment
        if (objSize > maxSize) {
            objSize = maxSize;
        }
    }
    ObjVec result;
    result.reserve(count * MULTI);
    for (size_t j = 0; j < count; ++j) {
        size_t size = objTemplates[j].length() + sizeof(coretypes::String);
        if (checkOomInTenured) {
            // Leaving 5MB in tenured seems OK
            auto free =
                reinterpret_cast<GenerationalSpaces *>(objectAllocator->GetHeapSpace())->GetCurrentFreeTenuredSize();
            constexpr size_t BIG_SIZE = 5000000U;
            if (size + BIG_SIZE > free) {
                return result;
            }
        }
        for (size_t i = 0; i < MULTI; ++i) {
            // create string of '\0's
            coretypes::String *stringObj =
                coretypes::String::CreateFromMUtf8(reinterpret_cast<const uint8_t *>(&objTemplates[j][0]),
                                                   objTemplates[j].length(), objTemplates[j].length(), true, ctx, vm);
            ASSERT(stringObj != nullptr);
            ASSERT(stringObj->GetLength() == objTemplates[j].length());
            ASSERT(spaceChecker(ToUintPtr(stringObj)));
            *allocated += GetAlignedObjectSize(size);
            *requested += size;
            result.push_back(stringObj);
        }
    }
    return result;
}

void G1GCFullGCTest::InitRoot()
{
    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    Class *klass = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)
                       ->GetClass(ctx.GetStringArrayClassDescriptor());
    ASSERT_NE(klass, nullptr);
    root = coretypes::Array::Create(klass, ROOT_MAX_SIZE);
    rootSize = 0;
    MakeObjectsPermAlive({root});
}

void G1GCFullGCTest::MakeObjectsAlive(const ObjVec &objects, int every)
{
    int cnt = every;
    for (auto *obj : objects) {
        cnt--;
        if (cnt != 0) {
            continue;
        }
        root->Set(rootSize, obj);
        rootSize++;
        ASSERT(rootSize < ROOT_MAX_SIZE);
        cnt = every;
    }
}

void G1GCFullGCTest::MakeObjectsGarbage(size_t startIdx, size_t afterEndIdx, int every)
{
    int cnt = every;
    for (size_t i = startIdx; i < afterEndIdx; ++i) {
        cnt--;
        if (cnt != 0) {
            continue;
        }
        root->Set(i, static_cast<ObjectHeader *>(nullptr));
        rootSize++;
        cnt = every;
    }
}

void G1GCFullGCTest::MakeObjectsPermAlive(const ObjVec &objects, int every)
{
    HanVec result;
    result.reserve(objects.size() / every);
    int cnt = every;
    for (auto *obj : objects) {
        cnt--;
        if (cnt != 0) {
            continue;
        }
        result.push_back(internalAllocator->New<VMHandle<ObjectHeader *>>(thread, obj));
        cnt = every;
    }
    handles.push_back(result);
}

void G1GCFullGCTest::DumpHandles()
{
    for (auto &hv : handles) {
        for (auto *handle : hv) {
            std::cout << "vector " << (void *)&hv << " handle " << (void *)handle << " obj " << handle->GetPtr()
                      << std::endl;
        }
    }
}

void G1GCFullGCTest::DumpAliveObjects()
{
    std::cout << "Alive root array : " << handles[0][0]->GetPtr() << std::endl;
    for (size_t i = 0; i < rootSize; ++i) {
        if (root->Get<ObjectHeader *>(i) != nullptr) {
            std::cout << "Alive idx " << i << " : " << root->Get<ObjectHeader *>(i) << std::endl;
        }
    }
}

void G1GCFullGCTest::DeleteHandles()
{
    for (auto &hv : handles) {
        for (auto *handle : hv) {
            internalAllocator->Delete(handle);
        }
    }
    handles.clear();
}

template <class LanguageConfig>
void G1GCFullGCTest::PrepareTest()
{
    if constexpr (std::is_same<LanguageConfig, ark::PandaAssemblyLanguageConfig>::value) {
        DeleteHandles();
        ctx = Runtime::GetCurrent()->GetLanguageContext(panda_file::SourceLang::PANDA_ASSEMBLY);
        objectAllocator = thread->GetVM()->GetGC()->GetObjectAllocator();
        vm = Runtime::GetCurrent()->GetPandaVM();
        internalAllocator = Runtime::GetCurrent()->GetClassLinker()->GetAllocator();
        gc = vm->GetGC();
        ms = vm->GetMemStats();
        gcMs = vm->GetGCStats();
        gccnt = internalAllocator->New<GCCounter>();
        gc->AddListener(gccnt);
        InitRoot();
    } else {
        UNREACHABLE();  // NYI
    }
}

bool G1GCFullGCTest::IsInYoung(uintptr_t addr)
{
    switch (gcType) {
        case GCType::G1_GC: {
            auto memPool = PoolManager::GetMmapMemPool();
            if (memPool->GetSpaceTypeForAddr(reinterpret_cast<ObjectHeader *>(addr)) != SpaceType::SPACE_TYPE_OBJECT) {
                return false;
            }
            return AddrToRegion(ToVoidPtr(addr))->HasFlag(RegionFlag::IS_EDEN);
        }
        default:
            UNREACHABLE();  // NYI
    }
    return false;
}

TEST_F(G1GCFullGCTest, TestIntensiveAlloc)
{
    std::string gctype = static_cast<std::string>(GCStringFromType(GCType::G1_GC));
    SetupRuntime(gctype);
    {
        HandleScope<ObjectHeader *> scope(thread);
        PrepareTest<ark::PandaAssemblyLanguageConfig>();
        [[maybe_unused]] size_t bytes {};
        [[maybe_unused]] size_t rawObjectsSize {};

        [[maybe_unused]] size_t youngSize =
            reinterpret_cast<GenerationalSpaces *>(
                reinterpret_cast<ObjectAllocatorGenBase *>(objectAllocator)->GetHeapSpace())
                ->GetCurrentYoungSize();
        [[maybe_unused]] size_t heapSize = mem::MemConfig::GetHeapSizeLimit();
        [[maybe_unused]] auto g1Alloc = reinterpret_cast<ObjectAllocatorG1<MT_MODE_MULTI> *>(objectAllocator);
        [[maybe_unused]] size_t maxYSize = g1Alloc->GetYoungAllocMaxSize();

        [[maybe_unused]] auto ySpaceCheck = [this](uintptr_t addr) -> bool { return IsInYoung(addr); };
        [[maybe_unused]] auto hSpaceCheck = [this](uintptr_t addr) -> bool { return !IsInYoung(addr); };
        [[maybe_unused]] auto tFree =
            reinterpret_cast<GenerationalSpaces *>(objectAllocator->GetHeapSpace())->GetCurrentFreeTenuredSize();
        const size_t yObjSize = maxYSize / 10;
        gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
        size_t initialHeap = ms->GetFootprintHeap();

        {
            // Ordinary young shall not be broken when intermixed with explicits
            int i = 0;
            size_t allocated = 0;
            while (allocated < 2U * heapSize) {
                ObjVec ov1 = MakeAllocations<decltype(ySpaceCheck), 1>(yObjSize, yObjSize, 1, &bytes, &rawObjectsSize,
                                                                       ySpaceCheck);
                allocated += bytes;
                // NOLINTNEXTLINE(readability-magic-numbers)
                if (i++ % 100_I == 0) {
                    gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                }
            }
            gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
            ASSERT_EQ(initialHeap, ms->GetFootprintHeap());
        }

        {
            // Intensive allocations surviving 1 young
            auto oldRootSize = rootSize;
            size_t allocated = 0;
            bool gcHappened = false;
            GCHook gchook([&oldRootSize, this, &gcHappened]() {
                MakeObjectsGarbage(oldRootSize, this->rootSize);
                oldRootSize = this->rootSize;
                gcHappened = true;
            });
            gc->AddListener(&gchook);
            while (allocated < 4U * heapSize) {
                ObjVec ov1 = MakeAllocations<decltype(ySpaceCheck), 1>(yObjSize, yObjSize, 1, &bytes, &rawObjectsSize,
                                                                       ySpaceCheck);
                MakeObjectsAlive(ov1, 1);
                tFree = reinterpret_cast<GenerationalSpaces *>(objectAllocator->GetHeapSpace())
                            ->GetCurrentFreeTenuredSize();
                allocated += bytes;
            }
            MakeObjectsGarbage(oldRootSize, rootSize);
            gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
            ASSERT_EQ(initialHeap, ms->GetFootprintHeap());
            gc->RemoveListener(&gchook);
        }

        MakeObjectsGarbage(0, rootSize);
        gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
        ASSERT_EQ(initialHeap, ms->GetFootprintHeap());
    }
    ResetRuntime();
}

TEST_F(G1GCFullGCTest, TestExplicitFullNearLimit)
{
    SetupRuntime(static_cast<std::string>(GCStringFromType(GCType::G1_GC)));
    {
        HandleScope<ObjectHeader *> scope(thread);
        PrepareTest<ark::PandaAssemblyLanguageConfig>();
        [[maybe_unused]] size_t bytes;
        [[maybe_unused]] size_t rawObjectsSize;

        [[maybe_unused]] size_t youngSize =
            reinterpret_cast<GenerationalSpaces *>(
                reinterpret_cast<ObjectAllocatorGenBase *>(objectAllocator)->GetHeapSpace())
                ->GetCurrentYoungSize();
        [[maybe_unused]] size_t heapSize = mem::MemConfig::GetHeapSizeLimit();
        [[maybe_unused]] auto g1Alloc = reinterpret_cast<ObjectAllocatorG1<MT_MODE_MULTI> *>(objectAllocator);

        [[maybe_unused]] auto ySpaceCheck = [this](uintptr_t addr) -> bool { return IsInYoung(addr); };
        [[maybe_unused]] auto hSpaceCheck = [this](uintptr_t addr) -> bool { return !IsInYoung(addr); };
        [[maybe_unused]] auto tFree =
            reinterpret_cast<GenerationalSpaces *>(objectAllocator->GetHeapSpace())->GetCurrentFreeTenuredSize();
        const size_t yObjSize = g1Alloc->GetYoungAllocMaxSize() / 10U;
        gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
        size_t initialHeap = ms->GetFootprintHeap();

        {
            // Allocating until we are close to OOM, then do release this mem,
            // do explicit full and allocate the same size once again
            auto oldRootSize = rootSize;
            int i = 0;
            // NOLINTNEXTLINE(readability-magic-numbers)
            while (tFree > 2.2F * youngSize) {
                ObjVec ov1 = MakeAllocations<decltype(ySpaceCheck), 1>(yObjSize, yObjSize, 1, &bytes, &rawObjectsSize,
                                                                       ySpaceCheck);
                MakeObjectsAlive(ov1, 1);
                tFree = reinterpret_cast<GenerationalSpaces *>(objectAllocator->GetHeapSpace())
                            ->GetCurrentFreeTenuredSize();
                i++;
            }
            gc->WaitForGCInManaged(GCTask(GCTaskCause::EXPLICIT_CAUSE));
            MakeObjectsGarbage(oldRootSize, rootSize);

            oldRootSize = rootSize;
            while (--i > 0) {
                ObjVec ov1 = MakeAllocations<decltype(ySpaceCheck), 1>(yObjSize, yObjSize, 1, &bytes, &rawObjectsSize,
                                                                       ySpaceCheck);
                MakeObjectsAlive(ov1, 1);
                tFree = reinterpret_cast<GenerationalSpaces *>(objectAllocator->GetHeapSpace())
                            ->GetCurrentFreeTenuredSize();
            }
            MakeObjectsGarbage(oldRootSize, rootSize);
            gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
        }

        MakeObjectsGarbage(0, rootSize);
        gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
        ASSERT_EQ(initialHeap, ms->GetFootprintHeap());
    }
    ResetRuntime();
}

TEST_F(G1GCFullGCTest, TestOOMFullNearLimit)
{
    SetupRuntime(static_cast<std::string>(GCStringFromType(GCType::G1_GC)));
    {
        HandleScope<ObjectHeader *> scope(thread);
        PrepareTest<ark::PandaAssemblyLanguageConfig>();
        [[maybe_unused]] size_t bytes;
        [[maybe_unused]] size_t rawObjectsSize;

        [[maybe_unused]] size_t youngSize =
            reinterpret_cast<GenerationalSpaces *>(
                reinterpret_cast<ObjectAllocatorGenBase *>(objectAllocator)->GetHeapSpace())
                ->GetCurrentYoungSize();
        [[maybe_unused]] size_t heapSize = mem::MemConfig::GetHeapSizeLimit();
        [[maybe_unused]] auto g1Alloc = reinterpret_cast<ObjectAllocatorG1<MT_MODE_MULTI> *>(objectAllocator);

        [[maybe_unused]] auto ySpaceCheck = [this](uintptr_t addr) -> bool { return IsInYoung(addr); };
        [[maybe_unused]] auto hSpaceCheck = [this](uintptr_t addr) -> bool { return !IsInYoung(addr); };
        [[maybe_unused]] auto tFree =
            reinterpret_cast<GenerationalSpaces *>(objectAllocator->GetHeapSpace())->GetCurrentFreeTenuredSize();
        const size_t yObjSize = g1Alloc->GetYoungAllocMaxSize() / 10U;
        gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
        size_t initialHeap = ms->GetFootprintHeap();

        {
            // Allocating until we are close to OOM, then do release this mem,
            // then allocate the same size once again checking if we can handle it w/o OOM
            auto oldRootSize = rootSize;
            int i = 0;
            // NOLINTNEXTLINE(readability-magic-numbers)
            while (tFree > 2.2F * youngSize) {
                ObjVec ov1 = MakeAllocations<decltype(ySpaceCheck), 1>(yObjSize, yObjSize, 1, &bytes, &rawObjectsSize,
                                                                       ySpaceCheck);
                MakeObjectsAlive(ov1, 1);
                tFree = reinterpret_cast<GenerationalSpaces *>(objectAllocator->GetHeapSpace())
                            ->GetCurrentFreeTenuredSize();
                i++;
            }
            MakeObjectsGarbage(oldRootSize, rootSize);

            oldRootSize = rootSize;
            while (--i > 0) {
                ObjVec ov1 = MakeAllocations<decltype(ySpaceCheck), 1>(yObjSize, yObjSize, 1, &bytes, &rawObjectsSize,
                                                                       ySpaceCheck);
                MakeObjectsAlive(ov1, 1);
                tFree = reinterpret_cast<GenerationalSpaces *>(objectAllocator->GetHeapSpace())
                            ->GetCurrentFreeTenuredSize();
            }
            MakeObjectsGarbage(oldRootSize, rootSize);
            gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
        }

        MakeObjectsGarbage(0, rootSize);
        gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
        ASSERT_EQ(initialHeap, ms->GetFootprintHeap());
    }
    ResetRuntime();
}
}  // namespace ark::mem::test
