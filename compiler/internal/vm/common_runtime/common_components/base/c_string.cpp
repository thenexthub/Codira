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
#include "common_components/base/c_string.h"
#include "common_components/log/log.h"

#include <libgen.h>

namespace common {
// The last two characters are units, such as "kb" or "ms".
constexpr int8_t LAST_CHARACTERS_SIZE = 2;
// nDecimal = ceil(sizeof(int32_t) * 8 * log(2)) = ceil(2.41 * sizeof(int32_t))
// consider overhead for sign & '\0' and alignment, replace 2.41 by 4
constexpr int8_t MIN_ALIGN_SPACE = 4;

CString::CString()
{
    str_ = reinterpret_cast<char*>(malloc(capacity_));
    LOGF_IF(str_ == nullptr) << "CString::Init failed";
    *str_ = '\0';
    length_ = 0;
}

CString::CString(const char* initStr)
{
    if (initStr != nullptr) {
        size_t initLen = strlen(initStr);
        while (capacity_ < initLen + 1) {
            capacity_ <<= 1;
        }
        str_ = reinterpret_cast<char*>(malloc(capacity_));
        LOGF_IF(str_ == nullptr) << "CString::Init failed";
        if (*initStr != '\0') {
            LOGF_IF(memcpy_s(str_, capacity_, initStr, initLen) != EOK) << "CString::CString memcpy_s failed";
        }
        length_ = initLen;
        str_[length_] = '\0';
    }
}

CString::CString(char c)
{
    str_ = reinterpret_cast<char*>(malloc(capacity_));
    LOGF_IF(str_ == nullptr) << "CString::Init failed";
    str_[0] = c;
    str_[1] = '\0';
    length_ = 1;
}

// nDecimal = ceil(sizeof(int32_t) * 8 * log(2)) = ceil(2.41 * sizeof(int32_t))
// consider overhead for sign & '\0' and alignment, replace 2.41 by 4
CString::CString(int32_t number)
{
    capacity_ = sizeof(int32_t) * MIN_ALIGN_SPACE;
    str_ = reinterpret_cast<char*>(malloc(capacity_));
    LOGF_IF(str_ == nullptr) << "CString::Init failed";
    int ret = sprintf_s(str_, capacity_, "%d", number);
    LOGF_IF(ret == -1) << "CString::Init failed";
    length_ = static_cast<size_t>(ret);
}

CString::CString(int64_t number)
{
    capacity_ = sizeof(int64_t) * MIN_ALIGN_SPACE;
    str_ = reinterpret_cast<char*>(malloc(capacity_));
    LOGF_IF(str_ == nullptr) << "CString::Init failed";
    int ret = sprintf_s(str_, capacity_, "%lld", number);
    LOGF_IF(ret == -1) << "CString::Init failed";
    length_ = static_cast<size_t>(ret);
}

CString::CString(uint32_t number)
{
    capacity_ = sizeof(uint32_t) * MIN_ALIGN_SPACE;
    str_ = reinterpret_cast<char*>(malloc(capacity_));
    LOGF_IF(str_ == nullptr) << "CString::Init failed";
    int ret = sprintf_s(str_, capacity_, "%u", number);
    LOGF_IF(ret == -1) << "CString::Init failed";
    length_ = static_cast<size_t>(ret);
}

CString::CString(uint64_t number)
{
    capacity_ = sizeof(uint64_t) * MIN_ALIGN_SPACE;
    str_ = reinterpret_cast<char*>(malloc(capacity_));
    LOGF_IF(str_ == nullptr) << "CString::Init failed";
    int ret = sprintf_s(str_, capacity_, "%lu", number);
    LOGF_IF(ret == -1) << "CString::Init failed";
    length_ = static_cast<size_t>(ret);
}

CString::CString(const CString& other)
{
    size_t initLen = other.Length();
    while (capacity_ < initLen + 1) {
        capacity_ <<= 1;
    }
    str_ = reinterpret_cast<char*>(malloc(capacity_));
    LOGF_IF(str_ == nullptr) << "CString::Init failed";
    if (!other.IsEmpty()) {
        LOGF_IF(memcpy_s(str_, capacity_, other.Str(), initLen) != EOK) << "CString::CString memcpy_s failed";
    }
    length_ = initLen;
    str_[length_] = '\0';
}

CString::CString(size_t number, char ch)
{
    while (capacity_ < number + 1) {
        capacity_ <<= 1;
    }
    str_ = reinterpret_cast<char*>(malloc(capacity_));
    LOGF_IF(str_ == nullptr) << "CString::Init failed";
    LOGF_IF(memset_s(str_, capacity_, ch, number) != EOK) << "CString::CString memset_s failed";
    length_ = number;
    str_[length_] = '\0';
}

CString& CString::operator=(const CString& other)
{
    if (this == &other) {
        return *this;
    }
    size_t initLen = other.Length();
    while (capacity_ < initLen + 1) {
        capacity_ <<= 1;
    }
    if (str_ != nullptr) {
        free(str_);
    }
    str_ = reinterpret_cast<char*>(malloc(capacity_));
    LOGF_IF(str_ == nullptr) << "CString::operator= malloc failed";
    if (!other.IsEmpty()) {
        LOGF_IF(memcpy_s(str_, capacity_, other.Str(), initLen) != EOK) << "CString::operator= memcpy_s failed";
    }
    length_ = initLen;
    str_[length_] = '\0';
    return *this;
}

CString::~CString()
{
    if (str_ != nullptr) {
        free(str_);
        str_ = nullptr;
    }
}

const char& CString::operator[](size_t index) const
{
    if (index >= length_) {
        LOG_COMMON(FATAL) << "CString[index] failed index=" << index;
    }
    return str_[index];
}

char& CString::operator[](size_t index)
{
    if (index >= length_) {
        LOG_COMMON(FATAL) << "CString[index] failed index=" << index;
    }
    return str_[index];
}

void CString::EnsureSpace(size_t addLen)
{
    if (addLen == 0 || capacity_ >= length_ + addLen + 1) {
        return;
    }
    while (capacity_ < length_ + addLen + 1) {
        capacity_ <<= 1;
    }
    char* newStr = reinterpret_cast<char*>(malloc(capacity_));
    LOGF_IF(newStr == nullptr) << "CString::Init failed";
    LOGF_IF(memcpy_s(newStr, capacity_, str_, length_) != EOK) << "CString::EnsureSpace memcpy_s failed";
    if (str_ != nullptr) {
        free(str_);
    }
    str_ = newStr;
}

CString& CString::Append(const CString& addStr, size_t addLen)
{
    if (addStr.IsEmpty()) {
        return *this;
    }
    if (addLen == 0) {
        addLen = strlen(addStr.str_);
    }
    EnsureSpace(addLen);
    DCHECK_CC(addLen <= addStr.length_);
    LOGF_IF(memcpy_s(str_ + length_, capacity_ - length_, addStr.str_, addLen) != EOK) <<
        "CString::Append memcpy_s failed";
    length_ += addLen;
    DCHECK_CC(str_ != nullptr);
    str_[length_] = '\0';
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
    DCHECK_CC(addLen <= strlen(addStr));
    LOGF_IF(memcpy_s(str_ + length_, capacity_ - length_, addStr, addLen) != EOK) <<
        "CString::Append memcpy_s failed";
    length_ += addLen;
    str_[length_] = '\0';
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

size_t CString::Length() const { return length_; }

const char* CString::Str() const noexcept { return str_; }

char* CString::GetStr() const noexcept { return str_; }

CString& CString::Truncate(size_t index)
{
    if (index >= length_) {
        LOG_COMMON(ERROR) << "CString::Truncate input parameter error";
        return *this;
    }
    length_ = index;
    str_[index] = '\0';
    return *this;
}

CString& CString::Insert(size_t index, const char* addStr)
{
    if (index >= length_ || *addStr == '\0') {
        LOG_COMMON(ERROR) << "CString::Insert input parameter error";
        return *this;
    }
    CString subStr = SubStr(index);
    Truncate(index);
    Append(addStr);
    Append(subStr);
    return *this;
}

int CString::Find(const char* subStr, size_t begin) const
{
    if (begin >= length_) {
        LOG_COMMON(ERROR) << "CString::Find input parameter error";
        return -1;
    }
    char* ret = strstr(str_ + begin, subStr);
    return ret != nullptr ? ret - str_ : -1;
}

int CString::Find(const char subStr, size_t begin) const
{
    if (begin >= length_) {
        LOG_COMMON(ERROR) << "CString::Find input parameter error";
        return -1;
    }
    char* ret = strchr(str_ + begin, subStr);
    return ret != nullptr ? ret - str_ : -1;
}

int CString::RFind(const char* subStr) const
{
    int index = -1;
    int ret = Find(subStr);
    while (ret != -1) {
        index = ret;
        if (ret + strlen(subStr) == length_) {
            break;
        }
        ret = Find(subStr, ret + strlen(subStr));
    }
    return index;
}

CString CString::SubStr(size_t index, size_t len) const
{
    CString newStr;
    if (index + len > length_) {
        LOG_COMMON(ERROR) << "CString::SubStr input parameter error\n";
        return newStr;
    }
    newStr.EnsureSpace(len);
    if (memcpy_s(newStr.str_, newStr.capacity_, str_ + index, len) != EOK) {
        LOG_COMMON(ERROR) << "CString::SubStr memcpy_s failed";
        return newStr;
    }
    newStr.length_ = len;
    DCHECK_CC(newStr.str_ != nullptr);
    newStr.str_[newStr.length_] = '\0';
    return newStr;
}

CString CString::SubStr(size_t index) const
{
    if (index >= length_) {
        LOG_COMMON(ERROR) << "CString::SubStr input parameter error\n";
        return CString();
    }
    return SubStr(index, length_ - index);
}

std::vector<CString> CString::Split(CString& source, char separator)
{
    std::vector<CString> tokens;
    const char s[2] = { separator, '\0' };
    char* tmpSave = nullptr;

    CString tmp = source;
    CString token = strtok_s(tmp.str_, s, &tmpSave);
    while (!token.IsEmpty()) {
        tokens.push_back(token);
        token = strtok_s(nullptr, s, &tmpSave);
    }
    return tokens;
}

char* CString::BaseName(const CString& path) { return basename(path.str_); }

// helpers for logging one line with no more than 256 characters
CString CString::FormatString(const char* format, ...)
{
    constexpr size_t defaultBufferSize = 256;
    char buf[defaultBufferSize];

    va_list argList;
    va_start(argList, format);
    if (vsprintf_s(buf, sizeof(buf), format, argList) == -1) {
        va_end(argList);
        return "invalid arguments for FormatString";
    }
    va_end(argList);

    return CString(buf);
}

// Check whether `s` can convert to a positive number.
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
    double number = std::strtod(s.str_, &endptr);
    if (*endptr != '\0') {
        return false;
    }
    if (number > 0) {
        return true;
    }
    return false;
}
// Check whether `s` can convert to a number.
bool CString::IsNumber(const CString& s)
{
    if (s.Length() == 0) {
        return false;
    }
    size_t i = 0;
    char it = s.Str()[i];
    if (it == '-') {
        if (s.Length() == 1) {
            return false;
        }
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

// If `s` is "1" or "true" or "TRUE", return true.
// Otherwise return false.
bool CString::ParseFlagFromEnv(const CString& s) { return s == "1" || s == "true" || s == "TRUE"; }

CString CString::RemoveBlankSpace() const
{
    size_t index = 0;
    CString noBlankSpaceStr(this->Str());
    if (length_ == 0) {
        return noBlankSpaceStr;
    }
    DCHECK_CC(noBlankSpaceStr.str_ != nullptr);
    for (size_t i = 0; i < length_; i++) {
        if (str_[i] != ' ') {
            noBlankSpaceStr.str_[index++] = str_[i];
        }
    }
    noBlankSpaceStr.str_[index] = '\0';
    noBlankSpaceStr.length_ = index;
    return noBlankSpaceStr;
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

size_t CString::ParseTimeFromEnv(const CString& env)
{
    size_t tempTime = 0;
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
        temp = static_cast<size_t>(std::atoi(noBlankStr.Str()));
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

void CString::Replace(size_t pos, CString cStr)
{
    size_t repLen = cStr.Length();
    LOGF_IF(pos + repLen > length_) << "CString::Replace failed, input is too long";
    LOGF_IF(memcpy_s(str_ + pos, repLen, cStr.Str(), repLen) != EOK) << "CString::Replace memcpy_s failed";
}

void CString::ReplaceAll(CString replacement, CString target)
{
    if (replacement.Length() == 0 || target.Length() == 0) {
        return;
    }
    int index = -1;
    int ret = Find(target.Str());
    while (ret != -1) {
        index = ret;
        Replace(index, replacement);
        ret = Find(target.Str(), ret + target.length_);
    }
    return;
}

} // namespace common
