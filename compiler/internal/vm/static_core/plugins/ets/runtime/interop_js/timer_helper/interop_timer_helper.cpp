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

#include <array>
#include <iostream>
#include <node_api.h>
#include "interop_timer_helper.h"
#include "timer.h"

// NOLINTBEGIN(readability-identifier-naming, readability-redundant-declaration)
// CC-OFFNXT(G.FMT.10-CPP) project code style
napi_status __attribute__((weak)) napi_define_properties(napi_env env, napi_value object, size_t property_count,
                                                         const napi_property_descriptor *properties);
// NOLINTEND(readability-identifier-naming, readability-redundant-declaration)

namespace ark::ets::interop::js::helper {

static constexpr const char *MODULE_PREFIX = "[INTEROP_TIMER_HELPER] ";

static napi_value SetTimeout(napi_env env, napi_callback_info info)
{
    return SetTimeoutImpl(env, info, false);
}

static napi_value SetInterval(napi_env env, napi_callback_info info)
{
    return SetTimeoutImpl(env, info, true);
}

static napi_value ClearTimer(napi_env env, napi_callback_info info)
{
    return ClearTimerImpl(env, info);
}

napi_value Init(napi_env env, napi_value exports)
{
    const std::array desc = {
        napi_property_descriptor {"setTimeout", 0, SetTimeout, 0, 0, 0, napi_enumerable, 0},
        napi_property_descriptor {"setInterval", 0, SetInterval, 0, 0, 0, napi_enumerable, 0},
        napi_property_descriptor {"clearTimeout", 0, ClearTimer, 0, 0, 0, napi_enumerable, 0},
        napi_property_descriptor {"clearInterval", 0, ClearTimer, 0, 0, 0, napi_enumerable, 0},
    };

    if (napi_define_properties(env, exports, desc.size(), desc.data()) != napi_ok) {
        std::cerr << MODULE_PREFIX << "Failed to define properties" << std::endl;
        std::abort();  // CC-OFF(G.STD.16-CPP) fatal error
    }

    return exports;
}

}  // namespace ark::ets::interop::js::helper
