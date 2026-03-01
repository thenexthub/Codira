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

#include "Format/NodeFormatter/Expr/ArrayLitFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void ArrayLitFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto al = As<ASTKind::ARRAY_LIT>(node);
    AddArrayLit(doc, *al, level);
}

void ArrayLitFormatter::AddArrayLit(Doc& doc, const Codira::AST::ArrayLit& arrayLit, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    doc.members.emplace_back(DocType::STRING, level, "[");
    if (astToFormatSource.IsMultipleLineArrayLit(arrayLit.rightSquarePos.line, arrayLit.children) ||
        astToFormatSource.IsMultipleLineExpr(arrayLit.children)) {
        AddBreakLineArrayLit(doc, arrayLit, level);
        return;
    }
    if (!arrayLit.children.empty()) {
        AddArrayLitChildren(doc, arrayLit, level);
    }
    doc.members.emplace_back(DocType::STRING, level, "]");
    if (arrayLit.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}

void ArrayLitFormatter::AddBreakLineArrayLit(Doc& doc, const Codira::AST::ArrayLit& arrayLit, int level)
{
    for (auto it = arrayLit.children.begin(); it != arrayLit.children.end(); ++it) {
        doc.members.emplace_back(DocType::LINE, level + 1, "");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(it->get(), level + 1));
        if (*it != arrayLit.children.back()) {
            doc.members.emplace_back(DocType::STRING, level + 1, ",");
        }
    }
    doc.members.emplace_back(DocType::LINE, level, "");
    doc.members.emplace_back(DocType::STRING, level, "]");
    if (arrayLit.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}

void ArrayLitFormatter::AddArrayLitChildren(Doc& doc, const Codira::AST::ArrayLit& arrayLit, int level)
{
    Doc args(DocType::ARGS, level, "");
    for (auto& n : arrayLit.children) {
        Doc group(DocType::GROUP, level, "");
        group.members.emplace_back(astToFormatSource.ASTToDoc(n.get(), level));
        args.members.emplace_back(group);
        if (n != arrayLit.children.back()) {
            args.members.emplace_back(DocType::STRING, level, ",");
            args.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
        }
    }
    doc.members.emplace_back(args);
    doc.members.emplace_back(DocType::BREAK_PARENT, level, "");
}
} // namespace Codira::Format
