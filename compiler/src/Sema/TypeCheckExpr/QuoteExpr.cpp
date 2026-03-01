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

namespace {
void ChkIfImportLibAST(DiagnosticEngine& diag, const ImportManager& im, const QuoteExpr& qe)
{
    if (qe.GetFullPackageName() == AST_PACKAGE_NAME) {
        return;
    }
    auto importedPkgs = im.GetAllImportedPackages();
    for (auto& importedPkg : importedPkgs) {
        if (importedPkg->srcPackage && importedPkg->srcPackage->fullPackageName == AST_PACKAGE_NAME) {
            return;
        }
    }
    diag.DiagnoseRefactor(DiagKindRefactor::sema_use_expr_without_import, qe, "std.ast", "quote");
}
} // namespace

bool TypeChecker::TypeCheckerImpl::ChkQuoteExpr(ASTContext& ctx, Ty& target, QuoteExpr& qe)
{
    ChkIfImportLibAST(diag, importManager, qe);
    if (!Ty::IsTyCorrect(Synthesize(ctx, &qe))) {
        return false;
    }
    if (!typeManager.IsSubtype(qe.ty, &target)) {
        DiagMismatchedTypes(diag, qe, target);
        qe.ty = TypeManager::GetInvalidTy();
        return false;
    }
    if (qe.desugarExpr) {
        if (!Check(ctx, &target, qe.desugarExpr.get())) {
            qe.ty = TypeManager::GetInvalidTy();
            return false;
        }
    }
    return true;
}

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynQuoteExpr(ASTContext& ctx, QuoteExpr& qe)
{
    ChkIfImportLibAST(diag, importManager, qe);
    if (qe.desugarExpr) {
        qe.ty = Synthesize(ctx, qe.desugarExpr.get());
    } else {
        qe.ty = TypeManager::GetInvalidTy();
    }
    return qe.ty;
}
