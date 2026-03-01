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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_REFLECT_FIELD_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_REFLECT_FIELD_H

#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_primitives.h"

namespace ark::ets {

namespace test {
class EtsReflectTest;
}  // namespace test

class EtsCoroutine;

class EtsReflectField : public ObjectHeader {
public:
    EtsReflectField() = delete;
    ~EtsReflectField() = delete;

    NO_COPY_SEMANTIC(EtsReflectField);
    NO_MOVE_SEMANTIC(EtsReflectField);

    static EtsReflectField *Create(EtsCoroutine *etsCoroutine, bool isStatic = false);

    static EtsReflectField *CreateFromEtsField(EtsCoroutine *coro, EtsField *field);

    EtsObject *AsObject()
    {
        return EtsObject::FromCoreType(this);
    }

    const EtsObject *AsObject() const
    {
        return EtsObject::FromCoreType(this);
    }

    static EtsReflectField *FromEtsObject(EtsObject *field)
    {
        return reinterpret_cast<EtsReflectField *>(field);
    }

    void SetOwnerType(EtsClass *ownerType)
    {
        ASSERT(ownerType != nullptr);
        ObjectAccessor::SetObject(this, MEMBER_OFFSET(EtsReflectField, ownerType_),
                                  ownerType->AsObject()->GetCoreType());
    }

    void SetEtsField(EtsLong field)
    {
        [[maybe_unused]] auto *etsField = reinterpret_cast<EtsField *>(field);
        ASSERT(etsField != nullptr);
        ObjectAccessor::SetPrimitive(this, MEMBER_OFFSET(EtsReflectField, etsField_), field);
    }

    void SetName(EtsString *name)
    {
        ObjectAccessor::SetObject(this, MEMBER_OFFSET(EtsReflectField, name_), name->AsObject()->GetCoreType());
    }

    void SetAttributes(EtsInt attr)
    {
        ObjectAccessor::SetPrimitive(this, MEMBER_OFFSET(EtsReflectField, attr_), attr);
    }

    void SetAccessModifier(EtsByte accessMod)
    {
        ObjectAccessor::SetPrimitive(this, MEMBER_OFFSET(EtsReflectField, accessMod_), accessMod);
    }

    EtsField *GetEtsField()
    {
        return reinterpret_cast<EtsField *>(etsField_);
    }

private:
    ObjectPointer<EtsString> name_;
    ObjectPointer<EtsClass> ownerType_;
    EtsLong etsField_;
    EtsInt attr_;
    EtsByte accessMod_;

    friend class test::EtsReflectTest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_REFLECT_FIELD_H
