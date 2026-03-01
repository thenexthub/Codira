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

#include "TypeCheckUtil.h"

#include "Codira/AST/Create.h"
#include "Codira/AST/Utils.h"

using namespace Codira;
using namespace TypeCheckUtil;

namespace Codira::Sema::Desugar::AfterTypeCheck {
/**
 * Desugar IsExpr to TypePattern of MatchExpr.
 * *************** before desugar ****************
 * e is T
 * *************** after desugar ****************
 * match (e) {
 *     case _: T => true
 *     case _ => false
 * }
 * */
void DesugarIsExpr(TypeManager& typeManager, IsExpr& ie)
{
    if (!Ty::IsTyCorrect(ie.ty) || !ie.ty->IsBoolean() || ie.desugarExpr) {
        return;
    }
    CODEC_NULLPTR_CHECK(ie.leftExpr);
    CODEC_NULLPTR_CHECK(ie.isType);
    CODEC_NULLPTR_CHECK(ie.leftExpr->ty);
    CODEC_NULLPTR_CHECK(ie.isType->ty);
    auto boolTy = ie.ty;
    std::vector<OwnedPtr<MatchCase>> matchCases;
    auto trueExpr = CreateLitConstExpr(LitConstKind::BOOL, "true", boolTy);
    auto falseExpr = CreateLitConstExpr(LitConstKind::BOOL, "false", boolTy);
    trueExpr->begin = ie.isPos;
    falseExpr->begin = ie.isPos;
    matchCases.emplace_back(CreateMatchCase(
        CreateRuntimePreparedTypePattern(
            typeManager, MakeOwnedNode<WildcardPattern>(), std::move(ie.isType), *ie.leftExpr
        ),
        std::move(trueExpr)));
    auto wildcard = MakeOwnedNode<WildcardPattern>();
    wildcard->ty = ie.leftExpr->ty;
    matchCases.emplace_back(
        CreateMatchCase(std::move(wildcard), std::move(falseExpr)));
    ie.desugarExpr = CreateMatchExpr(std::move(ie.leftExpr), std::move(matchCases), boolTy, Expr::SugarKind::IS);
    AddCurFile(*ie.desugarExpr, ie.curFile);
}
} // namespace Codira::Sema::Desugar::AfterTypeCheck
