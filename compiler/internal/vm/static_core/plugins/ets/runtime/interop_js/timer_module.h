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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_TIMER_MODULE_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_TIMER_MODULE_H

#include <uv.h>
#include <node_api.h>
#include <unordered_map>
#include <sys/types.h>
#include "ani/ani.h"
#include "libarkbase/macros.h"

class TimerModule {
public:
    static bool Init(ani_env *env);
    static ani_int StartTimer(ani_env *env, ani_object funcObject, ani_int delayMs, ani_boolean repeat);
    static void StopTimer(ani_env *env, ani_int timerId);

private:
    class TimerInfo {
    public:
        explicit TimerInfo(ani_object info) : info_(info) {}
        NO_COPY_SEMANTIC(TimerInfo);
        NO_MOVE_SEMANTIC(TimerInfo);
        ~TimerInfo() = default;

        bool IsOneShotTimer(ani_env *env);

        uv_timer_t *GetLinkedTimer(ani_env *env);

        void InvokeCallback(ani_env *env);

    private:
        ani_object info_;
    };

    static void TimerCallback(uv_timer_t *timer);
    static void DisarmTimer(uv_timer_t *timer);
    static void RepeatTimer(uv_timer_t *timer, uint64_t timerId);
    static bool CheckMainThread(ani_env *env);

    static ani_object GetTimerTable(ani_env *env);
    static std::pair<ani_object, bool> FindTimerInfo(ani_env *env, ani_int timerId);
    static void ClearTimerInfo(ani_env *env, ani_int timerId);

    static constexpr ani_int MIN_INTERVAL_MS = 1;
};

#endif  // PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_TIMER_MODULE_H
