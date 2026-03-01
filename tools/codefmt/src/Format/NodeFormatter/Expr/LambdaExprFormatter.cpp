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

#include "Format/NodeFormatter/Expr/LambdaExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void LambdaExprFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto lambdaExpr = As<ASTKind::LAMBDA_EXPR>(node);
    AddLambdaExpr(doc, *lambdaExpr, level);
}

void LambdaExprFormatter::AddLambdaExpr(Doc& doc, const Codira::AST::LambdaExpr& lambdaExpr, int level)
{
    doc.type = DocType::LAMBDA;
    doc.indent = level;

    if (lambdaExpr.TestAttr(AST::Attribute::MOCK_SUPPORTED)) {
        doc.members.emplace_back(DocType::STRING, level, "@EnsurePreparedToMock ");
    }

    AddLambdaExprOverFlowStrategy(doc, lambdaExpr, level);
    doc.members.emplace_back(DocType::STRING, level, "{");
    if (lambdaExpr.funcBody) {
        Doc blockMember(DocType::LAMBDA_BODY, level, "");
        if (lambdaExpr.begin.line != lambdaExpr.end.line) {
            blockMember.members.emplace_back(DocType::LINE, level + 1, "");
        }
        FuncOptions funcOptions;
        funcOptions.isLambda = true;
        blockMember.members.emplace_back(astToFormatSource.ASTToDoc(lambdaExpr.funcBody.get(),
            lambdaExpr.begin.line != lambdaExpr.end.line ? level + 1 : level, funcOptions));
        if (lambdaExpr.begin.line != lambdaExpr.end.line) {
            blockMember.members.emplace_back(DocType::LINE, level, "");
        }
        doc.members.emplace_back(blockMember);
    }
    doc.members.emplace_back(DocType::STRING, level, "}");
    if (lambdaExpr.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}

void LambdaExprFormatter::AddLambdaExprOverFlowStrategy(Doc& doc, const Codira::AST::LambdaExpr& lambdaExpr, int level)
{
    switch (lambdaExpr.overflowStrategy) {
        case Codira::OverflowStrategy::SATURATING: {
            doc.members.emplace_back(DocType::STRING, level, "@OverflowSaturating ");
            break;
        }
        case Codira::OverflowStrategy::THROWING: {
            doc.members.emplace_back(DocType::STRING, level, "@OverflowThrowing ");
            break;
        }
        case Codira::OverflowStrategy::WRAPPING: {
            doc.members.emplace_back(DocType::STRING, level, "@OverflowWrapping ");
            break;
        }
        default: {
            break;
        }
    }
}
} // namespace Codira::Format
