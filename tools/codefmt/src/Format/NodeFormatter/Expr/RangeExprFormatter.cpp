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

#include "Format/NodeFormatter/Expr/RangeExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void RangeExprFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto rangeExpr = As<ASTKind::RANGE_EXPR>(node);
    AddRangeExpr(doc, *rangeExpr, level);
}

void RangeExprFormatter::AddRangeExpr(Doc& doc, const Codira::AST::RangeExpr& rangeExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (rangeExpr.startExpr) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(rangeExpr.startExpr.get(), level));
    }
    if (rangeExpr.isClosed) {
        doc.members.emplace_back(DocType::STRING, level, TOKENS[static_cast<int>(TokenKind::CLOSEDRANGEOP)]);
    } else {
        doc.members.emplace_back(DocType::STRING, level, TOKENS[static_cast<int>(TokenKind::RANGEOP)]);
    }
    if (rangeExpr.stopExpr) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(rangeExpr.stopExpr.get(), level));
    }
    if (rangeExpr.stepExpr) {
        doc.members.emplace_back(DocType::STRING, level, " : ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(rangeExpr.stepExpr.get(), level));
    }
}
} // namespace Codira::Format
