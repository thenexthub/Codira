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

#include "Format/DocProcessor/DocProcessor.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

bool DocProcessor::Fits(const std::pair<Doc, Mode>& next, int rem)
{
    std::vector<std::pair<Doc, Mode>> fitCmd;
    fitCmd.emplace_back(next);
    while (rem >= 0) {
        if (fitCmd.empty()) {
            return true;
        }
        std::pair<Doc, Mode> current = fitCmd.back();
        fitCmd.pop_back();
        switch (current.first.type) {
            case DocType::STRING:
                rem -= static_cast<int>(current.first.value.length());
                return rem >= 0;
            case DocType::FUNC_ARG:
            case DocType::LAMBDA:
            case DocType::CONCAT:
                for (auto it = current.first.members.rbegin(); it != current.first.members.rend(); ++it) {
                    fitCmd.emplace_back(*it, Mode::MODE_FLAT);
                }
                break;
            case DocType::LINE:
            case DocType::SEPARATE:
                return true;
            case DocType::GROUP:
                return rem - CalculateDocLen(current.first) > 0;
            case DocType::SOFTLINE_WITH_SPACE:
                if (current.second == Mode::MODE_BREAK) {
                    return true;
                }
                rem -= 1;
                break;
            case DocType::BREAK_PARENT:
                if (current.second == Mode::MODE_BREAK) {
                    return true;
                }
                break;
            default:
                break;
        }
    }
    return false;
}

int DocProcessor::CalculateDocLen(Doc& doc)
{
    int len = 0;
    len += static_cast<int>(DisplayWidth(doc.value));
    for (auto& member : doc.members) {
        if (member.type == DocType::LINE) {
            return len;
        }
        len += CalculateDocLen(member);
    }
    return len;
}
} // namespace Codira::Format
