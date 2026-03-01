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
#include "Codira/AST/Match.h"
#include "Codira/AST/Utils.h"

using namespace Codira;
using namespace AST;
using namespace TypeCheckUtil;
using namespace Sema::Desugar::AfterTypeCheck;

namespace {
/**
 * Insert unitExpr if the type of 'ifExpr' @p ie is unit type but the 'thenBody' is not the type of unit.
 * Also complete the 'elseBody' of the 'ifExpr' if it was not existed.
 */
void InsertUnitForIfExpr(TypeManager& tyMgr, IfExpr& ie)
{
    if (ie.desugarExpr) {
        return; // Ignore desugared expression.
    }
    // All expression after typecheck must be welltyped.
    CODEC_NULLPTR_CHECK(ie.ty);
    CODEC_NULLPTR_CHECK(ie.thenBody);
    // If the 'ifExpr' is not unit typed or the type of then body is the subtype of unit type, then quit process.
    auto skip = !ie.ty->IsUnit() || tyMgr.IsSubtype(ie.thenBody->ty, ie.ty);
    if (skip) {
        return;
    }
    // If the type of 'then' is not unit, then the block must not be empty.
    CODEC_ASSERT(!ie.thenBody->body.empty());
    auto unitExpr = CreateUnitExpr(ie.ty);
    CopyBasicInfo(ie.thenBody->body.back().get(), unitExpr.get());
    ie.thenBody->body.push_back(std::move(unitExpr));
    ie.thenBody->ty = ie.ty;
    // If current ifExpr dose not have elseBody, create for it.
    if (!ie.elseBody) {
        // Added 'else' does not need position.
        auto elseBody = MakeOwnedNode<Block>();
        elseBody->body.push_back(CreateUnitExpr(ie.ty));
        elseBody->ty = ie.ty;
        ie.elseBody = std::move(elseBody);
        ie.hasElse = true;
        AddCurFile(*ie.elseBody, ie.curFile);
    }
}
} // namespace

namespace Codira::Sema::Desugar::AfterTypeCheck {
void DesugarIfExpr(TypeManager& typeManager, IfExpr& ifExpr)
{
    InsertUnitForIfExpr(typeManager, ifExpr);
}
} // namespace Codira::Sema::Desugar::AfterTypeCheck
