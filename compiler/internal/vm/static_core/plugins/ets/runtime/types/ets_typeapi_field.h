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

#ifndef PANDA_PLUGINS_ETS_TYPEAPI_FIELD_H_
#define PANDA_PLUGINS_ETS_TYPEAPI_FIELD_H_

#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_typeapi.h"
#include "plugins/ets/runtime/types/ets_typeapi_type.h"

namespace ark::ets {

namespace test {
class EtsTypeAPITest;
}  // namespace test

class EtsCoroutine;

class EtsTypeAPIField : public ObjectHeader {
public:
    EtsTypeAPIField() = delete;
    ~EtsTypeAPIField() = delete;

    NO_COPY_SEMANTIC(EtsTypeAPIField);
    NO_MOVE_SEMANTIC(EtsTypeAPIField);

    static EtsTypeAPIField *Create(EtsCoroutine *etsCoroutine = EtsCoroutine::GetCurrent());

    EtsObject *AsObject()
    {
        return EtsObject::FromCoreType(this);
    }

    const EtsObject *AsObject() const
    {
        return EtsObject::FromCoreType(this);
    }

    static EtsTypeAPIField *FromEtsObject(EtsObject *field)
    {
        return reinterpret_cast<EtsTypeAPIField *>(field);
    }

    void SetFieldType(EtsTypeAPIType *fieldType)
    {
        ASSERT(fieldType != nullptr);
        ObjectAccessor::SetObject(this, MEMBER_OFFSET(EtsTypeAPIField, fieldType_),
                                  fieldType->AsObject()->GetCoreType());
    }

    void SetOwnerType(EtsTypeAPIType *ownerType)
    {
        ASSERT(ownerType != nullptr);
        ObjectAccessor::SetObject(this, MEMBER_OFFSET(EtsTypeAPIField, ownerType_),
                                  ownerType->AsObject()->GetCoreType());
    }

    void SetName(EtsString *name)
    {
        ObjectAccessor::SetObject(this, MEMBER_OFFSET(EtsTypeAPIField, name_), name->AsObject()->GetCoreType());
    }

    void SetAccessMod(EtsTypeAPIAccessModifier accessMod)
    {
        ObjectAccessor::SetPrimitive(this, MEMBER_OFFSET(EtsTypeAPIField, accessMod_), accessMod);
    }

    void SetAttributes(EtsInt attr)
    {
        ObjectAccessor::SetPrimitive(this, MEMBER_OFFSET(EtsTypeAPIField, attr_), attr);
    }

private:
    ObjectPointer<EtsTypeAPIType> fieldType_;
    ObjectPointer<EtsTypeAPIType> ownerType_;
    ObjectPointer<EtsString> name_;
    FIELD_UNUSED EtsInt attr_;  // note alignment
    FIELD_UNUSED EtsByte accessMod_;

    friend class test::EtsTypeAPITest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_TYPEAPI_FIELD_H_