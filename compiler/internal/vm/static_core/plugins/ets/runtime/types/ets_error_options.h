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

#ifndef PANDA_PLUGINS_ETS_ERROR_OPTIONS_H
#define PANDA_PLUGINS_ETS_ERROR_OPTIONS_H

#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "types/ets_primitives.h"
#include "types/ets_typeapi.h"

namespace ark::ets {

class EtsCoroutine;

namespace test {
class EtsErrorOptionsTest;
}  // namespace test

class EtsErrorOptions : public ObjectHeader {
public:
    EtsErrorOptions() = delete;
    ~EtsErrorOptions() = delete;

    NO_COPY_SEMANTIC(EtsErrorOptions);
    NO_MOVE_SEMANTIC(EtsErrorOptions);

    EtsObject *AsObject()
    {
        return EtsObject::FromCoreType(this);
    }

    const EtsObject *AsObject() const
    {
        return EtsObject::FromCoreType(this);
    }

    static EtsErrorOptions *FromEtsObject(EtsObject *field)
    {
        return reinterpret_cast<EtsErrorOptions *>(field);
    }

    inline void SetCause(EtsObject *cause)
    {
        if (cause != nullptr) {
            ObjectAccessor::SetObject(this, MEMBER_OFFSET(EtsErrorOptions, cause_), cause->GetCoreType());
        } else {
            ObjectAccessor::SetObject(this, MEMBER_OFFSET(EtsErrorOptions, cause_), nullptr);
        }
    }

    inline static EtsErrorOptions *Create(EtsCoroutine *etsCoroutine)
    {
        EtsClass *klass = etsCoroutine->GetPandaVM()->GetClassLinker()->GetClass(
            panda_file_items::class_descriptors::ERROR_OPTIONS_IMPL.data());
        EtsObject *etsObject = EtsObject::Create(etsCoroutine, klass);
        return reinterpret_cast<EtsErrorOptions *>(etsObject);
    }

private:
    ObjectPointer<EtsObject> cause_;

    friend class test::EtsErrorOptionsTest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_ERROR_OPTIONS_H