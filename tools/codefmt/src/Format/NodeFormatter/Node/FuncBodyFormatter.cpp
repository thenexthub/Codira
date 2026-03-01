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

#include "Format/NodeFormatter/Node/FuncBodyFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void FuncBodyFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto funcBody = As<ASTKind::FUNC_BODY>(node);
    AddFuncBody(doc, *funcBody, level, funcOptions);
}

void FuncBodyFormatter::AddFuncBody(
    Doc& doc, const Codira::AST::FuncBody& funcBody, int level, FuncOptions funcOptions)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    auto& generic = funcBody.generic;
    if (generic) {
        astToFormatSource.AddGenericParams(doc, *generic, level);
    }
    AddFuncBodyIsLambda(doc, funcBody, level, funcOptions);

    if (funcBody.retType && !funcBody.retType->TestAttr(Attribute::COMPILER_ADD)) {
        doc.members.emplace_back(DocType::STRING, level, ": ");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(funcBody.retType.get(), level));
    }

    if (generic) {
        astToFormatSource.AddGenericBound(doc, *generic, level);
    }

    int changedlevel = level;
    if (funcBody.doubleArrowPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, funcBody.paramLists[0]->params.empty() ? "=>" : " =>");
        if (funcOptions.isLambda && funcBody.body) {
            if (funcBody.body->body.size() > 1) {
                changedlevel++;
                doc.members.emplace_back(DocType::LINE, changedlevel, "");
            } else if (funcBody.body->body.size() == 1) {
                doc.members.emplace_back(DocType::STRING, level, " ");
            }
        }
    }
    if (funcBody.body) {
        if (funcBody.body->leftCurlPos == INVALID_POSITION && funcBody.body->rightCurlPos == INVALID_POSITION) {
            doc.members.emplace_back(DocType::STRING, level, " ");
        }
    }
    funcOptions.patternOrEnum = funcBody.doubleArrowPos != INVALID_POSITION;
    doc.members.emplace_back(astToFormatSource.ASTToDoc(funcBody.body.get(), changedlevel, funcOptions));
}

void FuncBodyFormatter::AddFuncBodyIsLambda(
    Doc& doc, const Codira::AST::FuncBody& funcBody, int level, FuncOptions funcOptions)
{
    if (funcOptions.isLambda) {
        auto paramList = funcBody.paramLists[0].get();
        for (auto& it : paramList->params) {
            doc.members.emplace_back(astToFormatSource.ASTToDoc(it.get(), level, funcOptions));
            if (it != paramList->params.back()) {
                doc.members.emplace_back(DocType::STRING, level, ", ");
                doc.members.emplace_back(DocType::SOFTLINE, level + 1, "");
            }
        }
    } else {
        for (auto& it : funcBody.paramLists) {
            doc.members.emplace_back(astToFormatSource.ASTToDoc(it.get(), level, funcOptions));
        }
    }
}
} // namespace Codira::Format
