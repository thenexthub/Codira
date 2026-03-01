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

#include "StructuralRuleGITF04.h"
namespace Codira::CodeCheck {
using namespace Codira;
using namespace Codira::AST;
using namespace Meta;

void StructuralRuleGITF04::CheckFuncDeclParams(const Codira::AST::FuncDecl &funcDecl)
{
    if (funcDecl.funcBody == nullptr) {
        return;
    }
    if (funcDecl.ty) {
        Ptr<AST::FuncTy> funcTy = DynamicCast<AST::FuncTy*>(funcDecl.ty);
        if (funcTy && funcTy->retTy && funcTy->retTy->IsInterface()) {
            Diagnose(funcDecl.begin, funcDecl.end,
                CodeCheckDiagKind::G_ITF_04_avoid_directly_using_interfaces_as_types_02, funcDecl.identifier.Val());
        }
    }

    for (auto &paramList : funcDecl.funcBody->paramLists) {
        for (auto &param : paramList->params) {
            if (param->ty->IsInterface()) {
                Diagnose(param->begin, param->end,
                    CodeCheckDiagKind::G_ITF_04_avoid_directly_using_interfaces_as_types_01, param->identifier.Val());
            }
        }
    }
}

void StructuralRuleGITF04::FindFuncDecl(Codira::AST::Node *node)
{
    if (node == nullptr) {
        return;
    }

    Walker walker(node, [this](Node *node) -> VisitAction {
        match (*node)([this](const FuncDecl &funcDecl) { CheckFuncDeclParams(funcDecl); });
        return VisitAction::WALK_CHILDREN;
    });
    walker.Walk();
}

void StructuralRuleGITF04::MatchPattern(ASTContext &ctx, Ptr<Node> node)
{
    (void)ctx;
    FindFuncDecl(node);
}
}
