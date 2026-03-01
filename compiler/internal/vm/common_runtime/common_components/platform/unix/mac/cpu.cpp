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

#include "common_components/platform/cpu.h"

#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/xattr.h>
#include <unistd.h>

#include "common_components/log/log.h"

namespace common {
uint32_t NumberOfCpuCore()
{
    return static_cast<uint32_t>(sysconf(_SC_NPROCESSORS_ONLN));
}

size_t PhysicalSize()
{
    static constexpr int mibLength = 2;
    int mib[2];
    mib[0] = CTL_HW;
    mib[1] = HW_MEMSIZE;
    int64_t size = 0;
    size_t bufferLength = sizeof(size);
    if (sysctl(mib, mibLength, &size, &bufferLength, nullptr, 0) != 0) {
        LOG_COMMON(FATAL) << "sysctl error";
    }
    return static_cast<size_t>(size);
}
}  // namespace common
