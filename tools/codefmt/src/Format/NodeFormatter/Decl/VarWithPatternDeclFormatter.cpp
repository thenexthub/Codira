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

#include "Format/NodeFormatter/Decl/VarWithPatternDeclFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void VarWithPatternDeclFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto varWithPatternDecl = As<ASTKind::VAR_WITH_PATTERN_DECL>(node);
    AddVarWithPatternDecl(doc, *varWithPatternDecl, level);
}

void VarWithPatternDeclFormatter::AddVarWithPatternDecl(
    Doc& doc, const Codira::AST::VarWithPatternDecl& varWithPatternDecl, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (!varWithPatternDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, varWithPatternDecl.annotations, level);
    }
    astToFormatSource.AddModifier(doc, varWithPatternDecl.modifiers, level);
    doc.members.emplace_back(DocType::STRING, level,
        varWithPatternDecl.isVar ? "var " : varWithPatternDecl.isConst ? "" : "let ");

    if (varWithPatternDecl.irrefutablePattern) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(varWithPatternDecl.irrefutablePattern.get(), level));
    }

    if (varWithPatternDecl.type) {
        doc.members.emplace_back(DocType::STRING, level, ": ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(varWithPatternDecl.type.get(), level));
    }

    if (varWithPatternDecl.initializer) {
        doc.members.emplace_back(DocType::STRING, level, " = ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(varWithPatternDecl.initializer.get(), level));
    }
}
} // namespace Codira::Format
