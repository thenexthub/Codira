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

#include "libarkbase/os/pipe.h"

namespace ark::os::unix {

std::pair<UniqueFd, UniqueFd> CreatePipe()
{
    // Unsupported on windows platform
    UNREACHABLE();
}

int SetFdNonblocking(const UniqueFd &fd)
{
    // Unsupported on windows platform
    UNREACHABLE();
}

Expected<size_t, Error> ReadFromPipe(const UniqueFd &pipeFd, void *buf, size_t size)
{
    // Unsupported on windows platform
    UNREACHABLE();
}

Expected<size_t, Error> WriteToPipe(const UniqueFd &pipeFd, const void *buf, size_t size)
{
    // Unsupported on windows platform
    UNREACHABLE();
}

Expected<size_t, Error> WaitForEvent(const UniqueFd *handles, size_t size, EventType type)
{
    // Unsupported on windows platform
    UNREACHABLE();
}

std::optional<Error> Dup2(const UniqueFd &source, const UniqueFd &target)
{
    // Unsupported on windows platform
    UNREACHABLE();
}

}  // namespace ark::os::unix
