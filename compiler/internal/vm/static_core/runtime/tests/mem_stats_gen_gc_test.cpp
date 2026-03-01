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
class MemStatsGenGCTest : public testing::Test {
public:
    using ObjVec = PandaVector<ObjectHeader *>;
    using HanVec = PandaVector<VMHandle<ObjectHeader *> *>;
    static constexpr size_t ROOT_MAX_SIZE = 100000U;
    static constexpr int MIX_TEST_ALLOC_TIMES = 5;
    static constexpr int FULL_TEST_ALLOC_TIMES = 2;

    static constexpr GCTaskCause MIXED_G1_GC_CAUSE = GCTaskCause::YOUNG_GC_CAUSE;
    static constexpr GCTaskCause FULL_GC_CAUSE = GCTaskCause::EXPLICIT_CAUSE;

    enum class TargetSpace {
        YOUNG,
        TENURED_REGULAR,
        /*
         * Some allocators have Large objects, it's not the same as Humongous. Objects can be less than Humongous but be
         * allocated directly in the tenured space for example.
         */
        TENURED_LARGE,
        HUMONGOUS
    };

    // this class allows to iterate over configurations for JIT and TLAB
    class Config {
    public:
        enum class JITConfig : bool { NO_JIT = false, JIT };

        enum class TLABConfig : bool { NO_TLAB = false, TLAB };

        bool IsJITEnabled() const
        {
            return static_cast<bool>(jitCfg_);
        }

        bool IsTLABEnabled() const
        {
            return static_cast<bool>(tlabCfg_);
        }

        bool End() const
        {
            return jitCfg_ == JITConfig::JIT && tlabCfg_ == TLABConfig::NO_TLAB;
        }

        Config &operator++()
        {
            if (jitCfg_ == JITConfig::NO_JIT) {
                jitCfg_ = JITConfig::JIT;
            } else {
                jitCfg_ = JITConfig::NO_JIT;
                if (tlabCfg_ == TLABConfig::TLAB) {
                    tlabCfg_ = TLABConfig::NO_TLAB;
                } else {
                    tlabCfg_ = TLABConfig::TLAB;
                }
            }
            return *this;
        }

    private:
        JITConfig jitCfg_ {JITConfig::NO_JIT};
        TLABConfig tlabCfg_ {TLABConfig::TLAB};
    };

    class GCCounter : public GCListener {
    public:
        void GCStarted([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSize) override
        {
            count++;
        }

        void GCFinished([[maybe_unused]] const GCTask &task, [[maybe_unused]] size_t heapSizeBeforeGc,
                        [[maybe_unused]] size_t heapSize) override
        {
        }

        // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
        int count = 0;
    };

    struct GcData {
        size_t count;
        size_t minSize;
        size_t maxSize;
        bool checkOom;
    };

    struct MemOpReport {
        size_t allocatedCount;
        size_t allocatedBytes;
        size_t savedCount;
        size_t savedBytes;
    };

    struct RealStatsLocations {
        uint32_t *youngFreedObjectsCount;
        uint64_t *youngFreedObjectsSize;
        uint32_t *youngMovedObjectsCount;
        uint64_t *youngMovedObjectsSize;
        uint32_t *tenuredFreedObjectsCount;
        uint64_t *tenuredFreedObjectsSize;
    };

