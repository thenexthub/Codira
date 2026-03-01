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

#include "runtime_controller.h"

#include <algorithm>
#include <string>
#include <string_view>
#include <sys/param.h>
#include <unistd.h>

#include "libarkbase/utils/logger.h"
#include "runtime/include/mem/panda_containers.h"

namespace ark {

/// Data directory of applications for default user.
constexpr std::string_view DIR_DATA_DATA = "/data/data/";

/// Data directory of applications for non-default users.
constexpr std::string_view DIR_DATA_USER = "/data/user/";

static bool StartsWith(std::string_view s, std::string_view prefix)
{
    return (s.size() >= prefix.size()) && std::equal(prefix.begin(), prefix.end(), s.begin());
}

static bool StartsWithData(std::string_view path)
{
    if (path.empty() || (path[0] != '/')) {
        return false;
    }
    return StartsWith(path, DIR_DATA_DATA) || StartsWith(path, DIR_DATA_USER);
}

static bool IsInPermitList(std::string_view path)
{
    size_t pos = path.rfind('/');
    if (pos == std::string::npos) {
        LOG(ERROR, RUNTIME) << "Failed to get file name from path: " << path;
        return false;
    }
    std::string_view fileName = path.substr(pos + 1U);
    return StartsWith(fileName, "HMS-Ohos-");
}

bool RuntimeController::CanLoadPandaFileInternal(std::string_view realPath) const
{
    return (!StartsWithData(realPath)) || IsInPermitList(realPath);
}

bool RuntimeController::CanLoadPandaFile(const std::string &path) const
{
    if (IsZidaneApp() && (!IsMultiFramework())) {
        // Avoid large frame.
        PandaVector<char> buffer(PATH_MAX, 0);
        if (realpath(path.c_str(), buffer.data()) == nullptr) {
            LOG(ERROR, RUNTIME) << "Failed to get realpath for " << path;
            return true;  // Allow loading panda file.
        }
        std::string_view realPath = buffer.data();
        bool allow = CanLoadPandaFileInternal(realPath);
        if (!allow) {
            LOG(WARNING, RUNTIME) << "Disallow loading panda file in data directory : " << path;
        }
        return allow;
    }
    return true;
}

}  // namespace ark
