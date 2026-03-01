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

#include "Format/NodeFormatter/Decl/ClassDeclFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void ClassDeclFormatter::AddClassDeclInheritedTypes(Doc& doc, const Codira::AST::ClassDecl& classDecl, int level)
{
    if (!classDecl.inheritedTypes.empty() && classDecl.upperBoundPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, " <: ");
        for (const auto& ty : classDecl.inheritedTypes) {
            doc.members.emplace_back(astToFormatSource.ASTToDoc(ty.get(), level));
            if (ty != classDecl.inheritedTypes.back()) {
                doc.members.emplace_back(DocType::STRING, level, " & ");
            }
        }
    }
}

void ClassDeclFormatter::AddClassDecl(Doc& doc, const Codira::AST::ClassDecl& classDecl, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (!classDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, classDecl.annotations, level);
    }
    astToFormatSource.AddModifier(doc, classDecl.modifiers, level);
    doc.members.emplace_back(DocType::STRING, level, "class " + classDecl.identifier.GetRawText());
    auto& generic = classDecl.generic;
    if (generic) {
        astToFormatSource.AddGenericParams(doc, *generic, level);
    }
    AddClassDeclInheritedTypes(doc, classDecl, level);
    if (generic) {
        astToFormatSource.AddGenericBound(doc, *generic, level);
    }
    doc.members.emplace_back(astToFormatSource.ASTToDoc(classDecl.body.get(), level));
}

void ClassDeclFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto classDecl = As<ASTKind::CLASS_DECL>(node);
    AddClassDecl(doc, *classDecl, level);
}
} // namespace Codira::Format
