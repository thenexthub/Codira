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

#ifndef COMMON_COMPONENTS_BASE_HASH_UTILS_H
#define COMMON_COMPONENTS_BASE_HASH_UTILS_H

#include <cstdint>

#include "common_components/base/c_string.h"

namespace common {

struct HashString {
    // 211 is a proper prime, which can reduce the conflict rate.
    const uint32_t properPrime = 211;
    
    size_t operator()(const char* key) const
    {
        uint32_t hash = 0;
        while ((*key) != '\0') {
            uint32_t keyChar = *key;
            hash = hash * properPrime + keyChar;
            key += 1;
        }
        return hash;
    }
};
struct EqualString {
    bool operator()(const char* lhs, const char* rhs) const { return strcmp(lhs, rhs) == 0; }
};

} // namespace common

#endif // COMMON_COMPONENTS_BASE_HASH_UTILS_H
