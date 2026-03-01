/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_BOX_CLASSES_H_
#define PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_BOX_CLASSES_H_

#include "libarkbase/macros.h"
#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets {

template <typename T>
class EtsBoxPrimitive : public EtsObject {
public:
    EtsBoxPrimitive() = delete;
    ~EtsBoxPrimitive() = delete;

    static EtsBoxPrimitive *Create(EtsCoroutine *coro, T value);

    static Class *GetBoxClass(EtsCoroutine *coro);

    static EtsClass *GetEtsBoxClass(EtsCoroutine *coro);

    static T Unbox(EtsObject *obj)
    {
        auto val = EtsBoxPrimitive<T>::FromCoreType(obj);
        ASSERT(val != nullptr);
        return val->GetValue();
    }

    static constexpr EtsBoxPrimitive *FromCoreType(EtsObject *obj)
    {
        ASSERT(obj->GetClass()->IsBoxed());
        return static_cast<EtsBoxPrimitive<T> *>(obj);
    }

    T GetValue() const
    {
        return value_;
    }

    void SetValue(T value)
    {
        value_ = value;
    }

    const EtsObject *AsObject() const
    {
        return reinterpret_cast<const EtsObject *>(this);
    }

    EtsObject *AsObject()
    {
        return reinterpret_cast<EtsObject *>(this);
    }

    NO_COPY_SEMANTIC(EtsBoxPrimitive);
    NO_MOVE_SEMANTIC(EtsBoxPrimitive);

    static constexpr size_t GetValueOffset()
    {
        return MEMBER_OFFSET(EtsBoxPrimitive, value_);
    }

private:
    T value_;
};
}  // namespace ark::ets

#endif  // PANDA_RUNTIME_ETS_FFI_CLASSES_ETS_BOX_CLASSES_H_
