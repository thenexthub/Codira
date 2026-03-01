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

#include "TypeCheckerImpl.h"

using namespace Codira;
using namespace AST;

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynParenExpr(ASTContext& ctx, ParenExpr& pe)
{
    (void)Synthesize(ctx, pe.expr.get());
    if (!pe.expr || !Ty::IsTyCorrect(pe.expr->ty)) {
        pe.ty = TypeManager::GetInvalidTy();
        return TypeManager::GetInvalidTy();
    }

    if (pe.expr->ty->IsIdeal()) {
        ReplaceIdealTy(*pe.expr);
    }
    pe.ty = pe.expr->ty;
    if (pe.expr->isConst) {
        pe.isConst = true;
        pe.constNumValue = pe.expr->constNumValue;
    }
    return pe.ty;
}

bool TypeChecker::TypeCheckerImpl::ChkParenExpr(ASTContext& ctx, Ty& target, ParenExpr& pe)
{
    if (Check(ctx, &target, pe.expr.get())) {
        CODEC_NULLPTR_CHECK(pe.expr); // When the Check's result is true, pe.expr must not be nullptr.
        pe.ty = pe.expr->ty;
        if (pe.expr->isConst) {
            pe.isConst = true;
            pe.constNumValue = pe.expr->constNumValue;
        }
        return true;
    } else {
        pe.ty = TypeManager::GetInvalidTy();
        return false;
    }
}