    void SetupRuntime(const std::string &gcTypeParam, const Config &cfg)
    {
        RuntimeOptions options;
        options.SetShouldLoadBootPandaFiles(false);
        options.SetShouldInitializeIntrinsics(false);
        options.SetUseTlabForAllocations(cfg.IsTLABEnabled());
        options.SetGcType(gcTypeParam);
        options.SetGcTriggerType("debug-never");
        options.SetRunGcInPlace(true);
        options.SetCompilerEnableJit(cfg.IsJITEnabled());
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

    template <typename F, size_t REPEAT, MemStatsGenGCTest::TargetSpace SPACE>
    ObjVec MakeAllocationsWithRepeats(size_t minSize, size_t maxSize, size_t count, size_t *allocated,
                                      size_t *requested, F spaceChecker, bool checkOomInTenured);

    void InitRoot();
    void MakeObjectsAlive(const ObjVec &objects, int every = 1);
    void MakeObjectsPermAlive(const ObjVec &objects, int every = 1);
    void MakeObjectsGarbage(size_t startIdx, size_t afterEndIdx, int every = 1);
    void DumpHandles();
    void DumpAliveObjects();
    void DeleteHandles();
    bool IsInYoung(uintptr_t addr);
    MemOpReport HelpAllocTenured();

    bool NeedToCheckYoungFreedCount()
    {
        return (gcType != GCType::G1_GC) || Runtime::GetOptions().IsG1TrackFreedObjects();
    }

    template <class LanguageConfig>
    void PrepareTest();

    template <class LanguageConfig>
    typename GenerationalGC<LanguageConfig>::MemStats *GetGenMemStats();

    // Allocate a series of objects in a specific space. If DO_SAVE is true, a subsequence of objects
    // is going to be kept alive and put into the roots array this->root_
    // If IS_SINGLE is true, then only 1 object is allocated of unaligned size
    // If IS_SINGLE is false, then an array of objects of different sizes is allocated in triplets twice
    // Saved subsequence contains 2 equal subsequences of objects (2 of 3 objs in each triplets are garbage)
    template <MemStatsGenGCTest::TargetSpace SPACE, bool DO_SAVE = false, bool IS_SIMPLE = false>
    typename MemStatsGenGCTest::MemOpReport MakeAllocations();

    template <typename T>
    RealStatsLocations GetGenMemStatsDetails(T gms);

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

private:
    template <MemStatsGenGCTest::TargetSpace SPACE>
    auto CreateSpaceCheck();

    template <MemStatsGenGCTest::TargetSpace SPACE>
    bool InitG1Gc(MemStatsGenGCTest::GcData &gcData);

    template <MemStatsGenGCTest::TargetSpace SPACE>
    bool InitGenGc(MemStatsGenGCTest::GcData &gcData);

    template <typename F, size_t REPEAT, MemStatsGenGCTest::TargetSpace SPACE, bool DO_SAVE>
    void MakeAllocationsWithSingleRepeat(MemStatsGenGCTest::GcData &gcData, MemStatsGenGCTest::MemOpReport &report,
                                         size_t &bytes, size_t &rawObjectsSize, [[maybe_unused]] F spaceCheck);

    template <typename F, size_t REPEAT, MemStatsGenGCTest::TargetSpace SPACE, bool DO_SAVE>
    void MakeAllocationsWithSeveralRepeat(MemStatsGenGCTest::GcData &gcData, MemStatsGenGCTest::MemOpReport &report,
                                          size_t &bytes, size_t &rawObjectsSize, [[maybe_unused]] F spaceCheck);

    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

template <typename F, size_t REPEAT, MemStatsGenGCTest::TargetSpace SPACE>
MemStatsGenGCTest::ObjVec MemStatsGenGCTest::MakeAllocationsWithRepeats(size_t minSize, size_t maxSize, size_t count,
                                                                        size_t *allocated, size_t *requested,
                                                                        [[maybe_unused]] F spaceChecker,
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
    result.reserve(count * REPEAT);
    for (size_t j = 0; j < count; ++j) {
        size_t size = objTemplates[j].length() + sizeof(coretypes::String);
        if (checkOomInTenured) {
            // Leaving 5MB in tenured seems OK
            auto free =
                reinterpret_cast<GenerationalSpaces *>(objectAllocator->GetHeapSpace())->GetCurrentFreeTenuredSize();
            constexpr size_t FIVE_MB = 5000000U;
            if (size + FIVE_MB > free) {
                return result;
            }
        }
        for (size_t i = 0; i < REPEAT; ++i) {
            // create string of '\0's
            coretypes::String *stringObj =
                coretypes::String::CreateFromMUtf8(reinterpret_cast<const uint8_t *>(&objTemplates[j][0]),
                                                   objTemplates[j].length(), objTemplates[j].length(), true, ctx, vm);
            ASSERT(stringObj != nullptr);
            ASSERT(stringObj->GetLength() == objTemplates[j].length());
            ASSERT(spaceChecker(ToUintPtr(stringObj)) == true);
            if (gcType == GCType::G1_GC && SPACE == TargetSpace::HUMONGOUS) {
                // for humongous objects in G1 we calculate size of the region instead of just alignment size
                Region *region = AddrToRegion(stringObj);
                *allocated += region->Size();
            } else {
                *allocated += GetAlignedObjectSize(size);
            }
            *requested += size;
            result.push_back(stringObj);
        }
    }
    return result;
}

template <typename F, size_t REPEAT, MemStatsGenGCTest::TargetSpace SPACE, bool DO_SAVE>
void MemStatsGenGCTest::MakeAllocationsWithSingleRepeat(MemStatsGenGCTest::GcData &gcData,
                                                        MemStatsGenGCTest::MemOpReport &report, size_t &bytes,
                                                        size_t &rawObjectsSize, [[maybe_unused]] F spaceCheck)
{
    ObjVec ov1 = MakeAllocationsWithRepeats<F, REPEAT, SPACE>(gcData.minSize + 1, gcData.maxSize, 1, &bytes,
                                                              &rawObjectsSize, spaceCheck, gcData.checkOom);
    report.allocatedCount += 1;
    report.allocatedBytes += bytes;
    if constexpr (DO_SAVE) {
        MakeObjectsAlive(ov1, 1);
        report.savedCount = report.allocatedCount;
        report.savedBytes = report.allocatedBytes;
    }
}

template <typename F, size_t REPEAT, MemStatsGenGCTest::TargetSpace SPACE, bool DO_SAVE>
void MemStatsGenGCTest::MakeAllocationsWithSeveralRepeat(MemStatsGenGCTest::GcData &gcData,
                                                         MemStatsGenGCTest::MemOpReport &report, size_t &bytes,
                                                         size_t &rawObjectsSize, [[maybe_unused]] F spaceCheck)
{
    ObjVec ov1 = MakeAllocationsWithRepeats<decltype(spaceCheck), 3U, SPACE>(
        gcData.minSize, gcData.maxSize, gcData.count, &bytes, &rawObjectsSize, spaceCheck, gcData.checkOom);
    report.allocatedCount += gcData.count * 3U;
    report.allocatedBytes += bytes;
    ObjVec ov2 = MakeAllocationsWithRepeats<decltype(spaceCheck), 3U, SPACE>(
        gcData.minSize, gcData.maxSize, gcData.count, &bytes, &rawObjectsSize, spaceCheck, gcData.checkOom);
    report.allocatedCount += gcData.count * 3U;
    report.allocatedBytes += bytes;
    if constexpr (DO_SAVE) {
        MakeObjectsAlive(ov1, 3_I);
        MakeObjectsAlive(ov2, 3_I);
        report.savedCount = report.allocatedCount / 3U;
        report.savedBytes = report.allocatedBytes / 3U;
    }
}

template <MemStatsGenGCTest::TargetSpace SPACE>
auto MemStatsGenGCTest::CreateSpaceCheck()
{
    auto spaceCheck = [this](uintptr_t addr) -> bool {
        if constexpr (SPACE == TargetSpace::YOUNG) {
            return IsInYoung(addr);
        } else if constexpr (SPACE == TargetSpace::TENURED_REGULAR) {
            return !IsInYoung(addr);
        } else if constexpr (SPACE == TargetSpace::TENURED_LARGE) {
            return !IsInYoung(addr);
        } else if constexpr (SPACE == TargetSpace::HUMONGOUS) {
            return !IsInYoung(addr);
        }
        UNREACHABLE();
    };
    return spaceCheck;
}

void MemStatsGenGCTest::InitRoot()
{
    ClassLinker *classLinker = Runtime::GetCurrent()->GetClassLinker();
    Class *klass = classLinker->GetExtension(panda_file::SourceLang::PANDA_ASSEMBLY)
                       ->GetClass(ctx.GetStringArrayClassDescriptor());
    ASSERT_NE(klass, nullptr);
    root = coretypes::Array::Create(klass, ROOT_MAX_SIZE);
    rootSize = 0;
    MakeObjectsPermAlive({root});
}

template <MemStatsGenGCTest::TargetSpace SPACE>
bool MemStatsGenGCTest::InitG1Gc(MemStatsGenGCTest::GcData &gcData)
{
    auto g1Alloc = reinterpret_cast<ObjectAllocatorG1<MT_MODE_MULTI> *>(objectAllocator);
    // NOLINTNEXTLINE(readability-magic-numbers)
    gcData.count = 15U;
    if constexpr (SPACE == TargetSpace::YOUNG) {
        gcData.minSize = 0;
        gcData.maxSize = g1Alloc->GetYoungAllocMaxSize();
    } else if constexpr (SPACE == TargetSpace::TENURED_REGULAR) {
        gcData.minSize = g1Alloc->GetYoungAllocMaxSize() + 1;
        gcData.maxSize = g1Alloc->GetRegularObjectMaxSize();
        if (gcData.minSize >= gcData.maxSize) {
            // Allocator configuration disallows allocating directly in this space
            return false;
        }
    } else if constexpr (SPACE == TargetSpace::TENURED_LARGE) {
        gcData.minSize = g1Alloc->GetYoungAllocMaxSize() + 1;
        gcData.minSize = std::max(gcData.minSize, g1Alloc->GetRegularObjectMaxSize() + 1);
        gcData.maxSize = g1Alloc->GetLargeObjectMaxSize();
        if (gcData.minSize >= gcData.maxSize) {
            // Allocator configuration disallows allocating directly in this space
            return false;
        }
    } else {
        ASSERT(SPACE == TargetSpace::HUMONGOUS);
        gcData.count = 3U;
        gcData.minSize = g1Alloc->GetYoungAllocMaxSize() + 1;
        gcData.minSize = std::max(gcData.minSize, g1Alloc->GetRegularObjectMaxSize() + 1);
        gcData.minSize = std::max(gcData.minSize, g1Alloc->GetLargeObjectMaxSize() + 1);
        gcData.maxSize = gcData.minSize * 3U;
        gcData.checkOom = true;
    }
    return true;
}

template <MemStatsGenGCTest::TargetSpace SPACE>
bool MemStatsGenGCTest::InitGenGc(MemStatsGenGCTest::GcData &gcData)
{
    auto genAlloc = reinterpret_cast<ObjectAllocatorGen<MT_MODE_MULTI> *>(objectAllocator);
    // NOLINTNEXTLINE(readability-magic-numbers)
    gcData.count = 15U;
    if constexpr (SPACE == TargetSpace::YOUNG) {
        gcData.minSize = 0;
        gcData.maxSize = genAlloc->GetYoungAllocMaxSize();
    } else if constexpr (SPACE == TargetSpace::TENURED_REGULAR) {
        gcData.minSize = genAlloc->GetYoungAllocMaxSize() + 1;
        gcData.maxSize = genAlloc->GetRegularObjectMaxSize();
        if (gcData.minSize >= gcData.maxSize) {
            // Allocator configuration disallows allocating directly in this space
            return false;
        }
    } else if constexpr (SPACE == TargetSpace::TENURED_LARGE) {
        gcData.minSize = genAlloc->GetYoungAllocMaxSize() + 1;
        gcData.minSize = std::max(gcData.minSize, genAlloc->GetRegularObjectMaxSize() + 1);
        gcData.maxSize = genAlloc->GetLargeObjectMaxSize();
        if (gcData.minSize >= gcData.maxSize) {
            // Allocator configuration disallows allocating directly in this space
            return false;
        }
    } else {
        ASSERT(SPACE == TargetSpace::HUMONGOUS);
        gcData.count = 3U;
        gcData.minSize = genAlloc->GetYoungAllocMaxSize() + 1;
        gcData.minSize = std::max(gcData.minSize, genAlloc->GetRegularObjectMaxSize() + 1);
        gcData.minSize = std::max(gcData.minSize, genAlloc->GetLargeObjectMaxSize() + 1);
        gcData.maxSize = gcData.minSize * 3U;
        gcData.checkOom = true;
    }
    return true;
}

void MemStatsGenGCTest::MakeObjectsAlive(const ObjVec &objects, int every)
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

void MemStatsGenGCTest::MakeObjectsGarbage(size_t startIdx, size_t afterEndIdx, int every)
{
    int cnt = every;
    for (size_t i = startIdx; i < afterEndIdx; ++i) {
        cnt--;
        if (cnt != 0) {
            continue;
        }
        root->Set(i, static_cast<ObjectHeader *>(nullptr));
        cnt = every;
    }
}

void MemStatsGenGCTest::MakeObjectsPermAlive(const ObjVec &objects, int every)
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

void MemStatsGenGCTest::DumpHandles()
{
    for (auto &hv : handles) {
        for (auto *handle : hv) {
            std::cout << "vector " << (void *)&hv << " handle " << (void *)handle << " obj " << handle->GetPtr()
                      << std::endl;
        }
    }
}

void MemStatsGenGCTest::DumpAliveObjects()
{
    std::cout << "Alive root array : " << handles[0][0]->GetPtr() << std::endl;
    for (size_t i = 0; i < rootSize; ++i) {
        if (root->Get<ObjectHeader *>(i) != nullptr) {
            std::cout << "Alive idx " << i << " : " << root->Get<ObjectHeader *>(i) << std::endl;
        }
    }
}

void MemStatsGenGCTest::DeleteHandles()
{
    for (auto &hv : handles) {
        for (auto *handle : hv) {
            internalAllocator->Delete(handle);
        }
    }
    handles.clear();
}

template <class LanguageConfig>
void MemStatsGenGCTest::PrepareTest()
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
        UNREACHABLE();
    }
}

template <class LanguageConfig>
typename GenerationalGC<LanguageConfig>::MemStats *MemStatsGenGCTest::GetGenMemStats()
{
    // An explicit getter, because the typename has to be template-specialized
    return &reinterpret_cast<GenerationalGC<LanguageConfig> *>(gc)->memStats_;
}

bool MemStatsGenGCTest::IsInYoung(uintptr_t addr)
{
    switch (gcType) {
        case GCType::G1_GC: {
            auto memPool = PoolManager::GetMmapMemPool();
            if (memPool->GetSpaceTypeForAddr(reinterpret_cast<ObjectHeader *>(addr)) != SpaceType::SPACE_TYPE_OBJECT) {
                return false;
            }
            return AddrToRegion(reinterpret_cast<ObjectHeader *>(addr))->HasFlag(RegionFlag::IS_EDEN);
        }
        default:
            UNREACHABLE();  // NYI
    }
    return false;
}

template <MemStatsGenGCTest::TargetSpace SPACE, bool DO_SAVE, bool IS_SINGLE>
typename MemStatsGenGCTest::MemOpReport MemStatsGenGCTest::MakeAllocations()
{
    [[maybe_unused]] int gcCnt = gccnt->count;
    MemStatsGenGCTest::MemOpReport report {};
    report.allocatedCount = 0;
    report.allocatedBytes = 0;
    report.savedCount = 0;
    report.savedBytes = 0;
    size_t bytes = 0;
    [[maybe_unused]] size_t rawObjectsSize;  // currently not tracked by memstats
    MemStatsGenGCTest::GcData gcData {};
    gcData.count = 0;
    gcData.minSize = 0;
    gcData.maxSize = 0;
    gcData.checkOom = false;
    size_t youngSize = reinterpret_cast<GenerationalSpaces *>(
                           reinterpret_cast<ObjectAllocatorGenBase *>(objectAllocator)->GetHeapSpace())
                           ->GetCurrentYoungSize();
    switch (gcType) {
        case GCType::G1_GC: {
            if (!InitG1Gc<SPACE>(gcData)) {
                return report;
            }
            break;
        }
        default:
            UNREACHABLE();
    }

    auto spaceCheck = CreateSpaceCheck<SPACE>();

    if constexpr (SPACE == TargetSpace::YOUNG) {
        // To prevent Young GC collection while we're allocating
        gcData.maxSize = std::min(youngSize / (gcData.count * 6U), gcData.maxSize);
    }

    if (IS_SINGLE) {
        MakeAllocationsWithSingleRepeat<decltype(spaceCheck), 1, SPACE, DO_SAVE>(gcData, report, bytes, rawObjectsSize,
                                                                                 spaceCheck);
    } else {
        MakeAllocationsWithSeveralRepeat<decltype(spaceCheck), 3U, SPACE, DO_SAVE>(gcData, report, bytes,
                                                                                   rawObjectsSize, spaceCheck);
    }

    // We must not have uncounted GCs
    ASSERT(gcCnt == gccnt->count);
    return report;
}

typename MemStatsGenGCTest::MemOpReport MemStatsGenGCTest::HelpAllocTenured()
{
    MemStatsGenGCTest::MemOpReport report {};
    report.allocatedCount = 0;
    report.allocatedBytes = 0;
    report.savedCount = 0;
    report.savedBytes = 0;

    auto oldRootSize = rootSize;

    // One way to get objects into tenured space - by promotion
    auto r = MakeAllocations<TargetSpace::YOUNG, true>();
    gc->WaitForGCInManaged(GCTask(GCTaskCause::YOUNG_GC_CAUSE));
    MakeObjectsGarbage(oldRootSize, oldRootSize + (rootSize - oldRootSize) / 2U);

    report.allocatedCount = r.savedCount;
    report.allocatedBytes = r.savedBytes;
    report.savedCount = r.savedCount / 2U;
    report.savedBytes = r.savedBytes / 2U;

    // Another way - by direct allocation in tenured if possible
    auto r2 = MakeAllocations<TargetSpace::TENURED_REGULAR, true>();

    report.allocatedCount += r2.allocatedCount;
    report.allocatedBytes += r2.allocatedBytes;
    report.savedCount += r2.savedCount;
    report.savedBytes += r2.savedBytes;

    // Large objects are also tenured in terms of gen memstats
    auto r3 = MakeAllocations<TargetSpace::TENURED_LARGE, true>();

    report.allocatedCount += r3.allocatedCount;
    report.allocatedBytes += r3.allocatedBytes;
    report.savedCount += r3.savedCount;
    report.savedBytes += r3.savedBytes;

    auto r4 = MakeAllocations<TargetSpace::HUMONGOUS, true>();

    report.allocatedCount += r4.allocatedCount;
    report.allocatedBytes += r4.allocatedBytes;
    report.savedCount += r4.savedCount;
    report.savedBytes += r4.savedBytes;
    return report;
}

template <typename T>
MemStatsGenGCTest::RealStatsLocations MemStatsGenGCTest::GetGenMemStatsDetails(T gms)
{
    RealStatsLocations loc {};
    loc.youngFreedObjectsCount = &gms->youngFreeObjectCount_;
    loc.youngFreedObjectsSize = &gms->youngFreeObjectSize_;
    loc.youngMovedObjectsCount = &gms->youngMoveObjectCount_;
    loc.youngMovedObjectsSize = &gms->youngMoveObjectSize_;
    loc.tenuredFreedObjectsCount = &gms->tenuredFreeObjectCount_;
    loc.tenuredFreedObjectsSize = &gms->tenuredFreeObjectSize_;
    return loc;
}

TEST_F(MemStatsGenGCTest, TrivialGarbageStatsGenGcTest)
{
    for (int gctypeIdx = 0; static_cast<GCType>(gctypeIdx) <= GCType::GCTYPE_LAST; ++gctypeIdx) {
        auto gcTypeLocal = static_cast<GCType>(gctypeIdx);
        if (gcTypeLocal == GCType::EPSILON_G1_GC || gcTypeLocal == GCType::INVALID_GC) {
            continue;
        }
        if (!IsGenerationalGCType(gcTypeLocal)) {
            continue;
        }
        std::string gctype = static_cast<std::string>(GCStringFromType(gcTypeLocal));
        for (MemStatsGenGCTest::Config cfg; !cfg.End(); ++cfg) {
            SetupRuntime(gctype, cfg);

            {
                HandleScope<ObjectHeader *> scope(thread);
                PrepareTest<ark::PandaAssemblyLanguageConfig>();
                auto *genMs = GetGenMemStats<ark::PandaAssemblyLanguageConfig>();
                RealStatsLocations loc = GetGenMemStatsDetails<decltype(genMs)>(genMs);

                gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));  // Heap doesn't have unexpected garbage now

                // Make a trivial allocation of unaligned size and make it garbage
                auto r = MakeAllocations<TargetSpace::YOUNG, false, true>();
                gc->WaitForGCInManaged(GCTask(GCTaskCause::YOUNG_GC_CAUSE));
                ASSERT_EQ(2U, gccnt->count);
                if (NeedToCheckYoungFreedCount()) {
                    ASSERT_EQ(*loc.youngFreedObjectsCount, r.allocatedCount);
                }
                ASSERT_EQ(*loc.youngFreedObjectsSize, r.allocatedBytes);
                ASSERT_EQ(*loc.youngMovedObjectsCount, 0);
                ASSERT_EQ(*loc.youngMovedObjectsSize, 0);
                ASSERT_EQ(*loc.tenuredFreedObjectsCount, 0);
                ASSERT_EQ(*loc.tenuredFreedObjectsSize, 0);
                if (NeedToCheckYoungFreedCount()) {
                    ASSERT_EQ(gcMs->GetObjectsFreedCount(), r.allocatedCount);
                }
                if (PANDA_TRACK_TLAB_ALLOCATIONS) {
                    ASSERT_EQ(gcMs->GetObjectsFreedBytes(), r.allocatedBytes);
                }
                ASSERT_EQ(gcMs->GetLargeObjectsFreedCount(), 0);
                ASSERT_EQ(gcMs->GetLargeObjectsFreedBytes(), 0);
            }

            ResetRuntime();
        }
    }
}

