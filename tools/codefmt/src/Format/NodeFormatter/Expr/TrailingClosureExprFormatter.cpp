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

#include "Format/NodeFormatter/Expr/TrailingClosureExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void TrailingClosureExprFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto trailingClosureExpr = StaticAs<ASTKind::TRAIL_CLOSURE_EXPR>(node);
    AddTrailingClosureExpr(doc, *trailingClosureExpr, level);
}

void TrailingClosureExprFormatter::AddTrailingClosureExpr(
    Doc& doc, const Codira::AST::TrailingClosureExpr& trailingClosureExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    auto& node = trailingClosureExpr.expr;
    Doc group(DocType::GROUP, level, "");
    if (node) {
        group.members.emplace_back(astToFormatSource.ASTToDoc(node.get(), level));
    }
    group.members.emplace_back(DocType::STRING, level, " ");
    FuncOptions funcOptions;
    funcOptions.isLambda = true;
    if (trailingClosureExpr.lambda) {
        group.members.emplace_back(astToFormatSource.ASTToDoc(trailingClosureExpr.lambda.get(), level, funcOptions));
    }
    doc.members.emplace_back(group);
}
} // namespace Codira::Format
