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

#include "plugins/ets/runtime/static_type_converter.h"
#include "plugins/ets/runtime/ets_utils.h"
#include "plugins/ets/runtime/types/ets_type.h"
#include "plugins/ets/runtime/types/ets_bigint.h"
#include "plugins/ets/runtime/types/ets_box_primitive.h"
#include "plugins/ets/runtime/types/ets_box_primitive-inl.h"
#include "plugins/ets/runtime/types/ets_class.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "common_interfaces/objects/base_type.h"

namespace ark::ets {
StaticTypeConverter StaticTypeConverter::stcTypeConverter_;

static bool EtsToBoolean(EtsBoolean value)
{
    ASSERT(value == 0 || value == 1);
    return static_cast<bool>(value);
}

common::BoxedValue StaticTypeConverter::WrapBoxed(common::BaseType value)
{
    EtsObject *etsObject = nullptr;
    auto *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    [[maybe_unused]] HandleScope<ObjectHeader *> scope(coro);
    auto ptypes = PlatformTypes(coro);
    std::visit(
        [&](auto &&arg) -> void {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, std::monostate> || std::is_same_v<T, common::BaseUndefined>) {
                etsObject = nullptr;
            } else if constexpr (std::is_same_v<T, bool>) {
                etsObject = EtsBoxPrimitive<EtsBoolean>::Create(coro, arg);
            } else if constexpr (std::is_same_v<T, EtsLong>) {
                etsObject = EtsBoxPrimitive<EtsLong>::Create(coro, static_cast<EtsLong>(arg));
            } else if constexpr (std::is_same_v<T, EtsInt>) {
                etsObject = EtsBoxPrimitive<EtsInt>::Create(coro, static_cast<EtsInt>(arg));
            } else if constexpr (std::is_same_v<T, EtsShort>) {
                etsObject = EtsBoxPrimitive<EtsShort>::Create(coro, static_cast<EtsShort>(arg));
            } else if constexpr (std::is_same_v<T, EtsChar>) {
                etsObject = EtsBoxPrimitive<EtsChar>::Create(coro, static_cast<EtsChar>(arg));
            } else if constexpr (std::is_same_v<T, EtsFloat>) {
                etsObject = EtsBoxPrimitive<EtsFloat>::Create(coro, static_cast<EtsFloat>(arg));
            } else if constexpr (std::is_same_v<T, EtsDouble>) {
                etsObject = EtsBoxPrimitive<EtsDouble>::Create(coro, static_cast<EtsDouble>(arg));
            } else if constexpr (std::is_same_v<T, common::BaseNull>) {
                etsObject = EtsObject::FromCoreType(coro->GetNullValue());
            } else if constexpr (std::is_same_v<T, common::BaseBigInt>) {
                uint32_t len = arg.length;
                bool sign = arg.sign;
                auto etsIntArray = EtsIntArray::Create(len);
                for (uint32_t i = 0; i < len; i++) {
                    etsIntArray->Set(i, static_cast<int32_t>(arg.data[i]));
                }
                ASSERT(ptypes->coreBigInt != nullptr);
                VMHandle<EtsIntArray> arrHandle(coro, etsIntArray->GetCoreType());
                auto bigInt = EtsBigInt::FromEtsObject(EtsObject::Create(ptypes->coreBigInt));
                bigInt->SetFieldObject(EtsBigInt::GetBytesOffset(), reinterpret_cast<EtsObject *>(arrHandle.GetPtr()));
                bigInt->SetFieldPrimitive(EtsBigInt::GetSignOffset(), arg.data.empty() ? 0 : sign == 0 ? 1 : -1);
                etsObject = reinterpret_cast<EtsObject *>(bigInt);
            } else if constexpr (std::is_same_v<T, common::BaseObject *>) {
                etsObject = reinterpret_cast<EtsObject *>(arg);
            } else {
                UNREACHABLE();
            }
        },
        value);
    return reinterpret_cast<common::BoxedValue>(etsObject);
}

// NOLINTBEGIN(readability-else-after-return)
common::BaseType StaticTypeConverter::UnwrapBoxed(common::BoxedValue value)
{
    if (value == nullptr) {
        return common::BaseUndefined {};
    }

    auto *etsObj = reinterpret_cast<EtsObject *>(value);
    EtsCoroutine *coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);
    auto *cls = etsObj->GetClass();
    auto ptypes = PlatformTypes(coro);
    if (cls == ptypes->coreBoolean) {
        auto etsType = EtsBoxPrimitive<EtsBoolean>::Unbox(etsObj);
        return EtsToBoolean(etsType);
    } else if (cls == ptypes->coreDouble) {
        return EtsBoxPrimitive<EtsDouble>::Unbox(etsObj);
    } else if (cls == ptypes->coreInt) {
        return EtsBoxPrimitive<EtsInt>::Unbox(etsObj);
    } else if (cls == ptypes->coreByte) {
        return EtsBoxPrimitive<EtsByte>::Unbox(etsObj);
    } else if (cls == ptypes->coreShort) {
        return EtsBoxPrimitive<EtsShort>::Unbox(etsObj);
    } else if (cls == ptypes->coreLong) {
        return EtsBoxPrimitive<EtsLong>::Unbox(etsObj);
    } else if (cls == ptypes->coreFloat) {
        return EtsBoxPrimitive<EtsFloat>::Unbox(etsObj);
    } else if (cls == ptypes->coreChar) {
        return EtsBoxPrimitive<EtsChar>::Unbox(etsObj);
    } else if (cls == ptypes->coreBigInt) {
        common::BaseBigInt baseBigInt;
        auto bigInt = EtsBigInt::FromEtsObject(etsObj);
        auto etsValue = bigInt->GetBytes();
        auto etsValueSign = bigInt->GetSign();
        baseBigInt.length = etsValue->GetLength();
        baseBigInt.sign = etsValueSign < 0;
        for (uint32_t i = 0; i < baseBigInt.length; i++) {
            baseBigInt.data.emplace_back(static_cast<uint32_t>(etsValue->Get(i)));
        }
        return baseBigInt;
    } else if (etsObj->GetCoreType() == coro->GetNullValue()) {
        return common::BaseNull {};
    }
    return reinterpret_cast<common::BoxedValue>(etsObj);
}
// NOLINTEND(readability-else-after-return)
}  // namespace ark::ets
