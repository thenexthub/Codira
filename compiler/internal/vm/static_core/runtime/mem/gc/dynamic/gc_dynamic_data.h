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
#ifndef PANDA_RUNTIME_MEM_GC_GC_DYNAMIC_DATA_H
#define PANDA_RUNTIME_MEM_GC_GC_DYNAMIC_DATA_H

#include "runtime/include/mem/panda_containers.h"
#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/mem/gc/gc_extension_data.h"

namespace ark::mem {
class GCDynamicData : public GCExtensionData {
public:
    explicit GCDynamicData(InternalAllocatorPtr a) : allocator_(a)
    {
        dynWeakReferences_ = a->New<PandaStack<coretypes::TaggedType *>>(a->Adapter());

#ifndef NDEBUG
        SetLangType(LANG_TYPE_DYNAMIC);
#endif  // NDEBUG
    }

    ~GCDynamicData() override
    {
        allocator_->Delete(dynWeakReferences_);
    }

    PandaStack<coretypes::TaggedType *> *GetDynWeakReferences()
    {
        return dynWeakReferences_;
    }

private:
    PandaStack<coretypes::TaggedType *> *dynWeakReferences_;
    InternalAllocatorPtr allocator_;

    NO_COPY_SEMANTIC(GCDynamicData);
    NO_MOVE_SEMANTIC(GCDynamicData);
};

}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_GC_GC_DYNAMIC_DATA_H
