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

#ifndef COMMON_COMPONENTS_OBJECTS_STRING_TABLE_INTEGER_CACHE_H
#define COMMON_COMPONENTS_OBJECTS_STRING_TABLE_INTEGER_CACHE_H

#include <array>
#include "common_interfaces/objects/string/base_string.h"
#include "common_interfaces/objects/string/line_string.h"

namespace common {

/*
 Used only when 0 < string->GetLength() <= 4
 +-----------------------------+
 |        String Payload       |    <-- 32 bits
 +--------------+--------------+
 | IsInteger    | IntegerValue |    <-- 16 + 16 bits
 +--------------+--------------+
 */
class IntegerCache final {
static constexpr size_t OBJECT_ALIGN = 8;
static_assert(LineString::DATA_OFFSET % OBJECT_ALIGN == 0);

public:
    NO_MOVE_SEMANTIC_CC(IntegerCache);
    NO_COPY_SEMANTIC_CC(IntegerCache);
    static constexpr size_t MAX_INTEGER_CACHE_SIZE = 4;

    IntegerCache() = delete;

    static void InitIntegerCache(BaseString* string)
    {
        if (string->IsUtf8() && string->GetLength() <= MAX_INTEGER_CACHE_SIZE && string->GetLength() > 0) {
            IntegerCache* cache = Extract(string);
            cache->isInteger_ = 0;
        }
    }

    static IntegerCache* Extract(BaseString* string)
    {
        DCHECK_CC(string->IsUtf8() && string->GetLength() <= MAX_INTEGER_CACHE_SIZE
            && string->GetLength() > 0 && string->IsInternString());
        IntegerCache* cache = reinterpret_cast<IntegerCache*>(LineString::Cast(string)->GetData());
        return cache;
    }

    bool IsInteger() const
    {
        return isInteger_ != 0;
    }

    uint16_t GetInteger() const
    {
        DCHECK_CC(IsInteger());
        return integer_;
    }

    void SetInteger(uint16_t value)
    {
        integer_ = value;
        isInteger_ = 1;
    }

private:
    [[maybe_unused]] std::array<uint8_t, MAX_INTEGER_CACHE_SIZE> payload_;
    uint16_t isInteger_ = 0;
    uint16_t integer_ = 0;
};
}
#endif //COMMON_COMPONENTS_OBJECTS_STRING_TABLE_INTEGER_CACHE_H

