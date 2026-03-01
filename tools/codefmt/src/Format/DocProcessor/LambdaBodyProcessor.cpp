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

#include "Format/DocProcessor/LambdaBodyProcessor.h"
#include "Format/ASTToFormatSource.h"

using namespace Codira::Format;

namespace {
bool hasLineType(Doc& doc)
{
    for (auto& member : doc.members) {
        if (member.type == DocType::LINE) {
            return true;
        }
        if (hasLineType(member)) {
            return true;
        }
    }
    return false;
}

void IncreaseIndent(Doc& doc)
{
    for (auto& member : doc.members) {
        member.indent++;
        IncreaseIndent(member);
    }
}
} // namespace

void LambdaBodyProcessor::DocToString(
    std::string& formatted, int& pos, std::pair<Doc, Mode>& current, std::vector<std::pair<Doc, Mode>>& leftCmd)
{
    if (current.first.members.size() == 1) {
        auto hasLine = hasLineType(current.first);
        auto rem = options.lineLength - pos;
        bool overLength;
        if (!hasLine) {
            auto length = CalculateDocLen(current.first);
            overLength = rem - length <= 0 ? true : false;
        }
        if (hasLine || overLength) {
            formatted += options.newLine;
            formatted += std::string((current.first.indent + 1) * options.indentWidth, ' ');
            pos = (current.first.indent + 1) * options.indentWidth;
        }
        for (auto& member : current.first.members) {
            astToFormatSource.DocToString(member, pos, formatted);
        }
        if (hasLine || overLength) {
            formatted += options.newLine;
            formatted += std::string(current.first.indent * options.indentWidth, ' ');
            pos = current.first.indent * options.indentWidth;
        }
    } else {
        for (auto& member : current.first.members) {
            astToFormatSource.DocToString(member, pos, formatted);
        }
    }
}
