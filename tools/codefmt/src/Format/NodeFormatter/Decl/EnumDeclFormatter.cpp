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

#include "Format/NodeFormatter/Decl/EnumDeclFormatter.h"
#include "Format/ASTToFormatSource.h"
#include "Codira/AST/Node.h"

namespace Codira::Format {
using namespace Codira::AST;

void EnumDeclFormatter::AddEnumDecl(Doc& doc, const Codira::AST::EnumDecl& enumDecl, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;

    if (!enumDecl.annotations.empty()) {
        astToFormatSource.AddAnnotations(doc, enumDecl.annotations, level);
    }
    astToFormatSource.AddModifier(doc, enumDecl.modifiers, level);
    doc.members.emplace_back(DocType::STRING, level, "enum ");
    doc.members.emplace_back(DocType::STRING, level, enumDecl.identifier.GetRawText());
    auto& generic = enumDecl.generic;
    if (generic) {
        astToFormatSource.AddGenericParams(doc, *generic, level);
    }
    if (!enumDecl.inheritedTypes.empty()) {
        AddEnumInheritedTypes(doc, enumDecl, level);
    }
    if (generic) {
        astToFormatSource.AddGenericBound(doc, *generic, level);
    }
    doc.members.emplace_back(DocType::STRING, level, " {");
    doc.members.emplace_back(DocType::LINE, level + 1, "");
    if (enumDecl.constructors.size() > 1 &&
        enumDecl.constructors.front()->begin.line != enumDecl.constructors.back()->begin.line) {
        AddEnumBreakLineConstructors(doc, enumDecl, level);
        return;
    }
    Doc valueGroup(DocType::GROUP, level + 1, "");
    FuncOptions funcOptions;
    funcOptions.patternOrEnum = true;
    for (auto& cons : enumDecl.constructors) {
        if (cons != enumDecl.constructors.front() || HasFirstOptionalSeparator(enumDecl)) {
            valueGroup.members.emplace_back(DocType::STRING, level + 1, "| ");
        }
        valueGroup.members.emplace_back(astToFormatSource.ASTToDoc(cons.get(), level + 1, funcOptions));
        if (cons != enumDecl.constructors.back()) {
            valueGroup.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
        }
    }
    if (enumDecl.hasEllipsis) {
        valueGroup.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
        valueGroup.members.emplace_back(DocType::STRING, level + 1, "| ...");
    }
    doc.members.emplace_back(valueGroup);
    AddEnumMembers(doc, enumDecl, level);
    doc.members.emplace_back(DocType::LINE, level, "");
    doc.members.emplace_back(DocType::STRING, level, "}");
}

void EnumDeclFormatter::AddEnumMembers(Doc& doc, const Codira::AST::EnumDecl& enumDecl, int level)
{
    if (!enumDecl.members.empty()) {
        doc.members.emplace_back(DocType::SEPARATE, level + 1, "");
        doc.members.emplace_back(DocType::LINE, level + 1, "");
        astToFormatSource.AddBodyMembers(doc, enumDecl.members, level + 1);
    }
}

void EnumDeclFormatter::AddEnumInheritedTypes(Doc& doc, const Codira::AST::EnumDecl& enumDecl, int level)
{
    doc.members.emplace_back(DocType::STRING, level, " <: ");
    Doc group(DocType::GROUP, level + 1, "");
    for (auto& type : enumDecl.inheritedTypes) {
        group.members.emplace_back(astToFormatSource.ASTToDoc(type.get(), level + 1));
        if (type != enumDecl.inheritedTypes.back()) {
            group.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
            group.members.emplace_back(DocType::STRING, level + 1, "&");
            group.members.emplace_back(DocType::SOFTLINE_WITH_SPACE, level + 1, "");
        }
    }
    doc.members.emplace_back(group);
}

void EnumDeclFormatter::AddEnumBreakLineConstructors(Doc& doc, const Codira::AST::EnumDecl& enumDecl, int level)
{
    auto& constructors = enumDecl.constructors;
    bool firstConstructorHasBitOr = false;
    if (!constructors.empty() && !enumDecl.bitOrPosVector.empty() &&
        (constructors.front()->begin > enumDecl.bitOrPosVector.front())) {
        firstConstructorHasBitOr = true;
    }
    bool first = true;
    FuncOptions funcOptions;
    funcOptions.patternOrEnum = true;
    for (auto& cons : constructors) {
        if (!first || firstConstructorHasBitOr) {
            doc.members.emplace_back(DocType::STRING, level + 1, "| ");
        }
        first = false;
        doc.members.emplace_back(astToFormatSource.ASTToDoc(cons.get(), level + 1, funcOptions));
        if (cons != constructors.back()) {
            doc.members.emplace_back(DocType::LINE, level + 1, "");
        }
    }
    if (enumDecl.hasEllipsis) {
        doc.members.emplace_back(DocType::LINE, level + 1, "");
        doc.members.emplace_back(DocType::STRING, level + 1, "| ...");
    }
    if (!enumDecl.members.empty()) {
        doc.members.emplace_back(DocType::SEPARATE, level + 1, "");
        doc.members.emplace_back(DocType::LINE, level + 1, "");
        astToFormatSource.AddBodyMembers(doc, enumDecl.members, level + 1);
    }
    doc.members.emplace_back(DocType::LINE, level, "");
    doc.members.emplace_back(DocType::STRING, level, "}");
}

void EnumDeclFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto enumDecl = As<ASTKind::ENUM_DECL>(node);
    AddEnumDecl(doc, *enumDecl, level);
}

bool EnumDeclFormatter::HasFirstOptionalSeparator(const EnumDecl& ed)
{
    if (!ed.constructors.empty() && !ed.bitOrPosVector.empty() &&
        (ed.constructors.front()->begin > ed.bitOrPosVector.front())) {
        return true;
    }
    if (ed.bitOrPosVector.empty()) { // enum E { }
        return false;
    }
    if (ed.constructors.empty()) { // enum E { | }
        return true;
    }
    return ed.bitOrPosVector.front() < ed.constructors.front()->GetBegin();
}
} // namespace Codira::Format
