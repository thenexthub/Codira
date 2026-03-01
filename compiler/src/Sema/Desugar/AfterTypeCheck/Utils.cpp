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

#include "Desugar/AfterTypeCheck.h"

#include "Codira/AST/Create.h"
#include "Codira/AST/Walker.h"
#include "Codira/AST/ASTCasting.h"

namespace Codira::Sema::Desugar::AfterTypeCheck {
Ptr<Decl> LookupEnumMember(Ptr<Decl> decl, const std::string& identifier)
{
    if (decl == nullptr || decl->astKind != ASTKind::ENUM_DECL) {
        return nullptr;
    }
    auto enumDecl = RawStaticCast<EnumDecl*>(decl);
    for (auto& member : enumDecl->constructors) {
        if (member->identifier == identifier) {
            return member.get();
        }
    }
    return nullptr;
}

void UnitifyBlock(const Expr& posSrc, Block& b, Ty& unitTy)
{
    auto unitExpr = CreateUnitExpr();
    unitExpr->begin = posSrc.begin;
    unitExpr->begin.Mark(PositionStatus::IGNORE);
    unitExpr->end = posSrc.end;
    unitExpr->ty = &unitTy;
    b.body.push_back(std::move(unitExpr));
    b.ty = &unitTy;
}

void RearrangeRefLoop(const Expr& src, Expr& dst, Ptr<Node> loopBody)
{
    if (loopBody == nullptr) {
        return;
    }
    std::function<VisitAction(Ptr<Node>)> visitFunc = [&src, &dst](Ptr<Node> node) {
        if (auto je = DynamicCast<JumpExpr*>(node); je) {
            if (je->refLoop == &src) {
                je->refLoop = &dst;
            }
        }
        // skip the nested loop structure and lambda
        if (node->astKind == ASTKind::FUNC_DECL || node->astKind == ASTKind::LAMBDA_EXPR) {
            return VisitAction::SKIP_CHILDREN;
        }
        return VisitAction::WALK_CHILDREN;
    };
    Walker(loopBody, visitFunc).Walk();
}
} // namespace Codira::Sema::Desugar::AfterTypeCheck
