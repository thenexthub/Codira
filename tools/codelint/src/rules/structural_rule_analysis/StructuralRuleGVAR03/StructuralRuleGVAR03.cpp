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

#include "StructuralRuleGVAR03.h"

using namespace Codira;
using namespace Codira::AST;
using namespace Meta;
using namespace CodeCheck;

/**
 * Avoid using global variables
 * Print warning while declaring or referring global variables
 */

void StructuralRuleGVAR03::CheckVarDecl(const VarDecl &varDecl)
{
    if (varDecl.TestAttr(Attribute::GLOBAL)) {
        Diagnose(varDecl.begin, varDecl.end, CodeCheckDiagKind::G_VAR_03_global_variable_declaration_information,
            varDecl.identifier.Val());
    }
}

void StructuralRuleGVAR03::FindGlobalVar(Ptr<Node> node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const VarDecl &varDecl) {
                CheckVarDecl(varDecl);
                return VisitAction::SKIP_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGVAR03::MatchPattern(ASTContext &ctx, Ptr<Node> node)
{
    (void)ctx;
    FindGlobalVar(node);
}
