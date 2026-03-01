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

#include <cstdint>
#include "libarkbase/macros.h"
#include "libarkbase/utils/logger.h"

namespace ark::trace::internal {

PANDA_PUBLIC_API int g_traceMarkerFd = -1;

bool DoInit()
{
    LOG(ERROR, TRACE) << "Tracing not implemented for this platform.";
    return false;
}

PANDA_PUBLIC_API void DoBeginTracePoint([[maybe_unused]] const char *str)
{
    UNREACHABLE();
}

PANDA_PUBLIC_API void DoEndTracePoint()
{
    UNREACHABLE();
}

void DoIntTracePoint([[maybe_unused]] const char *str, [[maybe_unused]] int32_t val)
{
    UNREACHABLE();
}

void DoInt64TracePoint([[maybe_unused]] const char *str, [[maybe_unused]] int64_t val)
{
    UNREACHABLE();
}

}  // end namespace ark::trace::internal
