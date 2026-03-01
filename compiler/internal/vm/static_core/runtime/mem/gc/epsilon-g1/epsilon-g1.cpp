/**
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "runtime/mem/gc/g1/g1-helpers.h"
#include "runtime/mem/gc/epsilon-g1/epsilon-g1.h"
#include "runtime/include/panda_vm.h"
#include "runtime/include/runtime.h"

namespace ark::mem {
template <class LanguageConfig>
EpsilonG1GC<LanguageConfig>::EpsilonG1GC(ObjectAllocatorBase *objectAllocator, const GCSettings &settings)
    : G1GC<LanguageConfig>(objectAllocator, settings)
{
    this->SetType(GCType::EPSILON_G1_GC);
}

template <class LanguageConfig>
void EpsilonG1GC<LanguageConfig>::StartGC()
{
    // NOLINTNEXTLINE(bugprone-parent-virtual-call)
    GC::StartGC();
}

template <class LanguageConfig>
void EpsilonG1GC<LanguageConfig>::StopGC()
{
    // NOLINTNEXTLINE(bugprone-parent-virtual-call)
    GC::StopGC();
}

template <class LanguageConfig>
void EpsilonG1GC<LanguageConfig>::InitializeImpl()
{
    // GC saved the PandaVM instance, so we get allocator from the PandaVM.
    InternalAllocatorPtr allocator = this->GetInternalAllocator();
    this->CreateCardTable(allocator, PoolManager::GetMmapMemPool()->GetMinObjectAddress(),
                          PoolManager::GetMmapMemPool()->GetTotalObjectSize());

    auto barrierSet =
        allocator->New<GCG1BarrierSet>(allocator, &PreWrbFuncEntrypoint, &PostWrbUpdateCardFuncEntrypoint,
                                       ark::helpers::math::GetIntLog2(this->GetG1ObjectAllocator()->GetRegionSize()),
                                       this->GetCardTable(), this->updatedRefsQueue_, &this->queueLock_);
    ASSERT(barrierSet != nullptr);
    this->SetGCBarrierSet(barrierSet);

    LOG(DEBUG, GC) << "EpsilonG1 GC initialized...";
}

template <class LanguageConfig>
void EpsilonG1GC<LanguageConfig>::OnThreadTerminate(ManagedThread *thread,
                                                    [[maybe_unused]] mem::BuffersKeepingFlag keepBuffers)
{
    LOG(DEBUG, GC) << "Call OnThreadTerminate";
    // Clearing buffers to remove memory leaks in internal allocator

    auto preBuff = thread->MovePreBuff();
    ASSERT(preBuff != nullptr);
    this->GetInternalAllocator()->Delete(preBuff);

    auto *localBuffer = thread->GetG1PostBarrierBuffer();
    thread->ResetG1PostBarrierBuffer();
    ASSERT(localBuffer != nullptr);
    this->GetInternalAllocator()->Delete(localBuffer);
}

template <class LanguageConfig>
void EpsilonG1GC<LanguageConfig>::RunPhasesImpl([[maybe_unused]] GCTask &task)
{
    LOG(DEBUG, GC) << "EpsilonG1 GC RunPhases...";
    GCScopedPauseStats scopedPauseStats(this->GetPandaVm()->GetGCStats());
}

template <class LanguageConfig>
bool EpsilonG1GC<LanguageConfig>::WaitForGC([[maybe_unused]] GCTask task)
{
    return false;
}

template <class LanguageConfig>
void EpsilonG1GC<LanguageConfig>::InitGCBits([[maybe_unused]] ark::ObjectHeader *objHeader)
{
}

template <class LanguageConfig>
bool EpsilonG1GC<LanguageConfig>::Trigger([[maybe_unused]] PandaUniquePtr<GCTask> task)
{
    return false;
}

template <class LanguageConfig>
void EpsilonG1GC<LanguageConfig>::MarkReferences([[maybe_unused]] GCMarkingStackType *references,
                                                 [[maybe_unused]] GCPhase gcPhase)
{
}

TEMPLATE_CLASS_LANGUAGE_CONFIG(EpsilonG1GC);

}  // namespace ark::mem
