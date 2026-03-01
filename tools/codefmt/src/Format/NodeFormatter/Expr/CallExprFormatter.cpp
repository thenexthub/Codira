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

#include "Format/NodeFormatter/Expr/CallExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void CallExprFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto expr = As<ASTKind::CALL_EXPR>(node);
    AddCallExpr(doc, *expr, level, funcOptions);
}

void CallExprFormatter::AddCallExpr(
    Doc& doc, const Codira::AST::CallExpr& callExpr, int level, FuncOptions funcOptions)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    Doc group(DocType::GROUP, level, "");

    group.members.emplace_back(astToFormatSource.ASTToDoc(callExpr.baseFunc.get(), level, funcOptions));
    if (callExpr.baseFunc->astKind == ASTKind::LAMBDA_EXPR && callExpr.args.empty()) {
        group.members.emplace_back(DocType::STRING, level, "()");
        doc.members.emplace_back(group);
        return;
    }
    group.members.emplace_back(DocType::STRING, level, "(");
    doc.members.emplace_back(group);

    if (astToFormatSource.IsMultipleLineCallExpr(callExpr) || astToFormatSource.IsMultipleLineArg(callExpr.args)) {
        AddBreakLineCallArgs(doc, callExpr, level);
        return;
    }
    Doc args(DocType::ARGS, level, "");
    args.members.emplace_back(DocType::SOFTLINE, level + 1, "");
    if (!callExpr.args.empty()) {
        for (auto& n : callExpr.args) {
            args.members.emplace_back(astToFormatSource.ASTToDoc(n.get(), level));
            if (n->commaPos != INVALID_POSITION) {
                args.members.emplace_back(DocType::STRING, level, ",");
            }
            if (n != callExpr.args.back()) {
                args.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
            }
        }
    }
    doc.members.emplace_back(args);
    doc.members.emplace_back(DocType::STRING, level, ")");
    if (callExpr.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}

void CallExprFormatter::AddBreakLineCallArgs(Doc& doc, const Codira::AST::CallExpr& callExpr, int level)
{
    for (auto it = callExpr.args.begin(); it != callExpr.args.end(); ++it) {
        doc.members.emplace_back(DocType::LINE, level + 1, "");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(it->get(), level + 1));
        if ((*it)->commaPos != INVALID_POSITION) {
            doc.members.emplace_back(DocType::STRING, level + 1, ",");
        }
        if (*it == callExpr.args.back()) {
            break;
        }
    }
    doc.members.emplace_back(DocType::LINE, level, "");
    doc.members.emplace_back(DocType::STRING, level, ")");
    if (callExpr.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}
} // namespace Codira::Format
