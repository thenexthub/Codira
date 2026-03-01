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

#include "Format/NodeFormatter/Pattern/VarOrEnumPatternFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void VarOrEnumPatternFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto pattern = As<ASTKind::VAR_OR_ENUM_PATTERN>(node);
    AddVarOrEnumPattern(doc, *pattern, level);
}

void VarOrEnumPatternFormatter::AddVarOrEnumPattern(
    Doc& doc, const Codira::AST::VarOrEnumPattern& varOrEnumPattern, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    doc.members.emplace_back(DocType::STRING, level, varOrEnumPattern.identifier.GetRawText());
}
} // namespace Codira::Format
