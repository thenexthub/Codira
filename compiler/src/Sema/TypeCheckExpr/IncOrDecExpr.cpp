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

#include "DiagSuppressor.h"
#include "Diags.h"
#include "Codira/AST/RecoverDesugar.h"

using namespace Codira;
using namespace AST;
using namespace Sema;

bool TypeChecker::TypeCheckerImpl::ChkIncOrDecExpr(ASTContext& ctx, Ty& target, IncOrDecExpr& ide)
{
    if (!Ty::IsTyCorrect(SynIncOrDecExpr(ctx, ide))) {
        return false;
    }
    if (typeManager.IsSubtype(ide.ty, &target)) {
        return true;
    }
    DiagMismatchedTypesWithFoundTy(diag, ide, target, *ide.ty, "the type of an assignment expression is always 'Unit'");
    ide.ty = TypeManager::GetInvalidTy();
    return false;
}

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynIncOrDecExpr(ASTContext& ctx, IncOrDecExpr& ide)
{
    if (ide.desugarExpr == nullptr) { // `ide` or parent of `ide` is broken.
        return TypeManager::GetInvalidTy();
    }
    auto& ae = *StaticCast<AssignExpr*>(ide.desugarExpr.get());
    auto leftTy = Synthesize(ctx, ae.leftValue.get());
    if (!Ty::IsTyCorrect(leftTy)) {
        ide.ty = TypeManager::GetInvalidTy();
    } else if (!leftTy->IsInteger()) {
        DiagMismatchedTypesWithFoundTy(diag, *ae.leftValue, "integer type", leftTy->String(),
            "the base of increment or decrement expressions should be of integer type");
        ide.ty = TypeManager::GetInvalidTy();
    } else {
        if (ae.leftValue->astKind == ASTKind::SUBSCRIPT_EXPR && ae.leftValue->desugarExpr != nullptr) {
            RecoverToSubscriptExpr(StaticCast<SubscriptExpr&>(*ae.leftValue));
        }
        ide.ty = Synthesize(ctx, ide.desugarExpr.get());
    }
    return ide.ty;
}
