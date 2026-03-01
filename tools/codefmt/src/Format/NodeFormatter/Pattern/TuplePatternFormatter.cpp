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

#include "Format/NodeFormatter/Pattern/TuplePatternFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;
void Codira::Format::TuplePatternFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto pattern = As<ASTKind::TUPLE_PATTERN>(node);
    AddTuplePattern(doc, *pattern, level);
}

void TuplePatternFormatter::AddTuplePattern(Doc& doc, const Codira::AST::TuplePattern& tuplePattern, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    doc.members.emplace_back(DocType::STRING, level, "(");
    for (auto& par : tuplePattern.patterns) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(par.get(), level + 1));
        if (par != tuplePattern.patterns.back()) {
            doc.members.emplace_back(DocType::STRING, level + 1, ", ");
        }
    }
    doc.members.emplace_back(DocType::STRING, level, ")");
}
} // namespace Codira::Format
