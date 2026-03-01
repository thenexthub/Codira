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

#include "StructuralRuleGPKG01.h"

namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;

void StructuralRuleGPKG01::FindImportNode(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        match (*node)(
            [this](const ImportSpec &importSpec) {
                CheckImportItemName(importSpec);
            });
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

void StructuralRuleGPKG01::CheckImportItemName(const Codira::AST::ImportSpec& importSpec)
{
    if (importSpec.importPos == INVALID_POSITION || importSpec.TestAttr(AST::Attribute::COMPILER_ADD)) {
        return;
    }
    const auto& ic = importSpec.content;
    if (ic.kind == AST::ImportKind::IMPORT_ALL) {
        Diagnose(ic.begin, ic.end, CodeCheckDiagKind::G_PKG_01_avoid_wildcard,
            ic.prefixPaths.empty() ? std::string() : Utils::JoinStrings(ic.prefixPaths, "."));
    } else if (ic.kind == AST::ImportKind::IMPORT_MULTI) {
        for (const auto& item : ic.items) {
            if (item.kind == AST::ImportKind::IMPORT_ALL) {
                Diagnose(item.begin, item.end, CodeCheckDiagKind::G_PKG_01_avoid_wildcard,
                    item.prefixPaths.empty() ? std::string() : Utils::JoinStrings(item.prefixPaths, "."));
            }
        }
    }
}

void StructuralRuleGPKG01::MatchPattern(ASTContext &ctx, Ptr<Node> node)
{
    (void)ctx;
    FindImportNode(node);
}
}
