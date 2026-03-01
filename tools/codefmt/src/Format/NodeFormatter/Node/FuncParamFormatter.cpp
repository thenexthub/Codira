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

#include "Format/NodeFormatter/Node/FuncParamFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void FuncParamFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto param = As<ASTKind::FUNC_PARAM>(node);
    AddFuncParam(doc, *param, level, funcOptions);
}
void FuncParamFormatter::AddFuncParam(
    Doc& doc, const Codira::AST::FuncParam& funcParam, int level, FuncOptions funcOptions)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    doc.members.emplace_back(DocType::SOFTLINE, level, "");
    if (!funcParam.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, funcParam.annotations, level, false);
    }
    astToFormatSource.AddModifier(doc, funcParam.modifiers, level);
    if (funcParam.isMemberParam) {
        doc.members.emplace_back(DocType::STRING, level, funcParam.isVar ? "var " : "let ");
    }
    if (funcOptions.patternOrEnum) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(funcParam.type.get(), level));
    } else {
        std::string id = funcParam.identifier.GetRawText() + (funcParam.isNamedParam ? "!" : "");
        if (funcParam.type) {
            doc.members.emplace_back(DocType::STRING, level, id + ": ");
            doc.members.emplace_back(astToFormatSource.ASTToDoc(funcParam.type.get(), level));
        } else {
            doc.members.emplace_back(DocType::STRING, level, id);
        }
    }
    if (funcParam.assignment) {
        doc.members.emplace_back(DocType::STRING, level, " = ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(funcParam.assignment.get(), level));
    }
}
} // namespace Codira::Format
