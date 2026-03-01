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

#include "platform/platform.h"

#include <cstdarg>

#include "unistd.h"

namespace platform {
void OS::Abort()
{
    std::abort();
}

uint64_t OS::GetUid()
{
    return static_cast<uint64_t>(getuid());
}

uint64_t OS::GetPid()
{
    return static_cast<uint64_t>(getppid());
}

uint64_t OS::GetTid()
{
    return static_cast<uint64_t>(gettid());
}

void OS::PrintString(OS::LogLevel level, const char* str)
{
    (void)level;
    printf("%s", str);
}

void VPrint(const char* format, va_list args)
{
    vprintf(format, args);
}

void VPrintError(const char* format, va_list args)
{
    (void)vfprintf(stderr, format, args);
}

void OS::Print(OS::LogLevel level, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    if (static_cast<uint64_t>(level) > static_cast<uint64_t>(OS::LogLevel::LOG_WARN)) {
        VPrintError(format, args);
    } else {
        VPrint(format, args);
    }
    va_end(args);
}

} // namespace platform
