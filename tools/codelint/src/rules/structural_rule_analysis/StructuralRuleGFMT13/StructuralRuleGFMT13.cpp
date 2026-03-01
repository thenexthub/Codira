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

#include "StructuralRuleGFMT13.h"
#include "Codira/Basic/Match.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;

void StructuralRuleGFMT13::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    FindComments(node);
}
void StructuralRuleGFMT13::FindComments(Ptr<Codira::AST::Node>& node)
{
    if (!node) {
        return;
    }
    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        if (node->astKind == ASTKind::FILE) {
            auto pFile = As<ASTKind::FILE>(node.get());
            GetTopLevelComments(pFile);
        }
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

bool StructuralRuleGFMT13::ContainsCopyright(const std::string& str)
{
    std::regex pattern(R"(Copyright\s*\(c\)|版权所有\s*\(c\))");

    return std::regex_search(str, pattern);
}

void StructuralRuleGFMT13::AnalyzeComments(std::vector<AST::CommentGroup>& leadingComments, Codira::Position& pos)
{
    if (leadingComments.empty()) {
        Diagnose(pos, pos, CodeCheckDiagKind::G_FMT_13_header_comments_copyright_01);
        return;
    }
    auto comment = leadingComments[0].cms[0].info.Value();
    if (!ContainsCopyright(comment)) {
        Diagnose(pos, pos, CodeCheckDiagKind::G_FMT_13_header_comments_copyright_01);
    } else {
        bool isBlockComment = leadingComments[0].cms[0].kind == CommentKind::BLOCK;
        if (!isBlockComment) {
            Diagnose(pos, pos, CodeCheckDiagKind::G_FMT_13_header_comments_copyright_02);
        }
    }
}

void StructuralRuleGFMT13::GetTopLevelComments(Ptr<Codira::AST::File> pFile)
{
    if (!pFile->comments.leadingComments.empty()) {
        AnalyzeComments(pFile->comments.leadingComments, pFile->begin);
        return;
    }
    if (pFile->package) {
        AnalyzeComments(pFile->package->comments.leadingComments, pFile->begin);
        return;
    }
    if (!pFile->imports.empty() && !pFile->imports[0]->begin.IsZero()) {
        AnalyzeComments(pFile->imports[0]->comments.leadingComments, pFile->begin);
        return;
    }
    if (!pFile->decls.empty()) {
        Ptr<Decl> decl = pFile->decls[0]->GetDesugarDecl() ? pFile->decls[0]->GetDesugarDecl() : pFile->decls[0].get();
        if (!decl) {
            return;
        }
        AnalyzeComments(decl->comments.leadingComments, pFile->begin);
        return;
    }
}
} // namespace Codira::CodeCheck
