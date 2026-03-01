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

#include "Format/NodeFormatter/Expr/AsExprFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void AsExprFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto asExpr = As<ASTKind::AS_EXPR>(node);
    AddAsExpr(doc, *asExpr, level);
}

void AsExprFormatter::AddAsExpr(Doc& doc, const Codira::AST::AsExpr& asExpr, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (asExpr.leftExpr) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(asExpr.leftExpr.get(), level));
    }
    doc.members.emplace_back(DocType::STRING, level, " as ");
    if (asExpr.asType) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(asExpr.asType.get(), level));
    }
}
} // namespace Codira::Format
