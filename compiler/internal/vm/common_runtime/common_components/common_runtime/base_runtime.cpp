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

#include "common_interfaces/base_runtime.h"

#include "common_components/common_runtime/base_runtime_param.h"
#include "common_components/common_runtime/hooks.h"
#include "common_components/common/page_pool.h"
#include "common_components/heap/collector/heuristic_gc_policy.h"
#include "common_components/heap/heap.h"
#include "common_components/heap/heap_manager.h"
#include "common_components/mutator/mutator_manager.h"
#include "common_interfaces/thread/thread_state_transition.h"

namespace common {

std::mutex BaseRuntime::vmCreationLock_;
BaseRuntime *BaseRuntime::baseRuntimeInstance_ = nullptr;
bool BaseRuntime::initialized_ = false;

BaseRuntime *BaseRuntime::GetInstance()
{
    if (UNLIKELY_CC(baseRuntimeInstance_ == nullptr)) {
        std::unique_lock<std::mutex> lock(vmCreationLock_);
        if (baseRuntimeInstance_ == nullptr) {
            baseRuntimeInstance_ = new BaseRuntime();
        }
    }
    return baseRuntimeInstance_;
}

void BaseRuntime::DestroyInstance()
{
    std::unique_lock<std::mutex> lock(vmCreationLock_);
    delete baseRuntimeInstance_;
    baseRuntimeInstance_ = nullptr;
}

template <typename T>
inline T *NewAndInit()
{
    T *temp = new (std::nothrow) T();
    LOGF_CHECK(temp != nullptr) << "NewAndInit failed";
    temp->Init();
    return temp;
}

template <typename T, typename A>
inline T *NewAndInit(A arg)
{
    T *temp = new (std::nothrow) T();
    LOGF_CHECK(temp != nullptr) << "NewAndInit failed";
    temp->Init(arg);
    return temp;
}

template <typename T>
inline void CheckAndFini(T *&module)
{
    if (module != nullptr) {
        module->Fini();
    }

    delete module;
    module = nullptr;
}

bool BaseRuntime::HasBeenInitialized()
{
    std::unique_lock<std::mutex> lock(vmCreationLock_);
    return initialized_;
}

void BaseRuntime::Init()
{
    Init(BaseRuntimeParam::DefaultRuntimeParam());
}

void BaseRuntime::Init(const RuntimeParam &param)
{
    CheckAndInitBaseRuntime(param);
}

void BaseRuntime::InitFromDynamic()
{
    InitFromDynamic(BaseRuntimeParam::DefaultRuntimeParam());
}

void BaseRuntime::InitFromDynamic(const RuntimeParam &param)
{
    std::unique_lock<std::mutex> lock(vmCreationLock_);
    if (initialized_) {
        LOG_COMMON(FATAL) << "BaseRuntime has been initialized and don't need init again.";
        return;
    }

    param_ = param;
    size_t pagePoolSize = param_.heapParam.heapSize;
#if defined(PANDA_TARGET_32)
    pagePoolSize = pagePoolSize / 128;  // 128 means divided.
#endif
    PagePool::Instance().Init(pagePoolSize * KB / COMMON_PAGE_SIZE);
    mutatorManager_ = NewAndInit<MutatorManager>();
    heapManager_ = NewAndInit<HeapManager>(param_);
    VLOG(INFO, "Arkcommon runtime started.");
    // Record runtime parameter to report. heap growth value needs to plus 1.
    VLOG(DEBUG,
         "Runtime parameter:\n\tHeap size: %zu(KB)\n\tRegion size: %zu(KB)\n\tExemption threshold: %.2f\n\t"
         "Heap utilization: %.2f\n\tHeap growth: %.2f\n\tAllocation rate: %.2f(MB/s)\n\tAlloction wait time: %zuns\n\t"
         "GC Threshold: %zu(KB)\n\tGarbage threshold: %.2f\n\tGC interval: %zums\n\tBackup GC interval: %zus\n\t"
         "Log level: %d\n\tThread stack size: %zu(KB)\n\tArkcommon stack size: %zu(KB)\n\t"
         "Processor number: %d",
         pagePoolSize, param_.heapParam.regionSize, param_.heapParam.exemptionThreshold,
         param_.heapParam.heapUtilization, 1 + param_.heapParam.heapGrowth, param_.heapParam.allocationRate,
         param_.heapParam.allocationWaitTime, param_.gcParam.gcThreshold / KB, param_.gcParam.garbageThreshold,
         param_.gcParam.gcInterval / MILLI_SECOND_TO_NANO_SECOND,
         param_.gcParam.backupGCInterval / SECOND_TO_NANO_SECOND);

    initialized_ = true;
}

void BaseRuntime::Fini()
{
    CheckAndFiniBaseRuntime();
}

void BaseRuntime::FiniFromDynamic()
{
    std::unique_lock<std::mutex> lock(vmCreationLock_);
    if (!initialized_) {
        LOG_COMMON(FATAL) << "BaseRuntime has been initialized and don't need init again.";
        return;
    }

    {
        // since there might be failure during initialization,
        // here we need to check and call fini.
        CheckAndFini<HeapManager>(heapManager_);
        CheckAndFini<MutatorManager>(mutatorManager_);
        PagePool::Instance().Fini();
    }

    VLOG(INFO, "Arkcommon runtime shutdown.");
    initialized_ = false;
}

void BaseRuntime::PreFork(ThreadHolder *holder)
{
    // Need appspawn space and compress gc.
    RequestGC(GC_REASON_APPSPAWN, false, GC_TYPE_FULL);
    {
        ThreadNativeScope scope(holder);
        HeapManager::StopRuntimeThreads();
    }
}

void BaseRuntime::PostFork([[maybe_unused]] bool enableWarmStartup)
{
    HeapManager::StartRuntimeThreads();
#ifdef ENABLE_COLD_STARTUP_GC_POLICY
    if (!enableWarmStartup) {
        StartupStatusManager::OnAppStartup();
    }
#endif
}

void BaseRuntime::NotifyWarmStart()
{
    if (!Heap::GetHeap().IsGcStarted() && !Heap::GetHeap().OnStartupEvent()) {
        StartupStatusManager::OnAppStartup();
    }
}

void BaseRuntime::WriteRoot(void *obj)
{
    Heap::GetBarrier().WriteRoot(reinterpret_cast<BaseObject *>(obj));
}

void BaseRuntime::WriteBarrier(void *obj, void *field, void *ref, Mutator *mutator)
{
    DCHECK_CC(field != nullptr);
    Heap::GetBarrier().WriteBarrier(mutator, reinterpret_cast<BaseObject *>(obj),
                                    *reinterpret_cast<RefField<> *>(field), reinterpret_cast<BaseObject *>(ref));
}

void *BaseRuntime::ReadBarrier(void *obj, void *field)
{
    return reinterpret_cast<void *>(Heap::GetBarrier().ReadRefField(reinterpret_cast<BaseObject *>(obj),
                                                                    *reinterpret_cast<RefField<false> *>(field)));
}

void *BaseRuntime::ReadBarrier(void *field)
{
    return reinterpret_cast<void *>(Heap::GetBarrier().ReadStaticRef(*reinterpret_cast<RefField<false> *>(field)));
}

void *BaseRuntime::AtomicReadBarrier(void *obj, void *field, std::memory_order order)
{
    return reinterpret_cast<void *>(Heap::GetBarrier().AtomicReadRefField(
        reinterpret_cast<BaseObject *>(obj), *reinterpret_cast<RefField<true> *>(field), order));
}

void BaseRuntime::RequestGC(GCReason reason, bool async, GCType gcType)
{
    if (reason < GC_REASON_BEGIN || reason > GC_REASON_END || gcType < GC_TYPE_BEGIN || gcType > GC_TYPE_END) {
        VLOG(ERROR, "Invalid gc reason or gc type, gc reason: %s, gc type: %s", GCReasonToString(reason),
             GCTypeToString(gcType));
        return;
    }
    HeapManager::RequestGC(reason, async, gcType);
}

void BaseRuntime::WaitForGCFinish()
{
    Heap::GetHeap().WaitForGCFinish();
}

void BaseRuntime::EnterGCCriticalSection()
{
    return Heap::GetHeap().MarkGCStart();
}
void BaseRuntime::ExitGCCriticalSection()
{
    return Heap::GetHeap().MarkGCFinish();
}

bool BaseRuntime::ForEachObj(HeapVisitor &visitor, bool safe)
{
    return Heap::GetHeap().ForEachObject(visitor, safe);
}

void BaseRuntime::NotifyNativeAllocation(size_t bytes)
{
    Heap::GetHeap().NotifyNativeAllocation(bytes);
}

void BaseRuntime::NotifyNativeFree(size_t bytes)
{
    Heap::GetHeap().NotifyNativeFree(bytes);
}

void BaseRuntime::NotifyNativeReset(size_t oldBytes, size_t newBytes)
{
    Heap::GetHeap().NotifyNativeReset(oldBytes, newBytes);
}

size_t BaseRuntime::GetNotifiedNativeSize()
{
    return Heap::GetHeap().GetNotifiedNativeSize();
}

void BaseRuntime::ChangeGCParams(bool isBackground)
{
    return Heap::GetHeap().ChangeGCParams(isBackground);
}

bool BaseRuntime::CheckAndTriggerHintGC(MemoryReduceDegree degree)
{
    return Heap::GetHeap().CheckAndTriggerHintGC(degree);
}

void BaseRuntime::NotifyHighSensitive(bool isStart)
{
    Heap::GetHeap().NotifyHighSensitive(isStart);
}

void BaseRuntime::FillFreeObject(void *object, size_t size)
{
    common::FillFreeObject(object, size);
}
}  // namespace common

