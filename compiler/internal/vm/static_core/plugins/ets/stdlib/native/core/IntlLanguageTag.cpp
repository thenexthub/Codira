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

#include "IntlLanguageTag.h"
#include "libarkbase/macros.h"

#include <algorithm>
#include <locale>
#include <sstream>
#include <string_view>

namespace ark::ets::stdlib::intl {

// CC-OFFNXT(G.NAM.03-CPP) project code style
static bool IsAlpha(std::string_view str)
{
    return std::all_of(str.begin(), str.end(), [](auto ch) { return std::isalpha(ch, std::locale::classic()); });
}

static bool IsNum(std::string_view str)
{
    return std::all_of(str.begin(), str.end(), [](auto ch) { return std::isdigit(ch, std::locale::classic()); });
}

static bool IsAlphaNum(std::string_view str)
{
    return std::all_of(str.begin(), str.end(), [](auto ch) {
        return std::isalpha(ch, std::locale::classic()) || std::isdigit(ch, std::locale::classic());
    });
}

static bool IsVariant(std::string_view subtag)
{
    auto size = subtag.size();
    if (size == INTL_INDEX_FOUR) {
        return IsNum(subtag.substr(0, 1)) && IsAlphaNum(subtag.substr(1));
    }
    if (INTL_INDEX_FOUR + 1 <= size && size <= INTL_INDEX_EIGHT) {
        return IsAlphaNum(subtag);
    }
    return false;
}

static bool IsRegion(std::string_view subtag)
{
    if (subtag.size() == INTL_INDEX_TWO) {
        return IsAlpha(subtag);
    }
    if (subtag.size() == INTL_INDEX_THREE) {
        return IsNum(subtag);
    }
    return false;
}

static bool IsScript(std::string_view subtag)
{
    if (subtag.size() == INTL_INDEX_FOUR) {
        return IsAlpha(subtag);
    }
    return false;
}

static bool IsExtension(std::string_view subtag)
{
    if (subtag.size() == 1) {
        return IsAlphaNum(subtag);
    }
    return false;
}

static bool IsLanguage(std::string_view subtag)
{
    auto size = subtag.size();
    if ((INTL_INDEX_TWO <= size && size <= INTL_INDEX_THREE) || (INTL_INDEX_FIVE <= size && size <= INTL_INDEX_EIGHT)) {
        return IsAlpha(subtag);
    }
    return false;
}

bool IsPrivateSubTag(const std::string &result, size_t &len)
{
    if ((len > 1) && (result[1] == '-')) {
        ASSERT(result[0] == 'x' || result[0] == 'i');
        return true;
    }
    return false;
}

class SplitView {
public:
    class Iterator {
    public:
        Iterator(std::string_view input, char delimiter, size_t pos) : input_(input), delimiter_(delimiter), pos_(pos)
        {
            next_ = input_.find(delimiter_, pos_);
            if (next_ == std::string_view::npos) {
                current_ = input_.substr(pos_);
            } else {
                current_ = input_.substr(pos_, next_ - pos_);
            }
        }

        std::string_view operator*() const
        {
            return current_;
        }

        Iterator &operator++()
        {
            pos_ = next_ == std::string_view::npos ? input_.size() : next_ + 1;
            if (pos_ < input_.size()) {
                next_ = input_.find(delimiter_, pos_);
                current_ = input_.substr(pos_, next_ == std::string_view::npos ? std::string_view::npos : next_ - pos_);
            } else {
                current_ = std::string_view();
                next_ = std::string_view::npos;
            }
            return *this;
        }

        bool operator==(const Iterator &other) const
        {
            return pos_ == other.pos_ && input_ == other.input_;
        }

        bool operator!=(const Iterator &other) const
        {
            return !(*this == other);
        }

    private:
        std::string_view input_;
        char delimiter_;
        size_t pos_;
        size_t next_;
        std::string_view current_;
    };

    SplitView(std::string_view input, char delimiter) : input_(input), delimiter_(delimiter) {}

    Iterator Begin() const
    {
        return Iterator(input_, delimiter_, 0);
    }

    Iterator End() const
    {
        return Iterator(input_, delimiter_, input_.size());
    }

private:
    std::string_view input_;
    char delimiter_;
};

bool IsStructurallyValidLanguageTag(const std::string &tag)
{
    auto subtags = SplitView(tag, '-');
    auto it = subtags.Begin();
    auto end = subtags.End();
    if (it == end) {
        return true;
    }
    if (!IsLanguage(*it)) {
        return false;
    }
    ++it;
    if (it == end) {
        return true;
    }
    if (IsExtension(*it)) {
        return true;
    }
    if (IsScript(*it)) {
        ++it;
        if (it == end) {
            return true;
        }
    }
    if (IsRegion(*it)) {
        ++it;
    }
    for (; it != end; ++it) {
        if (IsExtension(*it)) {
            return true;
        }
        if (!IsVariant(*it)) {
            return false;
        }
    }
    return true;
}

std::string ToStdStringLanguageTag(const icu::Locale &locale)
{
    UErrorCode status = U_ZERO_ERROR;
    auto result = locale.toLanguageTag<std::string>(status);
    if (U_FAILURE(status) != 0) {
        return "";
    }
    size_t findBeginning = result.find("-u-");
    std::string finalRes;
    std::string tempRes;
    if (findBeginning == std::string::npos) {
        return result;
    }
    size_t specialBeginning = findBeginning + INTL_INDEX_THREE;
    size_t specialCount = 0;
    while ((specialBeginning < result.size()) && (result[specialBeginning] != '-')) {
        specialCount++;
        specialBeginning++;
    }
    if (findBeginning != std::string::npos) {
        // It begin with "-u-xx" or with more elements.
        tempRes = result.substr(0, findBeginning + INTL_INDEX_THREE + specialCount);
        if (result.size() <= findBeginning + INTL_INDEX_THREE + specialCount) {
            return result;
        }
        std::string leftStr = result.substr(findBeginning + INTL_INDEX_THREE + specialCount + 1);
        std::istringstream temp(leftStr);
        std::string buffer;
        std::vector<std::string> resContainer;
        while (getline(temp, buffer, '-')) {
            if (buffer != "true" && buffer != "yes") {
                resContainer.push_back(buffer);
            }
        }
        for (auto &it : resContainer) {
            std::string tag = "-";
            tag += it;
            finalRes += tag;
        }
    }
    if (!finalRes.empty()) {
        tempRes += finalRes;
    }
    result = tempRes;
    return result;
}

}  // namespace ark::ets::stdlib::intl