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

#include "session_manager.h"

namespace ark::tooling::inspector {
const std::string &SessionManager::AddSession(PtThread thread)
{
    ASSERT(thread.GetManagedThread());

    os::memory::LockHolder lock(mutex_);
    return sessions_.insert({!sessions_.empty() ? std::to_string(thread.GetId()) : "", thread}).first->first;
}

void SessionManager::RemoveSession(const std::string &id)
{
    os::memory::LockHolder lock(mutex_);
    sessions_.erase(id);
}

std::string SessionManager::GetSessionIdByThread(PtThread thread) const
{
    ASSERT(thread.GetManagedThread());

    os::memory::LockHolder lock(mutex_);

    auto it = sessions_.find("");
    if (it != sessions_.end() && it->second == thread) {
        return "";
    }

    return std::to_string(thread.GetId());
}

PtThread SessionManager::GetThreadBySessionId(const std::string &id) const
{
    os::memory::LockHolder lock(mutex_);

    auto it = sessions_.find(id);
    if (it == sessions_.end()) {
        return PtThread::NONE;
    }

    return it->second;
}

void SessionManager::EnumerateSessions(const std::function<void(const std::string &, PtThread)> &handler) const
{
    os::memory::LockHolder lock(mutex_);
    for (auto &[id, thread] : sessions_) {
        handler(id, thread);
    }
}
}  // namespace ark::tooling::inspector
