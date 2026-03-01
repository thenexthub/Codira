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

#ifndef PANDA_PLUGINS_ETS_TOOLS_FORMAT_LOGGER_H
#define PANDA_PLUGINS_ETS_TOOLS_FORMAT_LOGGER_H
#include "libarkbase/utils/logger.h"
#include <securec.h>

namespace ark {
inline std::string FormatString(const char *fmt, ...)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    va_list args;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    va_start(args, fmt);
    const size_t bufferSize = 512;
    std::array<char, bufferSize> buffer {};
    int put = vsnprintf_s(buffer.data(), bufferSize, bufferSize - 1, fmt, args);
    if (put < 0) {
        va_end(args);
        return "";
    }
    va_end(args);
    return std::string(buffer.data());
}

// NOLINTBEGIN(cppcoreguidelines-macro-usage)
#define LOG_INFO_SDK(...) IMPL_LOG(INFO, SDK, false) << ark::FormatString(__VA_ARGS__)
#define LOG_WARNING_SDK(...) IMPL_LOG(WARNING, SDK, false) << ark::FormatString(__VA_ARGS__)
#define LOG_ERROR_SDK(...) IMPL_LOG(ERROR, SDK, false) << ark::FormatString(__VA_ARGS__)
#define LOG_FATAL_SDK(...) IMPL_LOG(FATAL, SDK, false) << ark::FormatString(__VA_ARGS__)
// NOLINTEND(cppcoreguidelines-macro-usage)
}  // namespace ark
#endif
