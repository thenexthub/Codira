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

#ifndef ETS_RUNTIME_INTERPRETER_ANY_OPCODES_H
#define ETS_RUNTIME_INTERPRETER_ANY_OPCODES_H

#include "plugins/ets/runtime/ets_stubs.h"

namespace ark::intrinsics {

ObjectHeader *ETSAnyLdbyname(ManagedThread *thread, ObjectHeader *thisObj, panda_file::File::StringData name)
{
    auto ldObj =
        ets::EtsLdbyname(ets::EtsCoroutine::CastFromThread(thread), ets::EtsObject::FromCoreType(thisObj), name)
            ->GetCoreType();
    return ldObj;
}

void ETSAnyStbyname(ManagedThread *thread, ObjectHeader *thisObj, panda_file::File::StringData name, ObjectHeader *val)
{
    ets::EtsStbyname(ets::EtsCoroutine::CastFromThread(thread), ets::EtsObject::FromCoreType(thisObj), name,
                     ets::EtsObject::FromCoreType(val));
}

// NOTE(nsizov): remove maybe_unused when implementations delivered
ObjectHeader *ETSAnyLdbyidx(ManagedThread *thread, ObjectHeader *thisObj, double key)
{
    auto idx = static_cast<uint32_t>(key);
    auto ldObj = ets::EtsLdbyidx(ets::EtsCoroutine::CastFromThread(thread), ets::EtsObject::FromCoreType(thisObj), idx)
                     ->GetCoreType();
    return ldObj;
}

// NOTE(nsizov): remove maybe_unused when implementations delivered
void ETSAnyStbyidx(ManagedThread *thread, ObjectHeader *thisObj, double key, ObjectHeader *val)
{
    auto idx = static_cast<uint32_t>(key);
    ets::EtsStbyidx(ets::EtsCoroutine::CastFromThread(thread), ets::EtsObject::FromCoreType(thisObj), idx,
                    ets::EtsObject::FromCoreType(val));
}

ObjectHeader *ETSAnyLdbyval(ManagedThread *thread, ObjectHeader *thisObj, ObjectHeader *key)
{
    auto ldObj = ets::EtsLdbyval(ets::EtsCoroutine::CastFromThread(thread), ets::EtsObject::FromCoreType(thisObj),
                                 ets::EtsObject::FromCoreType(key))
                     ->GetCoreType();
    return ldObj;
}

void ETSAnyStbyval(ManagedThread *thread, ObjectHeader *thisObj, ObjectHeader *key, ObjectHeader *val)
{
    if (key == nullptr) {
        interpreter::RuntimeInterface::ThrowNullPointerException();
        return;
    }
    ets::EtsStbyval(ets::EtsCoroutine::CastFromThread(thread), ets::EtsObject::FromCoreType(thisObj),
                    ets::EtsObject::FromCoreType(key), ets::EtsObject::FromCoreType(val));
}

bool ETSAnyIsinstance(ManagedThread *thread, ObjectHeader *lhs, ObjectHeader *rhs)
{
    if (rhs == nullptr) {
        interpreter::RuntimeInterface::ThrowNullPointerException();
        return false;
    }
    return ets::EtsIsinstance(ets::EtsCoroutine::CastFromThread(thread), ets::EtsObject::FromCoreType(lhs),
                              ets::EtsObject::FromCoreType(rhs));
}

ObjectHeader *ETSAnyCall(ManagedThread *thread, ObjectHeader *func, Span<VMHandle<ObjectHeader>> args)
{
    return ets::EtsCall(ets::EtsCoroutine::CastFromThread(thread), ets::EtsObject::FromCoreType(func), args)
        ->GetCoreType();
}

ObjectHeader *ETSAnyCallThis(ManagedThread *thread, ObjectHeader *thisObj, panda_file::File::StringData name,
                             Span<VMHandle<ObjectHeader>> args)
{
    return ets::EtsCallThis(ets::EtsCoroutine::CastFromThread(thread), ets::EtsObject::FromCoreType(thisObj), name,
                            args)
        ->GetCoreType();
}

ObjectHeader *ETSAnyCallNew(ManagedThread *thread, ObjectHeader *ctor, Span<VMHandle<ObjectHeader>> args)
{
    return ets::EtsCallNew(ets::EtsCoroutine::CastFromThread(thread), ets::EtsObject::FromCoreType(ctor), args)
        ->GetCoreType();
}

}  // namespace ark::intrinsics
#endif  // ETS_RUNTIME_INTERPRETER_ANY_OPCODES_H