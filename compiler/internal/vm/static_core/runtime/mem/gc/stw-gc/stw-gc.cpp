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

#include "runtime/mem/gc/stw-gc/stw-gc.h"

#include "libarkbase/trace/trace.h"
#include "libarkbase/utils/logger.h"
#include "runtime/include/object_header-inl.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/gc/gc_root-inl.h"
#include "runtime/mem/gc/workers/gc_workers_task_pool.h"
#include "runtime/mem/object_helpers.h"
#include "runtime/mem/pygote_space_allocator-inl.h"
#include "runtime/mem/gc/static/gc_marker_static-inl.h"
#include "runtime/mem/gc/dynamic/gc_marker_dynamic-inl.h"
#include "runtime/mem/gc/gc_adaptive_stack_inl.h"

namespace ark::mem {

template <class LanguageConfig>
StwGC<LanguageConfig>::StwGC(ObjectAllocatorBase *objectAllocator, const GCSettings &settings)
    : GCLang<LanguageConfig>(objectAllocator, settings), marker_(this)
{
    this->SetType(GCType::STW_GC);
}

template <class LanguageConfig>
void StwGC<LanguageConfig>::InitializeImpl()
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    InternalAllocatorPtr allocator = this->GetInternalAllocator();
    auto barrierSet = allocator->New<GCDummyBarrierSet>(allocator);
    ASSERT(barrierSet != nullptr);
    this->SetGCBarrierSet(barrierSet);
    this->CreateWorkersTaskPool();
    LOG_DEBUG_GC << "STW GC Initialized";
}

template <class LanguageConfig>
bool StwGC<LanguageConfig>::CheckGCCause(GCTaskCause cause) const
{
    // Causes for generational GC are unsuitable for STW (non-generational) GC
    if (cause == GCTaskCause::YOUNG_GC_CAUSE || cause == GCTaskCause::MIXED) {
        return false;
    }
    return cause != GCTaskCause::CROSSREF_CAUSE && cause != GCTaskCause::INVALID_CAUSE;
}

template <class LanguageConfig>
void StwGC<LanguageConfig>::RunPhasesImpl(GCTask &task)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    auto memStats = this->GetPandaVm()->GetMemStats();
    GCScopedPauseStats scopedPauseStats(this->GetPandaVm()->GetGCStats(), this->GetStats());
    [[maybe_unused]] size_t bytesInHeapBeforeGc = memStats->GetFootprintHeap();
    marker_.BindBitmaps(true);
    Mark(task);
    Sweep();
    marker_.ReverseMark();
    [[maybe_unused]] size_t bytesInHeapAfterGc = memStats->GetFootprintHeap();
    ASSERT(bytesInHeapAfterGc <= bytesInHeapBeforeGc);
    task.collectionType = GCCollectionType::FULL;
}

template <class LanguageConfig>
void StwGC<LanguageConfig>::Mark(const GCTask &task)
{
    GCScope<TRACE_PHASE> gcScope(__FUNCTION__, this, GCPhase::GC_PHASE_MARK);

    // Iterate over roots and add other roots
    bool useGcWorkers = this->GetSettings()->ParallelMarkingEnabled();
    GCMarkingStackType objectsStack(this, useGcWorkers ? this->GetSettings()->GCRootMarkingStackMaxSize() : 0,
                                    useGcWorkers ? this->GetSettings()->GCWorkersMarkingStackMaxSize() : 0,
                                    GCWorkersTaskTypes::TASK_MARKING,
                                    this->GetSettings()->GCMarkingStackNewTasksFrequency());

    this->VisitRoots(
        [&objectsStack, &useGcWorkers, this](const GCRoot &gcRoot) {
            auto *object = gcRoot.GetObjectHeader();
            LOG_DEBUG_GC << "Handle root " << GetDebugInfoAboutObject(object);
            if (marker_.MarkIfNotMarked(object)) {
                objectsStack.PushToStack(gcRoot.GetType(), object);
            }
            if (!useGcWorkers) {
                MarkStack(&objectsStack);
            }
        },
        VisitGCRootFlags::ACCESS_ROOT_ALL);

    this->GetPandaVm()->VisitStringTable(
        [this, &objectsStack](GCRoot root) {
            auto str = root.GetObjectHeader();
            if (this->marker_.MarkIfNotMarked(str)) {
                ASSERT(str != nullptr);
                objectsStack.PushToStack(RootType::STRING_TABLE, str);
            }
        },
        VisitGCRootFlags::ACCESS_ROOT_ALL);
    MarkStack(&objectsStack);
    ASSERT(objectsStack.Empty());
    if (useGcWorkers) {
        this->GetWorkersTaskPool()->WaitUntilTasksEnd();
    }
    auto refClearPred = [this]([[maybe_unused]] const ObjectHeader *obj) { return this->InGCSweepRange(obj); };
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    this->GetPandaVm()->HandleReferences(task, refClearPred);
}

