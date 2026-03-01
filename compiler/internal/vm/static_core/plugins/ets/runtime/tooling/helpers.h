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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HELPERS_H
#define PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HELPERS_H

#include <string_view>

#include "include/tooling/pt_thread.h"
#include "include/tooling/vreg_value.h"
#include "libarkbase/utils/bit_utils.h"
#include <libarkfile/include/type.h>
#include "types/ets_primitives.h"

namespace ark::ets::tooling {

template <typename T>
struct EtsTypeName {
};

// CC-OFFNXT(G.PRE.02) code generation
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define ETS_TYPE_NAME(TYPE_NAME)                             \
    template <>                                              \
    struct EtsTypeName<TYPE_NAME> {                          \
        static constexpr std::string_view NAME = #TYPE_NAME; \
    }

ETS_TYPE_NAME(EtsBoolean);
ETS_TYPE_NAME(EtsByte);
ETS_TYPE_NAME(EtsShort);
ETS_TYPE_NAME(EtsChar);
ETS_TYPE_NAME(EtsInt);
ETS_TYPE_NAME(EtsFloat);
ETS_TYPE_NAME(EtsDouble);
ETS_TYPE_NAME(EtsLong);

#undef ETS_TYPE_NAME

template <>
struct EtsTypeName<ObjectHeader *> {
    static constexpr std::string_view NAME = "EtsObject";
};

template <typename T, typename std::enable_if_t<std::is_same_v<EtsBoolean, T> || std::is_same_v<EtsByte, T> ||
                                                    std::is_same_v<EtsShort, T> || std::is_same_v<EtsChar, T> ||
                                                    std::is_same_v<EtsInt, T> || std::is_same_v<EtsLong, T>,
                                                int> = 0>
constexpr T VRegValueToEtsValue(ark::tooling::VRegValue value)
{
    return static_cast<T>(value.GetValue());
}

template <typename T, typename std::enable_if_t<std::is_same_v<EtsFloat, T>, int> = 0>
constexpr EtsFloat VRegValueToEtsValue(ark::tooling::VRegValue value)
{
    size_t bitsCount = sizeof(EtsFloat) * 8U;
    return bit_cast<EtsFloat>(
        static_cast<int32_t>(ExtractBits(static_cast<uint64_t>(value.GetValue()), 0U, bitsCount)));
}

template <typename T, typename std::enable_if_t<std::is_same_v<EtsDouble, T>, int> = 0>
constexpr EtsDouble VRegValueToEtsValue(ark::tooling::VRegValue value)
{
    return bit_cast<EtsDouble>(value.GetValue());
}

template <typename T, typename std::enable_if_t<std::is_same_v<ObjectHeader *, T>, int> = 0>
constexpr ObjectHeader *VRegValueToEtsValue(ark::tooling::VRegValue value)
{
    return reinterpret_cast<ObjectHeader *>(value.GetValue());
}

template <typename T, typename std::enable_if_t<std::is_same_v<EtsBoolean, T> || std::is_same_v<EtsByte, T> ||
                                                    std::is_same_v<EtsShort, T> || std::is_same_v<EtsChar, T> ||
                                                    std::is_same_v<EtsInt, T> || std::is_same_v<EtsLong, T>,
                                                int> = 0>
constexpr ark::tooling::VRegValue EtsValueToVRegValue(T value)
{
    return ark::tooling::VRegValue(value);
}

template <typename T, typename std::enable_if_t<std::is_same_v<EtsFloat, T>, int> = 0>
constexpr ark::tooling::VRegValue EtsValueToVRegValue(EtsFloat value)
{
    return ark::tooling::VRegValue(bit_cast<int32_t>(value));
}

template <typename T, typename std::enable_if_t<std::is_same_v<EtsDouble, T>, int> = 0>
constexpr ark::tooling::VRegValue EtsValueToVRegValue(EtsDouble value)
{
    return ark::tooling::VRegValue(bit_cast<int64_t>(value));
}

template <typename T, typename std::enable_if_t<std::is_same_v<ObjectHeader *, T>, int> = 0>
constexpr ark::tooling::VRegValue EtsValueToVRegValue(ObjectHeader *value)
{
    return ark::tooling::VRegValue(reinterpret_cast<int64_t>(value));
}

inline ark::tooling::PtThread CoroutineToPtThread(EtsCoroutine *coroutine)
{
    return ark::tooling::PtThread(coroutine);
}

}  // namespace ark::ets::tooling

#endif  // PANDA_PLUGINS_ETS_RUNTIME_TOOLING_HELPERS_H
