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

#include "Format/NodeFormatter/Node/AnnotationFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void AnnotationFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto annotation = As<ASTKind::ANNOTATION>(node);
    AddAnnotation(doc, *annotation, level);
}

void AnnotationFormatter::AddAnnotation(Doc& doc, const Codira::AST::Annotation& annotation, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    std::string compileTimeVisibleStr = annotation.isCompileTimeVisible ? "!" : "";
    doc.members.emplace_back(DocType::STRING, level, "@" + compileTimeVisibleStr + annotation.identifier);
    if (!annotation.args.empty()) {
        doc.members.emplace_back(DocType::STRING, level, "[");
        for (auto& arg : annotation.args) {
            doc.members.emplace_back(astToFormatSource.ASTToDoc(arg.get(), level));
            if (arg->commaPos != INVALID_POSITION) {
                doc.members.emplace_back(DocType::STRING, level, ",");
            }
            if (arg != annotation.args.back()) {
                doc.members.emplace_back(DocType::STRING, level, " ");
            }
        }
        doc.members.emplace_back(DocType::STRING, level, "]");
    }
    if (annotation.condExpr) {
        doc.members.emplace_back(DocType::STRING, level, "[");
        doc.members.emplace_back(astToFormatSource.ASTToDoc(annotation.condExpr.get(), level));
        doc.members.emplace_back(DocType::STRING, level, "]");
    }
}
} // namespace Codira::Format
