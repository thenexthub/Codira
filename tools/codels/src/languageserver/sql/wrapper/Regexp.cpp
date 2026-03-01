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

#include "Regexp.h"
#include "Connection.h"

#include <optional>
#include <regex>
#include <string>
#include <unordered_map>

namespace sqldb {

static std::optional<bool> regexp(std::optional<std::string_view> Regex, std::optional<std::string_view> Text)
{
    thread_local std::unordered_map<std::string, std::regex> RegexCache;
    if (Regex && Text) {
        auto [ElemIt, Inserted] = RegexCache.try_emplace({Regex->begin(), Regex->end()}, Regex->begin(), Regex->end());
        return std::regex_search(Text->begin(), Text->end(), ElemIt->second);
    }
    return std::nullopt;
}

void registerRegexpFunction(Connection &DB) { DB.scalar("regexp", regexp); }

} // namespace sqldb
