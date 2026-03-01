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

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynResumeExpr(ASTContext& ctx, ResumeExpr& re)
{
    Ty* resumptionParamTy = typeManager.GetAnyTy();

    if (re.enclosing) {
        resumptionParamTy = (*re.enclosing)->commandResultTy;
        re.ty = TypeManager::GetNothingTy();
    } else {
        diag.DiagnoseRefactor(DiagKindRefactor::sema_implicit_resume_outside_handler, re);
        re.ty = TypeManager::GetInvalidTy();
    }

    if (re.throwingExpr) {
        auto exception = importManager.GetCoreDecl<ClassDecl>(CLASS_EXCEPTION);
        auto error = importManager.GetCoreDecl<ClassDecl>(CLASS_ERROR);
        Synthesize(ctx, re.throwingExpr.get());
        CODEC_NULLPTR_CHECK(re.throwingExpr->ty);

        if (!typeManager.IsSubtype(re.throwingExpr->ty, exception->ty) &&
            !typeManager.IsSubtype(re.throwingExpr->ty, error->ty)) {
            diag.DiagnoseRefactor(DiagKindRefactor::sema_resume_throwing_mismatch_type, re);
            re.ty = TypeManager::GetInvalidTy();
        }
    } else if (re.withExpr) {
        if (!Check(ctx, resumptionParamTy, re.withExpr)) {
            // `Check` produces the error message.
            re.ty = TypeManager::GetInvalidTy();
        }
    } else {
        auto unitTy = typeManager.GetPrimitiveTy(TypeKind::TYPE_UNIT);
        if (!typeManager.IsSubtype(resumptionParamTy, unitTy)) {
            diag.DiagnoseRefactor(DiagKindRefactor::sema_resume_no_with, re, Ty::ToString(resumptionParamTy));
            re.ty = TypeManager::GetInvalidTy();
        }
    }
    return re.ty;
}