TEST_F(MemStatsGenGCTest, TrivialAliveAndTenuredStatsGenGcTest)
{
    for (int gctypeIdx = 0; static_cast<GCType>(gctypeIdx) <= GCType::GCTYPE_LAST; ++gctypeIdx) {
        auto gcTypeLocal = static_cast<GCType>(gctypeIdx);
        if (gcTypeLocal == GCType::EPSILON_G1_GC || gcTypeLocal == GCType::INVALID_GC) {
            continue;
        }
        if (!IsGenerationalGCType(gcTypeLocal)) {
            continue;
        }
        std::string gctype = static_cast<std::string>(GCStringFromType(gcTypeLocal));
        for (MemStatsGenGCTest::Config cfg; !cfg.End(); ++cfg) {
            SetupRuntime(gctype, cfg);

            {
                HandleScope<ObjectHeader *> scope(thread);
                PrepareTest<ark::PandaAssemblyLanguageConfig>();
                auto *genMs = GetGenMemStats<ark::PandaAssemblyLanguageConfig>();
                RealStatsLocations loc = GetGenMemStatsDetails<decltype(genMs)>(genMs);

                gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));  // Heap doesn't have unexpected garbage now

                // Make a trivial allocation of unaligned size and make it alive
                auto r = MakeAllocations<TargetSpace::YOUNG, true, true>();
                gc->WaitForGCInManaged(GCTask(GCTaskCause::YOUNG_GC_CAUSE));
                ASSERT_EQ(2U, gccnt->count);
                ASSERT_EQ(*loc.youngFreedObjectsCount, 0);
                ASSERT_EQ(*loc.youngFreedObjectsSize, 0);
                ASSERT_EQ(*loc.youngMovedObjectsCount, r.savedCount);
                ASSERT_EQ(*loc.youngMovedObjectsSize, r.savedBytes);
                ASSERT_EQ(*loc.tenuredFreedObjectsCount, 0);
                ASSERT_EQ(*loc.tenuredFreedObjectsSize, 0);

                // Expecting that r.saved_bytes/count have been promoted into tenured
                // Make them garbage
                MakeObjectsGarbage(0, rootSize);
                gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                ASSERT_EQ(3U, gccnt->count);
                ASSERT_EQ(*loc.youngFreedObjectsCount, 0);
                ASSERT_EQ(*loc.youngFreedObjectsSize, 0);
                ASSERT_EQ(*loc.youngMovedObjectsCount, 0);
                ASSERT_EQ(*loc.youngMovedObjectsSize, 0);
                ASSERT_EQ(*loc.tenuredFreedObjectsCount, r.savedCount);
                ASSERT_EQ(*loc.tenuredFreedObjectsSize, r.savedBytes);
            }

            ResetRuntime();
        }
    }
}

