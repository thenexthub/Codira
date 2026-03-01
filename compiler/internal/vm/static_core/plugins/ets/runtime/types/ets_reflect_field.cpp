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

#include "plugins/ets/runtime/types/ets_reflect_field.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/types/ets_typeapi.h"

namespace ark::ets {

EtsReflectField *EtsReflectField::Create(EtsCoroutine *etsCoroutine, bool isStatic)
{
    EtsClass *klass = isStatic ? PlatformTypes(etsCoroutine)->coreReflectStaticField
                               : PlatformTypes(etsCoroutine)->coreReflectInstanceField;
    EtsObject *etsObject = EtsObject::Create(etsCoroutine, klass);
    return EtsReflectField::FromEtsObject(etsObject);
}

/* static */
EtsReflectField *EtsReflectField::CreateFromEtsField(EtsCoroutine *coro, EtsField *field)
{
    ASSERT(coro != nullptr);
    ASSERT(field != nullptr);

    [[maybe_unused]] EtsHandleScope scope(coro);

    auto reflectField = EtsHandle<EtsReflectField>(coro, EtsReflectField::Create(coro, field->IsStatic()));
    if (UNLIKELY(reflectField.GetPtr() == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }

    reflectField->SetEtsField(reinterpret_cast<EtsLong>(field));
    reflectField->SetOwnerType(field->GetDeclaringClass());

    uint32_t attr = 0;
    attr |= (field->IsStatic()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::STATIC) : 0U;
    attr |= (field->IsReadonly()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::READONLY) : 0U;

    reflectField->SetAttributes(attr);

    int8_t accessMod = field->IsPublic()
                           ? static_cast<int8_t>(EtsTypeAPIAccessModifier::PUBLIC)
                           : (field->IsProtected() ? static_cast<int8_t>(EtsTypeAPIAccessModifier::PROTECTED)
                                                   : static_cast<int8_t>(EtsTypeAPIAccessModifier::PRIVATE));
    reflectField->SetAccessModifier(accessMod);

    return reflectField.GetPtr();
}

}  // namespace ark::ets
