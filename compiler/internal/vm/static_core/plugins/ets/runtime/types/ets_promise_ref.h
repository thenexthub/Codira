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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_PROMISE_REF_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_PROMISE_REF_H

#include "plugins/ets/runtime/types/ets_object.h"

namespace ark::ets {

class EtsCoroutine;

namespace test {
class EtsPromiseTest;
}  // namespace test

class EtsPromiseRef : public EtsObject {
public:
    EtsPromiseRef() = delete;
    ~EtsPromiseRef() = delete;

    NO_COPY_SEMANTIC(EtsPromiseRef);
    NO_MOVE_SEMANTIC(EtsPromiseRef);

    PANDA_PUBLIC_API static EtsPromiseRef *Create(EtsCoroutine *etsCoroutine);

    EtsObject *AsObject()
    {
        return this;
    }

    const EtsObject *AsObject() const
    {
        return this;
    }

    EtsObject *GetTarget(EtsCoroutine *coro) const
    {
        auto *obj = ObjectAccessor::GetObject(coro, this, MEMBER_OFFSET(EtsPromiseRef, target_));
        return EtsObject::FromCoreType(obj);
    }

    void SetTarget(EtsCoroutine *coro, EtsObject *t)
    {
        ObjectAccessor::SetObject(coro, this, MEMBER_OFFSET(EtsPromiseRef, target_), t->GetCoreType());
    }

private:
    ObjectPointer<EtsObject> target_;

    friend class test::EtsPromiseTest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_PROMISE_REF_H
