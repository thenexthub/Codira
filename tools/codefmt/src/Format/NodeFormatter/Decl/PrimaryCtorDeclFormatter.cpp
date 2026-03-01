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

#include "Format/NodeFormatter/Decl/PrimaryCtorDeclFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void PrimaryCtorDeclFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto primaryCtorDecl = As<ASTKind::PRIMARY_CTOR_DECL>(node);
    AddPrimaryCtorDecl(doc, *primaryCtorDecl, level, funcOptions);
}

void PrimaryCtorDeclFormatter::AddPrimaryCtorDecl(
    Doc& doc, const Codira::AST::PrimaryCtorDecl& primaryCtorDecl, int level, FuncOptions funcOptions)
{
    if (primaryCtorDecl.TestAttr(Attribute::COMPILER_ADD)) {
        return;
    }
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (!primaryCtorDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, primaryCtorDecl.annotations, level);
    }
    astToFormatSource.AddModifier(doc, primaryCtorDecl.modifiers, level);
    if (!primaryCtorDecl.TestAttr(Attribute::CONSTRUCTOR) && !funcOptions.patternOrEnum) {
        doc.members.emplace_back(DocType::STRING, level, "func ");
        doc.members.emplace_back(DocType::STRING, level, primaryCtorDecl.identifier.GetRawText());
    } else {
        doc.members.emplace_back(DocType::STRING, level, primaryCtorDecl.identifier.GetRawText());
    }
    doc.members.emplace_back(
        astToFormatSource.ASTToDoc(primaryCtorDecl.funcBody.get(), level, funcOptions));
}
} // namespace Codira::Format
