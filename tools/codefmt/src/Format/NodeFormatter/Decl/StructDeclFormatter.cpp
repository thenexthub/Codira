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

#include "Format/NodeFormatter/Decl/StructDeclFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void StructDeclFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto structDecl = As<ASTKind::STRUCT_DECL>(node);
    AddStructDecl(doc, *structDecl, level);
}

void StructDeclFormatter::AddStructDecl(Doc& doc, const Codira::AST::StructDecl& structDecl, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (!structDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, structDecl.annotations, level);
    }
    astToFormatSource.AddModifier(doc, structDecl.modifiers, level);
    doc.members.emplace_back(DocType::STRING, level, "struct ");
    doc.members.emplace_back(DocType::STRING, level, structDecl.identifier.GetRawText());
    auto& generic = structDecl.generic;
    if (generic) {
        astToFormatSource.AddGenericParams(doc, *generic, level);
    }
    if (!structDecl.inheritedTypes.empty() && structDecl.upperBoundPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, " <: ");
        for (const auto& ty : structDecl.inheritedTypes) {
            doc.members.emplace_back(astToFormatSource.ASTToDoc(ty.get(), level));
            if (ty != structDecl.inheritedTypes.back()) {
                doc.members.emplace_back(DocType::STRING, level, " & ");
            }
        }
    }
    if (generic) {
        astToFormatSource.AddGenericBound(doc, *generic, level);
    }
    doc.members.emplace_back(astToFormatSource.ASTToDoc(structDecl.body.get(), level));
}
} // namespace Codira::Format
