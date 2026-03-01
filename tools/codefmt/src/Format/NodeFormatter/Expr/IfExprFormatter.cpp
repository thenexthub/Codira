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

#include "Format/NodeFormatter/Expr/IfExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void IfExprFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto ifExpr = As<ASTKind::IF_EXPR>(node);
    AddIfExpr(doc, *ifExpr, level);
}

void IfExprFormatter::AddIfExpr(Doc& doc, const Codira::AST::IfExpr& ifExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    doc.members.emplace_back(DocType::STRING, level, "if ");

    if (ifExpr.condExpr) {
        doc.members.emplace_back(DocType::STRING, level, "(");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(ifExpr.condExpr.get(), level));
        doc.members.emplace_back(DocType::STRING, level, ")");
    }
    doc.members.emplace_back(astToFormatSource.ASTToDoc(ifExpr.thenBody.get(), level));
    if (ifExpr.hasElse) {
        ifExpr.thenBody->body.empty() && ifExpr.thenBody->leftCurlPos.line == ifExpr.thenBody->rightCurlPos.line
            ? doc.members.emplace_back(DocType::LINE, level, "")
            : doc.members.emplace_back(DocType::STRING, level, " ");
        doc.members.emplace_back(DocType::STRING, level, "else");
        if (ifExpr.elseBody->astKind == ASTKind::IF_EXPR) {
            doc.members.emplace_back(DocType::STRING, level, " ");
        }
        doc.members.emplace_back(astToFormatSource.ASTToDoc(ifExpr.elseBody.get(), level));
    }
}
} // namespace Codira::Format
