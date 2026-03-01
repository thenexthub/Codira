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

#include <algorithm>
#include <regex>

#include "assert_util.h"
#include "logger.h"
#include "configuration/configuration_constants.h"

namespace {
constexpr std::string_view REGEX_NUMERIC_PLACEHOLDER = "<([0-9]+)>";
constexpr std::string_view REGEX_DOT_PLACEHOLDER = R"((\.+))";
constexpr int BRACE_SIZE = 2;

bool ContainRegexSymbol(const std::string &str)
{
    const std::string regexSymbols = R"(\.^$-+()[]{}|?*)";
    for (const auto &c : str) {
        if (regexSymbols.find(c) != std::string::npos) {
            return true;
        }
    }
    return false;
}

std::string EscapeParentheses(const std::string &str)
{
    std::string result;
    result.reserve(str.size() + BRACE_SIZE);
    for (const char c : str) {
        if (c == '{' || c == '}') {
            result += '\\';
        }
        result += c;
    }
    return result;
}

std::string RemoveSpaces(const std::string &str)
{
    std::string result = str;
    result.erase(std::remove(result.begin(), result.end(), ' '), result.end());
    return result;
}

std::string ReplaceDot(const std::string &str)
{
    std::smatch match;
    std::string result;

    auto searchStart = str.cbegin();

    while (std::regex_search(searchStart, str.cend(), match, std::regex(REGEX_DOT_PLACEHOLDER.data()))) {
        result.append(searchStart, match[0].first);
        if (match[1].str().size() == 1) {
            result.append("\\.");
        } else {
            result.append(match[1].str());
        }
        searchStart = match[0].second;
    }
    result.append(searchStart, str.cend());
    return result;
}

std::string ReplaceNumericWildcards(const std::string &str, const std::vector<std::string> &matchedPatterns)
{
    std::smatch match;
    std::string result;

    auto searchStart = str.cbegin();
    while (std::regex_search(searchStart, str.cend(), match, std::regex(REGEX_NUMERIC_PLACEHOLDER.data()))) {
        result.append(searchStart, match[0].first);
        auto matchNum = std::stoi(match[1].str());
        // CC-OFFNXT(G.STD.16) fatal error
        GUARD_ASSERT(
            matchNum <= 0, ark::guard::ErrorCode::CLASS_SPECIFICATION_FORMAT_ERROR,
            "ClassSpecification parsing failed: Back reference can not be 0, found invalid back reference" +
                match[0].str());
        size_t num = matchNum - 1;
        if (num < matchedPatterns.size()) {
            result.append(matchedPatterns[num]);
        }
        searchStart = match[0].second;
    }
    result.append(searchStart, str.cend());
    return result;
}

std::string ReplaceWildcardToRegex(const std::vector<std::pair<std::string, std::string>> &wildcardRules,
                                   const std::string &input)
{
    std::string result;
    const auto replaceDot = ReplaceDot(input);
    const std::string_view sv(replaceDot);
    size_t i = 0;
    std::vector<std::string> matchedPatterns;
    while (i < sv.size()) {
        bool matched = false;
        for (const auto &[pattern, replace] : wildcardRules) {
            const auto patternLen = pattern.size();
            if (i + patternLen <= sv.size() && sv.substr(i, patternLen) == pattern) {
                result += replace;
                matchedPatterns.emplace_back(replace);
                i += patternLen;
                matched = true;
                break;
            }
        }
        if (!matched) {
            result += sv[i];
            ++i;
        }
    }
    return ReplaceNumericWildcards(result, matchedPatterns);
}
}  // namespace

std::string ark::guard::StringUtil::ReplaceSlashWithDot(const std::string &str)
{
    std::string result = str;
    std::replace(result.begin(), result.end(), '/', '.');
    return result;
}

std::vector<std::string> ark::guard::StringUtil::ReplaceSlashWithDot(const std::vector<std::string> &strs)
{
    std::vector<std::string> result;
    result.reserve(strs.size());
    for (const auto &str : strs) {
        result.emplace_back(ReplaceSlashWithDot(str));
    }
    return result;
}

