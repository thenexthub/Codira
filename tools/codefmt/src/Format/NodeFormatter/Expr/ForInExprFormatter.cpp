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

#include "Format/NodeFormatter/Expr/ForInExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;
void ForInExprFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto forInExpr = As<ASTKind::FOR_IN_EXPR>(node);
    AddForInExpr(doc, *forInExpr, level);
}

void ForInExprFormatter::AddForInExpr(Doc& doc, const Codira::AST::ForInExpr& forInExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    doc.members.emplace_back(DocType::STRING, level, "for ");
    if (forInExpr.leftParenPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, "(");
    }
    if (forInExpr.pattern) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(forInExpr.pattern.get(), level));
    }
    doc.members.emplace_back(DocType::STRING, level, " in ");
    if (forInExpr.inExpression) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(forInExpr.inExpression.get(), level));
    }
    if (forInExpr.patternGuard) {
        doc.members.emplace_back(DocType::STRING, level, " where ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(forInExpr.patternGuard.get(), level));
    }
    if (forInExpr.rightParenPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, ")");
    }
    if (forInExpr.body) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(forInExpr.body.get(), level));
    }
}
} // namespace Codira::Format
