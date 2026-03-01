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

#include "Format/NodeFormatter/Type/TupleTypeFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;
void Codira::Format::TupleTypeFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto type = As<ASTKind::TUPLE_TYPE>(node);
    AddTupleType(doc, *type, level);
}

void TupleTypeFormatter::AddTupleType(Doc& doc, const Codira::AST::TupleType& tupleType, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (!tupleType.GetTypeParameterNameRawText().empty()) {
        doc.members.emplace_back(DocType::STRING, level, tupleType.GetTypeParameterNameRawText());
        doc.members.emplace_back(DocType::STRING, level, ": ");
    }
    doc.members.emplace_back(DocType::STRING, level, "(");
    for (auto& field : tupleType.fieldTypes) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(field.get(), level));
        if (field != tupleType.fieldTypes.back()) {
            doc.members.emplace_back(DocType::STRING, level, ", ");
            doc.members.emplace_back(DocType::SOFTLINE, level + 1, "");
        }
    }
    doc.members.emplace_back(DocType::STRING, level, ")");
}
} // namespace Codira::Format
