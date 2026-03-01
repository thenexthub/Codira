/**
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ECMASCRIPT_TOOLING_CLIENT_UTILS_UTILS_H
#define ECMASCRIPT_TOOLING_CLIENT_UTILS_UTILS_H

#include <cstdint>
#include <string>
#include <vector>
#include <optional>

namespace OHOS::ArkCompiler::Toolchain {
class Utils {
public:
    static bool RealPath(const std::string &path, std::string &realPath, bool readOnly = true);
    static bool GetCurrentTime(char *date, char *tim, size_t size);
    static bool StrToUInt(const char *content, uint32_t *result);
    static bool StrToInt32(const std::string &str, int32_t &result);
    static bool StrToInt32(const std::string &str, std::optional<int32_t> &result);
    static std::vector<std::string> SplitString(const std::string &str, const std::string &delimiter);
    static bool IsNumber(const std::string &str);
};
} // OHOS::ArkCompiler::Toolchain
#endif