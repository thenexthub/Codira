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

#include "source_manager.h"

#include <string>
#include <string_view>

#include "types/numeric_id.h"

namespace ark::tooling::inspector {
ScriptId SourceManager::GetScriptId(std::string_view fileName)
{
    os::memory::LockHolder lock(mutex_);

    auto p = fileNameToId_.emplace(std::string(fileName), fileNameToId_.size());
    ScriptId id(p.first->second);

    if (p.second) {
        std::string_view name {p.first->first};
        idToFileName_.emplace(id, name);
    }

    return id;
}

std::string_view SourceManager::GetSourceFileName(ScriptId id) const
{
    os::memory::LockHolder lock(mutex_);

    auto it = idToFileName_.find(id);
    if (it != idToFileName_.end()) {
        return it->second;
    }

    LOG(ERROR, DEBUGGER) << "No file with script id " << id;

    return {};
}

}  // namespace ark::tooling::inspector
