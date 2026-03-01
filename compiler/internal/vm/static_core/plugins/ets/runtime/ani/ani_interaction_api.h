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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_ANI_INTERACTION_API_H
#define PANDA_PLUGINS_ETS_RUNTIME_ANI_INTERACTION_API_H

#include <cstdint>
#include <string_view>

struct __ani_interaction_api;  // CC-OFF(G.NAM.01) Interface from external header

namespace ark::ets::ani {
const __ani_interaction_api *GetInteractionAPI();
bool IsVersionSupported(uint32_t version);

struct EnumArrayNames {
    static constexpr std::string_view NAMES = "#NamesArray";
    static constexpr std::string_view VALUES = "#ValuesArray";
    static constexpr std::string_view STRING_VALUES = "#StringValuesArray";
    static constexpr std::string_view BOXED_ITEMS = "#ItemsArray";
};
}  // namespace ark::ets::ani

#endif  // PANDA_PLUGINS_ETS_RUNTIME_ANI_INTERACTION_API_H
