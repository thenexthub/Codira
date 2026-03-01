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

#include "Diags.h"

using namespace Codira;
using namespace Sema;

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynOptionalChainExpr(ASTContext& ctx, OptionalChainExpr& oce)
{
    CODEC_NULLPTR_CHECK(oce.desugarExpr);
    oce.ty = Synthesize(ctx, oce.desugarExpr.get());
    return oce.ty;
}

bool TypeChecker::TypeCheckerImpl::ChkOptionalChainExpr(ASTContext& ctx, Ty& target, OptionalChainExpr& oce)
{
    CODEC_NULLPTR_CHECK(oce.desugarExpr);
    if (!Ty::IsTyCorrect(SynOptionalChainExpr(ctx, oce))) {
        return false;
    }
    if (!CheckOptionBox(target, *oce.desugarExpr->ty)) {
        DiagMismatchedTypes(diag, oce, target);
        oce.desugarExpr->ty = TypeManager::GetInvalidTy();
        oce.ty = TypeManager::GetInvalidTy();
        return false;
    }
    return true;
}
