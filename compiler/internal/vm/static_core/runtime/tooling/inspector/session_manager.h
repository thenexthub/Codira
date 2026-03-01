/**
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef PANDA_TOOLING_INSPECTOR_SESSION_MANAGER_H
#define PANDA_TOOLING_INSPECTOR_SESSION_MANAGER_H

#include "include/tooling/pt_thread.h"

namespace ark::tooling::inspector {
class SessionManager {
public:
    SessionManager() = default;
    ~SessionManager() = default;

    NO_COPY_SEMANTIC(SessionManager);
    NO_MOVE_SEMANTIC(SessionManager);

    const std::string &AddSession(PtThread thread);
    void RemoveSession(const std::string &id);

    [[nodiscard]] std::string GetSessionIdByThread(PtThread thread) const;
    [[nodiscard]] PtThread GetThreadBySessionId(const std::string &id) const;

    void EnumerateSessions(const std::function<void(const std::string &, PtThread)> &handler) const;

private:
    mutable os::memory::Mutex mutex_;
    std::map<std::string, PtThread> sessions_ GUARDED_BY(mutex_);
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_SESSION_MANAGER_H
