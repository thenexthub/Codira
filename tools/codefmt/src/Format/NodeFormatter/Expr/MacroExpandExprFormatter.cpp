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

#include "Format/NodeFormatter/Expr/MacroExpandExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void MacroExpandExprFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto macroExpandExpr = As<ASTKind::MACRO_EXPAND_EXPR>(node);
    AddMacroExpandExpr(doc, *macroExpandExpr, level);
}

void MacroExpandExprFormatter::AddMacroExpandExpr(
    Doc& doc, const Codira::AST::MacroExpandExpr& macroExpandExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    std::string compileTimeVisibleStr = macroExpandExpr.invocation.isCompileTimeVisible ? "!" : "";
    std::string macroStr = "@" + compileTimeVisibleStr + macroExpandExpr.invocation.fullName;

    if (macroExpandExpr.invocation.leftSquarePos != INVALID_POSITION &&
        macroExpandExpr.invocation.rightSquarePos != INVALID_POSITION) {
        macroStr += astToFormatSource.sm.GetContentBetween(macroExpandExpr.invocation.leftSquarePos.fileID,
            macroExpandExpr.invocation.leftSquarePos, macroExpandExpr.invocation.rightSquarePos + 1);
    }

    if (macroExpandExpr.invocation.leftParenPos != INVALID_POSITION &&
        macroExpandExpr.invocation.rightParenPos != INVALID_POSITION) {
        macroStr += astToFormatSource.sm.GetContentBetween(macroExpandExpr.invocation.leftParenPos.fileID,
            macroExpandExpr.invocation.leftParenPos, macroExpandExpr.invocation.rightParenPos + 1);
    }

    doc.members.emplace_back(DocType::STRING, level, macroStr);
    if (macroExpandExpr.invocation.decl != nullptr) {
        if (macroExpandExpr.invocation.decl->begin.line == macroExpandExpr.invocation.identifierPos.line) {
            doc.members.emplace_back(DocType::STRING, level, " ");
        } else {
            doc.members.emplace_back(DocType::LINE, level, "");
        }
        doc.members.emplace_back(astToFormatSource.ASTToDoc(macroExpandExpr.invocation.decl.get(), level));
    }
}
} // namespace Codira::Format
