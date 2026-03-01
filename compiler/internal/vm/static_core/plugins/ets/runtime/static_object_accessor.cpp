/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include <array>
#include "libarkbase/utils/logger.h"
#include "plugins/ets/runtime/static_object_accessor.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_box_primitive-inl.h"
#include "common_interfaces/objects/base_object.h"

namespace ark::ets {

StaticObjectAccessor StaticObjectAccessor::stcObjAccessor_;

bool StaticObjectAccessor::HasProperty([[maybe_unused]] common::ThreadHolder *thread, const common::BaseObject *obj,
                                       const char *name)
{
    const EtsObject *etsObj = reinterpret_cast<const EtsObject *>(obj);  // NOLINT(modernize-use-auto)
    return etsObj->GetClass()->GetFieldIDByName(name) != nullptr;
}

common::BoxedValue StaticObjectAccessor::GetProperty([[maybe_unused]] common::ThreadHolder *thread,
                                                     const common::BaseObject *obj, const char *name)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    const EtsObject *etsObj = reinterpret_cast<const EtsObject *>(obj);  // NOLINT(modernize-use-auto)
    if (etsObj == nullptr) {
        return nullptr;
    }
    auto fieldIndex = etsObj->GetClass()->GetFieldIndexByName(name);
    EtsField *field = etsObj->GetClass()->GetFieldByIndex(fieldIndex);
    if (field == nullptr) {
        return nullptr;
    }
    return reinterpret_cast<common::BoxedValue>(GetPropertyValue(coro, etsObj, field));
}

bool StaticObjectAccessor::SetProperty([[maybe_unused]] common::ThreadHolder *thread, common::BaseObject *obj,
                                       const char *name, common::BoxedValue value)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    auto *etsObj = reinterpret_cast<EtsObject *>(obj);
    if (etsObj == nullptr) {
        return false;
    }

    auto fieldIndex = etsObj->GetClass()->GetFieldIndexByName(name);
    EtsField *field = etsObj->GetClass()->GetFieldByIndex(fieldIndex);
    if (field == nullptr) {
        return false;
    }

    return SetPropertyValue(coro, etsObj, field, reinterpret_cast<EtsObject *>(value));
}

bool StaticObjectAccessor::HasElementByIdx([[maybe_unused]] common::ThreadHolder *thread,
                                           [[maybe_unused]] const common::BaseObject *obj,
                                           [[maybe_unused]] const uint32_t index)
{
    LOG(ERROR, RUNTIME) << "HasElementByIdx has no meaning for static object";
    return false;
}

common::BoxedValue StaticObjectAccessor::GetElementByIdx([[maybe_unused]] common::ThreadHolder *thread,
                                                         const common::BaseObject *obj, const uint32_t index)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    const EtsObject *etsObj = reinterpret_cast<const EtsObject *>(obj);  // NOLINT(modernize-use-auto)
    if (etsObj == nullptr) {
        return nullptr;
    }
    EtsObject *etsObject = const_cast<EtsObject *>(etsObj);  // NOLINT(modernize-use-auto)
    EtsMethod *method = etsObj->GetClass()->GetDirectMethod(GET_INDEX_METHOD, "I:Lstd/core/Object;");
    std::array args {ark::Value(reinterpret_cast<ObjectHeader *>(etsObject)), ark::Value(index)};
    ark::Value value = method->GetPandaMethod()->Invoke(coro, args.data());
    return reinterpret_cast<common::BoxedValue>(EtsObject::FromCoreType(value.GetAs<ObjectHeader *>()));
}

bool StaticObjectAccessor::SetElementByIdx([[maybe_unused]] common::ThreadHolder *thread, common::BaseObject *obj,
                                           uint32_t index, const common::BoxedValue value)
{
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    auto *etsObj = reinterpret_cast<EtsObject *>(obj);
    if (etsObj == nullptr) {
        return false;
    }
    EtsMethod *method = etsObj->GetClass()->GetDirectMethod(SET_INDEX_METHOD, "ILstd/core/Object;:V");

    std::array args {ark::Value(reinterpret_cast<ObjectHeader *>(etsObj)), ark::Value(index),
                     ark::Value(reinterpret_cast<ObjectHeader *>(value))};
    method->GetPandaMethod()->Invoke(coro, args.data());
    if (UNLIKELY(coro->HasPendingException())) {
        return false;  // NOLINT(readability-simplify-boolean-expr)
    }
    return true;
}
}  // namespace ark::ets
