/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "regexp_8.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define PCRE2_CODE_UNIT_WIDTH 8
#include "pcre2.h"

#include "plugins/ets/runtime/ets_exceptions.h"

#include <utility>

namespace ark::ets::stdlib {

constexpr int PCRE2_MATCH_DATA_UNIT_WIDTH = 2;
constexpr int PCRE2_CHARACTER_WIDTH = 1;
constexpr int PCRE2_GROUPS_NAME_ENTRY_SHIFT = 3;

Pcre2Obj RegExp8::CreatePcre2Object(const uint8_t *pattern, uint32_t flags, uint32_t extraFlags, const int len)
{
    int errorNumber;
    PCRE2_SPTR patternStr = static_cast<PCRE2_SPTR>(pattern);
    PCRE2_SIZE errorOffset;
    auto *compileContext = pcre2_compile_context_create(nullptr);
    pcre2_set_compile_extra_options(compileContext, extraFlags);
    pcre2_set_newline(compileContext, PCRE2_NEWLINE_ANY);
    auto re = pcre2_compile(patternStr, len, flags, &errorNumber, &errorOffset, compileContext);
    pcre2_compile_context_free(compileContext);
    return reinterpret_cast<Pcre2Obj>(re);
}

RegExpExecResult RegExp8::Execute(Pcre2Obj re, uint32_t matchFlags, const uint8_t *str, const int len,
                                  const int startOffset)
{
    auto *expr = reinterpret_cast<pcre2_code *>(re);
    auto *matchData = pcre2_match_data_create_from_pattern(expr, nullptr);
    std::vector<std::pair<int32_t, int32_t>> indices;
    auto resultCount = pcre2_match(expr, str, len, startOffset, matchFlags, matchData, nullptr);
    auto *ovector = pcre2_get_ovector_pointer(matchData);
    RegExpExecResult result;
    result.isWide = false;
    if (resultCount < 0) {
        result.isSuccess = false;
        pcre2_match_data_free(matchData);
        return result;
    }

    const auto lastIndex = resultCount * PCRE2_MATCH_DATA_UNIT_WIDTH;
    for (decltype(resultCount) i = 0; i < lastIndex; i += PCRE2_MATCH_DATA_UNIT_WIDTH) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto substringStart = ovector[i];
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto substringEnd = ovector[i + 1];
        indices.emplace_back(std::make_pair(substringStart, substringEnd));
    }

    int nameCount;
    pcre2_pattern_info(expr, PCRE2_INFO_NAMECOUNT, &nameCount);

    if (nameCount > 0) {
        RegExp8::ExtractGroups(re, nameCount, result, reinterpret_cast<void *>(ovector));
    }

    result.isSuccess = true;
    result.indices = std::move(indices);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    result.index = ovector[0];
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    result.endIndex = ovector[1];
    auto groupCount = static_cast<size_t>(pcre2_get_ovector_count(matchData));
    while (result.indices.size() < groupCount) {
        result.indices.emplace_back(std::make_pair(-1, -1));
    }
    pcre2_match_data_free(matchData);
    return result;
}

void RegExp8::ExtractGroups(Pcre2Obj expression, int count, RegExpExecResult &result, void *data)
{
    PCRE2_SPTR nameTable;
    PCRE2_SPTR tabPtr;
    int nameEntrySize;

    auto *expr = reinterpret_cast<pcre2_code *>(expression);
    auto *ovector = reinterpret_cast<PCRE2_SIZE *>(data);

    pcre2_pattern_info(expr, PCRE2_INFO_NAMETABLE, &nameTable);
    pcre2_pattern_info(expr, PCRE2_INFO_NAMEENTRYSIZE, &nameEntrySize);

    tabPtr = nameTable;
    for (int i = 0; i < count; i++) {
        auto n = static_cast<int32_t>(static_cast<PCRE2_UCHAR8>(tabPtr[0] << 8U) | tabPtr[1]);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto index = static_cast<int32_t>(ovector[2 * n]);
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        auto endIndex = static_cast<int32_t>(ovector[2 * n + 1]);
        auto tabConstCharPtr = reinterpret_cast<const char *>(tabPtr + 2);
        size_t size = nameEntrySize - PCRE2_GROUPS_NAME_ENTRY_SHIFT;
        while (size > 0) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (static_cast<uint8_t>(*(tabConstCharPtr + size - PCRE2_CHARACTER_WIDTH)) != 0) {
                break;
            }
            size -= PCRE2_CHARACTER_WIDTH;
        }
        auto key = std::string(reinterpret_cast<const char *>(tabPtr + 2), size);
        result.namedGroups[key] = {index, endIndex};
        tabPtr += nameEntrySize;
    }
}

