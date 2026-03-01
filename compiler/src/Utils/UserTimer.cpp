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

#include "UserTimer.h"

#include <algorithm>
#include <utility>

#include "Codira/Utils/CheckUtils.h"

namespace Codira {

std::string UserTimer::GetJson() const
{
    auto [result, _] = GetDataAndOrder();
    std::string output;
    output += "{";
    for (auto& phrase : result) {
        output += ("\n   \"" + phrase.first + "\": {");
        for (auto& sec : phrase.second) {
            output += ("\n      \"" + sec.first + "\": " + std::to_string(sec.second) + ",");
        }
        if (!phrase.second.empty()) {
            output.pop_back();
        }
        output += "   \n    },";
    }
    if (!result.empty()) {
        output.pop_back();
    }
    output += "\n}\n";
    return output;
}

void UserTimer::Start(const std::string& title, const std::string& subtitle, const std::string& desc)
{
    auto infoIt = std::find_if(infoList.begin(), infoList.end(), [&title, &subtitle, &desc](const Info& it) {
        return it.title == title && it.subtitle == subtitle && it.desc == desc;
    });
    // The first time, create a new Info.
    if (infoIt == infoList.end()) {
        infoList.emplace_back(Info(title, subtitle, desc));
        return;
    }
    // Not the first time, must be done, reset the status.
    if ((*infoIt).isDone) {
#if defined(__APPLE__) || defined(__MINGW64__)
        (*infoIt).start = std::chrono::system_clock::now();
#else
        (*infoIt).start = std::chrono::high_resolution_clock::now();
#endif
        (*infoIt).isDone = false; // It's still not over.
    }
}

void UserTimer::Stop(const std::string& title, const std::string& subtitle, const std::string& desc)
{
    auto infoIt = std::find_if(infoList.begin(), infoList.end(), [&title, &subtitle, &desc](const Info& it) {
        return it.title == title && it.subtitle == subtitle && it.desc == desc;
    });
    CODEC_ASSERT(infoIt != infoList.end() && !(*infoIt).isDone);

#if defined(__APPLE__) || defined(__MINGW64__)
    (*infoIt).end = std::chrono::system_clock::now();
#else
    (*infoIt).end = std::chrono::high_resolution_clock::now();
#endif
    // The costMs has an initial value. Therefore, the costMs can be correctly processed regardless of whether merge
    // is required.
    (*infoIt).costMs = (*infoIt).costMs + ((*infoIt).end - (*infoIt).start);
    (*infoIt).isDone = true;
}

std::pair<UserTimer::ResultDataType, std::vector<std::string>> UserTimer::GetDataAndOrder() const
{
    ResultDataType result;
    std::vector<std::string> order; // Record the order of inserting keys in the map.
    for (const auto& info : std::as_const(infoList)) {
        if (!info.isDone) {
#ifdef _WIN32
            printf("[ %s ] only has beginning time, does not have ending time\n", info.title.c_str());
#else
            printf("[ \033[31;1m%s\033[0m ] only has beginning time, does not have ending time\n", info.title.c_str());
#endif
            continue;
        }
        std::vector<std::pair<std::string, long int>> ele;
        auto eleRet = result.emplace(info.title, ele);
        if (eleRet.second) {
            order.emplace_back(info.title);
        }
        eleRet.first->second.emplace_back(info.subtitle, static_cast<long int>(info.costMs.count()));
    }
    return std::make_pair(result, order);
}

} // namespace Codira
