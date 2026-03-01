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

#include "Format/NodeFormatter/Decl/FuncDeclFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void FuncDeclFormatter::AddFuncDecl(
    Doc& doc, const Codira::AST::FuncDecl& funcDecl, int level, FuncOptions funcOptions)
{
    if (funcDecl.TestAttr(Attribute::COMPILER_ADD)) {
        return;
    }
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (!funcDecl.annotations.empty()) {
        if (funcOptions.patternOrEnum) {
            astToFormatSource.AddAnnotations(doc, funcDecl.annotations, level, false);
        } else {
            astToFormatSource.AddAnnotations(doc, funcDecl.annotations, level);
        }
    }

    astToFormatSource.AddModifier(doc, funcDecl.modifiers, level);
    if (!funcDecl.TestAttr(Attribute::CONSTRUCTOR) && !funcOptions.patternOrEnum &&
        (!funcDecl.isGetter && !funcDecl.isSetter)) {
        if (funcDecl.TestAttr(Attribute::MACRO_FUNC)) {
            doc.members.emplace_back(DocType::STRING, level, "macro ");
        } else if (!funcDecl.IsFinalizer()) {
            doc.members.emplace_back(DocType::STRING, level, "func ");
        }
        doc.members.emplace_back(DocType::STRING, level, funcDecl.identifier.GetRawText());
    } else {
        doc.members.emplace_back(DocType::STRING, level, funcDecl.identifier.GetRawText());
    }
    doc.members.emplace_back(astToFormatSource.ASTToDoc(funcDecl.funcBody.get(), level, funcOptions));
}

void FuncDeclFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto funcDecl = As<ASTKind::FUNC_DECL>(node);
    AddFuncDecl(doc, *funcDecl, level, funcOptions);
}
} // namespace Codira::Format