void RegExp8::FreePcre2Object(Pcre2Obj re)
{
    pcre2_code_free(reinterpret_cast<pcre2_code *>(re));
}

bool RegExp8::IsUncountable(const uint8_t *pattern, const size_t len, size_t index)
{
    uint8_t next = '\0';
    if (index < len - 1U) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        next = static_cast<uint8_t>(pattern[index + 1U]);
    }
    uint8_t next2 = '\0';
    if (index < len - 2U) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        next2 = static_cast<uint8_t>(pattern[index + 2U]);
    }
    uint8_t next3 = '\0';
    if (index < len - 3U) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        next3 = static_cast<uint8_t>(pattern[index + 3U]);
    }
    const bool isLookahead = next == '?' && (next2 == '=' || next2 == '!');
    const bool isLookbehind = next == '?' && next2 == '<' && (next3 == '=' || next3 == '!');
    const bool isAtomicOrNonCapturing = next == '?' && (next2 == ':' || next2 == '>');
    return isLookahead || isLookbehind || isAtomicOrNonCapturing;
}

void RegExp8::SanitizeGroupCaptureResults(const std::vector<bool> &countableGroups,
                                          const std::map<size_t, size_t> &parentGroups, RegExpExecResult &result)
{
    auto groupId = 0U;
    auto countedGroupId = groupId;
    std::map<size_t, size_t> groupToCountableGroup;
    for (size_t i = 1U; i < countableGroups.size(); i++) {
        groupId++;
        if (countableGroups[i]) {
            countedGroupId++;
            groupToCountableGroup[groupId] = countedGroupId;
        }
    }

    for (const auto [childIndex, parentIndex] : parentGroups) {
        if (childIndex >= countableGroups.size() || parentIndex >= countableGroups.size()) {
            continue;
        }
        if (!countableGroups[childIndex] || !countableGroups[parentIndex]) {
            continue;
        }
        const auto countedChildIndex = groupToCountableGroup[childIndex];
        const auto countedResultIndex = groupToCountableGroup[parentIndex];
        if (countedChildIndex >= result.indices.size() || countedResultIndex >= result.indices.size()) {
            continue;
        }
        const auto &child = result.indices[countedChildIndex];
        const auto &parent = result.indices[countedResultIndex];
        if (child.first < parent.first || child.second > parent.second) {
            result.indices[countedChildIndex].first = -1;
            result.indices[countedChildIndex].second = -1;
        }
    }
}

void RegExp8::EraseExtraGroups(const uint8_t *pattern, const size_t len, RegExpExecResult &result)
{
    if (result.indices.size() < 2U) {
        return;
    }

    // define that "group" is any sequence starting with '(' and ending with ')'
    // while "countable/counted group" is any capturing group
    size_t groupId = 0;
    std::vector<size_t> currentGroups;
    std::map<size_t, size_t> parentGroups;
    std::vector<bool> countableGroups = {false};
    uint8_t prev = '\0';
    uint8_t prev2 = '\0';
    bool inClass = false;
    for (size_t i = 0U; i < len; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const uint8_t &cur = pattern[i];
        const bool notSupressed = prev != '\\' || prev2 == '\\';
        if (cur == '(' && notSupressed && !inClass) {
            groupId++;
            countableGroups.push_back(!IsUncountable(pattern, len, i));
            currentGroups.push_back(groupId);
            if (currentGroups.size() > 1) {
                parentGroups[groupId] = currentGroups[currentGroups.size() - 2U];
            }
        } else if (cur == ')' && notSupressed && !inClass) {
            if (!currentGroups.empty()) {
                currentGroups.pop_back();
            }
        } else if (cur == '[' && notSupressed) {
            inClass = true;
        } else if (cur == ']' && notSupressed) {
            inClass = false;
        }
        prev2 = prev;
        prev = cur;
    }
    SanitizeGroupCaptureResults(countableGroups, parentGroups, result);
}

}  // namespace ark::ets::stdlib
