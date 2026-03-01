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

#ifndef ARKCOMPILER_TOOLCHAIN_WEBSOCKET_STRING_UTILS_H
#define ARKCOMPILER_TOOLCHAIN_WEBSOCKET_STRING_UTILS_H

#include <algorithm>
#include <cctype>
#include <string>

namespace OHOS::ArkCompiler::Toolchain {
// all cctype function arguments must be representable as unsigned char
inline void TrimLeft(std::string &str)
{
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) { return !std::isspace(ch); }));
}

inline void TrimRight(std::string &str)
{
    str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(),
              str.end());
}

inline void Trim(std::string &str)
{
    TrimLeft(str);
    TrimRight(str);
}

inline void ToLowerCase(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
}
} // namespace OHOS::ArkCompiler::Toolchain

#endif // ARKCOMPILER_TOOLCHAIN_WEBSOCKET_STRING_UTILS_H

