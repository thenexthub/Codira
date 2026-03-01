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

#include "Format/NodeFormatter/Expr/BlockFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void BlockFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions& funcOptions)
{
    auto block = As<ASTKind::BLOCK>(node);
    AddBlock(doc, *block, level, funcOptions);
}

void BlockFormatter::AddBlock(Doc& doc, const Codira::AST::Block& block, int level, FuncOptions funcOptions)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (funcOptions.isLambda) {
        AddBlockIsLambda(doc, block, level);
        return;
    }
    if (block.leftCurlPos == INVALID_POSITION && block.rightCurlPos == INVALID_POSITION) {
        // match case block
        astToFormatSource.AddBodyMembers(doc, block.body, level);
        return;
    }

    if (block.body.empty()) {
        astToFormatSource.AddEmptyBody(doc, block, level, block.leftCurlPos.line == block.rightCurlPos.line);
        return;
    }

    if (block.TestAttr(Attribute::UNSAFE)) {
        doc.members.emplace_back(DocType::STRING, level, "unsafe");
        block.body.size() == 1 &&
            block.body.back()->end.line == block.rightCurlPos.line ?
            AddSameLineCurl(doc, block, level) : AddDiffLineCurl(doc, block, level);
        return;
    }
    AddDiffLineCurl(doc, block, level);
}

void BlockFormatter::AddBlockIsLambda(Doc& doc, const Codira::AST::Block& block, int level)
{
    int lastEndLine = -1;
    for (auto& n : block.body) {
        if (lastEndLine != -1) {
            if (n->begin.line > lastEndLine + 1) {
                doc.members.emplace_back(DocType::SEPARATE, level, "");
            }
            doc.members.emplace_back(DocType::LINE, level, "");
        }
        doc.members.emplace_back(astToFormatSource.ASTToDoc(n.get(), level));

        lastEndLine = n->end.line;
    }
}

void BlockFormatter::AddSameLineCurl(Doc& doc, const Codira::AST::Block& block, int level)
{
    doc.members.emplace_back(DocType::STRING, level, " { ");
    for (auto& n : block.body) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(n.get(), level));
    }
    doc.members.emplace_back(DocType::STRING, level, " }");
    if (block.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}

void BlockFormatter::AddDiffLineCurl(Doc& doc, const Codira::AST::Block& block, int level)
{
    doc.members.emplace_back(DocType::STRING, level, " {");
    doc.members.emplace_back(DocType::LINE, level + 1, "");
    astToFormatSource.AddBodyMembers(doc, block.body, level + 1);
    doc.members.emplace_back(DocType::LINE, level, "");
    doc.members.emplace_back(DocType::STRING, level, "}");
    if (block.hasSemi) {
        doc.members.emplace_back(DocType::STRING, level, ";");
    }
}
} // namespace Codira::Format
