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


#include "CString.h"

#include <libgen.h>

namespace MapleRuntime {
// The last two characters are units, such as "kb" or "ms".
constexpr int8_t LAST_CHARACTERS_SIZE = 2;
// nDecimal = ceil(sizeof(int32_t) * 8 * log(2)) = ceil(2.41 * sizeof(int32_t))
// consider overhead for sign & '\0' and alignment, replace 2.41 by 4
constexpr int8_t MIN_ALIGN_SPACE = 4;

CString::CString()
{
    str = reinterpret_cast<char*>(malloc(capacity));
    PRINT_FATAL_IF(str == nullptr, "CString::Init failed");
    *str = '\0';
    length = 0;
}

CString::CString(const char* initStr)
{
    if (initStr != nullptr) {
        size_t initLen = strlen(initStr);
        while (capacity < initLen + 1) {
            capacity <<= 1;
        }
        str = reinterpret_cast<char*>(malloc(capacity));
        PRINT_FATAL_IF(str == nullptr, "CString::Init failed");
        if (*initStr != '\0') {
            PRINT_FATAL_IF(memcpy_s(str, capacity, initStr, initLen) != EOK, "CString::CString memcpy_s failed");
        }
        length = initLen;
        str[length] = '\0';
    }
}

CString::CString(char c)
{
    str = reinterpret_cast<char*>(malloc(capacity));
    PRINT_FATAL_IF(str == nullptr, "CString::Init failed");
    str[0] = c;
    str[1] = '\0';
    length = 1;
}

// nDecimal = ceil(sizeof(int32_t) * 8 * log(2)) = ceil(2.41 * sizeof(int32_t))
// consider overhead for sign & '\0' and alignment, replace 2.41 by 4
CString::CString(int32_t number)
{
    capacity = sizeof(int32_t) * MIN_ALIGN_SPACE;
    str = reinterpret_cast<char*>(malloc(capacity));
    PRINT_FATAL_IF(str == nullptr, "CString::Init failed");
    int ret = sprintf_s(str, capacity, "%d", number);
    PRINT_FATAL_IF(ret == -1, "CString::Init failed");
    length = ret;
}

CString::CString(int64_t number)
{
    capacity = sizeof(int64_t) * MIN_ALIGN_SPACE;
    str = reinterpret_cast<char*>(malloc(capacity));
    PRINT_FATAL_IF(str == nullptr, "CString::Init failed");
    int ret = sprintf_s(str, capacity, "%lld", number);
    PRINT_FATAL_IF(ret == -1, "CString::Init failed");
    length = ret;
}

CString::CString(uint32_t number)
{
    capacity = sizeof(uint32_t) * MIN_ALIGN_SPACE;
    str = reinterpret_cast<char*>(malloc(capacity));
    PRINT_FATAL_IF(str == nullptr, "CString::Init failed");
    int ret = sprintf_s(str, capacity, "%u", number);
    PRINT_FATAL_IF(ret == -1, "CString::Init failed");
    length = ret;
}

CString::CString(uint64_t number)
{
    capacity = sizeof(uint64_t) * MIN_ALIGN_SPACE;
    str = reinterpret_cast<char*>(malloc(capacity));
    PRINT_FATAL_IF(str == nullptr, "CString::Init failed");
    int ret = sprintf_s(str, capacity, "%lu", number);
    PRINT_FATAL_IF(ret == -1, "CString::Init failed");
    length = ret;
}

CString::CString(const CString& other)
{
    size_t initLen = other.Length();
    while (capacity < initLen + 1) {
        capacity <<= 1;
    }
    str = reinterpret_cast<char*>(malloc(capacity));
    PRINT_FATAL_IF(str == nullptr, "CString::Init failed");
    if (!other.IsEmpty()) {
        PRINT_FATAL_IF(memcpy_s(str, capacity, other.Str(), initLen) != EOK, "CString::CString memcpy_s failed");
    }
    length = initLen;
    str[length] = '\0';
}

CString::CString(const FixedCString& other)
{
    size_t initLen = other.Length();
    while (capacity < initLen + 1) {
        capacity <<= 1;
    }
    str = reinterpret_cast<char*>(malloc(capacity));
    PRINT_FATAL_IF(str == nullptr, "CString::Init failed");
    if (!other.IsEmpty()) {
        PRINT_FATAL_IF(memcpy_s(str, capacity, other.Str(), initLen) != EOK, "CString::CString memcpy_s failed");
    }
    length = initLen;
    str[length] = '\0';
}

CString::CString(size_t number, char ch)
{
    while (capacity < number + 1) {
        capacity <<= 1;
    }
    str = reinterpret_cast<char*>(malloc(capacity));
    PRINT_FATAL_IF(str == nullptr, "CString::Init failed");
    PRINT_FATAL_IF(memset_s(str, capacity, ch, number) != EOK, "CString::CString memset_s failed");
    length = number;
    str[length] = '\0';
}

CString& CString::operator=(const CString& other)
{
    if (this == &other) {
        return *this;
    }
    size_t initLen = other.Length();
    while (capacity < initLen + 1) {
        capacity <<= 1;
    }
    if (str != nullptr) {
        free(str);
    }
    str = reinterpret_cast<char*>(malloc(capacity));
    PRINT_FATAL_IF(str == nullptr, "CString::operator= malloc failed");
    if (!other.IsEmpty()) {
        PRINT_FATAL_IF(memcpy_s(str, capacity, other.Str(), initLen) != EOK, "CString::operator= memcpy_s failed");
    }
    length = initLen;
    str[length] = '\0';
    return *this;
}

CString::~CString()
{
    if (str != nullptr) {
        free(str);
        str = nullptr;
    }
}

void CString::EnsureSpace(size_t addLen)
{
    if (addLen == 0 || capacity >= length + addLen + 1) {
        return;
    }
    while (capacity < length + addLen + 1) {
        capacity <<= 1;
    }
    char* newStr = reinterpret_cast<char*>(malloc(capacity));
    PRINT_FATAL_IF(newStr == nullptr, "CString::Init failed");
    PRINT_FATAL_IF(memcpy_s(newStr, capacity, str, length) != EOK, "CString::EnsureSpace memcpy_s failed");
    if (str != nullptr) {
        free(str);
    }
    str = newStr;
}

CString& CString::Append(const CString& addStr, size_t addLen)
{
    if (addStr.IsEmpty()) {
        return *this;
    }
    if (addLen == 0) {
        addLen = strlen(addStr.str);
    }
    EnsureSpace(addLen);
    PRINT_FATAL_IF(memcpy_s(str + length, capacity - length, addStr.str, addLen) != EOK,
                   "CString::Append memcpy_s failed");
    length += addLen;
    str[length] = '\0';
    return *this;
}

CString& CString::Append(const char* addStr, size_t addLen)
{
    if (addStr == nullptr || *addStr == '\0') {
        return *this;
    }
    if (addLen == 0) {
        addLen = strlen(addStr);
    }
    EnsureSpace(addLen);
    PRINT_FATAL_IF(memcpy_s(str + length, capacity - length, addStr, addLen) != EOK, "CString::Append memcpy_s failed");
    length += addLen;
    str[length] = '\0';
    return *this;
}

CString CString::Combine(const char* addStr) const
{
    CString newStr;
    newStr.Append(*this);
    newStr.Append(addStr);
    return newStr;
}

CString CString::Combine(const CString& addStr) const
{
    CString newStr;
    newStr.Append(*this);
    newStr.Append(addStr);
    return newStr;
}

size_t CString::Length() const { return length; }

const char* CString::Str() const noexcept { return str; }

char* CString::GetStr() const noexcept { return str; }

CString& CString::Truncate(size_t index)
{
    PRINT_ERROR_RETURN_IF(index >= length, *this, "CString::Truncate input parameter error");
    length = index;
    str[index] = '\0';
    return *this;
}

CString& CString::Insert(size_t index, const char* addStr)
{
    PRINT_ERROR_RETURN_IF(index >= length || *addStr == '\0', *this, "CString::Insert input parameter error");
    CString subStr = SubStr(index);
    Truncate(index);
    Append(addStr);
    Append(subStr);
    return *this;
}

int CString::Find(const char* subStr, size_t begin) const
{
    PRINT_ERROR_RETURN_IF(begin >= length, -1, "CString::Find input parameter error");
    char* ret = strstr(str + begin, subStr);
    return ret != nullptr ? ret - str : -1;
}

int CString::Find(const char chr, size_t begin) const
{
    PRINT_ERROR_RETURN_IF(begin >= length, -1, "CString::Find input parameter error");
    char* ret = strchr(str + begin, chr);
    return ret != nullptr ? ret - str : -1;
}

int CString::RFind(const char* subStr) const
{
    int index = -1;
    int ret = Find(subStr);
    while (ret != -1) {
        index = ret;
        if (ret + strlen(subStr) == length) {
            break;
        }
        ret = Find(subStr, ret + strlen(subStr));
    }
    return index;
}

CString CString::SubStr(size_t index, size_t len) const
{
    CString newStr;
    PRINT_ERROR_RETURN_IF(index + len > length, newStr, "CString::SubStr input parameter error\n");
    newStr.EnsureSpace(len);
    PRINT_ERROR_RETURN_IF(memcpy_s(newStr.str, newStr.capacity, str + index, len) != EOK, newStr,
                          "CString::SubStr memcpy_s failed");
    newStr.length = len;
    newStr.str[newStr.length] = '\0';
    return newStr;
}

CString CString::SubStr(size_t index) const
{
    PRINT_ERROR_RETURN_IF(index >= length, CString(), "CString::SubStr input parameter error\n");
    return SubStr(index, length - index);
}

CString CString::RemoveBlankSpace() const
{
    size_t index = 0;
    CString noBlankSpaceStr(this->Str());
    if (length == 0) {
        return noBlankSpaceStr;
    }
    for (size_t i = 0; i < length; i++) {
        if (str[i] != ' ') {
            noBlankSpaceStr.str[index++] = str[i];
        }
    }
    noBlankSpaceStr.str[index] = '\0';
    noBlankSpaceStr.length = index;
    return noBlankSpaceStr;
}

void CString::Replace(size_t pos, CString cStr)
{
    size_t repLen = cStr.Length();
    PRINT_FATAL_IF(pos + repLen > length, "CString::Replace failed, input is too long");
    PRINT_FATAL_IF(memcpy_s(str + pos, repLen, cStr.Str(), repLen) != EOK, "CString::Replace memcpy_s failed");
}

#ifdef __OHOS__
void CString::ReplaceAll(CString replacement, CString target)
{
    int index = -1;
    int ret = Find(target.Str());
    while (ret != -1) {
        index = ret;
        Replace(index, replacement);
        ret = Find(target.Str(), ret + target.length);
    }
    return;
}
#endif

std::vector<CString> CString::Split(CString& source, char separator)
{
    std::vector<CString> tokens;
    const char s[2] = { separator, '\0' };
    char* tmpSave = nullptr;

    CString tmp = source;
    CString token = strtok_s(tmp.str, s, &tmpSave);
    while (!token.IsEmpty()) {
        tokens.push_back(token);
        token = strtok_s(nullptr, s, &tmpSave);
    }
    return tokens;
}

char* CString::BaseName(const CString& path) { return basename(path.str); }

CString CString::FormatString(const char* format, ...)
{
    constexpr size_t defaultBufferSize = 256;
    char buf[defaultBufferSize];

    va_list argList;
    va_start(argList, format);
    if (vsprintf_s(buf, sizeof(buf), format, argList) == -1) {
        return "invalid arguments for FormatString";
    }
    va_end(argList);

    return CString(buf);
}

size_t CString::ParseSizeFromEnv(const CString& env)
{
    size_t tempSize = 0;
    CString noBlankStr = env.RemoveBlankSpace();
    size_t len = noBlankStr.Length();
    if (len <= LAST_CHARACTERS_SIZE) {
        return 0;
    }
    // Split size and unit.
    CString num = noBlankStr.SubStr(0, len - 2);
    CString unit = noBlankStr.SubStr(len - 2, 2);

    if (IsPosNumber(num)) {
        tempSize = std::strtoul(num.Str(), nullptr, 0);
    } else {
        return 0;
    }

    unit.ToLowerCase();

    if (unit == "kb") {
        return tempSize;
    } else if (unit == "mb") {
        tempSize *= 1024; // unit: 1024 * 1KB = 1MB
    } else if (unit == "gb") {
        tempSize *= 1024UL * 1024; // unit: 1024 * 1024 * 1KB = 1GB
    } else {
        return 0;
    }
    return tempSize;
}

uint64_t CString::ParseTimeFromEnv(const CString& env)
{
    uint64_t tempTime = 0;
    CString noBlankStr = env.RemoveBlankSpace();
    size_t len = noBlankStr.Length();
    if (len <= 1) { // The last one characters are units, such as "s".
        return 0;
    }
    // Split size and unit.
    CString num = noBlankStr.SubStr(0, len - 1);
    CString unit = noBlankStr.SubStr(len - 1, 1);
    if (unit == "s" && IsPosNumber(num)) {
        tempTime = std::strtoul(num.Str(), nullptr, 0);
        tempTime *= 1000UL * 1000 * 1000; // unit: 1000 * 1000 * 1000 * 1ns= 1s
        return tempTime;
    }
    num = noBlankStr.SubStr(0, len - LAST_CHARACTERS_SIZE); // The last two characters are units, such as "ms".
    unit = noBlankStr.SubStr(len - LAST_CHARACTERS_SIZE, LAST_CHARACTERS_SIZE);
    if (IsPosNumber(num)) {
        tempTime = std::strtoul(num.Str(), nullptr, 0);
    } else {
        return 0;
    }
    unit.ToLowerCase();
    if (unit == "ns") {
        return tempTime;
    } else if (unit == "us") {
        tempTime *= 1000; // unit: 1000 *  1ns= 1us
    } else if (unit == "ms") {
        tempTime *= 1000 * 1000; // unit: 1000 * 1000 *  1ns= 1ms
    } else {
        return 0;
    }

    return tempTime;
}

int CString::ParseNumFromEnv(const CString& env)
{
    int temp = 0;
    CString noBlankStr = env.RemoveBlankSpace();
    if (IsNumber(noBlankStr)) {
        temp = std::atoi(noBlankStr.Str());
    } else {
        return 0;
    }

    return temp;
}

size_t CString::ParsePosNumFromEnv(const CString& env)
{
    size_t temp = 0;
    CString noBlankStr = env.RemoveBlankSpace();
    if (IsPosNumber(noBlankStr)) {
        temp = std::atoi(noBlankStr.Str());
    } else {
        return 0;
    }

    return temp;
}

double CString::ParsePosDecFromEnv(const CString& env)
{
    double temp = 0;
    CString noBlankStr = env.RemoveBlankSpace();
    if (IsPosDecimal(noBlankStr)) {
        temp = std::atof(noBlankStr.Str());
    } else {
        return 0;
    }
    return temp;
}

double CString::ParseValidFromEnv(const CString& env)
{
    double temp = -1;
    CString noBlankStr = env.RemoveBlankSpace();
    if (IsPosDecimal(noBlankStr)) {
        temp = std::atof(noBlankStr.Str());
    } else {
        return -1;
    }
    return temp;
}

// If `s` is "1" or "true" or "TRUE", return true.
// Otherwise return false.
bool CString::ParseFlagFromEnv(const CString& s) { return s == "1" || s == "true" || s == "TRUE"; }

bool CString::IsPosNumber(const CString& s)
{
    if (s.Length() == 0 || s == "+" || s == "0") {
        return false;
    }
    size_t i = 0;
    char it = s.Str()[i];
    if (it == '+') {
        i++;
    }
    for (; i < s.Length(); ++i) {
        it = s.Str()[i];
        if (it < '0' || it > '9') {
            return false;
        }
    }
    return true;
}

bool CString::IsPosDecimal(const CString& s)
{
    if (s.Length() == 0 || s == "+" || s == "0") {
        return false;
    }
    char* endptr;
    double number = std::strtod(s.str, &endptr);
    if (*endptr != '\0') {
        return false;
    }
    if (number > 0) {
        return true;
    }
    return false;
}

bool CString::IsNumber(const CString& s)
{
    if (s.Length() == 0) {
        return false;
    }
    size_t i = 0;
    char it = s.Str()[i];
    if (it == '-') {
        i++;
    }
    for (; i < s.Length(); ++i) {
        it = s.Str()[i];
        if (it < '0' || it > '9') {
            return false;
        }
    }
    return true;
}
} // namespace MapleRuntime
