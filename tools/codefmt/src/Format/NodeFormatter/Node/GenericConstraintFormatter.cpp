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

#include "Format/NodeFormatter/Node/GenericConstraintFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void GenericConstraintFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto genericConstraint = As<ASTKind::GENERIC_CONSTRAINT>(node);
    AddGenericConstraint(doc, *genericConstraint, level);
}
void GenericConstraintFormatter::AddGenericConstraint(
    Doc& doc, const Codira::AST::GenericConstraint& genericConstraint, int level)
{
    doc.type = DocType::GROUP;
    doc.indent = level;

    if (genericConstraint.wherePos != INVALID_POSITION) {
        doc.members.emplace_back(DocType::STRING, level, " where ");
    }
    doc.members.emplace_back(astToFormatSource.ASTToDoc(genericConstraint.type.get()));
    doc.members.emplace_back(DocType::STRING, level, " <: ");
    for (auto& ub : genericConstraint.upperBounds) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(ub.get()));
        if (ub != genericConstraint.upperBounds.back()) {
            doc.members.emplace_back(DocType::STRING, level, " & ");
        }
    }
}
} // namespace Codira::Format
