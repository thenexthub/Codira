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


#include "CodiraRuntime.h"

#include "Base/Log.h"
#include "ExceptionManager.h"
#include "Heap/Heap.h"
#include "HeapManager.h"
#include "ObjectManager.h"
#include "StackManager.h"
#if defined(__linux__) || defined(hongmeng) || defined(__APPLE__)
#include "SignalManager.h"
#endif
#include "TypeInfoManager.h"
#ifdef _WIN64
#include "WinModuleManager.h"
#endif
#include "Common/ScopedObjectAccess.h"
#include "Concurrency/Concurrency.h"
#include "Heap/Collector/FinalizerProcessor.h"
#include "Heap/Heap.h"
#include "LoaderManager.h"
#include "LogManager.h"
#include "Mutator/Mutator.h"
#include "Mutator/MutatorManager.h"
#include "Heap/Allocator/RegionSpace.h"
#ifdef CODIRA_GWPASAN_SUPPORT
#include "Sanitizer/SanitizerInterface.h"
#endif
namespace MapleRuntime {
StackGrowConfig CodiraRuntime::stackGrowConfig = StackGrowConfig::UNDEF;
extern "C" MRT_EXPORT void MRT_LibraryOnLoad(uint64_t address, bool enableGC)
{
    ScopedEntryTrace trace("CODERT_LOAD_LIBRARY");
    if (address == 0) {
        return;
    }
    if (!enableGC) {
        Heap::GetHeap().EnableGC(false);
    }
    LoaderManager* loaderManager = LoaderManager::GetInstance();
    loaderManager->LoadFile(address);
}

extern "C" MRT_EXPORT void MRT_PreInitializePackage(uint64_t address)
{
    LoaderManager* loaderManager = LoaderManager::GetInstance();
    loaderManager->PreInitializePackage(address);
}

extern "C" MRT_EXPORT void MRT_LibraryUnLoad(uint64_t address)
{
    if (address == 0 || LoaderManager::GetReleaseStatus()) {
        return;
    }
    LoaderManager* loaderManager = LoaderManager::GetInstance();
    loaderManager->UnloadFile(address);
}

#if defined(MRT_DEBUG) && (MRT_DEBUG == 1)
void DumpAllStackTrace0()
{
    LOG(RTLOG_INFO, "---------------------------Dump all codethread stack trace--------------------------- \n");
    MutatorManager::Instance().VisitAllMutators([&](Mutator& mutator) {
        // don't print finalizerProcessorThread
        if (mutator.GetTid() != Heap::GetHeap().GetFinalizerProcessor().GetTid()) {
            LOG(RTLOG_INFO, "codethread thread:%d mutator:%p :\n", mutator.GetCODEThreadId(), &mutator);
            StackManager::PrintStackTrace(&(mutator.GetUnwindContext()));
            LOG(RTLOG_INFO, "\n");
        }
    });
}

// dump and print StackTraces of all codethreads
extern "C" void MRT_DumpAllStackTrace()
{
    ScopedEnterSaferegion enterSaferegion(false);
    if (MutatorManager::Instance().WorldStopped()) {
        DumpAllStackTrace0();
    } else {
        ScopedStopTheWorld stw("dump all stack trace");
        DumpAllStackTrace0();
    }
}
#ifdef __APPLE__
extern "C" MRT_EXPORT void CODE_MRT_DumpAllStackTrace();
__asm__(".global _CODE_MRT_DumpAllStackTrace\n\t.set _CODE_MRT_DumpAllStackTrace, _MRT_DumpAllStackTrace");
#else
extern "C" MRT_EXPORT void CODE_MRT_DumpAllStackTrace() __attribute__((alias("MRT_DumpAllStackTrace")));
#endif
#endif

// Runtime implementation
Runtime* Runtime::runtime = nullptr;
std::atomic<bool> g_initialized = { false };

void CodiraRuntime::CreateAndInit(const RuntimeParam& runtimeParam)
{
    ScopedEntryTrace trace("CODERT_CreateAndInit");
    if (runtime != nullptr) {
    }
    // need to create runtime
    if (g_initialized.load(std::memory_order_acquire)) {
        return;
    }
    auto codeRuntime = new (std::nothrow) CodiraRuntime(runtimeParam);
    CHECK_DETAIL(codeRuntime != nullptr, "new CodiraRuntime failed");
    Runtime::runtime = codeRuntime;
    codeRuntime->Init();
    g_initialized.store(true, std::memory_order_release);
}

void CodiraRuntime::FiniAndDelete()
{
    ScopedEntryTrace trace("CODERT_FiniAndDelete");
    if (Runtime::runtime == nullptr) {
        LOG(RTLOG_ERROR, "Fini called but Codira runtime is not running");
        return;
    }
    auto codeRuntime = reinterpret_cast<CodiraRuntime*>(Runtime::runtime);
    Runtime::runtime = nullptr;
    codeRuntime->Fini();
    delete codeRuntime;
}

CodiraRuntime::CodiraRuntime(const RuntimeParam& runtimeParam) : param(runtimeParam) {}

template<typename T>
inline T* NewAndInit()
{
    T* temp = new (std::nothrow) T();
    CHECK_DETAIL(temp != nullptr, "NewAndInit failed");
    temp->Init();
    return temp;
}

template<typename T, typename A>
inline T* NewAndInit(A arg)
{
    T* temp = new (std::nothrow) T();
    CHECK_DETAIL(temp != nullptr, "NewAndInit failed");
    temp->Init(arg);
    return temp;
}

// LoaderManager must be initialized before ConcurrencyModel because of the reservedStackSize field.
// ConcurrencyModel must be initialized before SignalManager because of the stackGuardCheck field.
// HeapManager must be initialized before HeapManager because of inherited properties of signal handlers.
void CodiraRuntime::Init()
{
    PagePool::Instance().Init(param.heapParam.heapSize * KB / MRT_PAGE_SIZE);
#ifdef CODIRA_GWPASAN_SUPPORT
    // setup gwpasan before heap initialized
    MapleRuntime::Sanitizer::SetupGwpAsanAsNeeded();
#endif
#ifdef _WIN64
    winModuleManager = NewAndInit<WinModuleManager>();
#endif
    stackManager = NewAndInit<StackManager>();
    logManager = NewAndInit<LogManager>();
    mutatorManager = NewAndInit<MutatorManager>();
    concurrencyModel = NewAndInit<Concurrency>(param.coParam);
#if defined(__linux__) || defined(hongmeng) || defined(__APPLE__)
    signalManager = NewAndInit<SignalManager>();
#endif
    heapManager = NewAndInit<HeapManager>(param.heapParam);
    typeInfoManager = &TypeInfoManager::GetTypeInfoManager();
    typeInfoManager->Init();
    loaderManager = LoaderManager::GetInstance();
    loaderManager->Init();
    objectManager = NewAndInit<ObjectManager>();
    exceptionManager = NewAndInit<ExceptionManager>();

    LOG(RTLOG_INFO, "Codira runtime started.");
    // Record runtime parameter to report. heap growth value needs to plus 1.
#if not defined(__EULER__)
    VLOG(REPORT,
        "Runtime parameter:\n\tHeap size: %zu(KB)\n\tRegion size: %zu(KB)\n\tExemption threshold: %.2f\n\t"
        "Heap utilization: %.2f\n\tHeap growth: %.2f\n\tAllocation rate: %.2f(MB/s)\n\tAllocation wait time: %zuns\n\t"
        "GC Threshold: %zu(KB)\n\tGarbage threshold: %.2f\n\tGC interval: %llums\n\tBackup GC interval: %llus\n\t"
        "Log level: %d\n\tThread stack size: %zu(KB)\n\tCodira stack size: %zu(KB)\n\t"
        "Processor number: %d", param.heapParam.heapSize, param.heapParam.regionSize,
        param.heapParam.exemptionThreshold, param.heapParam.heapUtilization, 1 + param.heapParam.heapGrowth,
        param.heapParam.allocationRate, param.heapParam.allocationWaitTime,
        param.gcParam.gcThreshold / KB, param.gcParam.garbageThreshold,
        param.gcParam.gcInterval / MILLI_SECOND_TO_NANO_SECOND, param.gcParam.backupGCInterval / SECOND_TO_NANO_SECOND,
        static_cast<int>(param.logParam.logLevel), param.coParam.thStackSize,
        param.coParam.coStackSize, param.coParam.processorNum);
#else
    VLOG(REPORT,
        "Runtime parameter:\n\tHeap size: %zu(KB)\n\tRegion size: %zu(KB)\n\tExemption threshold: %.2f\n\t"
        "Heap utilization: %.2f\n\tCache Ratio: %.2f\n\tHeap growth: %.2f\n\tAllocation rate: %.2f(MB/s)\n\t"
        "Allocation wait time: %zuns\n\tGC Threshold: %zu(KB)\n\t"
        "Garbage threshold: %.2f\n\tGC interval: %zums\n\tBackup GC interval: %zus\n\t"
        "Log level: %d\n\tThread stack size: %zu(KB)\n\tCodira stack size: %zu(KB)\n\tProcessor number: %d",
        param.heapParam.heapSize, param.heapParam.regionSize, param.heapParam.exemptionThreshold,
        param.heapParam.heapUtilization, reinterpret_cast<RegionSpace&>(Heap::GetHeap().GetAllocator()).
        GetRegionManager().GetCacheRatio(), 1 + param.heapParam.heapGrowth,
        param.heapParam.allocationRate, param.heapParam.allocationWaitTime,
        param.gcParam.gcThreshold / KB, param.gcParam.garbageThreshold,
        param.gcParam.gcInterval / MILLI_SECOND_TO_NANO_SECOND, param.gcParam.backupGCInterval / SECOND_TO_NANO_SECOND,
        static_cast<int>(param.logParam.logLevel), param.coParam.thStackSize,
        param.coParam.coStackSize, param.coParam.processorNum);
#endif
}

void* CodiraRuntime::CreateSubSchedulerAndInit(ScheduleType type)
{
    std::lock_guard<std::mutex> guard(mtx);
    auto concurrency = new (std::nothrow) Concurrency();
    CHECK_DETAIL(concurrency != nullptr, "NewAndInit failed");
    auto defaultParam = CodiraRuntime::GetConcurrencyParam();
    struct ConcurrencyParam coParam = {
        .thStackSize = defaultParam.thStackSize,
        .coStackSize = defaultParam.coStackSize,
        .processorNum = 1,
    };
    concurrency->Init(coParam, type);
    void* scheduler = concurrency->GetThreadScheduler();
    subModelMap.insert({scheduler, concurrency});
    return scheduler;
}

void* CodiraRuntime::CreateSingleThreadScheduler()
{
    auto defaultParam = CodiraRuntime::GetConcurrencyParam();
    ScheduleAttr attr;
    ScheduleAttrInit(&attr);
    ScheduleAttrThstackSizeSet(&attr, defaultParam.thStackSize * KB);
    ScheduleAttrCostackSizeSet(&attr, defaultParam.coStackSize * KB);
    ScheduleAttrProcessorNumSet(&attr, 1);
    ScheduleAttrStackGrowSet(&attr, false);
    ScheduleAttrStackProtectSet(&attr, false);

    ScheduleGetTlsHookRegister((GetTlsHookFunc)MRT_GetThreadLocalData);

    // should not use system page size to calculate reserved stack size,
    // because the page size could be different in different system.
#ifdef _WIN64
    constexpr uint32_t reservedStackSize = 24 * KB;
#else
    constexpr uint32_t reservedStackSize = 8 * KB;
#endif
    CODEThreadStackReversedSet(reservedStackSize);
    return ScheduleNew(SCHEDULE_UI_THREAD, &attr);
}

bool CodiraRuntime::CheckSubSchedulerValid(void* scheduler)
{
    // Check scheduler exist.
    auto found = subModelMap.find(scheduler);
    if (found == subModelMap.end()) {
        return false;
    }
    return true;
}

template<typename T>
inline void CheckAndFini(T*& module)
{
    if (module != nullptr) {
        module->Fini();
    }

    delete module;
    module = nullptr;
}

void CodiraRuntime::Fini()
{
    // To avoid foreign thread access finalized runtime.
    ThreadLocal::ThreadLocalFini();
    // since there might be failure during initialization,
    // here we need to check and call fini.
#ifdef _WIN64
    CheckAndFini<WinModuleManager>(winModuleManager);
#endif
    CheckAndFini<ObjectManager>(objectManager);
    CheckAndFini<HeapManager>(heapManager);
    CheckAndFini<ExceptionManager>(exceptionManager);
    CheckAndFini<StackManager>(stackManager);
    loaderManager->Fini();
    typeInfoManager->Fini();
    CheckAndFini<MutatorManager>(mutatorManager);
    CheckAndFini<ConcurrencyModel>(concurrencyModel);
#if defined(__linux__) || defined(hongmeng) || defined(__APPLE__)
    CheckAndFini<SignalManager>(signalManager);
#endif
    CheckAndFini<LogManager>(logManager);
    PagePool::Instance().Fini();

    LOG(RTLOG_INFO, "Codira runtime shutdown.");
}

bool CodiraRuntime::FiniSubScheduler(void* scheduler)
{
    std::lock_guard<std::mutex> guard(mtx);
    if (!CheckSubSchedulerValid(scheduler)) {
        return false;
    }
    auto concurrency = subModelMap[scheduler];
    CheckAndFini<ConcurrencyModel>(concurrency);
    subModelMap.erase(scheduler);
    return true;
}

extern "C" bool MRT_CODELibInit(const char* libName) { return LoaderManager::GetInstance()->LibInit(libName); }
#ifdef __APPLE__
extern "C" MRT_EXPORT void CODE_MRT_LibraryOnLoad(uint64_t address, bool enableGC);
__asm__(".global _CODE_MRT_LibraryOnLoad\n\t.set _CODE_MRT_LibraryOnLoad, _MRT_LibraryOnLoad");
extern "C" MRT_EXPORT void CODE_MRT_LibraryUnLoad(uint64_t address);
__asm__(".global _CODE_MRT_LibraryUnLoad\n\t.set _CODE_MRT_LibraryUnLoad, _MRT_LibraryUnLoad");
extern "C" MRT_EXPORT void CODE_MRT_PreInitializePackage(uint64_t address);
__asm__(".global _CODE_MRT_PreInitializePackage\n\t.set _CODE_MRT_PreInitializePackage, _MRT_PreInitializePackage");
#else
extern "C" MRT_EXPORT void CODE_MRT_LibraryOnLoad(uint64_t address, bool enableGC)
    __attribute__((alias("MRT_LibraryOnLoad")));
extern "C" MRT_EXPORT void CODE_MRT_LibraryUnLoad(uint64_t address) __attribute__((alias("MRT_LibraryUnLoad")));
extern "C" MRT_EXPORT void CODE_MRT_PreInitializePackage(uint64_t address)
    __attribute__((alias("MRT_PreInitializePackage")));
#endif
} // namespace MapleRuntime
