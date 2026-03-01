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
#ifndef PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_VALUE_H_
#define PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_VALUE_H_

#include "runtime/include/runtime.h"
#include "ets_type.h"
#include "plugins/ets/runtime/ani/ani.h"
#include "libarkbase/utils/utils.h"

namespace ark::ets {

class EtsObject;

template <typename T>
constexpr EtsType GetEtsType()
{
    if constexpr (std::is_same<T, EtsBoolean>::value) {  // NOLINT
        return EtsType::BOOLEAN;
    }
    if constexpr (std::is_same<T, EtsByte>::value) {  // NOLINT
        return EtsType::BYTE;
    }
    if constexpr (std::is_same<T, EtsChar>::value) {  // NOLINT
        return EtsType::CHAR;
    }
    if constexpr (std::is_same<T, EtsShort>::value) {  // NOLINT
        return EtsType::SHORT;
    }
    if constexpr (std::is_same<T, EtsInt>::value) {  // NOLINT
        return EtsType::INT;
    }
    if constexpr (std::is_same<T, EtsLong>::value) {  // NOLINT
        return EtsType::LONG;
    }
    if constexpr (std::is_same<T, EtsFloat>::value) {  // NOLINT
        return EtsType::FLOAT;
    }
    if constexpr (std::is_same<T, EtsDouble>::value) {  // NOLINT
        return EtsType::DOUBLE;
    }
    if constexpr (std::is_same<T, EtsObject>::value) {  // NOLINT
        return EtsType::OBJECT;
    }
    if constexpr (std::is_same<T, void>::value) {  // NOLINT
        return EtsType::VOID;
    }
    return EtsType::UNKNOWN;
}

class EtsValue {
public:
    EtsValue() = default;

    template <typename T>
    explicit EtsValue(T value)
    {
        static_assert(std::is_arithmetic_v<T> || std::is_same_v<T, EtsObject *> || std::is_same_v<T, ani_ref> ||
                      std::is_same_v<T, std::nullptr_t>);
        static_assert(sizeof(T) <= sizeof(holder_));

        MemcpyUnsafe(&holder_, &value, sizeof(value));
    }

    template <typename T>
    T GetAs()
    {
        static_assert(std::is_arithmetic_v<T> || std::is_same_v<T, EtsObject *> || std::is_same_v<T, ani_ref>);
        static_assert(sizeof(T) <= sizeof(holder_));

        T tmp;
        MemcpyUnsafe(&tmp, &holder_, sizeof(T));
        return tmp;
    }

private:
    int64_t holder_ {0};
};

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_FFI_CLASSES_ETS_VALUE_H_
