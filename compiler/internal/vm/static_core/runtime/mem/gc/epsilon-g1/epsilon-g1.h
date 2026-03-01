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
#ifndef PANDA_RUNTIME_MEM_GC_EPSILON_G1_EPSILON_G1_H
#define PANDA_RUNTIME_MEM_GC_EPSILON_G1_EPSILON_G1_H

#include "runtime/mem/gc/g1/g1-gc.h"

namespace ark::mem {

template <MTModeT MT_MODE>
class AllocConfig<GCType::EPSILON_G1_GC, MT_MODE> {
public:
    using ObjectAllocatorType = ObjectAllocatorG1<MT_MODE>;
    using CodeAllocatorType = CodeAllocator;
};

template <class LanguageConfig>
class EpsilonG1GC final : public G1GC<LanguageConfig> {
public:
    explicit EpsilonG1GC(ObjectAllocatorBase *objectAllocator, const GCSettings &settings);

    NO_COPY_SEMANTIC(EpsilonG1GC);
    NO_MOVE_SEMANTIC(EpsilonG1GC);
    ~EpsilonG1GC() override = default;

    void StartGC() override;
    void StopGC() override;

    void RunPhases(GCTask &task);
    bool WaitForGC(GCTask task) override;
    void InitGCBits(ark::ObjectHeader *objHeader) override;
    bool Trigger(PandaUniquePtr<GCTask> task) override;
    void OnThreadTerminate(ManagedThread *thread, mem::BuffersKeepingFlag keepBuffers) override;

private:
    void InitializeImpl() override;
    void RunPhasesImpl(GCTask &task) override;
    void MarkReferences(GCMarkingStackType *references, GCPhase gcPhase) override;
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_EPSILON_G1_EPSILON_G1_H