TEST_F(MemStatsGenGCTest, TrivialTenuredAndLargeStatsGenGcTest)
{
    for (int gctypeIdx = 0; static_cast<GCType>(gctypeIdx) <= GCType::GCTYPE_LAST; ++gctypeIdx) {
        auto gcTypeLocal = static_cast<GCType>(gctypeIdx);
        if (gcTypeLocal == GCType::EPSILON_G1_GC || gcTypeLocal == GCType::INVALID_GC) {
            continue;
        }
        if (!IsGenerationalGCType(gcTypeLocal)) {
            continue;
        }
        std::string gctype = static_cast<std::string>(GCStringFromType(gcTypeLocal));
        for (MemStatsGenGCTest::Config cfg; !cfg.End(); ++cfg) {
            SetupRuntime(gctype, cfg);

            {
                HandleScope<ObjectHeader *> scope(thread);
                PrepareTest<ark::PandaAssemblyLanguageConfig>();
                auto *genMs = GetGenMemStats<ark::PandaAssemblyLanguageConfig>();
                RealStatsLocations loc = GetGenMemStatsDetails<decltype(genMs)>(genMs);

                gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));  // Heap doesn't have unexpected garbage now

                // Make a trivial allocation of unaligned size in tenured space and make it garbage
                auto r = MakeAllocations<TargetSpace::TENURED_REGULAR, false, true>();
                gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                ASSERT_EQ(2U, gccnt->count);
                ASSERT_EQ(*loc.youngFreedObjectsCount, 0);
                ASSERT_EQ(*loc.youngFreedObjectsSize, 0);
                ASSERT_EQ(*loc.youngMovedObjectsCount, 0);
                ASSERT_EQ(*loc.youngMovedObjectsSize, 0);
                ASSERT_EQ(*loc.tenuredFreedObjectsCount, r.allocatedCount);
                ASSERT_EQ(*loc.tenuredFreedObjectsSize, r.allocatedBytes);

                // Make a trivial allocation of unaligned size large object and make it garbage
                r = MakeAllocations<TargetSpace::TENURED_LARGE, false, true>();
                gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                ASSERT_EQ(3U, gccnt->count);
                ASSERT_EQ(*loc.youngFreedObjectsCount, 0);
                ASSERT_EQ(*loc.youngFreedObjectsSize, 0);
                ASSERT_EQ(*loc.youngMovedObjectsCount, 0);
                ASSERT_EQ(*loc.youngMovedObjectsSize, 0);
                ASSERT_EQ(*loc.tenuredFreedObjectsCount, r.allocatedCount);
                ASSERT_EQ(*loc.tenuredFreedObjectsSize, r.allocatedBytes);

                r = MakeAllocations<TargetSpace::HUMONGOUS, false, true>();
                gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                ASSERT_EQ(*loc.youngFreedObjectsCount, 0);
                ASSERT_EQ(*loc.youngFreedObjectsSize, 0);
                ASSERT_EQ(*loc.youngMovedObjectsCount, 0);
                ASSERT_EQ(*loc.youngMovedObjectsSize, 0);
                ASSERT_EQ(*loc.tenuredFreedObjectsCount, r.allocatedCount);
                ASSERT_EQ(*loc.tenuredFreedObjectsSize, r.allocatedBytes);
            }

            ResetRuntime();
        }
    }
}

