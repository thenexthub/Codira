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

#include "Format/NodeFormatter/Decl/VarDeclFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void VarDeclFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto varDecl = As<ASTKind::VAR_DECL>(node);
    AddVarDecl(doc, *varDecl, level, funcOptions);
}

void VarDeclFormatter::AddVarDecl(Doc& doc, const Codira::AST::VarDecl& varDecl, int level, FuncOptions funcOptions)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (!varDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, varDecl.annotations, level);
    }
    astToFormatSource.AddModifier(doc, varDecl.modifiers, level);
    if (!funcOptions.patternOrEnum) {
        doc.members.emplace_back(DocType::STRING, level, varDecl.isVar ? "var " : varDecl.isConst ? "" : "let ");
    }

    doc.members.emplace_back(DocType::STRING, level, varDecl.identifier.GetRawText());
    if (varDecl.type) {
        doc.members.emplace_back(DocType::STRING, level, ": ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(varDecl.type.get(), level));
    }
    if (varDecl.initializer) {
        doc.members.emplace_back(DocType::STRING, level, " = ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(varDecl.initializer.get(), level));
    }
}
} // namespace Codira::Format
