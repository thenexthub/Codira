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
// This is a local header
// NOLINTNEXTLINE(modernize-deprecated-headers, hicpp-deprecated-headers)
#include "time.h"
#include <chrono>

namespace ark::time {

template <class T>
static uint64_t GetCurrentTime(bool needSystem)
{
    if (needSystem) {
        return std::chrono::duration_cast<T>(std::chrono::system_clock::now().time_since_epoch()).count();
    }
    return std::chrono::duration_cast<T>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

/// Return current time in milliseconds
uint64_t GetCurrentTimeInMillis(bool needSystem)
{
    return GetCurrentTime<std::chrono::milliseconds>(needSystem);
}

/// Return current time in microseconds
uint64_t GetCurrentTimeInMicros(bool needSystem)
{
    return GetCurrentTime<std::chrono::microseconds>(needSystem);
}

/// Return current time in nanoseconds
uint64_t GetCurrentTimeInNanos(bool needSystem)
{
    return GetCurrentTime<std::chrono::nanoseconds>(needSystem);
}

}  // namespace ark::time
