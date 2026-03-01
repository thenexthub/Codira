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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_APP_STATE_H
#define PANDA_PLUGINS_ETS_RUNTIME_APP_STATE_H

#include <atomic>
#include <cstdint>
#include <optional>

namespace ark::ets {

class AppState {
public:
    enum class State { FOREGROUND, BACKGROUND, SENSITIVE_START, SENSITIVE_END, COLD_START_FINISHED };

    AppState() = default;
    AppState(State state, int64_t timestamp) : state_(state), timestamp_(timestamp) {}

    std::optional<State> GetState() const
    {
        return state_;
    }

    int64_t GetTimeStamp() const
    {
        return timestamp_;
    }

private:
    std::optional<State> state_;
    int64_t timestamp_ {0};
};
}  // namespace ark::ets

#endif  // PANDA_PLUGINS_ETS_RUNTIME_APP_STATE_H