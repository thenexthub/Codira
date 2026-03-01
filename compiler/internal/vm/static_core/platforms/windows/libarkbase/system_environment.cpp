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
#include "libarkbase/macros.h"

namespace ark::os::system_environment {
int GetUidForName([[maybe_unused]] const std::string &name)
{
    UNREACHABLE();
}

int GetUidForName([[maybe_unused]] const char *name)
{
    UNREACHABLE();
}

int64_t GetSystemConfig([[maybe_unused]] int name)
{
    UNREACHABLE();
}

std::string GetEnvironmentVar([[maybe_unused]] const std::string &name)
{
    UNREACHABLE();
}

std::string GetEnvironmentVar([[maybe_unused]] const char *name)
{
    UNREACHABLE();
}

}  // namespace ark::os::system_environment
