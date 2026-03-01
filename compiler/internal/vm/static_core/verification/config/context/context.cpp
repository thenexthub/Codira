/**
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "context.h"

#include "runtime/include/mem/allocator.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ark::verifier::debug {

void DebugContext::AddMethod(const Method &method, bool isDebug)
{
    if (config->whitelistNotEmpty || isDebug) {
        auto id = method.GetUniqId();
        // this is calculated repeatedly for each class, but caching was tried and didn't improve performance
        auto name {ClassHelper::GetName<PandaString>(method.GetClassName().data)};
        if (config->whitelistNotEmpty) {
            InsertIntoWhitelist(name, true, id);
        }
        name += "::";
        name += utf::Mutf8AsCString(method.GetName().data);
        if (config->whitelistNotEmpty) {
            InsertIntoWhitelist(name, false, id);
        }
        if (isDebug) {
            InsertBreakpoints(name, id);
        }
    }
}

}  // namespace ark::verifier::debug
