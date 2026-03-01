/**
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef PANDA_TOOLING_INSPECTOR_SOURCE_MANAGER_H
#define PANDA_TOOLING_INSPECTOR_SOURCE_MANAGER_H

#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "include/tooling/pt_thread.h"
#include "libarkbase/os/mutex.h"

#include "types/numeric_id.h"

namespace ark::tooling::inspector {
class SourceManager final {
public:
    SourceManager() = default;
    ~SourceManager() = default;

    NO_COPY_SEMANTIC(SourceManager);
    NO_MOVE_SEMANTIC(SourceManager);

    std::pair<ScriptId, bool> GetScriptId(std::string_view fileName);
    [[nodiscard]] std::string_view GetSourceFileName(ScriptId id) const;

private:
    mutable os::memory::Mutex mutex_;
    std::unordered_map<std::string, ScriptId> fileNameToId_ GUARDED_BY(mutex_);
    std::unordered_map<ScriptId, std::string_view> idToFileName_ GUARDED_BY(mutex_);
    std::unordered_set<ScriptId> knownSources_ GUARDED_BY(mutex_);
};
}  // namespace ark::tooling::inspector

#endif  // PANDA_TOOLING_INSPECTOR_SOURCE_MANAGER_H
