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
#ifndef PANDA_RUNTIME_MEM_GC_CMCGCADAPTER_CMCGCADAPTER_H
#define PANDA_RUNTIME_MEM_GC_CMCGCADAPTER_CMCGCADAPTER_H

#include "runtime/mem/gc/lang/gc_lang.h"
#include "runtime/mem/gc/cmc-gc-adapter/cmc-allocator-adapter.h"

namespace ark::mem {

template <MTModeT MT_MODE>
class AllocConfig<GCType::CMC_GC, MT_MODE> {
public:
    using ObjectAllocatorType = CMCObjectAllocatorAdapter<MT_MODE>;
    using CodeAllocatorType = CodeAllocator;
};

template <class LanguageConfig>
class CMCGCAdapter final : public GCLang<LanguageConfig> {
public:
    explicit CMCGCAdapter(ObjectAllocatorBase *objectAllocator, const GCSettings &settings);

    NO_COPY_SEMANTIC(CMCGCAdapter);
    NO_MOVE_SEMANTIC(CMCGCAdapter);
    ~CMCGCAdapter() override = default;

    void RunPhases(GCTask &task);

    bool WaitForGC(GCTask task) override;

    bool IsPinningSupported() const final
    {
        return false;
    }

    void InitGCBits(ark::ObjectHeader *objHeader) override;

    void InitGCBitsForAllocationInTLAB(ark::ObjectHeader *objHeader) override;

    bool Trigger(PandaUniquePtr<GCTask> task) override;

    bool IsPostponeGCSupported() const override;

    void StopGC() override;

private:
    void MarkObject(ObjectHeader *object) override;
    bool IsMarked(const ObjectHeader *object) const override;
    void InitializeImpl() override;
    void RunPhasesImpl(GCTask &task) override;
    void MarkReferences(GCMarkingStackType *references, GCPhase gcPhase) override;
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_CMCGCADAPTER_CMCGCADAPTER_H