TEST_F(MemStatsGenGCTest, YoungStatsGenGcTest)
{
    for (int gctypeIdx = 0; static_cast<GCType>(gctypeIdx) <= GCType::GCTYPE_LAST; ++gctypeIdx) {
        if (static_cast<GCType>(gctypeIdx) == GCType::EPSILON_G1_GC ||
            static_cast<GCType>(gctypeIdx) == GCType::INVALID_GC) {
            continue;
        }
        if (!IsGenerationalGCType(static_cast<GCType>(gctypeIdx))) {
            continue;
        }
        std::string gctype = static_cast<std::string>(GCStringFromType(static_cast<GCType>(gctypeIdx)));
        for (MemStatsGenGCTest::Config cfg; !cfg.End(); ++cfg) {
            SetupRuntime(gctype, cfg);

            {
                HandleScope<ObjectHeader *> scope(thread);
                PrepareTest<ark::PandaAssemblyLanguageConfig>();
                auto *genMs = GetGenMemStats<ark::PandaAssemblyLanguageConfig>();
                RealStatsLocations loc = GetGenMemStatsDetails<decltype(genMs)>(genMs);

                gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                // Young shall be empty now.
                auto r = MakeAllocations<TargetSpace::YOUNG, true>();
                gc->WaitForGCInManaged(GCTask(GCTaskCause::YOUNG_GC_CAUSE));

                if (NeedToCheckYoungFreedCount()) {
                    ASSERT_EQ(*loc.youngFreedObjectsCount, r.allocatedCount - r.savedCount);
                }
                ASSERT_EQ(*loc.youngFreedObjectsSize, r.allocatedBytes - r.savedBytes);
                ASSERT_EQ(*loc.youngMovedObjectsCount, r.savedCount);
                ASSERT_EQ(*loc.youngMovedObjectsSize, r.savedBytes);
                ASSERT_EQ(*loc.tenuredFreedObjectsCount, 0);
                ASSERT_EQ(*loc.tenuredFreedObjectsSize, 0);
            }

            ResetRuntime();
        }
    }
}

