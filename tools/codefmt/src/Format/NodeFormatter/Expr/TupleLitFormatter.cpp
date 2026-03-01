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

#include "Format/NodeFormatter/Expr/TupleLitFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void TupleLitFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto tupleLit = As<ASTKind::TUPLE_LIT>(node);
    AddTupleLit(doc, *tupleLit, level);
}

void TupleLitFormatter::AddTupleLit(Doc& doc, const Codira::AST::TupleLit& tupleLit, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    doc.members.emplace_back(DocType::STRING, level, "(");
    Doc group(DocType::GROUP, level + 1, "");
    for (auto& expr : tupleLit.children) {
        group.members.emplace_back(astToFormatSource.ASTToDoc(expr.get(), level + 1));
        if (expr != tupleLit.children.back()) {
            group.members.emplace_back(DocType::STRING, level + 1, ",");
            group.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
        }
    }
    doc.members.emplace_back(group);
    doc.members.emplace_back(DocType::STRING, level, ")");
    if (tupleLit.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}
} // namespace Codira::Format
