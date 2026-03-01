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

#include "tooling/dynamic/client/manager/breakpoint_manager.h"

#include "tooling/dynamic/client/session/session.h"

using PtJson = panda::ecmascript::tooling::PtJson;
using Result = panda::ecmascript::tooling::Result;
namespace OHOS::ArkCompiler::Toolchain {
void BreakPointManager::Createbreaklocation(const std::unique_ptr<PtJson> json)
{
    if (json == nullptr) {
        LOGE("arkdb: json parse error");
        return;
    }

    if (!json->IsObject()) {
        LOGE("arkdb: json parse format error");
        json->ReleaseRoot();
        return;
    }
    Result ret;
    std::unique_ptr<PtJson> result;
    ret = json->GetObject("result", &result);
    if (ret != Result::SUCCESS) {
        LOGE("arkdb: find result error");
        return;
    }
    std::string breakpointId;
    ret = result->GetString("breakpointId", &breakpointId);
    if (ret == Result::SUCCESS) {
        Breaklocation breaklocation;
        breaklocation.breakpointId = breakpointId;
        std::vector<std::string> breaksplitstring;
        breaksplitstring = Utils::SplitString(breakpointId, ":");
        breaklocation.lineNumber = breaksplitstring[1]; // 1: linenumber
        breaklocation.columnNumber = breaksplitstring[2]; // 2: columnnumber
        breaklocation.url = breaksplitstring[3]; // 3: url
        breaklist_.push_back(breaklocation);
    } else {
        LOGE("arkdb: find breakpointId error");
        return;
    }
}

void BreakPointManager::Show()
{
    size_t size = breaklist_.size();
    for (size_t i = 0; i < size; i++) {
        std::cout << (i + 1) << ':' << " url:" << breaklist_[i].url;
        std::cout << " lineNumber:" << (std::atoi(breaklist_[i].lineNumber.c_str()) + 1)
            << " columnNumber:" << breaklist_[i].columnNumber << std::endl;
    }
}

void BreakPointManager::Deletebreaklist(unsigned int num)
{
    std::vector<Breaklocation>::iterator it = breaklist_.begin() + num - 1;
    breaklist_.erase(it);
}

std::vector<Breaklocation> BreakPointManager::Getbreaklist() const
{
    return breaklist_;
}
}