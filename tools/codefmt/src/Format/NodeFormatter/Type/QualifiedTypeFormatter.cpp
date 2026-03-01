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

#include "Format/NodeFormatter/Type/QualifiedTypeFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;
void Codira::Format::QualifiedTypeFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto qualifiedType = As<ASTKind::QUALIFIED_TYPE>(node);
    AddQualifiedType(doc, *qualifiedType, level);
}

void QualifiedTypeFormatter::AddQualifiedType(Doc& doc, const QualifiedType& qualifiedType, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (!qualifiedType.GetTypeParameterNameRawText().empty()) {
        doc.members.emplace_back(DocType::STRING, level, qualifiedType.GetTypeParameterNameRawText());
        doc.members.emplace_back(DocType::STRING, level, ": ");
    }
    if (qualifiedType.baseType) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(qualifiedType.baseType.get(), level));
    }
    doc.members.emplace_back(DocType::STRING, level, ".");
    doc.members.emplace_back(DocType::STRING, level, qualifiedType.field);
    if (qualifiedType.leftAnglePos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, "<");
        for (auto& argument : qualifiedType.typeArguments) {
            doc.members.emplace_back(astToFormatSource.ASTToDoc(argument.get(), level));
            if (argument != qualifiedType.typeArguments.back()) {
                doc.members.emplace_back(DocType::STRING, level, ", ");
            }
        }
        doc.members.emplace_back(DocType::STRING, level, ">");
    }
}
} // namespace Codira::Format
