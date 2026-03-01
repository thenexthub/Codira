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

#ifndef PANDA_PLUGINS_ETS_TYPEAPI_PARAMETER_H_
#define PANDA_PLUGINS_ETS_TYPEAPI_PARAMETER_H_

#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_runtime_linker.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_typeapi.h"
#include "plugins/ets/runtime/types/ets_typeapi_type.h"

namespace ark::ets {

namespace test {
class EtsTypeAPITest;
}  // namespace test

class EtsCoroutine;

class EtsTypeAPIParameter : public ObjectHeader {
public:
    EtsTypeAPIParameter() = delete;
    ~EtsTypeAPIParameter() = delete;

    NO_COPY_SEMANTIC(EtsTypeAPIParameter);
    NO_MOVE_SEMANTIC(EtsTypeAPIParameter);

    static EtsTypeAPIParameter *Create(EtsCoroutine *etsCoroutine = EtsCoroutine::GetCurrent());

    EtsObject *AsObject()
    {
        return EtsObject::FromCoreType(this);
    }

    const EtsObject *AsObject() const
    {
        return EtsObject::FromCoreType(this);
    }

    static EtsTypeAPIParameter *FromEtsObject(EtsObject *field)
    {
        return reinterpret_cast<EtsTypeAPIParameter *>(field);
    }

    void SetParameterType(EtsTypeAPIType *paramType)
    {
        ASSERT(paramType != nullptr);
        ObjectAccessor::SetObject(this, MEMBER_OFFSET(EtsTypeAPIParameter, paramType_),
                                  paramType->AsObject()->GetCoreType());
    }

    void SetName(EtsString *name)
    {
        ASSERT(name != nullptr);
        ObjectAccessor::SetObject(this, MEMBER_OFFSET(EtsTypeAPIParameter, name_), name->AsObject()->GetCoreType());
    }

    void SetAttributes(EtsInt attr)
    {
        ObjectAccessor::SetPrimitive(this, MEMBER_OFFSET(EtsTypeAPIParameter, attr_), attr);
    }

private:
    ObjectPointer<EtsTypeAPIType> paramType_;
    ObjectPointer<EtsString> name_;
    FIELD_UNUSED EtsInt attr_;

    friend class test::EtsTypeAPITest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_TYPEAPI_PARAMETER_H
