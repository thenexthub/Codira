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

#ifndef PANDA_PLUGINS_INTEROP_JS_TIMER_H
#define PANDA_PLUGINS_INTEROP_JS_TIMER_H

#include <node_api.h>
#include <uv.h>
#include <vector>

namespace ark::ets::interop::js::helper {

struct TimerInfo {
    explicit TimerInfo(napi_env env, napi_ref cb, std::vector<napi_ref> cbArgs, bool repeat);
    TimerInfo(const TimerInfo &info) = delete;
    TimerInfo(TimerInfo &&info) = delete;
    TimerInfo &operator=(const TimerInfo &info) = delete;
    TimerInfo &operator=(TimerInfo &&info) = delete;
    ~TimerInfo();

    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    napi_env env;
    napi_ref cb;
    std::vector<napi_ref> cbArgs;
    bool repeat;
    uv_timer_t *timer;
    uint32_t timerId;
    // NOLINTEND(misc-non-private-member-variables-in-classes)
};

napi_value SetTimeoutImpl(napi_env env, napi_callback_info info, bool repeat);
napi_value ClearTimerImpl(napi_env env, napi_callback_info info);

}  // namespace ark::ets::interop::js::helper

#endif  // PANDA_PLUGINS_INTEROP_JS_TIMER_H
