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

#include "Format/DocProcessor/MemberAccessProcessor.h"
#include "Format/ASTToFormatSource.h"

using namespace Codira::Format;

namespace {
void ParserCurrent(Doc& doc, Doc& ma)
{
    if (doc.type == DocType::CONCAT || doc.type == DocType::GROUP || doc.type == DocType::MEMBER_ACCESS) {
        for (auto& member : doc.members) {
            ParserCurrent(member, ma);
        }
    } else {
        ma.members.emplace_back(doc);
    }
}

void SplitVectorByFisrtDot(const std::vector<Doc>& original, std::vector<Doc>& firstHalf, std::vector<Doc>& secondHalf)
{
    auto it = std::find_if(original.begin(), original.end(), [](const Doc& doc) { return doc.value == "."; });
    if (it != original.end()) {
        auto index = std::distance(original.begin(), it);
        if (index > 0 && original[index - 1].type == DocType::LINE) {
            index--;
        }
        firstHalf.assign(original.begin(), original.begin() + index);
        secondHalf.assign(original.begin() + index, original.end());
    }
}

void IncreaseIndent(Doc& doc)
{
    for (auto& member : doc.members) {
        member.indent++;
        IncreaseIndent(member);
    }
}

void MemberAccessAddParentheses(std::vector<std::pair<Doc, Mode>>& leftCmd, Doc& ma)
{
    if (!leftCmd.empty() && leftCmd.back().first.value == "(") {
        ma.members.emplace_back(leftCmd.back().first);
        leftCmd.pop_back();
        while (leftCmd.back().first.value != ")") {
            ma.members.emplace_back(leftCmd.back().first);
            leftCmd.pop_back();
        }
        ma.members.emplace_back(leftCmd.back().first);
        leftCmd.pop_back();
    }

    if (!leftCmd.empty() && leftCmd.back().first.value == " ") {
        ma.members.emplace_back(leftCmd.back().first);
        leftCmd.pop_back();
        if (!leftCmd.empty() && leftCmd.back().first.type == DocType::LAMBDA) {
            ma.members.emplace_back(leftCmd.back().first);
            leftCmd.pop_back();
        }
    }
}

int SplitVectorByDOT(Doc& secondHalf, Doc& contact, std::vector<Doc>& lst)
{
    int count = 0;
    for (auto& member : secondHalf.members) {
        if (member.value == ".") {
            count++;
            if (!contact.members.empty()) {
                lst.emplace_back(contact);
                contact.members.clear();
            }
            lst.emplace_back(member);
        } else {
            contact.members.emplace_back(member);
        }
    }
    if (!contact.members.empty()) {
        lst.emplace_back(contact);
    }
    return count;
}

void MethodChainProcessor(Doc& member)
{
    if (member.type == DocType::DOT) {
        member.indent++;
        member.type = DocType::LINE_DOT;
    }
    for (auto& mem : member.members) {
        mem.indent++;
        IncreaseIndent(mem);
    }
}
}

void MemberAccessProcessor::NonMethodChianProcessor(Doc& member, int& pos, std::string& formatted)
{
    if (member.type == DocType::DOT) {
        int l = 0;
        for (auto& item : std::next(&member)->members) {
            if (item.type == DocType::LINE || item.type == DocType::SOFTLINE ||
                item.type == DocType::SOFTLINE_WITH_SPACE || item.type == DocType::DOT ||
                item.type == DocType::LINE_DOT) {
                break;
            }
            l += static_cast<int>(DisplayWidth(item.value));
        }
        if (options.lineLength - pos - l <= 0) {
            formatted += options.newLine;
            formatted += std::string((member.indent + 1) * options.indentWidth, ' ');
            pos = (member.indent + 1) * options.indentWidth;
            IncreaseIndent(*std::next(&member));
        }
    } else if (member.type == DocType::LINE_DOT) {
        IncreaseIndent(*std::next(&member));
    }
}

void MemberAccessProcessor::DocToString(
    std::string& formatted, int& pos, std::pair<Doc, Mode>& current, std::vector<std::pair<Doc, Mode>>& leftCmd)
{
    Doc ma;
    for (auto& member : current.first.members) {
        ParserCurrent(member, ma);
    }

    // if methodChain expr is a.b().c(xxx)
    // ma will be a.b().c, so need to add (xxx) to ma;
    MemberAccessAddParentheses(leftCmd, ma);

    // e.g. a.b().c().d()
    Doc firstHalf;  // firstHalf is a
    Doc secondHalf; // secondHalf is .b().c().d()
    SplitVectorByFisrtDot(ma.members, firstHalf.members, secondHalf.members);

    firstHalf.type = DocType::CONCAT;
    astToFormatSource.DocToString(firstHalf, pos, formatted);

    auto rem = options.lineLength - pos;
    auto length = CalculateDocLen(secondHalf);

    std::vector<Doc> lst;
    Doc contact;
    contact.type = DocType::CONCAT;
    int count = SplitVectorByDOT(secondHalf, contact, lst);

    for (auto& member : lst) {
        if (rem - length <= 0 && count > 1 && options.multipleLineMethodChainOverLineLength) {
            MethodChainProcessor(member);
        } else {
            NonMethodChianProcessor(member, pos, formatted);
        }
        astToFormatSource.DocToString(member, pos, formatted);
    }
}
