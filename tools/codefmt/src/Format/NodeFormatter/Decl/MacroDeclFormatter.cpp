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

#include "Format/NodeFormatter/Decl/MacroDeclFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void MacroDeclFormatter::AddMacroDecl(Doc& doc, const Codira::AST::MacroDecl& macroDecl, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (!macroDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, macroDecl.annotations, level);
    }
    astToFormatSource.AddModifier(doc, macroDecl.modifiers, level);
    doc.members.emplace_back(DocType::STRING, level, "macro " + macroDecl.identifier.GetRawText());
    doc.members.emplace_back(astToFormatSource.ASTToDoc(macroDecl.funcBody.get(), level));
}

void MacroDeclFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto macroDecl = As<ASTKind::MACRO_DECL>(node);
    AddMacroDecl(doc, *macroDecl, level);
}
} // namespace Codira::Format
