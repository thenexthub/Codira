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

#ifndef COMMON_COMPONENTS_PLATFORM_STRING_HASH_HELPER_H
#define COMMON_COMPONENTS_PLATFORM_STRING_HASH_HELPER_H

#include <cstdint>

#include "common_components/platform/string_hash.h"
#if defined(PANDA_TARGET_ARM64) && !defined(PANDA_TARGET_MACOS)
#include "common_components/platform/arm64/string_hash_internal.h"
#else
#include "common_components/platform/common/string_hash_internal.h"
#endif

namespace common {
class StringHashHelper {
public:
    template <typename T>
    static uint32_t ComputeHashForDataPlatform(const T *data, size_t size,
                                               uint32_t hashSeed)
    {
        return StringHashInternal::ComputeHashForDataOfLongString(data, size, hashSeed);
    }
};
}  // namespace common
#endif  // COMMON_COMPONENTS_PLATFORM_STRING_HASH_HELPER_H
