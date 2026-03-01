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

#include "Format/NodeFormatter/Type/OptionTypeFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;
void Codira::Format::OptionTypeFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto type = As<ASTKind::OPTION_TYPE>(node);
    AddOptionType(doc, *type, level);
}

void OptionTypeFormatter::AddOptionType(Doc& doc, const Codira::AST::OptionType& optionType, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    if (!optionType.GetTypeParameterNameRawText().empty()) {
        doc.members.emplace_back(DocType::STRING, level, optionType.GetTypeParameterNameRawText());
        doc.members.emplace_back(DocType::STRING, level, ": ");
    }
    for (size_t i = 0; i < optionType.questVector.size(); ++i) {
        doc.members.emplace_back(DocType::STRING, level, "?");
    }
    if (optionType.componentType) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(optionType.componentType.get(), level));
    }
}
} // namespace Codira::Format
