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

#include "Format/NodeFormatter/Decl/ExtendDeclFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void ExtendDeclFormatter::AddExtendDecl(Doc& doc, const Codira::AST::ExtendDecl& extendDecl, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (!extendDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, extendDecl.annotations, level);
    }

    if (extendDecl.TestAttr(AST::Attribute::COMMON)) {
        doc.members.emplace_back(DocType::STRING, level, "common");
        doc.members.emplace_back(DocType::STRING, level, " ");
    }

    if (extendDecl.TestAttr(AST::Attribute::PLATFORM)) {
        doc.members.emplace_back(DocType::STRING, level, "platform");
        doc.members.emplace_back(DocType::STRING, level, " ");
    }

    doc.members.emplace_back(DocType::STRING, level, "extend");
    if (extendDecl.generic) {
        astToFormatSource.AddGenericParams(doc, *extendDecl.generic, level);
    }
    doc.members.emplace_back(DocType::STRING, level, " ");

    doc.members.emplace_back(astToFormatSource.ASTToDoc(extendDecl.extendedType.get(), level));
    if (!extendDecl.inheritedTypes.empty()) {
        AddExtendDeclInheritedTypes(doc, extendDecl, level);
    }
    if (extendDecl.generic) {
        astToFormatSource.AddGenericBound(doc, *extendDecl.generic, level);
    }

    // Body
    if (extendDecl.members.empty()) {
        doc.members.emplace_back(DocType::STRING, level, " {}");
        return;
    }
    doc.members.emplace_back(DocType::STRING, level, " {");
    doc.members.emplace_back(DocType::LINE, level + 1, "");
    astToFormatSource.AddBodyMembers(doc, extendDecl.members, level + 1);
    doc.members.emplace_back(DocType::LINE, level, "");
    doc.members.emplace_back(DocType::STRING, level, "}");
}

void ExtendDeclFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto extendDecl = As<ASTKind::EXTEND_DECL>(node);
    AddExtendDecl(doc, *extendDecl, level);
}

void ExtendDeclFormatter::AddExtendDeclInheritedTypes(Doc& doc, const Codira::AST::ExtendDecl& extendDecl, int level)
{
    doc.members.emplace_back(DocType::STRING, level, " <: ");
    Doc group(DocType::GROUP, level + 1, "");
    for (auto& trait : extendDecl.inheritedTypes) {
        group.members.emplace_back(astToFormatSource.ASTToDoc(trait.get(), level + 1));
        if (trait != extendDecl.inheritedTypes.back()) {
            group.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
            group.members.emplace_back(DocType::STRING, level + 1, "&");
            group.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
        }
    }
    doc.members.emplace_back(group);
}
} // namespace Codira::Format
