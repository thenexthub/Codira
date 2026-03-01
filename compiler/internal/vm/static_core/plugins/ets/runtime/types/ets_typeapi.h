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

#ifndef PANDA_PLUGINS_ETS_TYPEAPI_H_
#define PANDA_PLUGINS_ETS_TYPEAPI_H_

#include <string>
#include <libarkfile/include/type.h>
#include "types/ets_primitives.h"
#include "libarkfile/file.h"
#include "types/ets_string.h"

namespace ark::ets {

enum class EtsTypeAPIKind : EtsByte {
    NONE = 0x0U,
    VOID = 0x1U,

    CHAR = 0x2U,
    BOOLEAN = 0x3U,
    BYTE = 0x4U,
    SHORT = 0x5U,
    INT = 0x6U,
    LONG = 0x7U,
    FLOAT = 0x8U,
    DOUBLE = 0x9U,

    CLASS = 0xAU,
    STRING = 0xBU,
    INTERFACE = 0xCU,
    ARRAY = 0xDU,
    TUPLE = 0xEU,
    FUNCTIONAL = 0xFU,
    METHOD = 0x10U,
    UNION = 0x11U,
    UNDEFINED = 0x12U,
    NUL = 0x13U,

    ENUM = 0x14U
};

constexpr EtsByte ETS_TYPE_KIND_VALUE_MASK = 1U << 6U;

constexpr EtsChar VOID_PRIMITIVE_TYPE_DESC = 'V';

enum class EtsValueTypeDesc : EtsChar {
    BOOLEAN = 'Z',
    BYTE = 'B',
    SHORT = 'S',
    CHAR = 'C',
    INT = 'I',
    LONG = 'J',
    FLOAT = 'F',
    DOUBLE = 'D',
};

// Type attributes "flat" representation
enum class EtsTypeAPIAttributes : EtsInt {
    STATIC = 1U << 0U,       // Field, Method
    READONLY = 1U << 1U,     // Field
    FINAL = 1U << 2U,        // Method, Class
    ABSTRACT = 1U << 3U,     // Method
    CONSTRUCTOR = 1U << 4U,  // Method
    REST = 1U << 5U,         // Parameter
    OPTIONAL = 1U << 6U,     // Parameter
    NATIVE = 1U << 7U,       // Method, Lambda
    ASYNC = 1U << 8U,        // Method, Lambda
    GETTER = 1U << 9U,       // Method
    SETTER = 1U << 10U       // Method
};

enum class EtsTypeAPIAccessModifier : EtsByte { PUBLIC = 0, PRIVATE = 1, PROTECTED = 2 };

}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_TYPEAPI_H_
