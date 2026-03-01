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

// NOLINTBEGIN(readability-identifier-naming, cppcoreguidelines-macro-usage,
//             cppcoreguidelines-special-member-functions, modernize-deprecated-headers,
//             readability-else-after-return, readability-duplicate-include,
//             misc-non-private-member-variables-in-classes, cppcoreguidelines-pro-type-member-init,
//             google-explicit-constructor, cppcoreguidelines-pro-type-union-access,
//             modernize-use-auto, llvm-namespace-comment,
//             cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays,
//             readability-implicit-bool-conversion)

#ifndef COMMON_INTERFACES_OBJECTS_BASE_TYPE_H
#define COMMON_INTERFACES_OBJECTS_BASE_TYPE_H

#include <vector>
#include <variant>
#include <cstdint>
#include <memory>

#include "objects/base_object.h"
#include "objects/string/base_string.h"
namespace panda::ecmascript {
class JSTaggedValue;
}

using JSTaggedValue = panda::ecmascript::JSTaggedValue;
namespace common {

using TaggedType = uint64_t;
struct BaseUndefined {};
struct BaseNull {};
struct BaseBigInt {
    uint32_t length;
    bool sign;
    std::vector<uint32_t> data;
};

// The common consensus type between static and dynamic
using BaseType =
    std::variant<std::monostate, bool, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, float, double, int64_t,
                 uint64_t, BaseUndefined, BaseNull, BaseBigInt, BaseObject *, BaseString *>;

// base type for static vm
using BoxedValue = BaseObject *;

}  // namespace common
#endif  // COMMON_INTERFACES_OBJECTS_BASE_TYPE_H
// NOLINTEND(readability-identifier-naming, cppcoreguidelines-macro-usage,
//           cppcoreguidelines-special-member-functions, modernize-deprecated-headers,
//           readability-else-after-return, readability-duplicate-include,
//           misc-non-private-member-variables-in-classes, cppcoreguidelines-pro-type-member-init,
//           google-explicit-constructor, cppcoreguidelines-pro-type-union-access,
//           modernize-use-auto, llvm-namespace-comment,
//           cppcoreguidelines-pro-type-vararg, modernize-avoid-c-arrays,
//           readability-implicit-bool-conversion)
