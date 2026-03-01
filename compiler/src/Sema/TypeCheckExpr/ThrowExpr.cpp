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

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynThrowExpr(ASTContext& ctx, ThrowExpr& te)
{
    CODEC_NULLPTR_CHECK(te.expr); // Parser guarantees.
    Synthesize(ctx, te.expr.get());
    te.ty = TypeManager::GetNothingTy();
    if (!Ty::IsTyCorrect(te.expr->ty)) {
        return TypeManager::GetInvalidTy();
    }
    if (te.expr->ty->IsEnum()) {
        if (auto refExpr = DynamicCast<RefExpr*>(te.expr.get()); refExpr && refExpr->ref.identifier == RESOURCE_NAME) {
            return TypeManager::GetNothingTy();
        }
    } else if (te.expr->ty->IsClass() || te.expr->ty->IsGeneric()) {
        // Check if the type of expression thrown is derived from `core.Exception` class
        // For class type and generic type.
        auto exception = importManager.GetCoreDecl<ClassDecl>(CLASS_EXCEPTION);
        auto error = importManager.GetCoreDecl<ClassDecl>(CLASS_ERROR);
        bool foundClass = exception && error;
        if (foundClass &&
            (typeManager.IsSubtype(te.expr->ty, exception->ty) || typeManager.IsSubtype(te.expr->ty, error->ty))) {
            return TypeManager::GetNothingTy();
        }
    }
    diag.Diagnose(te, DiagKind::sema_throw_expr_with_wrong_type);
    return TypeManager::GetInvalidTy();
}
