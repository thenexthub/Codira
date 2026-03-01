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
#ifndef PANDA_MEM_GC_REFERENCE_PROCESSOR_EMPTY_REFERENCE_PROCESSOR_H
#define PANDA_MEM_GC_REFERENCE_PROCESSOR_EMPTY_REFERENCE_PROCESSOR_H

#include "reference_processor.h"

namespace ark::mem {

class EmptyReferenceProcessor : public ark::mem::ReferenceProcessor {
public:
    EmptyReferenceProcessor() = default;
    ~EmptyReferenceProcessor() override = default;

    bool IsReference([[maybe_unused]] const BaseClass *baseCls, [[maybe_unused]] const ObjectHeader *ref,
                     [[maybe_unused]] const ReferenceCheckPredicateT &pred) const override
    {
        return false;
    }

    void HandleReference([[maybe_unused]] GC *gc, [[maybe_unused]] GCMarkingStackType *objectsStack,
                         [[maybe_unused]] const BaseClass *baseClass, [[maybe_unused]] const ObjectHeader *object,
                         [[maybe_unused]] const ReferenceProcessPredicateT &pred) override
    {
    }

    void ProcessReferences([[maybe_unused]] bool concurrent, [[maybe_unused]] bool clearSoftReferences,
                           [[maybe_unused]] GCPhase gcPhase,
                           [[maybe_unused]] const mem::GC::ReferenceClearPredicateT &pred) override
    {
    }

    void ProcessReferencesAfterCompaction([[maybe_unused]] const mem::GC::ReferenceClearPredicateT &pred) override {}

    ark::mem::Reference *CollectClearedReferences() override
    {
        return nullptr;
    }

    void ScheduleForEnqueue([[maybe_unused]] Reference *clearedReferences) override {}

    void Enqueue([[maybe_unused]] ark::mem::Reference *clearedReferences) override {}

    size_t GetReferenceQueueSize() const override
    {
        return 0;
    }

private:
    NO_COPY_SEMANTIC(EmptyReferenceProcessor);
    NO_MOVE_SEMANTIC(EmptyReferenceProcessor);
};

}  // namespace ark::mem
#endif  // PANDA_MEM_GC_REFERENCE_PROCESSOR_EMPTY_REFERENCE_PROCESSOR_H
