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

#include "Format/DocProcessor/ArgsProcessor.h"
#include "Format/ASTToFormatSource.h"

using namespace Codira::Format;

namespace {
void IncreaseIndent(Doc& doc)
{
    for (auto& member : doc.members) {
        member.indent++;
        IncreaseIndent(member);
    }
}
} // namespace

void ArgsProcessor::SoftLineProcessor(
    std::string& formatted, int& pos, std::pair<Doc, Mode>& current, size_t& i, bool& haveNotChangedLine)
{
    std::pair<Doc, Mode> next(current.first.members[i + 1], current.second);
    if (!Fits(next, options.lineLength - pos)) {
        formatted += options.newLine;
        formatted += std::string(current.first.members[i].indent * options.indentWidth, ' ');
        pos = current.first.members[i].indent * options.indentWidth;
        for (size_t j = i + 1; j < current.first.members.size(); ++j) {
            if (current.first.members[j].type == DocType::FUNC_ARG && haveNotChangedLine) {
                current.first.members[j].indent++;
                IncreaseIndent(current.first.members[j]);
            }
        }
        haveNotChangedLine = false;
    }
}

void ArgsProcessor::SoftLineWithSpaceProcessor(
    std::string& formatted, int& pos, std::pair<Doc, Mode>& current, size_t& i, bool& haveNotChangedLine)
{
    std::pair<Doc, Mode> next(current.first.members[i + 1], current.second);
    if (!Fits(next, options.lineLength - pos)) {
        formatted += options.newLine;
        formatted += std::string(current.first.members[i].indent * options.indentWidth, ' ');
        pos = current.first.members[i].indent * options.indentWidth;
        for (size_t j = i + 1; j < current.first.members.size(); ++j) {
            if (current.first.members[j].type == DocType::FUNC_ARG && haveNotChangedLine) {
                current.first.members[j].indent++;
                IncreaseIndent(current.first.members[j]);
            }
        }
        haveNotChangedLine = false;
    } else {
        if (next.first.type != DocType::LINE) {
            formatted += " ";
            pos += 1;
        }
    }
}

void ArgsProcessor::DocToString(
    std::string& formatted, int& pos, std::pair<Doc, Mode>& current, std::vector<std::pair<Doc, Mode>>& leftCmd)
{
    bool haveNotChangedLine = true;
    for (size_t i = 0; i < current.first.members.size(); ++i) {
        if (current.first.members[i].type == DocType::SOFTLINE) {
            if (current.first.members.size() == 1) {
                return;
            }
            SoftLineProcessor(formatted, pos, current, i, haveNotChangedLine);
            continue;
        }
        if (current.first.members[i].type == DocType::SOFTLINE_WITH_SPACE) {
            SoftLineWithSpaceProcessor(formatted, pos, current, i, haveNotChangedLine);
            continue;
        }
        astToFormatSource.DocToString(current.first.members[i], pos, formatted);
    }
}
