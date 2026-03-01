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

#include "libarkbase/os/system_environment.h"
#include <pwd.h>
#include <unistd.h>

namespace ark::os::system_environment {
int GetUidForName(const std::string &name)
{
    return GetUidForName(name.c_str());
}

int GetUidForName(const char *name)
{
    auto bufferSize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufferSize == -1) {
        // The following size should be enough in case when we are unable to retrive the max buffer length from the os
        bufferSize = 16384U;  // NOLINT(readability-magic-numbers)
    }

    passwd user {};
    passwd *bufferPtr = nullptr;
    std::string buffer;
    buffer.reserve(bufferSize);
    buffer.resize(bufferSize);

    if (getpwnam_r(name, &user, buffer.data(), bufferSize, &bufferPtr) == 0 && bufferPtr != nullptr) {
        return bufferPtr->pw_uid;
    }
    return -1;
}

int64_t GetSystemConfig(int name)
{
    return sysconf(name);
}

std::string GetEnvironmentVar(const std::string &name)
{
    return GetEnvironmentVar(name.c_str());
}

std::string GetEnvironmentVar(const char *name)
{
    if (auto *value = getenv(name); value != nullptr) {
        return value;
    }
    return {};
}

}  // namespace ark::os::system_environment
