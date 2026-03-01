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

#include "include/object_header.h"
#include "intrinsics.h"
#include "intrinsics/helpers/ets_intrinsics_helpers.h"
#include "libarkbase/utils/logger.h"
#include "runtime/handle_scope-inl.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "plugins/ets/runtime/types/ets_method.h"
#include "plugins/ets/runtime/ets_class_linker_extension.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/types/ets_map.h"
#include "plugins/ets/runtime/ets_stubs.h"

namespace ark::ets::intrinsics {

EtsBoolean StdCoreRuntimeIsLittleEndianPlatform()
{
    ASSERT(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ || __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__);
    return __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__;
}

uint8_t StdCoreRuntimeIsSameReference(EtsObject *source, EtsObject *target)
{
    return (source == target) ? UINT8_C(1) : UINT8_C(0);
}

EtsInt StdCoreRuntimeGetHashCode(EtsObject *source)
{
    ASSERT(source != nullptr);
    return bit_cast<EtsInt>(source->GetHashCode());
}

EtsLong StdRuntimeGetHashCodeByValue(EtsObject *source)
{
    return static_cast<EtsInt>(EtsEscompatMap::GetHashCode(source));
}

EtsBoolean StdRuntimeSameValueZero(EtsObject *a, EtsObject *b)
{
    return ark::ets::intrinsics::helpers::SameValueZero(EtsCoroutine::GetCurrent(), a, b);
}

static std::string GetClassName(EtsClass *cls)
{
    return cls->GetRuntimeClass()->GetName();
}

static ObjectHeader *CreateTypeCastException(EtsObject *source, EtsClass *target, bool inclUndefined)
{
    auto coro = EtsCoroutine::GetCurrent();
    ASSERT(coro != nullptr);

    auto message = PandaString(source == nullptr ? "undefined" : GetClassName(source->GetClass()));
    message.append(" cannot be cast to ");

    if (LIKELY(target != nullptr)) {
        message.append(GetClassName(target));
        if (inclUndefined) {
            message.append(" or undefined");
        }
    } else {
        message.append(inclUndefined ? "undefined" : "never");
    }

    auto exc =
        ets::SetupEtsException(coro, panda_file_items::class_descriptors::CLASS_CAST_ERROR.data(), message.data());
    if (UNLIKELY(exc == nullptr)) {
        ASSERT(coro->HasPendingException());
        return nullptr;
    }
    return exc->GetCoreType();
}

ObjectHeader *StdCoreRuntimeFailedTypeCastExclUndefinedStub(EtsObject *source, EtsClass *target)
{
    return CreateTypeCastException(source, target, false);
}

ObjectHeader *StdCoreRuntimeFailedTypeCastInclUndefinedStub(EtsObject *source, EtsClass *target)
{
    return CreateTypeCastException(source, target, true);
}

EtsClass *StdCoreRuntimeGetTypeInfo([[maybe_unused]] EtsObject *header)
{
    return nullptr;
}

ObjectHeader *StdCoreRuntimeAllocSameTypeArray(EtsClass *cls, int32_t length)
{
    if (UNLIKELY(!cls->IsArrayClass())) {
        // should not appear for the optimized version of intrinsic, which is always inlined
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::ERROR,
                          "class is not an array");
        return nullptr;
    }
    return coretypes::Array::Create(cls->GetRuntimeClass(), length);
}

}  // namespace ark::ets::intrinsics
