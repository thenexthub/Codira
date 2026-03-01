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

#include "runtime/mem/heap_manager.h"
#include "runtime/mem/lock_config_helper.h"
#include "runtime/mem/internal_allocator-inl.h"

#include <string>

#include "libarkbase/mem/mmap_mem_pool-inl.h"
#include "libarkbase/mem/pool_manager.h"
#include "runtime/handle_base-inl.h"
#include "runtime/include/locks.h"
#include "runtime/include/runtime_notification.h"
#include "runtime/include/thread_scopes.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime.h"
#include "runtime/mem/gc/epsilon/epsilon.h"
#include "runtime/mem/gc/epsilon-g1/epsilon-g1.h"
#include "runtime/mem/gc/g1/g1-gc.h"
#include "runtime/mem/gc/stw-gc/stw-gc.h"
#include "runtime/mem/gc/cmc-gc-adapter/cmc-gc-adapter.h"

namespace ark::mem {

bool HeapManager::Initialize(GCType gcType, MTModeT multithreadingMode, bool useTlab, MemStatsType *memStats,
                             InternalAllocatorPtr internalAllocator, bool createPygoteSpace)
{
    trace::ScopedTrace scopedTrace("HeapManager::Initialize");
    bool ret = false;
    memStats_ = memStats;
    internalAllocator_ = internalAllocator;
    switch (gcType) {
        case GCType::EPSILON_GC: {
            ret = Initialize<GCType::EPSILON_GC>(memStats, multithreadingMode, createPygoteSpace);
            break;
        }
        case GCType::EPSILON_G1_GC: {
            ret = Initialize<GCType::EPSILON_G1_GC>(memStats, multithreadingMode, createPygoteSpace);
            break;
        }
        case GCType::STW_GC: {
            ret = Initialize<GCType::STW_GC>(memStats, multithreadingMode, createPygoteSpace);
            break;
        }
        case GCType::G1_GC: {
            ret = Initialize<GCType::G1_GC>(memStats, multithreadingMode, createPygoteSpace);
            break;
        }
        case GCType::CMC_GC: {
            ret = Initialize<GCType::CMC_GC>(memStats, multithreadingMode, createPygoteSpace);
            break;
        }
        default:
            LOG(FATAL, GC) << "Invalid init for gc_type = " << static_cast<int>(gcType);
            break;
    }
    // We want to use common allocate scenario in AOT/JIT/Irtoc code with option run-gc-every-safepoint
    // to check state of memory in TriggerGCIfNeeded
    if (!objectAllocator_.AsObjectAllocator()->IsTLABSupported() || Runtime::GetOptions().IsRunGcEverySafepoint()) {
        useTlab = false;
    }
    useTlabForAllocations_ = useTlab;
    if (useTlabForAllocations_ && Runtime::GetOptions().IsAdaptiveTlabSize()) {
        isAdaptiveTlabSize_ = true;
    }
    // Now, USE_TLAB_FOR_ALLOCATIONS option is supported only for Generational GCs
    ASSERT(IsGenerationalGCType(gcType) || (!useTlabForAllocations_));
    return ret;
}

void HeapManager::SetPandaVM(PandaVM *vm)
{
    vm_ = vm;
    gc_ = vm_->GetGC();
    notificationManager_ = Runtime::GetCurrent()->GetNotificationManager();
}

bool HeapManager::Finalize()
{
    delete codeAllocator_;
    codeAllocator_ = nullptr;
    objectAllocator_->VisitAndRemoveAllPools(
        [](void *mem, [[maybe_unused]] size_t size) { PoolManager::GetMmapMemPool()->FreePool(mem, size); });
    delete static_cast<Allocator *>(objectAllocator_);

    return true;
}

ObjectHeader *HeapManager::AllocateObject(BaseClass *cls, size_t size, Alignment align, ManagedThread *thread,
                                          ObjectAllocatorBase::ObjMemInitPolicy objInitType, bool pinned)
{
    ASSERT(size >= ObjectHeader::ObjectHeaderSize());
    ASSERT(GetGC()->IsMutatorAllowed());
    ASSERT(cls != nullptr);
    TriggerGCIfNeeded();
    if (thread == nullptr) {
        // NOTE(dtrubenkov): try to avoid this
        thread = ManagedThread::GetCurrent();
        ASSERT(thread != nullptr);
    }
    void *mem = AllocateMemoryForObject(size, align, thread, objInitType, pinned);
    if (UNLIKELY(mem == nullptr)) {
        mem = TryGCAndAlloc(size, align, thread, objInitType, pinned);
        if (UNLIKELY(mem == nullptr)) {
            ThrowOutOfMemoryError("AllocateObject failed");
            return nullptr;
        }
    }
    LOG(DEBUG, MM_OBJECT_EVENTS) << "Alloc object at " << mem << " size: " << size << " cls: " << cls;
    ObjectHeader *object = InitObjectHeaderAtMem(cls, mem);
    object = RegisterFinalizableIfNeeded(object, cls, size, thread);
    return object;
}

ObjectHeader *HeapManager::RegisterFinalizableIfNeeded(ObjectHeader *obj, BaseClass *cls, size_t size,
                                                       ManagedThread *thread)
{
    bool isObjectFinalizable = IsObjectFinalized(cls);
    if (UNLIKELY(isObjectFinalizable || GetNotificationManager()->HasAllocationListeners())) {
        // Use object handle here as RegisterFinalizedObject can trigger GC
        [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
        VMHandle<ObjectHeader> handle(thread, obj);
        RegisterFinalizedObject(handle.GetPtr(), cls, isObjectFinalizable);
        GetNotificationManager()->ObjectAllocEvent(cls, handle.GetPtr(), thread, size);
        obj = handle.GetPtr();
    }
    return obj;
}

void *HeapManager::TryGCAndAlloc(size_t size, Alignment align, ManagedThread *thread,
                                 ObjectAllocatorBase::ObjMemInitPolicy objInitType, bool pinned)
{
    // do not try many times in case of OOM scenarios.
    constexpr size_t ALLOC_RETRY = 4;
    size_t allocTryCnt = 0;
    void *mem = nullptr;
    bool isGenerational = GetGC()->IsGenerational();
    ASSERT(!thread->HasPendingException());

    while (mem == nullptr && allocTryCnt < ALLOC_RETRY) {
        ++allocTryCnt;
        GCTaskCause cause;
        // add comment why -1
        if (allocTryCnt == ALLOC_RETRY - 1 || !isGenerational) {
            cause = GCTaskCause::OOM_CAUSE;
        } else {
            cause = GCTaskCause::YOUNG_GC_CAUSE;
        }
        GetGC()->WaitForGCInManaged(GCTask(cause));
        mem = AllocateMemoryForObject(size, align, thread, objInitType, pinned);
        if (mem != nullptr) {
            // we could set OOM in gc, but we need to clear it if next gc was successfully and we allocated memory
            thread->ClearException();
        } else {
            auto freedBytes = GetPandaVM()->GetGCStats()->GetObjectsFreedBytes();
            auto lastYoungMovedBytes = GetPandaVM()->GetMemStats()->GetLastYoungObjectsMovedBytes();
            // if last GC freed or moved from young space some bytes - it means that we have a progress in VM,
            // just this thread was unlucky to get some memory. We reset alloc_try_cnt to try again.
            if (freedBytes + lastYoungMovedBytes != 0) {
                allocTryCnt = 0;
            }
        }
    }
    return mem;
}

void *HeapManager::AllocateMemoryForObject(size_t size, Alignment align, ManagedThread *thread,
                                           ObjectAllocatorBase::ObjMemInitPolicy objInitType, bool pinned)
{
    void *mem = nullptr;
    if (UseTLABForAllocations() && size <= GetTLABMaxAllocSize() && !pinned) {
        ASSERT(thread != nullptr);
        ASSERT(GetGC()->IsTLABsSupported());
        // Try to allocate an object via TLAB
        TLAB *currentTlab = thread->GetTLAB();
        ASSERT(currentTlab != nullptr);  // A thread's TLAB must be initialized at least via some ZERO tlab values.
        mem = currentTlab->Alloc(size);
        bool shouldAllocNewTlab =
            !isAdaptiveTlabSize_ || currentTlab->GetFillFraction() >= TLAB::MIN_DESIRED_FILL_FRACTION;
        if (mem == nullptr && shouldAllocNewTlab) {
            // We couldn't allocate an object via current TLAB,
            // Therefore, create a new one and allocate in it.
            if (CreateNewTLAB(thread)) {
                currentTlab = thread->GetTLAB();
                mem = currentTlab->Alloc(size);
            }
        }
        if (PANDA_TRACK_TLAB_ALLOCATIONS && (mem != nullptr)) {
            memStats_->RecordAllocateObject(GetAlignedObjectSize(size), SpaceType::SPACE_TYPE_OBJECT);
        }
    }
    if (mem == nullptr) {  // if mem == nullptr, try to use common allocate scenario
        mem = objectAllocator_.AsObjectAllocator()->Allocate(size, align, thread, objInitType, pinned);
    }
    return mem;
}

template <bool IS_FIRST_CLASS_CLASS>
ObjectHeader *HeapManager::AllocateNonMovableObject(BaseClass *cls, size_t size, Alignment align, ManagedThread *thread,
                                                    ObjectAllocatorBase::ObjMemInitPolicy objInitType)
{
    ASSERT(size >= ObjectHeader::ObjectHeaderSize());
    ASSERT(GetGC()->IsMutatorAllowed());
    TriggerGCIfNeeded();
    void *mem = objectAllocator_.AsObjectAllocator()->AllocateNonMovable(size, align, thread, objInitType);
    if (UNLIKELY(mem == nullptr)) {
        GCTaskCause cause = GCTaskCause::OOM_CAUSE;
        GetGC()->WaitForGCInManaged(GCTask(cause));
        mem = objectAllocator_.AsObjectAllocator()->AllocateNonMovable(size, align, thread, objInitType);
    }
    if (UNLIKELY(mem == nullptr)) {
        if (ManagedThread::GetCurrent() != nullptr) {
            ThrowOutOfMemoryError("AllocateNonMovableObject failed");
        }
        return nullptr;
    }
    LOG(DEBUG, MM_OBJECT_EVENTS) << "Alloc non-movable object at " << mem << " size: " << size << " cls: " << cls;
    auto *object = InitObjectHeaderAtMem(cls, mem);
    // cls can be null for first class creation, when we create ClassRoot::Class
    // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
    if constexpr (IS_FIRST_CLASS_CLASS) {
        ASSERT(cls == nullptr);
        // NOLINTNEXTLINE(readability-braces-around-statements, readability-misleading-indentation)
    } else {
        ASSERT(cls != nullptr);
        bool isObjectFinalizable = IsObjectFinalized(cls);
        if (UNLIKELY(isObjectFinalizable || GetNotificationManager()->HasAllocationListeners())) {
            if (thread == nullptr) {
                thread = ManagedThread::GetCurrent();
            }
            [[maybe_unused]] HandleScope<ObjectHeader *> scope(thread);
            VMHandle<ObjectHeader> handle(thread, object);
            RegisterFinalizedObject(handle.GetPtr(), cls, isObjectFinalizable);
            GetNotificationManager()->ObjectAllocEvent(cls, handle.GetPtr(), thread, size);
            object = handle.GetPtr();
        }
    }
    return object;
}

ObjectHeader *HeapManager::InitObjectHeaderAtMem(BaseClass *cls, void *mem)
{
    ASSERT(mem != nullptr);
    ASSERT(GetGC()->IsMutatorAllowed());

    auto object = static_cast<ObjectHeader *>(mem);
    // we need zeroed memory here according to ISA
    ASSERT(object->AtomicGetMark().GetValue() == 0);
    ASSERT(object->ClassAddr<BaseClass *>() == nullptr);
    // The order is crucial here - we need to have 0 class word to avoid data race with concurrent sweep.
    // Otherwise we can remove not initialized object.
    GetGC()->InitGCBits(object);
    object->SetClass(cls);
    return object;
}

void HeapManager::TriggerGCIfNeeded()
{
    vm_->GetGCTrigger()->TriggerGcIfNeeded(GetGC());
}

Frame *HeapManager::AllocateExtFrame(size_t size, size_t extSz)
{
    ASSERT(GetGC()->IsMutatorAllowed());
    StackFrameAllocator *frameAllocator = GetCurrentStackFrameAllocator();
    return Frame::FromExt(frameAllocator->Alloc(size), extSz);
}

bool HeapManager::CreateNewTLAB(ManagedThread *thread)
{
    ASSERT(GetGC()->IsMutatorAllowed());
    ASSERT(thread != nullptr);
    size_t newTlabSize = 0;
    TLAB *oldTlab = thread->GetTLAB();
    ASSERT(oldTlab != nullptr);
    if (!isAdaptiveTlabSize_) {
        // Using initial tlab size
        newTlabSize = Runtime::GetOptions().GetInitTlabSize();
    } else {
        thread->GetWeightedTlabAverage()->StoreNewSample(oldTlab->GetOccupiedSize(), oldTlab->GetSize());
        newTlabSize = AlignUp(thread->GetWeightedTlabAverage()->GetLastCountedSumInSizeT(), DEFAULT_ALIGNMENT_IN_BYTES);
    }
    LOG(DEBUG, ALLOC) << "Allocating new tlab with size: " << newTlabSize;
    ASSERT(newTlabSize != 0);
    TLAB *newTlab = objectAllocator_.AsObjectAllocator()->CreateNewTLAB(newTlabSize);
    if (newTlab != nullptr) {
        RegisterTLAB(thread->GetTLAB());
        thread->UpdateTLAB(newTlab);
        return true;
    }
    return false;
}

void HeapManager::RegisterTLAB(const TLAB *tlab)
{
    ASSERT(tlab != nullptr);
    if (!PANDA_TRACK_TLAB_ALLOCATIONS && (tlab->GetOccupiedSize() != 0)) {
        memStats_->RecordAllocateObject(tlab->GetOccupiedSize(), SpaceType::SPACE_TYPE_OBJECT);
    }
}

void HeapManager::FreeExtFrame(Frame *frame, size_t extSz)
{
    ASSERT(GetGC()->IsMutatorAllowed());
    StackFrameAllocator *frameAllocator = GetCurrentStackFrameAllocator();
    frameAllocator->Free(Frame::ToExt(frame, extSz));
}

CodeAllocator *HeapManager::GetCodeAllocator() const
{
    return codeAllocator_;
}

InternalAllocatorPtr HeapManager::GetInternalAllocator()
{
    return internalAllocator_;
}

ObjectAllocatorPtr HeapManager::GetObjectAllocator() const
{
    return objectAllocator_;
}

StackFrameAllocator *HeapManager::GetCurrentStackFrameAllocator()
{
    return ManagedThread::GetCurrent()->GetStackFrameAllocator();
}

void HeapManager::PreZygoteFork()
{
    GetGC()->WaitForGCOnPygoteFork(GCTask(GCTaskCause::PYGOTE_FORK_CAUSE));
}

float HeapManager::GetTargetHeapUtilization() const
{
    return targetUtilization_;
}

void HeapManager::SetTargetHeapUtilization(float target)
{
    ASSERT_PRINT(target > 0.0F && target < 1.0F, "Target heap utilization should be in the range (0,1)");
    targetUtilization_ = target;
}

size_t HeapManager::GetTotalMemory() const
{
    return GetObjectAllocator().AsObjectAllocator()->GetHeapSpace()->GetHeapSize();
}

size_t HeapManager::GetFreeMemory() const
{
    return helpers::UnsignedDifference(GetTotalMemory(), vm_->GetMemStats()->GetFootprintHeap());
}

void HeapManager::ClampNewMaxHeapSize()
{
    objectAllocator_.AsObjectAllocator()->GetHeapSpace()->ClampCurrentMaxHeapSize();
}

/**
 * @brief Check whether the given object is an instance of the given class.
 * @param obj - ObjectHeader pointer
 * @param h_class - Class pointer
 * @param assignable - whether the subclass of h_class counts
 * @return true if obj is instanceOf h_class, otherwise false
 */
static bool MatchesClass(const ObjectHeader *obj, const Class *hClass, bool assignable)
{
    if (assignable) {
        return obj->IsInstanceOf(hClass);
    }
    return obj->ClassAddr<Class>() == hClass;
}

void HeapManager::CountInstances(const PandaVector<Class *> &classes, bool assignable, uint64_t *counts)
{
    auto objectsChecker = [&](ObjectHeader *obj) {
        for (size_t i = 0; i < classes.size(); ++i) {
            if (classes[i] == nullptr) {
                continue;
            }
            if (MatchesClass(obj, classes[i], assignable)) {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                ++counts[i];
            }
        }
    };
    IterateOverObjectsWithSuspendAllThread(objectsChecker);
}

void HeapManager::IterateOverObjectsWithSuspendAllThread(const ObjectVisitor &objectVisitor)
{
    GetObjectAllocator().AsObjectAllocator()->IterateOverObjectsSafe(objectVisitor);
}

uint64_t HeapManager::CountInstancesOfClass(Class *klass)
{
    PandaVector<Class *> classes = {klass};
    uint64_t count = 0;
    CountInstances(classes, false, &count);
    return count;
}

void HeapManager::SetIsFinalizableFunc(IsObjectFinalizebleFunc func)
{
    isObjectFinalizebleFunc_ = func;
}

void HeapManager::SetRegisterFinalizeReferenceFunc(RegisterFinalizeReferenceFunc func)
{
    registerFinalizeReferenceFunc_ = func;
}

bool HeapManager::IsObjectFinalized(BaseClass *cls)
{
    return isObjectFinalizebleFunc_ != nullptr && isObjectFinalizebleFunc_(cls);
}

void HeapManager::RegisterFinalizedObject(ObjectHeader *object, BaseClass *cls, bool isObjectFinalizable)
{
    if (isObjectFinalizable) {
        ASSERT(registerFinalizeReferenceFunc_ != nullptr);
        registerFinalizeReferenceFunc_(object, cls);
    }
}

template ObjectHeader *HeapManager::AllocateNonMovableObject<true>(BaseClass *cls, size_t size, Alignment align,
                                                                   ManagedThread *thread,
                                                                   ObjectAllocatorBase::ObjMemInitPolicy obj_init_type);

template ObjectHeader *HeapManager::AllocateNonMovableObject<false>(
    BaseClass *cls, size_t size, Alignment align, ManagedThread *thread,
    ObjectAllocatorBase::ObjMemInitPolicy obj_init_type);
}  // namespace ark::mem
