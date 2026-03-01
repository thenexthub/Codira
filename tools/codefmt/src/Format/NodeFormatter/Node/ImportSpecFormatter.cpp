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

#include "Format/NodeFormatter/Node/ImportSpecFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void ImportSpecFormatter::AddImportSpec(Doc& doc, const Codira::AST::ImportSpec& importSpec, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (importSpec.TestAttr(Attribute::COMPILER_ADD)) {
        return;
    }

    if (!importSpec.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, importSpec.annotations, level);
    }

    if (importSpec.modifier) {
        astToFormatSource.AddModifier(doc, *importSpec.modifier, level);
    }

    const auto& ic = importSpec.content;
    doc.members.emplace_back(DocType::STRING, level, "import ");
    if (ic.kind != ImportKind::IMPORT_MULTI) {
        AddImportContent(doc, ic, level);
        return;
    }

    if (auto prefix = Utils::JoinStrings(ic.prefixPaths, "."); !prefix.empty()) {
        doc.members.emplace_back(DocType::STRING, level, prefix + ".");
    }
    if (ic.leftCurlPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, "{");
    }
    auto it = ic.items.begin();
    if (it != ic.items.end()) {
        doc.members.emplace_back(DocType::SOFTLINE, level + 1, "");
        AddImportContent(doc, *it, level);
        ++it;
    }
    for (; it != ic.items.end(); ++it) {
        doc.members.emplace_back(DocType::STRING, level, ", ");
        doc.members.emplace_back(DocType::SOFTLINE, level + 1, "");
        AddImportContent(doc, *it, level);
    }
    if (ic.rightCurlPos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::SOFTLINE, level + 1, "");
        doc.members.emplace_back(DocType::STRING, level, "}");
    }
}

void ImportSpecFormatter::AddImportContent(Doc& doc, const Codira::AST::ImportContent& content, int level)
{
    if (content.kind == ImportKind::IMPORT_MULTI) {
        return;
    }
    auto prefix = Utils::JoinStrings(content.prefixPaths, ".");
    doc.members.emplace_back(
        DocType::STRING, level, prefix.empty() ? content.identifier : prefix + "." + content.identifier);
    if (content.kind == ImportKind::IMPORT_ALIAS) {
        doc.members.emplace_back(DocType::STRING, level, " as ");
        if (!content.aliasName.Val().empty()) {
            doc.members.emplace_back(DocType::STRING, level, content.aliasName.Val());
        }
    }
}

void ImportSpecFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto importSpec = As<ASTKind::IMPORT_SPEC>(node);
    AddImportSpec(doc, *importSpec, level);
}
} // namespace Codira::Format
