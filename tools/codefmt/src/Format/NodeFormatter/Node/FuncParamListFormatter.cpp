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

#include "Format/NodeFormatter/Node/FuncParamListFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
const int MIN_MUL_MEMBERS = 2;
using namespace Codira::AST;

void FuncParamListFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto paramList = As<ASTKind::FUNC_PARAM_LIST>(node);
    AddFuncParamList(doc, *paramList, level, funcOptions);
}

bool FuncParamListFormatter::IsMultipleLineMacroExpandParam(const AST::FuncParamList& funcParamList)
{
    if (funcParamList.params.size() > 1) {
        for (auto& n : funcParamList.params) {
            if (n->astKind == ASTKind::MACRO_EXPAND_PARAM) {
                return true;
            }
        }
    } else if (funcParamList.params.size() == 1) {
        if (funcParamList.params[0]->astKind == ASTKind::MACRO_EXPAND_PARAM) {
            auto  pMacroExpendParam = As<ASTKind::MACRO_EXPAND_PARAM>(funcParamList.params[0]);
            if (pMacroExpendParam->invocation.rightSquarePos != INVALID_POSITION &&
                pMacroExpendParam->invocation.rightSquarePos.line !=
                pMacroExpendParam->invocation.attrs.back().End().line) {
                return true;
            }
            if (pMacroExpendParam->invocation.decl &&
                pMacroExpendParam->invocation.decl->astKind == AST::ASTKind::MACRO_EXPAND_PARAM) {
                return true;
            }
        }
    }
    return false;
}

void FuncParamListFormatter::AddFuncParamList(
    Doc& doc, const AST::FuncParamList& funcParamList, int level, FuncOptions funcOptions)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (funcParamList.params.empty()) {
        AddEmptyParam(doc, level);
        return;
    }
    doc.members.emplace_back(DocType::STRING, level, "(");
    doc.members.emplace_back(DocType::SOFTLINE, level + 1, "");

    bool isMultipleLineMacroExpandParam = IsMultipleLineMacroExpandParam(funcParamList);
    if (isMultipleLineMacroExpandParam || IsMultipleLine(funcParamList.rightParenPos.line, funcParamList.params)) {
        astToFormatSource.AddBreakLineParam(doc, funcParamList, level, funcOptions);
        return;
    }

    for (auto& n : funcParamList.params) {
        Doc group(DocType::GROUP, level + 1, "");
        group.members.emplace_back(astToFormatSource.ASTToDoc(n.get(), level + 1, funcOptions));
        if (n != funcParamList.params.back()) {
            group.members.emplace_back(DocType::STRING, level + 1, ",");
            group.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
        }
        if (n == funcParamList.params.back() && n->commaPos != INVALID_POSITION) {
            group.members.emplace_back(DocType::STRING, level + 1, ", ...");
        }
        doc.members.emplace_back(group);
    }
    doc.members.emplace_back(DocType::STRING, level, ")");
}

bool FuncParamListFormatter::IsMultipleLine(const int& rightParentPosLine,
    const std::vector<OwnedPtr<AST::FuncParam>>& params) const
{
    if (params.size() < MIN_MUL_MEMBERS) {
        return false;
    }
    if (rightParentPosLine == params.back()->end.line) {
        return false;
    }
    return true;
}

void FuncParamListFormatter::AddEmptyParam(Doc& doc, int level)
{
    doc.members.emplace_back(DocType::STRING, level, "()");
}
} // namespace Codira::Format