TEST_F(MemStatsGenGCTest, TenuredStatsFullGenGcTest)
{
    for (int gctypeIdx = 0; static_cast<GCType>(gctypeIdx) <= GCType::GCTYPE_LAST; ++gctypeIdx) {
        if (static_cast<GCType>(gctypeIdx) == GCType::EPSILON_G1_GC ||
            static_cast<GCType>(gctypeIdx) == GCType::INVALID_GC) {
            continue;
        }
        if (!IsGenerationalGCType(static_cast<GCType>(gctypeIdx))) {
            continue;
        }
        std::string gctype = static_cast<std::string>(GCStringFromType(static_cast<GCType>(gctypeIdx)));
        for (MemStatsGenGCTest::Config cfg; !cfg.End(); ++cfg) {
            SetupRuntime(gctype, cfg);

            {
                HandleScope<ObjectHeader *> scope(thread);
                PrepareTest<ark::PandaAssemblyLanguageConfig>();
                auto *genMs = GetGenMemStats<ark::PandaAssemblyLanguageConfig>();
                RealStatsLocations loc = GetGenMemStatsDetails<decltype(genMs)>(genMs);

                gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                // Young shall be empty now.

                uint32_t tCount = 0;
                uint64_t tBytes = 0;

                for (int i = 0; i < FULL_TEST_ALLOC_TIMES; ++i) {
                    [[maybe_unused]] int gcCnt = gccnt->count;
                    auto r = HelpAllocTenured();
                    // HelpAllocTenured shall trigger young gc, which is allowed to be mixed
                    ASSERT(gcCnt + 1 == gccnt->count);
                    auto tfocY = *loc.tenuredFreedObjectsCount;
                    auto tfosY = *loc.tenuredFreedObjectsSize;
                    ASSERT(r.allocatedCount > 0);
                    gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                    ASSERT_EQ(*loc.youngFreedObjectsCount, 0);
                    ASSERT_EQ(*loc.youngFreedObjectsSize, 0);
                    ASSERT_EQ(*loc.youngMovedObjectsCount, 0);
                    ASSERT_EQ(*loc.youngMovedObjectsSize, 0);
                    ASSERT_EQ(*loc.tenuredFreedObjectsCount + tfocY, r.allocatedCount - r.savedCount);
                    ASSERT_EQ(*loc.tenuredFreedObjectsSize + tfosY, r.allocatedBytes - r.savedBytes);
                    tCount += r.savedCount;
                    tBytes += r.savedBytes;
                }

                // Empty everything
                auto ry = MakeAllocations<TargetSpace::YOUNG, false>();
                MakeObjectsGarbage(0, rootSize);

                gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                if (NeedToCheckYoungFreedCount()) {
                    ASSERT_EQ(*loc.youngFreedObjectsCount, ry.allocatedCount);
                }
                ASSERT_EQ(*loc.youngFreedObjectsSize, ry.allocatedBytes);
                ASSERT_EQ(*loc.youngMovedObjectsCount, 0);
                ASSERT_EQ(*loc.youngMovedObjectsSize, 0);
                ASSERT_EQ(*loc.tenuredFreedObjectsCount, tCount);
                ASSERT_EQ(*loc.tenuredFreedObjectsSize, tBytes);

                gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                ASSERT_EQ(*loc.youngFreedObjectsCount, 0);
                ASSERT_EQ(*loc.youngFreedObjectsSize, 0);
                ASSERT_EQ(*loc.youngMovedObjectsCount, 0);
                ASSERT_EQ(*loc.youngMovedObjectsSize, 0);
                ASSERT_EQ(*loc.tenuredFreedObjectsCount, 0);
                ASSERT_EQ(*loc.tenuredFreedObjectsSize, 0);
            }

            ResetRuntime();
        }
    }
}

