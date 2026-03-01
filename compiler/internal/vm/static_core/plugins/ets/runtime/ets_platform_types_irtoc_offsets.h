/**
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_PLATFORM_TYPES_IRTOC_OFFSETS_H_
#define PANDA_PLUGINS_ETS_RUNTIME_PLATFORM_TYPES_IRTOC_OFFSETS_H_

#include "plugins/ets/runtime/ets_platform_types.h"

namespace ark::ets {

class EtsPlatformTypes;

class EtsPlatformTypesIrtocOffsets {
public:
    // Class entry offsets:
    static constexpr size_t ESCOMPAT_ARRAY = MEMBER_OFFSET(EtsPlatformTypes, escompatArray);
    static constexpr size_t CORE_LINE_STRING = MEMBER_OFFSET(EtsPlatformTypes, coreLineString);
    static constexpr size_t CORE_SLICED_STRING = MEMBER_OFFSET(EtsPlatformTypes, coreSlicedString);
    static constexpr size_t CORE_TREE_STRING = MEMBER_OFFSET(EtsPlatformTypes, coreTreeString);

    // CC-OFFNXT(G.CMT.04-CPP) false positive
    // Managed object caches:
    static constexpr uint32_t GetAsciiCharCacheSize()
    {
        return EtsPlatformTypes::ASCII_CHAR_TABLE_SIZE;
    }
    static constexpr size_t GetAsciiCharCacheOffset()
    {
        return MEMBER_OFFSET(EtsPlatformTypes, asciiCharCache_);
    }
};

}  // namespace ark::ets

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_PLATFORM_TYPES_IRTOC_OFFSETS_H_