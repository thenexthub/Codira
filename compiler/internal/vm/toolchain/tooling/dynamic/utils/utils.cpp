/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include <climits>
#include <ctime>

#include "common/log_wrapper.h"
#include "tooling/dynamic/utils/utils.h"

namespace OHOS::ArkCompiler::Toolchain {
bool Utils::GetCurrentTime(char *date, char *tim, size_t size)
{
    time_t timep = time(nullptr);
    if (timep == -1) {
        LOGE("get timep failed");
        return false;
    }

    tm* currentDate = localtime(&timep);
    if (currentDate == nullptr) {
        LOGE("get currentDate failed");
        return false;
    }

    size_t timeResult = 0;
    timeResult = strftime(date, size, "%Y%m%d", currentDate);
    if (timeResult == 0) {
        LOGE("format date failed");
        return false;
    }

    tm* currentTime = localtime(&timep);
    if (currentTime == nullptr) {
        LOGE("get currentTime failed");
        return false;
    }

    timeResult = strftime(tim, size, "%H%M%S", currentTime);
    if (timeResult == 0) {
        LOGE("format time failed");
        return false;
    }
    return true;
}

bool Utils::StrToUInt(const char *content, uint32_t *result)
{
    const int dec = 10;
    char *endPtr = nullptr;
    *result = std::strtoul(content, &endPtr, dec);
    if (endPtr == content || *endPtr != '\0') {
        return false;
    }
    return true;
}

bool Utils::StrToInt32(const std::string &str, int32_t &result)
{
    const int dec = 10;
    char *endPtr = nullptr;
    long long num = std::strtoll(str.c_str(), &endPtr, dec);
    if (endPtr == str.c_str() || *endPtr != '\0' || num > INT_MAX || num < INT_MIN) {
        return false;
    }
    result = static_cast<int32_t>(num);
    return true;
}

bool Utils::StrToInt32(const std::string &str, std::optional<int32_t> &result)
{
    const int dec = 10;
    char *endPtr = nullptr;
    long long num = std::strtoll(str.c_str(), &endPtr, dec);
    if (endPtr == str.c_str() || *endPtr != '\0' || num > INT_MAX || num < INT_MIN) {
        return false;
    }
    result = static_cast<int32_t>(num);
    return true;
}

std::vector<std::string> Utils::SplitString(const std::string &str, const std::string &delimiter)
{
    std::size_t strIndex = 0;
    std::vector<std::string> value;
    std::size_t pos = str.find_first_of(delimiter, strIndex);
    while ((pos < str.size()) && (pos > strIndex)) {
        std::string subStr = str.substr(strIndex, pos - strIndex);
        value.push_back(std::move(subStr));
        strIndex = pos;
        strIndex = str.find_first_not_of(delimiter, strIndex);
        pos = str.find_first_of(delimiter, strIndex);
    }
    if (pos > strIndex) {
        std::string subStr = str.substr(strIndex, pos - strIndex);
        if (!subStr.empty()) {
            value.push_back(std::move(subStr));
        }
    }
    return value;
}

bool Utils::RealPath(const std::string &path, std::string &realPath, [[maybe_unused]] bool readOnly)
{
    realPath = "";
    if (path.empty() || path.size() > PATH_MAX) {
        LOGE("arkdb: File path is illeage");
        return false;
    }
    char buffer[PATH_MAX] = { '\0' };
#if defined(PANDA_TARGET_WINDOWS)
    if (!_fullpath(buffer, path.c_str(), sizeof(buffer) - 1)) {
        LOGE("arkdb: full path failure");
        return false;
    }
#endif
#if defined(PANDA_TARGET_UNIX)
    if (!realpath(path.c_str(), buffer)) {
        // Maybe file does not exist.
        if (!readOnly && errno == ENOENT) {
            realPath = path;
            return true;
        }
        LOGE("arkdb: realpath failure");
        return false;
    }
#endif
    realPath = std::string(buffer);
    return true;
}

bool Utils::IsNumber(const std::string &str)
{
    std::string tmpStr = str;
    tmpStr.erase(0, tmpStr.find_first_not_of("0"));
    if (tmpStr.size() > 9) { //9: size of int
        return false;
    }
    
    for (char c : tmpStr) {
        if (!std::isdigit(c)) {
            return false;
        }
    }
    return true;
}

} // OHOS::ArkCompiler::Toolchain