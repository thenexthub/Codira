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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ATOMIC_FLAG_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ATOMIC_FLAG_H

#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_primitives.h"

namespace ark::ets {

namespace test {
class EtsAtomicFlagTest;
}  // namespace test

class EtsAtomicFlag : private ObjectHeader {
public:
    EtsAtomicFlag() = delete;
    ~EtsAtomicFlag() = delete;

    NO_COPY_SEMANTIC(EtsAtomicFlag);
    NO_MOVE_SEMANTIC(EtsAtomicFlag);

    static EtsAtomicFlag *FromCoreType(ObjectHeader *flag)
    {
        return reinterpret_cast<EtsAtomicFlag *>(flag);
    }

    static const EtsAtomicFlag *FromCoreType(const ObjectHeader *flag)
    {
        return reinterpret_cast<const EtsAtomicFlag *>(flag);
    }

    ObjectHeader *GetCoreType()
    {
        return reinterpret_cast<ObjectHeader *>(this);
    }

    const ObjectHeader *GetCoreType() const
    {
        return reinterpret_cast<const ObjectHeader *>(this);
    }

    EtsObject *AsObject()
    {
        return EtsObject::FromCoreType(this);
    }

    const EtsObject *AsObject() const
    {
        return EtsObject::FromCoreType(this);
    }

    static EtsAtomicFlag *FromEtsObject(EtsObject *flag)
    {
        return reinterpret_cast<EtsAtomicFlag *>(flag);
    }

    static const EtsAtomicFlag *FromEtsObject(const EtsObject *flag)
    {
        return reinterpret_cast<const EtsAtomicFlag *>(flag);
    }

    void SetValue(EtsBoolean v)
    {
        // GCC does not support maybe_ununused attribute on class members
        UNUSED_VAR(flag_);
        ObjectAccessor::SetPrimitive<EtsBoolean, true>(this, MEMBER_OFFSET(EtsAtomicFlag, flag_), v);
    }

    EtsBoolean GetValue() const
    {
        // GCC does not support maybe_ununused attribute on class members
        UNUSED_VAR(flag_);
        return ObjectAccessor::GetPrimitive<EtsBoolean, true>(this, MEMBER_OFFSET(EtsAtomicFlag, flag_));
    }

private:
    EtsBoolean flag_;

    friend class test::EtsAtomicFlagTest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ATOMIC_FLAG_H
