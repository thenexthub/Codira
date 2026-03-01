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

#ifndef PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_APP_STATE_MANAGER_H
#define PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_APP_STATE_MANAGER_H

#include "libarkbase/os/mutex.h"
#include "plugins/ets/runtime/app_state.h"

namespace ark::ets {

class AppStateManager final {
public:
    static void Create()
    {
        ASSERT(instance_ == nullptr);
        instance_ = new AppStateManager();
    }

    static void Destroy()
    {
        delete instance_;
        instance_ = nullptr;
    }

    static AppStateManager *GetCurrent()
    {
        return instance_;
    }

    void UpdateAppState(AppState appState)
    {
        os::memory::LockHolder lh(appStateLock_);
        appState_ = appState;
    }

    AppState GetAppState() const
    {
        os::memory::LockHolder lh(appStateLock_);
        return appState_;
    }

private:
    AppStateManager() = default;
    ~AppStateManager() = default;
    NO_COPY_SEMANTIC(AppStateManager);
    NO_MOVE_SEMANTIC(AppStateManager);

    static inline AppStateManager *instance_ {nullptr};
    mutable os::memory::Mutex appStateLock_;
    AppState appState_;
};

}  // namespace ark::ets

#endif  // !PANDA_PLUGINS_ETS_RUNTIME_INTEROP_JS_APP_STATE_MANAGER_H
