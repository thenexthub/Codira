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

#include <vector>
#include "Codira/AST/Node.h"
#include "Format/ASTToFormatSource.h"
#include "Format/NodeFormatter/Node/FileFormatter.h"

namespace Codira::Format {
using namespace Codira::AST;

namespace {
std::vector<Codira::Position> GetWhenOrForeignPositions(const AST::Decl& decl)
{
    std::vector<Codira::Position> result;
    for (auto& ann : decl.annotations) {
        result.push_back(ann->begin);
    }
    for (auto& mod : decl.modifiers) {
        if (mod.modifier == TokenKind::FOREIGN) {
            result.push_back(mod.begin);
        }
    }
    return result;
}

void AddPackageSpec(Doc& doc, const Codira::AST::File& file, ASTToFormatSource& astToFormatSource, int level)
{
    if (file.package) {
        doc.members.emplace_back(astToFormatSource.ASTToDoc(file.package.get(), level));
    }
}

void AddImportSpecs(Doc& doc, const Codira::AST::File& file, ASTToFormatSource& astToFormatSource, int level)
{
    if (file.imports.empty()) {
        return;
    }

    if (file.package) {
        doc.members.emplace_back(DocType::SEPARATE, level, "");
        doc.members.emplace_back(DocType::SEPARATE, level, "");
    }

    for (unsigned int i = 0; i < file.imports.size(); i++) {
        auto importSpec = file.imports[i].get();
        if (i != 0 && !importSpec->TestAttr(Attribute::COMPILER_ADD)) {
            doc.members.emplace_back(DocType::LINE, level, "");
        }
        doc.members.emplace_back(astToFormatSource.ASTToDoc(importSpec, level));
    }
}

void AddBlockPrefix(const OwnedPtr<Decl>& decl, std::vector<Position>& beforeBlockAnnotationsPositions, Doc& doc,
    ASTToFormatSource& astToFormatSource, int level)
{
    std::vector<Ptr<Annotation>> annotationsToRender;
    for (auto& ann : decl->annotations) {
        for (auto beginPos : beforeBlockAnnotationsPositions) {
            if (ann->begin == beginPos) {
                annotationsToRender.push_back(ann.get());
            }
        }
    }

    std::set<Modifier> modifiersToRender;
    for (auto& mod : decl->modifiers) {
        for (auto beginPos : beforeBlockAnnotationsPositions) {
            if (mod.begin == beginPos) {
                (void)modifiersToRender.insert(mod);
            }
        }
    }

    for (auto it = annotationsToRender.begin(); it != annotationsToRender.end(); ++it) {
        if (it != annotationsToRender.begin()) {
            doc.members.emplace_back(DocType::LINE, level, "");
        }
        doc.members.emplace_back(astToFormatSource.ASTToDoc(*it));
    }
    if (!annotationsToRender.empty() && !modifiersToRender.empty()) {
        doc.members.emplace_back(DocType::LINE, level, "");
    }

    astToFormatSource.AddModifier(doc, modifiersToRender, level);

    if (modifiersToRender.empty()) {
        doc.members.emplace_back(DocType::STRING, level, " ");
    }

    doc.members.emplace_back(DocType::STRING, level, "{");
    doc.members.emplace_back(DocType::LINE, level + 1, "");
}

void AddBlockSuffix(Doc& doc, int level)
{
    doc.members.emplace_back(DocType::LINE, level, "");
    doc.members.emplace_back(DocType::STRING, level, "}");
}

void ShouldLastBlockADDSpearate(
    Doc& doc, int level, const File& file, std::vector<OwnedPtr<Codira::AST::Decl>>::const_iterator it)
{
    if (it->get()->astKind == AST::ASTKind::VAR_DECL && it != file.decls.end() - 1 &&
        std::next(it)->get()->astKind == AST::ASTKind::VAR_DECL) {
        doc.members.emplace_back(DocType::SEPARATE, level, "");
    }
}

void EraseModifiersAndAnnotationsForDecl(
    const OwnedPtr<Decl>& decl, std::vector<Position>& beforeBlockAnnotationsPositions)
{
    for (auto& beginPos : beforeBlockAnnotationsPositions) {
        auto& annotations = decl->annotations;
        auto annotationToErase = std::find_if(
            annotations.begin(), annotations.end(), [beginPos](auto& ann) { return ann->begin == beginPos; });
        if (annotationToErase != annotations.end()) {
            (void)annotations.erase(annotationToErase);
        }

        auto& modifiers = decl->modifiers;
        auto modifierToErase =
            std::find_if(modifiers.begin(), modifiers.end(), [beginPos](auto& m) { return m.begin == beginPos; });
        if (modifierToErase != modifiers.end()) {
            (void)modifiers.erase(modifierToErase);
        }
    }
}
} // namespace

void FileFormatter::AddFile(Doc& doc, const Codira::AST::File& file, int level)
{
    doc.type = DocType::CONCAT;
    doc.indent = level;
    AddPackageSpec(doc, file, astToFormatSource, level);
    AddImportSpecs(doc, file, astToFormatSource, level);

    std::unordered_map<Codira::Position, int, PositionHasher> modifierOrAnnoToPosMap;
    SetModifierOrAnnoToPosMap(file, modifierOrAnnoToPosMap);

    if (file.package || !file.imports.empty()) {
        doc.members.emplace_back(DocType::SEPARATE, level, "");
        doc.members.emplace_back(DocType::SEPARATE, level, "");
    }
    auto preDeclIsVar = false;
    auto mapCopy(modifierOrAnnoToPosMap);
    for (auto it = file.decls.begin(); it != file.decls.end(); ++it) {
        bool isFirstInBlock = false;
        bool isLastInBlock = false;

        std::vector<Codira::Position> beforeBlockAnnotationsPositions;
        for (auto pos : GetWhenOrForeignPositions(*it->get())) {
            if (modifierOrAnnoToPosMap.find(pos) != modifierOrAnnoToPosMap.end() && modifierOrAnnoToPosMap[pos] > 1) {
                beforeBlockAnnotationsPositions.emplace_back(pos);
                if (mapCopy[pos] == modifierOrAnnoToPosMap[pos]) {
                    isFirstInBlock = true;
                }
                mapCopy[pos]--;
                if (mapCopy[pos] == 0) {
                    isLastInBlock = true;
                }
            }
        }

        auto declarationLevel = beforeBlockAnnotationsPositions.empty() ? level : level + 1;
        if (*it != file.decls.front()) {
            if (preDeclIsVar) {
                if (it->get()->astKind != AST::ASTKind::VAR_DECL) {
                    doc.members.emplace_back(DocType::SEPARATE, level, "");
                }
                doc.members.emplace_back(DocType::LINE, isFirstInBlock ? level : declarationLevel, "");
            } else {
                doc.members.emplace_back(DocType::SEPARATE, level, "");
                doc.members.emplace_back(DocType::LINE, isFirstInBlock ? level : declarationLevel, "");
            }
        }
        if (isFirstInBlock) {
            AddBlockPrefix(*it, beforeBlockAnnotationsPositions, doc, astToFormatSource, level);
        }
        EraseModifiersAndAnnotationsForDecl(*it, beforeBlockAnnotationsPositions);
        preDeclIsVar = it->get()->astKind == ASTKind::VAR_DECL;
        doc.members.emplace_back(astToFormatSource.ASTToDoc(*it, declarationLevel));
        if (isLastInBlock) {
            AddBlockSuffix(doc, level);
            ShouldLastBlockADDSpearate(doc, level, file, it);
        }
    }
}

void FileFormatter::ASTToDoc(Doc& doc, Ptr<Codira::AST::Node> node, int level, FuncOptions&)
{
    auto file = As<ASTKind::FILE>(node);
    AddFile(doc, *file, level);
}

void FileFormatter::SetModifierOrAnnoToPosMap(
    const Codira::AST::File& file, std::unordered_map<Codira::Position, int, PositionHasher>& modifierOrAnnoToPosMap)
{
    for (auto& decl : file.decls) {
        for (auto pos : GetWhenOrForeignPositions(*decl)) {
            ++modifierOrAnnoToPosMap[pos];
        }
    }
}
} // namespace Codira::Format
