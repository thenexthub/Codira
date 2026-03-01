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

#include "Format/NodeFormatter/Expr/MemberAccessFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void MemberAccessFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto expr = As<ASTKind::MEMBER_ACCESS>(node);
    if (funcOptions.isMethodChainning || !options.allowMultiLineMethodChain) {
        AddMemberAccess(doc, *expr, level, funcOptions);
        return;
    }

    if (astToFormatSource.DepthInMultipleMethodChain(*expr) >= options.multipleLineMethodChainLevel) {
        funcOptions.isMethodChainning = true;
    }
    AddMemberAccess(doc, *expr, level, funcOptions);
}

void MemberAccessFormatter::AddMemberAccess(
    Doc& doc, const Codira::AST::MemberAccess& memberAccess, int level, FuncOptions funcOptions)
{
    doc.type = DocType::MEMBER_ACCESS;
    doc.indent = level;
    doc.members.emplace_back(astToFormatSource.ASTToDoc(memberAccess.baseExpr.get(), level, funcOptions));
    if (funcOptions.isMethodChainning) {
        doc.members.emplace_back(DocType::LINE_DOT, level + 1, ".");
    } else {
        doc.members.emplace_back(DocType::DOT, level, ".");
    }

    doc.members.emplace_back(DocType::STRING, level,
        memberAccess.field.IsRaw() ? memberAccess.field.GetRawText() : memberAccess.field.Val());
    if (!memberAccess.typeArguments.empty()) {
        doc.members.emplace_back(DocType::STRING, level, "<");
        for (auto& n : memberAccess.typeArguments) {
            doc.members.emplace_back(astToFormatSource.ASTToDoc(n.get(), level + 1));
            if (n != memberAccess.typeArguments.back()) {
                doc.members.emplace_back(DocType::STRING, level + 1, ", ");
            }
    }
        doc.members.emplace_back(DocType::STRING, level, ">");
    }
    if (memberAccess.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}
} // namespace Codira::Format
