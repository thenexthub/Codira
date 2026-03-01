/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "libarkbase/utils/logger.h"
#include "runtime/mark_word.h"
#include "runtime/include/runtime.h"
#include "runtime/include/panda_vm.h"
#include "runtime/mem/gc/cmc-gc-adapter/cmc-gc-adapter.h"
#ifdef ARK_HYBRID
#include "common_interfaces/base_runtime.h"
#endif

namespace ark::mem {
template <class LanguageConfig>
CMCGCAdapter<LanguageConfig>::CMCGCAdapter(ObjectAllocatorBase *objectAllocator, const GCSettings &settings)
    : GCLang<LanguageConfig>(objectAllocator, settings)
{
    this->SetType(GCType::CMC_GC);
}

template <class LanguageConfig>
void CMCGCAdapter<LanguageConfig>::InitializeImpl()
{
    InternalAllocatorPtr allocator = this->GetInternalAllocator();
    auto barrierSet = allocator->New<GCCMCBarrierSet>(allocator);
    ASSERT(barrierSet != nullptr);
    this->SetGCBarrierSet(barrierSet);
    LOG(DEBUG, GC) << "CMC GC adapter initialized...";
}

template <class LanguageConfig>
void CMCGCAdapter<LanguageConfig>::RunPhasesImpl([[maybe_unused]] GCTask &task)
{
    LOG(DEBUG, GC) << "CMC GC adapter RunPhases...";
    GCScopedPauseStats scopedPauseStats(this->GetPandaVm()->GetGCStats());
}

// NOLINTNEXTLINE(misc-unused-parameters)
template <class LanguageConfig>
bool CMCGCAdapter<LanguageConfig>::WaitForGC([[maybe_unused]] GCTask task)
{
#ifdef ARK_HYBRID
    common::GCReason reason = common::GCReason::GC_REASON_INVALID;
    common::GCType type = common::GCType::GC_TYPE_FULL;
    switch (task.reason) {
        case GCTaskCause::YOUNG_GC_CAUSE:
            reason = common::GCReason::GC_REASON_YOUNG;
            type = common::GCType::GC_TYPE_YOUNG;
            break;
        case GCTaskCause::PYGOTE_FORK_CAUSE:
            reason = common::GCReason::GC_REASON_APPSPAWN;
            break;
        case GCTaskCause::STARTUP_COMPLETE_CAUSE:
            reason = common::GCReason::GC_REASON_HINT;
            break;
        case GCTaskCause::NATIVE_ALLOC_CAUSE:
            reason = common::GCReason::GC_REASON_NATIVE;
            break;
        case GCTaskCause::HEAP_USAGE_THRESHOLD_CAUSE:
        case GCTaskCause::MIXED:
            reason = common::GCReason::GC_REASON_HEU;
            break;
        case GCTaskCause::EXPLICIT_CAUSE:
            reason = common::GCReason::GC_REASON_FORCE;
            break;
        case GCTaskCause::OOM_CAUSE:
            reason = common::GCReason::GC_REASON_OOM;
            break;
        case GCTaskCause::CROSSREF_CAUSE:
            reason = common::GCReason::GC_REASON_BACKUP;
            break;
        default:
            UNREACHABLE();
    }
    common::BaseRuntime::RequestGC(reason, false, type);
#endif
    return false;
}

template <class LanguageConfig>
void CMCGCAdapter<LanguageConfig>::InitGCBits([[maybe_unused]] ObjectHeader *objHeader)
{
}

template <class LanguageConfig>
void CMCGCAdapter<LanguageConfig>::InitGCBitsForAllocationInTLAB([[maybe_unused]] ObjectHeader *objHeader)
{
    LOG(FATAL, GC) << "TLABs are not supported by this GC";
}

template <class LanguageConfig>
bool CMCGCAdapter<LanguageConfig>::Trigger([[maybe_unused]] PandaUniquePtr<GCTask> task)
{
#ifdef ARK_HYBRID
    common::BaseRuntime::RequestGC(common::GCReason::GC_REASON_OOM, false, common::GCType::GC_TYPE_FULL);
#endif
    return false;
}

template <class LanguageConfig>
bool CMCGCAdapter<LanguageConfig>::IsPostponeGCSupported() const
{
    return true;
}

template <class LanguageConfig>
void CMCGCAdapter<LanguageConfig>::StopGC()
{
#ifdef ARK_HYBRID
    // Change to a more accurate function, when the function was provided (see #26240).
    common::BaseRuntime::WaitForGCFinish();
#endif
}

template <class LanguageConfig>
void CMCGCAdapter<LanguageConfig>::MarkObject([[maybe_unused]] ObjectHeader *object)
{
}

template <class LanguageConfig>
bool CMCGCAdapter<LanguageConfig>::IsMarked([[maybe_unused]] const ObjectHeader *object) const
{
    return false;
}

template <class LanguageConfig>
void CMCGCAdapter<LanguageConfig>::MarkReferences([[maybe_unused]] GCMarkingStackType *references,
                                                  [[maybe_unused]] GCPhase gcPhase)
{
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(CMCGCAdapter);

}  // namespace ark::mem
