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
#ifndef COMMON_COMPONENTS_BASE_C_STRING_H
#define COMMON_COMPONENTS_BASE_C_STRING_H

#include <cstdint>
#include <vector>
#include <cstdlib>

#include "common_interfaces/base/common.h"
#include <securec.h>

namespace common {
class CString {
public:
    CString();
    CString(const char* initStr);
    explicit CString(char c);
    explicit CString(int64_t number);
    explicit CString(int32_t number);
    explicit CString(uint64_t number);
    explicit CString(uint32_t number);
    CString(size_t number, char ch);
    CString(const CString& other);
    CString& operator=(const CString& other);
    ~CString();

    void EnsureSpace(size_t addLen);

    CString& Append(const char* addStr, size_t addLen = 0);
    CString& Append(const CString& addStr, size_t addLen = 0);

    template<typename T>
    CString& operator+=(const T addStr)
    {
        return Append(addStr);
    }

    CString Combine(const char* addStr) const;
    CString Combine(const CString& addStr) const;

    template<typename T>
    CString operator+(const T addStr) const
    {
        return Combine(addStr);
    }

    size_t Length() const;

    const char* Str() const noexcept;

    char* GetStr() const noexcept;

    bool IsEmpty() const { return length_ == 0; }

    CString& Truncate(size_t index);
    CString& Insert(size_t index, const char* addStr);

    int Find(const char* subStr, size_t begin = 0) const;
    int Find(const char subStr, size_t begin = 0) const;
    int RFind(const char* subStr) const;

    CString SubStr(size_t index, size_t len) const;
    CString SubStr(size_t index) const;

    bool EndsWith(const CString& suffix) const
    {
        return Length() >= suffix.Length() && SubStr(Length() - suffix.Length()) == suffix;
    }

    CString RemoveBlankSpace() const;

    void Replace(size_t pos, CString cStr);

    void ReplaceAll(CString replacement, CString target);

    bool operator==(const CString& other) const
    {
        DCHECK_CC(other.str_ != nullptr);
        return strcmp(str_, other.str_) == 0;
    }

    bool operator!=(const CString& other) const { return !(strcmp(str_, other.str_) == 0); }

    bool operator<(const CString& other) const { return strcmp(str_, other.str_) < 0; }

    const char& operator[](size_t index) const;

    char& operator[](size_t index);

    // Split a string into string tokens according to the separator, such as blank space
    static std::vector<CString> Split(CString& source, char separator = ' ');

    static CString Strip(CString& source, char separator = ' ');
    static char* BaseName(const CString& path);

    CString& ToLowerCase()
    {
        for (size_t i = 0; i < length_; ++i) {
            if (str_[i] >= 'A' && str_[i] <= 'Z') {
                str_[i] += ('a' - 'A');
            }
        }
        return *this;
    }

    // helpers for logging one line with no more than 256 characters
    static CString FormatString(const char* format, ...);

    static size_t ParseSizeFromEnv(const CString& env);
    static size_t ParseTimeFromEnv(const CString& env);
    static int ParseNumFromEnv(const CString& env);
    static size_t ParsePosNumFromEnv(const CString& env);
    static double ParsePosDecFromEnv(const CString& env);
    static bool ParseFlagFromEnv(const CString& s);
    static bool IsPosNumber(const CString& s);
    static bool IsPosDecimal(const CString& s);
    static bool IsNumber(const CString& s);
    static constexpr size_t C_STRING_MIN_SIZE = 16;

private:
    char* str_ = nullptr;
    size_t capacity_ = C_STRING_MIN_SIZE;
    size_t length_ = 0;
};
} // namespace common
#endif // COMMON_COMPONENTS_BASE_C_STRING_H