template <class LanguageConfig>
void StwGC<LanguageConfig>::MarkStack(GCMarkingStackType *stack)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    ASSERT(stack != nullptr);
    size_t objectsCount = 0;
    auto refPred = [this](const ObjectHeader *obj) { return this->InGCSweepRange(obj); };
    while (!stack->Empty()) {
        objectsCount++;
        auto *object = this->PopObjectFromStack(stack);
        ValidateObject(nullptr, object);
        auto *baseClass = object->template ClassAddr<BaseClass>();
        LOG_DEBUG_GC << "Current object: " << GetDebugInfoAboutObject(object);
        this->marker_.MarkInstance(stack, object, baseClass, refPred);
    }
    LOG_DEBUG_GC << "Iterated over " << objectsCount << " objects in the stack";
}

template <class LanguageConfig>
void StwGC<LanguageConfig>::Sweep()
{
    GCScope<TRACE_PHASE> gcScope(__FUNCTION__, this, GCPhase::GC_PHASE_SWEEP);
    if (!marker_.IsReverseMark()) {
        LOG_DEBUG_GC << "Sweep with MarkChecker";
        SweepImpl<DIRECT_MARK>();
    } else {
        LOG_DEBUG_GC << "Sweep with ReverseMarkChecker";
        SweepImpl<REVERSE_MARK>();
    }
}

template <class LanguageConfig>
template <typename StwGC<LanguageConfig>::MarkType MARK_TYPE>
void StwGC<LanguageConfig>::SweepImpl()
{
    this->GetPandaVm()->UpdateAndSweepVmRefs([this](ObjectHeader **ref) -> ObjectStatus {
        return this->marker_.template MarkChecker<static_cast<bool>(MARK_TYPE)>(*ref);
    });
    this->GetObjectAllocator()->Collect(
        [this](ObjectHeader *object) {
            auto status = this->marker_.template MarkChecker<static_cast<bool>(MARK_TYPE)>(object);
            if (status == ObjectStatus::DEAD_OBJECT) {
                LOG_DEBUG_OBJECT_EVENTS << "DELETE OBJECT: " << object;
            }
            return status;
        },
        GCCollectMode::GC_ALL);
    this->GetObjectAllocator()->VisitAndRemoveFreePools(
        [](void *mem, [[maybe_unused]] size_t size) { PoolManager::GetMmapMemPool()->FreePool(mem, size); });
}

template <class LanguageConfig>
void StwGC<LanguageConfig>::InitGCBits(ark::ObjectHeader *object)
{
    if (!marker_.IsReverseMark()) {
        object->SetUnMarkedForGC();
        ASSERT(!object->IsMarkedForGC());
    } else {
        object->SetMarkedForGC();
        ASSERT(object->IsMarkedForGC());
    }
    LOG_DEBUG_GC << "Init gc bits for object: " << std::hex << object << " bit: " << object->IsMarkedForGC()
                 << " reversed_mark: " << marker_.IsReverseMark();
}

template <class LanguageConfig>
void StwGC<LanguageConfig>::InitGCBitsForAllocationInTLAB([[maybe_unused]] ark::ObjectHeader *objHeader)
{
    LOG(FATAL, GC) << "TLABs are not supported by this GC";
}

template <class LanguageConfig>
bool StwGC<LanguageConfig>::Trigger(PandaUniquePtr<GCTask> task)
{
    return this->AddGCTask(true, std::move(task));
}

template <class LanguageConfig>
void StwGC<LanguageConfig>::WorkerTaskProcessing(GCWorkersTask *task, [[maybe_unused]] void *workerData)
{
    switch (task->GetType()) {
        case GCWorkersTaskTypes::TASK_MARKING: {
            auto *stack = task->Cast<GCMarkWorkersTask>()->GetMarkingStack();
            MarkStack(stack);
            this->GetInternalAllocator()->Delete(stack);
            break;
        }
        default:
            LOG(FATAL, GC) << "Unimplemented for " << GCWorkersTaskTypesToString(task->GetType());
            UNREACHABLE();
    }
}

template <class LanguageConfig>
bool StwGC<LanguageConfig>::IsPostponeGCSupported() const
{
    return true;
}

template <class LanguageConfig>
void StwGC<LanguageConfig>::MarkObject(ObjectHeader *object)
{
    marker_.Mark(object);
}

template <class LanguageConfig>
void StwGC<LanguageConfig>::UnMarkObject([[maybe_unused]] ObjectHeader *objectHeader)
{
    LOG(FATAL, GC) << "UnMarkObject for STW GC shouldn't be called";
}

template <class LanguageConfig>
void StwGC<LanguageConfig>::MarkReferences(GCMarkingStackType *references, [[maybe_unused]] GCPhase gcPhase)
{
    trace::ScopedTrace scopedTrace(__FUNCTION__);
    ASSERT(gcPhase == GCPhase::GC_PHASE_MARK);
    LOG_DEBUG_GC << "Start marking " << references->Size() << " references";
    MarkStack(references);
}

template <class LanguageConfig>
bool StwGC<LanguageConfig>::IsMarked(const ObjectHeader *object) const
{
    return marker_.IsMarked(object);
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(StwGC);
TEMPLATE_CLASS_LANGUAGE_CONFIG(StwGCMarker);

}  // namespace ark::mem
