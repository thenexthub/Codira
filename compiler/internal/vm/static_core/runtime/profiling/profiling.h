/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_PROFILING_H
#define PANDA_PROFILING_H

#include "libarkfile/bytecode_instruction.h"
#include <libarkfile/include/source_lang_enum.h>
#include "libarkbase/utils/expected.h"
#include "runtime/include/profiling_gen.h"
#include <string_view>
#include <iostream>

namespace ark::profiling {
enum class CallKind { UNKNOWN = 0, MONOMORPHIC, POLYMORPHIC, MEGAMORPHIC, COUNT };

inline const char *CallKindToString(CallKind callKind)
{
    static constexpr auto COUNT = static_cast<uint8_t>(CallKind::COUNT);
    static constexpr std::array<const char *, COUNT> CALL_KIND_NAMES = {"UNKNOWN", "MONOMORPHIC", "POLYMORPHIC",
                                                                        "MEGAMORPHIC"};
    auto idx = static_cast<uint8_t>(callKind);
    ASSERT(idx < COUNT);
    return CALL_KIND_NAMES[idx];
}

enum AnyInputType : uint8_t {
    DEFAULT = 0,
    INTEGER = 1,
    SPECIAL = 2,
    SPECIAL_INT = INTEGER | SPECIAL,
    LAST = SPECIAL_INT
};

using ProfileContainer = void *;
using ProfileType = void *;

// NOLINTNEXTLINE(misc-misplaced-const)
static constexpr ProfileType INVALID_PROFILE = nullptr;
static constexpr uint8_t MAX_FUNC_NUMBER = 4U;

}  // namespace ark::profiling

#endif  // PANDA_PROFILING_H