TEST_F(MemStatsGenGCTest, TenuredStatsMixGenGcTest)
{
    for (int gctypeIdx = 0; static_cast<GCType>(gctypeIdx) <= GCType::GCTYPE_LAST; ++gctypeIdx) {
        if (static_cast<GCType>(gctypeIdx) == GCType::EPSILON_G1_GC ||
            static_cast<GCType>(gctypeIdx) == GCType::INVALID_GC) {
            continue;
        }
        if (!IsGenerationalGCType(static_cast<GCType>(gctypeIdx))) {
            continue;
        }
        std::string gctype = static_cast<std::string>(GCStringFromType(static_cast<GCType>(gctypeIdx)));
        for (MemStatsGenGCTest::Config cfg; !cfg.End(); ++cfg) {
            SetupRuntime(gctype, cfg);

            {
                HandleScope<ObjectHeader *> scope(thread);
                PrepareTest<ark::PandaAssemblyLanguageConfig>();
                GCTaskCause mixedCause;
                switch (gcType) {
                    case GCType::G1_GC: {
                        mixedCause = MIXED_G1_GC_CAUSE;
                        break;
                    }
                    default:
                        UNREACHABLE();  // NIY
                }
                auto *genMs = GetGenMemStats<ark::PandaAssemblyLanguageConfig>();
                RealStatsLocations loc = GetGenMemStatsDetails<decltype(genMs)>(genMs);

                gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                // Young shall be empty now.

                uint32_t tCount = 0;
                uint64_t tBytes = 0;

                {
                    uint32_t deadCount = 0;
                    uint64_t deadBytes = 0;
                    uint32_t expectedDeadCount = 0;
                    uint64_t expectedDeadBytes = 0;
                    for (int i = 0; i < MIX_TEST_ALLOC_TIMES; ++i) {
                        [[maybe_unused]] int gcCnt = gccnt->count;
                        auto r = HelpAllocTenured();
                        // HelpAllocTenured shall trigger young gc, which is allowed to be mixed
                        ASSERT(gcCnt + 1 == gccnt->count);
                        deadCount += *loc.tenuredFreedObjectsCount;
                        deadBytes += *loc.tenuredFreedObjectsSize;
                        // Mixed can free not all the tenured garbage, so run it until it stalls
                        do {
                            gc->WaitForGCInManaged(GCTask(mixedCause));
                            ASSERT_EQ(*loc.youngFreedObjectsCount, 0);
                            ASSERT_EQ(*loc.youngFreedObjectsSize, 0);
                            ASSERT_EQ(*loc.youngMovedObjectsCount, 0);
                            ASSERT_EQ(*loc.youngMovedObjectsSize, 0);
                            deadCount += *loc.tenuredFreedObjectsCount;
                            deadBytes += *loc.tenuredFreedObjectsSize;
                        } while (*loc.tenuredFreedObjectsCount != 0);
                        tCount += r.savedCount;
                        tBytes += r.savedBytes;
                        expectedDeadCount += r.allocatedCount - r.savedCount;
                        expectedDeadBytes += r.allocatedBytes - r.savedBytes;
                    }
                    gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                    ASSERT_EQ(*loc.youngFreedObjectsCount, 0);
                    ASSERT_EQ(*loc.youngFreedObjectsSize, 0);
                    ASSERT_EQ(*loc.youngMovedObjectsCount, 0);
                    ASSERT_EQ(*loc.youngMovedObjectsSize, 0);
                    deadCount += *loc.tenuredFreedObjectsCount;
                    deadBytes += *loc.tenuredFreedObjectsSize;
                    ASSERT_EQ(deadCount, expectedDeadCount);
                    ASSERT_EQ(deadBytes, expectedDeadBytes);
                }

                // Empty everything
                auto ry = MakeAllocations<TargetSpace::YOUNG, false>();
                MakeObjectsGarbage(0, rootSize);
                {
                    uint32_t deadCount = 0;
                    uint64_t deadBytes = 0;
                    do {
                        gc->WaitForGCInManaged(GCTask(mixedCause));
                        if (NeedToCheckYoungFreedCount()) {
                            ASSERT_EQ(*loc.youngFreedObjectsCount, ry.allocatedCount);
                        }
                        ASSERT_EQ(*loc.youngFreedObjectsSize, ry.allocatedBytes);
                        ASSERT_EQ(*loc.youngMovedObjectsCount, 0);
                        ASSERT_EQ(*loc.youngMovedObjectsSize, 0);
                        deadCount += *loc.tenuredFreedObjectsCount;
                        deadBytes += *loc.tenuredFreedObjectsSize;
                        ry.allocatedCount = 0;
                        ry.allocatedBytes = 0;
                    } while (*loc.tenuredFreedObjectsCount != 0);
                    gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                    ASSERT_EQ(*loc.youngFreedObjectsCount, 0);
                    ASSERT_EQ(*loc.youngFreedObjectsSize, 0);
                    ASSERT_EQ(*loc.youngMovedObjectsCount, 0);
                    ASSERT_EQ(*loc.youngMovedObjectsSize, 0);
                    deadCount += *loc.tenuredFreedObjectsCount;
                    deadBytes += *loc.tenuredFreedObjectsSize;
                    ASSERT_EQ(deadCount, tCount);
                    ASSERT_EQ(deadBytes, tBytes);
                }
                gc->WaitForGCInManaged(GCTask(FULL_GC_CAUSE));
                ASSERT_EQ(*loc.youngFreedObjectsCount, 0);
                ASSERT_EQ(*loc.youngFreedObjectsSize, 0);
                ASSERT_EQ(*loc.youngMovedObjectsCount, 0);
                ASSERT_EQ(*loc.youngMovedObjectsSize, 0);
                ASSERT_EQ(*loc.tenuredFreedObjectsCount, 0);
                ASSERT_EQ(*loc.tenuredFreedObjectsSize, 0);
            }

            ResetRuntime();
        }
    }
}
}  // namespace ark::mem::test