bool ark::guard::StringUtil::IsSubStrMatched(const std::string &str, const std::string &subStr, size_t start /* = 0 */)
{
    if (start > str.size() || start + subStr.size() > str.size()) {
        return false;
    }
    return str.compare(start, subStr.size(), subStr) == 0;
}

bool ark::guard::StringUtil::IsMatch(const std::string &inputStr, const std::string &pattern)
{
    if (pattern.empty()) {
        return false;
    }
    auto processedInput = RemoveSpaces(inputStr);
    auto processedPattern = RemoveSpaces(pattern);
    if (ContainRegexSymbol(processedPattern)) {
        std::string escapedPattern = EscapeParentheses(processedPattern);
        auto rc = regex_match(processedInput, std::regex(escapedPattern, std::regex_constants::ECMAScript));
        return rc;
    }
    return processedInput == processedPattern;
}

std::string ark::guard::StringUtil::ConvertWildcardToRegex(const std::string &input)
{
    static const std::vector<std::pair<std::string, std::string>> wildcardRules = {
        {"**", ".*"}, {"*", "[^/\\.]*"}, {"?", "[^/\\.]"}};
    return ReplaceWildcardToRegex(wildcardRules, input);
}

std::string ark::guard::StringUtil::ConvertWildcardToRegexEx(const std::string &input)
{
    static const std::vector<std::pair<std::string, std::string>> wildcardRules = {
        {"***", ".*"},
        {"...", ".*"},
        {"**", ".*"},
        {"*", "[^/\\.]*"},
        {"?", "[^/\\.]"},
        {"%",
         "(f32|f64|i8|i16|i32|i64|i32|i64|u16|u1|std\\.core\\.String|std\\.core\\.BigInt|void|std\\.core\\.Null|std\\."
         "core\\.Object|std\\.core\\.Int|std\\.core\\.Byte|std\\.core\\.Short|std\\.core\\.Long|std\\.core\\.Double|"
         "std\\.core\\.Float|std\\.core\\.Char|std\\.core\\.Boolean)"}};
    return ReplaceWildcardToRegex(wildcardRules, input);
}

std::string ark::guard::StringUtil::RemovePrefixIfMatches(const std::string &inputStr, const std::string &prefix)
{
    GUARD_ASSERT(prefix.empty(), ErrorCode::GENERIC_ERROR, "remove invalid prefix to: " + inputStr);
    std::string_view sv(inputStr);
    if (sv.size() >= prefix.size() && sv.substr(0, prefix.size()) == prefix) {
        return std::string(sv.substr(prefix.size()));
    }
    return inputStr;
}

std::string ark::guard::StringUtil::EnsureStartWithPrefix(const std::string &inputStr, const std::string &prefix)
{
    if (inputStr.length() < prefix.length() || inputStr.substr(0, prefix.size()) != prefix) {
        return prefix + inputStr;
    }
    return inputStr;
}

std::string ark::guard::StringUtil::EnsureEndWithSuffix(const std::string &inputStr, const std::string &suffix)
{
    if (inputStr.length() < suffix.length() || inputStr.substr(inputStr.length() - suffix.length()) != suffix) {
        return inputStr + suffix;
    }
    return inputStr;
}

size_t ark::guard::StringUtil::FindLongestMatchedPrefix(const std::vector<std::string> &prefixes,
                                                        const std::string &input)
{
    size_t maxLength = 0;
    for (const auto &prefix : prefixes) {
        if (prefix.size() > maxLength && IsSubStrMatched(input, prefix)) {
            maxLength = prefix.size();
        }
    }
    return maxLength;
}

size_t ark::guard::StringUtil::FindLongestMatchedPrefixReg(const std::vector<std::string> &prefixes,
                                                           const std::string &input)
{
    size_t maxLength = 0;
    std::smatch match;
    for (const auto &regexStr : prefixes) {
        std::regex pattern(regexStr, std::regex_constants::ECMAScript);
        if (std::regex_search(input, match, pattern)) {
            maxLength = std::max(maxLength, static_cast<size_t>(match[0].length()));
        }
    }
    return maxLength;
}

std::string ark::guard::StringUtil::RemoveAllSpaces(const std::string &input)
{
    std::string result = input;
    result.erase(std::remove_if(result.begin(), result.end(), ::isspace), result.end());
    return result;
}