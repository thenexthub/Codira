/**
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PANDA_RUNTIME_MEM_FREE_OBJECT
#define PANDA_RUNTIME_MEM_FREE_OBJECT

#include "runtime/include/coretypes/tagged_value.h"
#include "runtime/include/object_header.h"
#include "runtime/include/object_accessor.h"
#include "libarkbase/macros.h"

namespace ark::mem {
class FreeObject : public ObjectHeader {
public:
    uint32_t GetSize() const
    {
        auto raw = ObjectAccessor::GetPrimitive<coretypes::TaggedType>(this, GetTaggedSizeOffset());
        return coretypes::TaggedValue::UnpackPrimitiveData(raw);
    }

    FreeObject *GetNext() const
    {
        auto raw = ObjectAccessor::GetPrimitive<coretypes::TaggedType>(this, GetTaggedNextOffset());
        return reinterpret_cast<FreeObject *>(static_cast<uintptr_t>(coretypes::TaggedValue::UnpackPrimitiveData(raw)));
    }

    static constexpr size_t GetTaggedNextOffset()
    {
        return MEMBER_OFFSET(FreeObject, taggedNext_);
    }

    static constexpr size_t GetTaggedSizeOffset()
    {
        return MEMBER_OFFSET(FreeObject, taggedSize_);
    }

private:
    // There should be primitive fields, but that's not friendly to concurrent marker
    coretypes::TaggedType taggedNext_ FIELD_UNUSED;
    coretypes::TaggedType taggedSize_ FIELD_UNUSED;
};
}  // namespace ark::mem

#endif  // PANDA_RUNTIME_MEM_FREE_OBJECT
