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

#include "UserCodeInfo.h"

namespace Codira {

void UserCodeInfo::RecordInfo(const std::string& item, int64_t value)
{
    codeInfo.emplace_back(item, value);
}

std::string UserCodeInfo::GetJson() const
{
    std::string output;
    output += "{";
    for (auto& it : codeInfo) {
        output += ("\n   \"" + it.first + "\": " + std::to_string(it.second) + ",");
    }
    if (!codeInfo.empty()) {
        output.pop_back();
    }
    output += "\n}\n";
    return output;
}
} // namespace Codira
