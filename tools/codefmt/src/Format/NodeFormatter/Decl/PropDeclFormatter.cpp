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

#include "Format/NodeFormatter/Decl/PropDeclFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void PropDeclFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto propDecl = As<ASTKind::PROP_DECL>(node);
    AddPropDecl(doc, *propDecl, level);
}

void PropDeclFormatter::AddPropDecl(Doc& doc, const Codira::AST::PropDecl& propDecl, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (!propDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, propDecl.annotations, level);
    }
    astToFormatSource.AddModifier(doc, propDecl.modifiers, level);

    doc.members.emplace_back(DocType::STRING, level, "prop ");
    doc.members.emplace_back(DocType::STRING, level, propDecl.identifier.GetRawText());
    if (propDecl.type) {
        doc.members.emplace_back(DocType::STRING, level, ": ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(propDecl.type.get(), level));
    }

    if (propDecl.leftCurlPos != INVALID_POSITION && propDecl.rightCurlPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, " {");
        doc.members.emplace_back(DocType::LINE, level + 1, "");
        astToFormatSource.AddBodyMembers(doc, propDecl.getters, level + 1);
        if (!propDecl.getters.empty() && !propDecl.setters.empty()) {
            doc.members.emplace_back(DocType::LINE, level + 1, "");
        }

        astToFormatSource.AddBodyMembers(doc, propDecl.setters, level + 1);
        doc.members.emplace_back(DocType::LINE, level, "");
        doc.members.emplace_back(DocType::STRING, level, "}");
    }
}
} // namespace Codira::Format
