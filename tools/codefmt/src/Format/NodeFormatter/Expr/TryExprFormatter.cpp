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

#include "Format/NodeFormatter/Expr/TryExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void TryExprFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto tryExpr = As<ASTKind::TRY_EXPR>(node);
    AddTryExpr(doc, *tryExpr, level);
}

void TryExprFormatter::AddTryExpr(Doc& doc, const Codira::AST::TryExpr& tryExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    // try-with-resource and normal try
    AddIsTryWithResource(doc, tryExpr, level);

    // tryBlock
    if (tryExpr.tryBlock) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(tryExpr.tryBlock.get(), level));
    }

    // catch block
    for (size_t i = 0; i < tryExpr.catchPosVector.size(); ++i) {
        doc.members.emplace_back(DocType::STRING, level, " catch ");
        doc.members.emplace_back(DocType::STRING, level, "(");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(tryExpr.catchPatterns.at(i).get(), level));
        doc.members.emplace_back(DocType::STRING, level, ")");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(tryExpr.catchBlocks.at(i).get(), level));
    }

    // finallyBlock
    if (tryExpr.finallyBlock) {
        doc.members.emplace_back(DocType::STRING, level, " finally");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(tryExpr.finallyBlock.get(), level));
    }
}

void TryExprFormatter::AddIsTryWithResource(Doc& doc, const Codira::AST::TryExpr& tryExpr, int level)
{
    if (!tryExpr.resourceSpec.empty()) {
        doc.members.emplace_back(DocType::STRING, level, "try (");
        FuncOptions funcOptions;
        funcOptions.patternOrEnum = true;
        for (auto& resDecl : tryExpr.resourceSpec) {
            doc.members.emplace_back(astToFormatSource.ASTToDoc(resDecl.get(), level, funcOptions));
            if (resDecl != tryExpr.resourceSpec.back()) {
                doc.members.emplace_back(DocType::STRING, level, ", ");
            }
        }
        doc.members.emplace_back(DocType::STRING, level, ")");
    } else {
        doc.members.emplace_back(DocType::STRING, level, "try");
    }
}
} // namespace Codira::Format
