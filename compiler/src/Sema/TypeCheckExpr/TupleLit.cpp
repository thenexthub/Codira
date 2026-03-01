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
#include "TypeCheckUtil.h"

using namespace Codira;
using namespace Sema;
using namespace TypeCheckUtil;

bool TypeChecker::TypeCheckerImpl::ChkTupleLit(ASTContext& ctx, Ty& target, TupleLit& tl)
{
    if (target.IsAny()) {
        tl.ty = Synthesize(ctx, &tl);
        ReplaceIdealTy(tl);
        return Ty::IsTyCorrect(tl.ty);
    }
    Ptr<Ty> targetTy = UnboxOptionType(&target);
    if (!Ty::IsTyCorrect(targetTy) || !targetTy->IsTuple()) {
        DiagMismatchedTypesWithFoundTy(diag, tl, targetTy->String(), "Tuple");
        tl.ty = TypeManager::GetNonNullTy(tl.ty);
        return false;
    }
    auto tupleTy = StaticCast<TupleTy*>(targetTy);
    auto typeArgs = tupleTy->typeArgs;
    if (typeArgs.size() != tl.children.size()) {
        tl.ty = Synthesize(ctx, &tl);
        ReplaceIdealTy(tl);
        DiagMismatchedTypes(diag, tl, *targetTy);
        return false;
    }
    // If the size of target elemTys and elements are equal, check one by one.
    std::vector<Ptr<Ty>> realElemTys;
    for (size_t i = 0; i < typeArgs.size(); ++i) {
        CODEC_NULLPTR_CHECK(tl.children[i]);
        if (!Check(ctx, typeArgs[i], tl.children[i].get())) {
            if (Ty::IsTyCorrect(typeArgs[i]) && Ty::IsTyCorrect(tl.children[i]->ty)) {
                DiagMismatchedTypes(diag, *tl.children[i], *typeArgs[i]);
            }
            tl.ty = Synthesize(ctx, &tl);
            ReplaceIdealTy(tl);
            return false;
        } else {
            realElemTys.push_back(tl.children[i]->ty);
        }
    }
    // Should use SetTy(), but have bugs on ideal type, use Join() instead at current stage.
    // TupleLit allow elements been boxed by given target type.
    // Eg. Option<Int64>*Option<Int64> <=> (1,1) or I1*I1 <=> (1,1) where Int64 extends I1 allow box.
    tl.ty = targetTy;
    return true;
}

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynTupleLit(ASTContext& ctx, TupleLit& tl)
{
    std::vector<Ptr<Ty>> elemTy;
    // Synthesize the type of each element.
    for (auto& it : tl.children) {
        if (!it) {
            tl.ty = TypeManager::GetInvalidTy();
            return tl.ty;
        }
        if (!Ty::IsTyCorrect(it->ty)) {
            (void)Synthesize(ctx, it.get());
        }
        ReplaceIdealTy(*it);
        elemTy.push_back(it->ty);
    }
    tl.ty = typeManager.GetTupleTy(elemTy);
    return tl.ty;
}
