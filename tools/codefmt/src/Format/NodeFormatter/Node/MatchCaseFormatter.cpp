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

#include "Format/NodeFormatter/Node/MatchCaseFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;
void Codira::Format::MatchCaseFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto matchCase = As<ASTKind::MATCH_CASE>(node);
    AddMatchCase(doc, *matchCase, level);
}
void MatchCaseFormatter::AddMatchCase(Doc& doc, const Codira::AST::MatchCase& matchCase, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    doc.members.emplace_back(DocType::STRING, level, "case ");
    for (auto& expr : matchCase.patterns) {
        if (expr != matchCase.patterns.front()) {
            doc.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
            Doc group(DocType::GROUP, level, "");
            group.members.emplace_back(DocType::STRING, level, "| ");
            group.members.emplace_back(astToFormatSource.ASTToDoc(expr.get(), level + 1));
            doc.members.emplace_back(group);
        } else {
            doc.members.emplace_back(astToFormatSource.ASTToDoc(expr.get(), level + 1));
        }
    }
    if (matchCase.patternGuard) {
        doc.members.emplace_back(DocType::STRING, level, " where ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(matchCase.patternGuard.get(), level));
    }
    doc.members.emplace_back(DocType::STRING, level, " =>");
    if (matchCase.exprOrDecls) {
        if (matchCase.exprOrDecls->body.size() == 1 && matchCase.exprOrDecls->body[0]->astKind != ASTKind::IF_EXPR) {
            doc.members.emplace_back(DocType::STRING, level, " ");
            doc.members.emplace_back(astToFormatSource.ASTToDoc(matchCase.exprOrDecls.get(), level));
        } else {
            doc.members.emplace_back(DocType::LINE, level + 1, "");
            doc.members.emplace_back(astToFormatSource.ASTToDoc(matchCase.exprOrDecls.get(), level + 1));
        }
    }
}
} // namespace Codira::Format
