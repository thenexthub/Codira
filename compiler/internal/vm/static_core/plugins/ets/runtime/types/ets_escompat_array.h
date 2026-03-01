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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ESCOMPAT_ARRAY_H
#define PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ESCOMPAT_ARRAY_H

#include "libarkbase/mem/object_pointer.h"
#include "plugins/ets/runtime/ets_platform_types.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/types/ets_object.h"
#include "plugins/ets/runtime/types/ets_array.h"

namespace ark::ets {

namespace test {
class EtsEscompatArrayTest;
}  // namespace test

// Mirror class for Array<T> from ETS stdlib
class EtsEscompatArray : public EtsObject {
public:
    EtsEscompatArray() = delete;
    ~EtsEscompatArray() = delete;

    NO_COPY_SEMANTIC(EtsEscompatArray);
    NO_MOVE_SEMANTIC(EtsEscompatArray);

    EtsObject *AsObject()
    {
        return this;
    }

    const EtsObject *AsObject() const
    {
        return this;
    }

    static EtsEscompatArray *FromEtsObject(EtsObject *etsObj)
    {
        return reinterpret_cast<EtsEscompatArray *>(etsObj);
    }

    static constexpr size_t GetBufferOffset()
    {
        return MEMBER_OFFSET(EtsEscompatArray, buffer_);
    }

    static constexpr size_t GetActualLengthOffset()
    {
        return MEMBER_OFFSET(EtsEscompatArray, actualLength_);
    }

    /// @brief Creates instance of `std.core.Array` with given length
    static EtsEscompatArray *Create(EtsCoroutine *coro, size_t length);

    /// @brief Checks whether the object's real type is `std.core.Array`
    static bool IsExactlyEscompatArray(const EtsObject *obj, EtsCoroutine *coro)
    {
        return obj->GetClass() == PlatformTypes(coro)->escompatArray;  // NOLINT (readability-implicit-bool-conversion)
    }

    /// @brief Checks whether the object's real type is `std.core.Array`
    bool IsExactlyEscompatArray(EtsCoroutine *coro) const
    {
        return EtsEscompatArray::IsExactlyEscompatArray(this, coro);
    }

    /// @brief Implementation of `std.core.Array` method `$_get`
    EtsObject *EscompatArrayGet(EtsInt index);

    /// @brief Implementation of `std.core.Array` method `$_set`
    void EscompatArraySet(EtsInt index, EtsObject *value);

    /// @brief Implementation of `std.core.Array` method `$_set_unsafe`
    EtsObject *EscompatArrayGetUnsafe(EtsInt index);

    /// @brief Implementation of `std.core.Array` method `$_get_unsafe`
    void EscompatArraySetUnsafe(EtsInt index, EtsObject *value);

    /**
     * @brief Implementation of `std.core.Array` method `pop`
     * Note that this method is valid only when `values` array is exactly `std.core.Array`.
     * Implementation of this method must be aligned with managed implementation of `pop` method
     */
    EtsObject *EscompatArrayPop();

    /// @brief Returns underlying buffer, object must be exactly `std.core.Array`
    EtsObjectArray *GetDataFromEscompatArray()
    {
        ASSERT(IsExactlyEscompatArray(EtsCoroutine::GetCurrent()));
        return GetDataFromEscompatArrayImpl();
    }

    /// @brief Returns `actualLength` of array, object must be exactly `std.core.Array`
    EtsInt GetActualLengthFromEscompatArray()
    {
        ASSERT(IsExactlyEscompatArray(EtsCoroutine::GetCurrent()));
        return GetActualLengthFromEscompatArrayImpl();
    }

    /**
     * @brief Returns length of array according to possible inheritance of underlying array
     * Note that this method is potentially throwing
     */
    bool GetLength(EtsCoroutine *coro, EtsInt *result);

    /**
     * @brief Sets value at index according to possible inheritance of underlying array
     * Note that this method is potentially throwing
     */
    bool SetRef(EtsCoroutine *coro, size_t index, EtsObject *ref);

    /**
     * @brief Gets value at index according to possible inheritance of underlying array
     * Note that this method is potentially throwing
     */
    std::optional<EtsObject *> GetRef(EtsCoroutine *coro, size_t index);

    /**
     * @brief Pop from array according to possible inheritance of underlying array
     * Note that this method is potentially throwing
     */
    EtsObject *Pop(EtsCoroutine *coro);

private:
    EtsObjectArray *GetDataFromEscompatArrayImpl()
    {
        auto *buff = ObjectAccessor::GetObject(this, GetBufferOffset());
        return EtsObjectArray::FromCoreType(buff);
    }

    EtsInt GetActualLengthFromEscompatArrayImpl()
    {
        return ObjectAccessor::GetFieldPrimitive<EtsInt>(this, GetActualLengthOffset(), std::memory_order_relaxed);
    }

    void SetActualLengthFromEscompatArrayImpl(EtsInt value)
    {
        ObjectAccessor::SetFieldPrimitive<EtsInt>(this, GetActualLengthOffset(), value, std::memory_order_relaxed);
    }

private:
    ObjectPointer<EtsObjectArray> buffer_;
    EtsInt actualLength_;

    friend class test::EtsEscompatArrayTest;
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TYPES_ETS_ESCOMPAT_ARRAY_H
