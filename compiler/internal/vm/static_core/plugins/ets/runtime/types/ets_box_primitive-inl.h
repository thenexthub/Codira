/**
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_BOX_PRIMITIVE_INL_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_BOX_PRIMITIVE_INL_H

#include "libarkbase/macros.h"
#include "plugins/ets/runtime/types/ets_box_primitive.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"

namespace ark::ets {
template <typename T>
EtsBoxPrimitive<T> *EtsBoxPrimitive<T>::Create(EtsCoroutine *coro, T value)
{
    auto *instance = reinterpret_cast<EtsBoxPrimitive<T> *>(ObjectHeader::Create(coro, GetBoxClass(coro)));
    ASSERT(instance != nullptr);
    instance->SetValue(value);
    return instance;
}

template <typename T>
EtsClass *EtsBoxPrimitive<T>::GetEtsBoxClass(EtsCoroutine *coro)
{
    auto ptypes = PlatformTypes(coro);
    EtsClass *boxClass = nullptr;

    if constexpr (std::is_same<T, EtsBoolean>::value) {
        boxClass = ptypes->coreBoolean;
    } else if constexpr (std::is_same<T, EtsByte>::value) {
        boxClass = ptypes->coreByte;
    } else if constexpr (std::is_same<T, EtsChar>::value) {
        boxClass = ptypes->coreChar;
    } else if constexpr (std::is_same<T, EtsShort>::value) {
        boxClass = ptypes->coreShort;
    } else if constexpr (std::is_same<T, EtsInt>::value) {
        boxClass = ptypes->coreInt;
    } else if constexpr (std::is_same<T, EtsLong>::value) {
        boxClass = ptypes->coreLong;
    } else if constexpr (std::is_same<T, EtsFloat>::value) {
        boxClass = ptypes->coreFloat;
    } else if constexpr (std::is_same<T, EtsDouble>::value) {
        boxClass = ptypes->coreDouble;
    }
    ASSERT(boxClass != nullptr);
    ASSERT(boxClass->IsBoxed());
    return boxClass;
}

template <typename T>
Class *EtsBoxPrimitive<T>::GetBoxClass(EtsCoroutine *coro)
{
    return GetEtsBoxClass(coro)->GetRuntimeClass();
}

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_BOX_PRIMITIVE_INL_H
