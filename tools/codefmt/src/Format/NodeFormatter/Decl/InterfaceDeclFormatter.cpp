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

#include "Format/NodeFormatter/Decl/InterfaceDeclFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void InterfaceDeclFormatter::AddInterfaceDecl(Doc& doc, const Codira::AST::InterfaceDecl& interfaceDecl, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (!interfaceDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, interfaceDecl.annotations, level);
    }
    astToFormatSource.AddModifier(doc, interfaceDecl.modifiers, level);
    doc.members.emplace_back(DocType::STRING, level, "interface ");
    doc.members.emplace_back(DocType::STRING, level, interfaceDecl.identifier.GetRawText());
    auto& generic = interfaceDecl.generic;
    if (generic) {
        astToFormatSource.AddGenericParams(doc, *generic, level);
    }
    if (!interfaceDecl.inheritedTypes.empty()) {
        AddInterfaceInheritedTypes(doc, interfaceDecl, level);
    }
    if (generic) {
        astToFormatSource.AddGenericBound(doc, *generic, level);
    }
    doc.members.emplace_back(astToFormatSource.ASTToDoc(interfaceDecl.body.get(), level));
}

void InterfaceDeclFormatter::AddInterfaceInheritedTypes(
    Doc& doc, const Codira::AST::InterfaceDecl& interfaceDecl, int level)
{
    doc.members.emplace_back(DocType::STRING, level, " <: ");
    Doc group(DocType::GROUP, level + 1, "");
    for (auto& type : interfaceDecl.inheritedTypes) {
        group.members.emplace_back(astToFormatSource.ASTToDoc(type.get(), level + 1));
        if (type != interfaceDecl.inheritedTypes.back()) {
            group.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
            group.members.emplace_back(DocType::STRING, level + 1, "&");
            group.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
        }
    }
    doc.members.emplace_back(group);
}

void InterfaceDeclFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto interfaceDecl = As<ASTKind::INTERFACE_DECL>(node);
    AddInterfaceDecl(doc, *interfaceDecl, level);
}
} // namespace Codira::Format
