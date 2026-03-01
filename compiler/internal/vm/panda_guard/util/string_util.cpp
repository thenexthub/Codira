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

#include "string_util.h"

#include <regex>

#include "utils/string_helpers.h"

using panda::helpers::string::Format;

namespace {
const std::regex ANONYMOUS_FUNCTION_NAME_REG = std::regex("\\^[\\da-f]+$");
const std::regex ANONYMOUS_NAMESPACE_NAME_REG = std::regex("=ens\\d+");

const char REGEX_DELIMITER = '/';
}  // namespace

std::vector<std::string> panda::guard::StringUtil::Split(const std::string &str, const std::string &delimiter)
{
    std::vector<std::string> tokens;
    auto start = str.find_first_not_of(delimiter, 0);
    auto pos = str.find_first_of(delimiter, 0);
    while (start != std::string::npos) {
        if (pos > start) {
            tokens.push_back(str.substr(start, pos - start));
        }
        start = str.find_first_not_of(delimiter, pos);
        pos = str.find_first_of(delimiter, start + 1);
    }

    return tokens;
}

std::vector<std::string> panda::guard::StringUtil::StrictSplit(const std::string &str, const std::string &delimiter)
{
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end;
    while (start <= str.size()) {
        end = str.find_first_of(delimiter, start);
        if (end == std::string::npos) {
            end = str.size();
        }

        if (start < end) {
            tokens.push_back(str.substr(start, end - start));
        } else {
            tokens.emplace_back("");
        }

        start = end + 1;
    }
    return tokens;
}

std::tuple<std::string, std::string> panda::guard::StringUtil::RSplitOnce(const std::string &str,
                                                                          const std::string &delimiter)
{
    auto pos = str.rfind(delimiter);
    if (pos == std::string::npos) {
        return std::make_tuple("", "");
    }

    return std::make_tuple(str.substr(0, pos), str.substr(pos + delimiter.length()));
}

bool panda::guard::StringUtil::IsAnonymousFunctionName(const std::string &functionName)
{
    if (functionName.empty()) {
        return true;
    }

    if (functionName[0] != '^') {
        return false;
    }
    return std::regex_search(functionName, ANONYMOUS_FUNCTION_NAME_REG);
}

bool panda::guard::StringUtil::IsAnonymousNameSpaceName(const std::string &name)
{
    if (name.empty()) {
        return false;
    }

    return std::regex_search(name, ANONYMOUS_NAMESPACE_NAME_REG);
}

bool panda::guard::StringUtil::IsPrefixMatched(const std::string &str, const std::string &prefix,
                                               size_t start /* = 0 */)
{
    if (start > str.size() || start + prefix.size() > str.size()) {
        return false;
    }
    return str.compare(start, prefix.size(), prefix) == 0;
}

bool panda::guard::StringUtil::IsSuffixMatched(const std::string &str, const std::string &suffix)
{
    if (str.size() < suffix.size()) {
        return false;
    }
    return StringUtil::IsPrefixMatched(str, suffix, str.size() - suffix.size());
}

std::string panda::guard::StringUtil::UnicodeEscape(std::string_view string)
{
    std::string out;
    while (!string.empty()) {
        auto iter =
            std::find_if(string.begin(), string.end(), [](char ch) { return ch == '"' || ch == '\\' || ch < ' '; });
        auto pos = iter - string.begin();
        out += string.substr(0, pos);
        if (iter == string.end()) {
            break;
        }

        out += '\\';
        switch (*iter) {
            case '"':
            case '\\':
                out += *iter;
                break;
            case '\b':
                out += 'b';
                break;
            case '\f':
                out += 'f';
                break;
            case '\n':
                out += 'n';
                break;
            case '\r':
                out += 'r';
                break;
            case '\t':
                out += 't';
                break;
            default:
                out += Format("u%04X", *iter);  // NOLINT(cppcoreguidelines-pro-type-vararg)
        }
        string.remove_prefix(pos + 1);
    }
    return out;
}

void panda::guard::StringUtil::RemoveSlashFromBothEnds(std::string &str)
{
    if (!str.empty() && str.front() == REGEX_DELIMITER && str.back() == REGEX_DELIMITER) {
        str.erase(str.begin());
        str.pop_back();
    }
}

bool panda::guard::StringUtil::IsNumber(const std::string &str)
{
    return std::all_of(str.begin(), str.end(), [](const char &ch) {
        if ((ch >= '0') && (ch <= '9')) {
            return true;
        }
        return false;
    });
}

std::tuple<std::string, std::string> panda::guard::StringUtil::SplitAnonymousName(const std::string &str)
{
    std::smatch match;
    if (std::regex_search(str.begin(), str.end(), match, ANONYMOUS_FUNCTION_NAME_REG)) {
        std::string suffix = match[0].str();
        std::string prefix = str.substr(0, str.find(suffix));
        return std::make_tuple(prefix, suffix);
    }
    return std::make_tuple(str, "");
}