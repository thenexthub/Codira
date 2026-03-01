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

#include <cctype>
#include "ets_coroutine.h"
#include "ets_handle.h"
#include "ets_handle_scope.h"
#include "intrinsics.h"
#include "intrinsics/helpers/reflection_helpers.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/types/ets_reflect_field.h"
#include "types/ets_class.h"
#include "types/ets_field.h"
#include "types/ets_object.h"
#include "types/ets_primitives.h"
#include "types/ets_type.h"

namespace ark::ets::intrinsics {

extern "C" EtsString *ReflectFieldGetNameInternal(EtsLong etsFieldPtr)
{
    return ManglingUtils::GetDisplayNameStringFromField(reinterpret_cast<EtsField *>(etsFieldPtr));
}

extern "C" EtsClass *ReflectFieldGetTypeInternal(EtsLong etsFieldPtr)
{
    // do not expose primitive types and string types except std.sore.String
    auto fieldType = reinterpret_cast<EtsField *>(etsFieldPtr)->GetType();
    if (fieldType == nullptr) {
        ASSERT(EtsCoroutine::GetCurrent()->HasPendingException());
        return nullptr;
    }

    return fieldType->ResolvePublicClass();
}

extern "C" void ReflectFieldSetValueInternal(ark::ObjectHeader *thisField, EtsObject *thisObj, EtsObject *arg)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    [[maybe_unused]] EtsHandleScope scope(coro);

    auto *reflectField = EtsReflectField::FromEtsObject(EtsObject::FromCoreType(thisField));
    ASSERT(reflectField != nullptr);
    auto *field = reflectField->GetEtsField();
    ASSERT(field != nullptr);
    EtsHandle thisObjH(coro, thisObj);
    EtsHandle argH(coro, arg);
    EtsClass *fieldType = field->GetType();
    if (fieldType == nullptr) {
        ASSERT(coro->HasPendingException());
        return;
    }
    bool isFieldPrimitive = fieldType->IsPrimitive();

    if (argH.GetPtr() == nullptr) {
        if (isFieldPrimitive) {
            ThrowEtsException(coro, panda_file_items::class_descriptors::NULL_POINTER_ERROR,
                              "undefined argument is not allowed for primitive reciever");
            return;
        }
        helpers::ReflectFieldSetReference(coro, thisObjH.GetPtr(), argH.GetPtr(), field);
        return;
    }

    EtsClass *argType = argH->GetClass();
    if (fieldType->IsAssignableFrom(argType)) {
        ASSERT(!fieldType->IsPrimitive());
        helpers::ReflectFieldSetReference(coro, thisObjH.GetPtr(), argH.GetPtr(), field);
    } else if (!(fieldType->IsPrimitive() && argType->IsBoxed() &&
                 helpers::ResolveAndSetPrimitive(
                     coro, {thisObjH.GetPtr(), argH.GetPtr(), argType, fieldType->GetEtsType(), field}))) {
        ThrowEtsException(coro, panda_file_items::class_descriptors::TYPE_ERROR,
                          "Value is not assignable to the provided field");
    }
}

extern "C" EtsObject *ReflectFieldGetValueInternal(ark::ObjectHeader *thisField, EtsObject *thisObj)
{
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    [[maybe_unused]] EtsHandleScope scope(coro);

    auto *reflectField = EtsReflectField::FromEtsObject(EtsObject::FromCoreType(thisField));
    ASSERT(reflectField != nullptr);
    auto *field = reflectField->GetEtsField();
    ASSERT(field != nullptr);

    EtsHandle thisObjH(coro, thisObj);

    if (!field->IsStatic() && !helpers::ValidateInstanceField(coro, thisObj, field)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }

    EtsClass *fieldType = field->GetType();
    if (fieldType == nullptr) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    if (fieldType->IsPrimitive()) {
        EtsType fieldEtsType = fieldType->GetEtsType();
        Value val = helpers::GetPrimitiveValue(coro, thisObjH.GetPtr(), fieldEtsType, field);
        return GetBoxedValue(coro, val, fieldEtsType);
    }

    return helpers::ReflectFieldGetReference(thisObjH.GetPtr(), field);
}

}  // namespace ark::ets::intrinsics
