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

#include "Format/NodeFormatter/Expr/UnaryExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;
void Codira::Format::UnaryExprFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto unaryExpr = As<ASTKind::UNARY_EXPR>(node);
    AddUnaryExpr(doc, *unaryExpr, level);
}

void UnaryExprFormatter::AddUnaryExpr(Doc& doc, const Codira::AST::UnaryExpr& unaryExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    Doc group(DocType::GROUP, level, "");
    group.members.emplace_back(DocType::STRING, level, TOKENS[static_cast<int>(unaryExpr.op)]);

    if (unaryExpr.op == TokenKind::SUB &&
        (unaryExpr.expr.get()->astKind == ASTKind::LIT_CONST_EXPR ||
            unaryExpr.expr.get()->astKind == ASTKind::UNARY_EXPR)) {
        group.members.emplace_back(DocType::STRING, level, " ");
    }
    group.members.emplace_back(astToFormatSource.ASTToDoc(unaryExpr.expr.get(), level));
    doc.members.emplace_back(group);
}
} // namespace Codira::Format
