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

#include "plugins/ets/runtime/types/ets_reflect_method.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/types/ets_typeapi.h"
#include "plugins/ets/runtime/ets_annotation.h"

namespace ark::ets {

/* static */
EtsReflectMethod *EtsReflectMethod::Create(EtsCoroutine *etsCoroutine, bool isStatic, bool isConstructor)
{
    EtsClass *klass = isConstructor ? PlatformTypes(etsCoroutine)->coreReflectConstructor
                                    : (isStatic ? PlatformTypes(etsCoroutine)->coreReflectStaticMethod
                                                : PlatformTypes(etsCoroutine)->coreReflectInstanceMethod);
    EtsObject *etsObject = EtsObject::Create(etsCoroutine, klass);
    return EtsReflectMethod::FromEtsObject(etsObject);
}

/* static */
EtsReflectMethod *EtsReflectMethod::CreateFromEtsMethod(EtsCoroutine *coro, EtsMethod *method)
{
    ASSERT(coro != nullptr);
    ASSERT(method != nullptr);

    bool isStatic = method->IsStatic();
    bool isConstructor = method->IsConstructor();

    [[maybe_unused]] EtsHandleScope scope(coro);

    auto reflectMethod = EtsHandle<EtsReflectMethod>(coro, EtsReflectMethod::Create(coro, isStatic, isConstructor));
    if (UNLIKELY(reflectMethod.GetPtr() == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }

    auto *ownerType = method->GetClass();
    reflectMethod->SetOwnerType(ownerType);
    reflectMethod->SetEtsMethod(reinterpret_cast<EtsLong>(method));

    // Set specific attributes
    uint32_t attr = 0;
    attr |= isStatic ? static_cast<uint32_t>(EtsTypeAPIAttributes::STATIC) : 0U;
    attr |= isConstructor ? static_cast<uint32_t>(EtsTypeAPIAttributes::CONSTRUCTOR) : 0U;
    attr |= (method->IsAbstract()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::ABSTRACT) : 0U;
    attr |= (method->IsGetter()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::GETTER) : 0U;
    attr |= (method->IsSetter()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::SETTER) : 0U;
    attr |= (method->IsFinal()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::FINAL) : 0U;
    panda_file::File::EntityId asyncAnnId = EtsAnnotation::FindAsyncAnnotation(method->GetPandaMethod());
    attr |= (method->IsNative() && !asyncAnnId.IsValid()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::NATIVE) : 0U;
    attr |= (asyncAnnId.IsValid()) ? static_cast<uint32_t>(EtsTypeAPIAttributes::ASYNC) : 0U;

    reflectMethod->SetAttributes(attr);

    int8_t accessMod = method->IsPublic()
                           ? static_cast<int8_t>(EtsTypeAPIAccessModifier::PUBLIC)
                           : (method->IsProtected() ? static_cast<int8_t>(EtsTypeAPIAccessModifier::PROTECTED)
                                                    : static_cast<int8_t>(EtsTypeAPIAccessModifier::PRIVATE));

    reflectMethod->SetAccessModifier(accessMod);

    return reflectMethod.GetPtr();
}

}  // namespace ark::ets
