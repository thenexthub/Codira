/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#ifndef PANDA_RUNTIME_MEM_GC_EPSILON_EPSILON_H
#define PANDA_RUNTIME_MEM_GC_EPSILON_EPSILON_H

#include "runtime/mem/gc/lang/gc_lang.h"

namespace ark::mem {

template <MTModeT MT_MODE>
class AllocConfig<GCType::EPSILON_GC, MT_MODE> {
public:
    using ObjectAllocatorType = ObjectAllocatorNoGen<MT_MODE>;
    using CodeAllocatorType = CodeAllocator;
};

template <class LanguageConfig>
class EpsilonGC final : public GCLang<LanguageConfig> {
public:
    explicit EpsilonGC(ObjectAllocatorBase *objectAllocator, const GCSettings &settings);

    NO_COPY_SEMANTIC(EpsilonGC);
    NO_MOVE_SEMANTIC(EpsilonGC);
    ~EpsilonGC() override = default;

    void RunPhases(GCTask &task);

    bool WaitForGC(GCTask task) override;

    bool IsPinningSupported() const final
    {
        // Epsilon GC does nothing, so objects are not moved
        return true;
    }

    void InitGCBits(ark::ObjectHeader *objHeader) override;

    void InitGCBitsForAllocationInTLAB(ark::ObjectHeader *objHeader) override;

    bool Trigger(PandaUniquePtr<GCTask> task) override;

    bool IsPostponeGCSupported() const override;

private:
    void MarkObject(ObjectHeader *object) override;
    bool IsMarked(const ObjectHeader *object) const override;
    void InitializeImpl() override;
    void RunPhasesImpl(GCTask &task) override;
    void MarkReferences(GCMarkingStackType *references, GCPhase gcPhase) override;
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_EPSILON_EPSILON_H
