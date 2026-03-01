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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_REFLECT_METHOD_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_REFLECT_METHOD_H

#include "libarkbase/mem/object_pointer.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_primitives.h"
#include "plugins/ets/runtime/types/ets_string.h"

namespace ark::ets {

namespace test {
class EtsReflectTest;
}  // namespace test

class EtsCoroutine;

class EtsReflectMethod : public ObjectHeader {
public:
    EtsReflectMethod() = delete;
    ~EtsReflectMethod() = delete;

    NO_COPY_SEMANTIC(EtsReflectMethod);
    NO_MOVE_SEMANTIC(EtsReflectMethod);

    static EtsReflectMethod *Create(EtsCoroutine *etsCoroutine, bool isStatic = false, bool isConstructor = false);

    static EtsReflectMethod *CreateFromEtsMethod(EtsCoroutine *coro, EtsMethod *method);

    EtsObject *AsObject()
    {
        return EtsObject::FromCoreType(this);
    }

    const EtsObject *AsObject() const
    {
        return EtsObject::FromCoreType(this);
    }

    static EtsReflectMethod *FromEtsObject(EtsObject *method)
    {
        return reinterpret_cast<EtsReflectMethod *>(method);
    }

    void SetName(EtsString *name)
    {
        ASSERT(name != nullptr);
        ObjectAccessor::SetObject(this, MEMBER_OFFSET(EtsReflectMethod, name_), name->AsObject()->GetCoreType());
    }

    void SetOwnerType(EtsClass *ownerType)
    {
        ASSERT(ownerType != nullptr);
        ObjectAccessor::SetObject(this, MEMBER_OFFSET(EtsReflectMethod, ownerType_),
                                  ownerType->AsObject()->GetCoreType());
    }

    void SetEtsMethod(EtsLong method)
    {
        [[maybe_unused]] auto *etsMethod = reinterpret_cast<EtsMethod *>(method);
        ASSERT(etsMethod != nullptr);
        ObjectAccessor::SetPrimitive(this, MEMBER_OFFSET(EtsReflectMethod, etsMethod_), method);
    }

    void SetAttributes(EtsInt attr)
    {
        ObjectAccessor::SetPrimitive(this, MEMBER_OFFSET(EtsReflectMethod, attr_), attr);
    }

    void SetAccessModifier(EtsByte accessMod)
    {
        ObjectAccessor::SetPrimitive(this, MEMBER_OFFSET(EtsReflectMethod, accessMod_), accessMod);
    }

    EtsMethod *GetEtsMethod()
    {
        return reinterpret_cast<EtsMethod *>(etsMethod_);
    }

private:
    ObjectPointer<EtsString> name_;
    ObjectPointer<EtsClass> ownerType_;
    EtsLong etsMethod_;
    EtsInt attr_;  // note alignment
    EtsByte accessMod_;

    friend class test::EtsReflectTest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_REFLECT_METHOD_H
