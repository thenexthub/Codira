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

#include "Codira/AST/Create.h"
#include "Codira/AST/Utils.h"

using namespace Codira;
using namespace AST;

// The desugar of a `SpawnArg` is the form `arg.getSchedulerHandle()`
void TypeChecker::TypeCheckerImpl::DesugarSpawnArgExpr(const ASTContext& ctx, const AST::SpawnExpr& se)
{
    if (!Ty::IsTyCorrect(se.ty) || se.arg->desugarExpr) {
        return;
    }
    Ptr<Expr> arg = se.arg.get();
    CODEC_ASSERT(arg && Ty::IsTyCorrect(arg->ty) && arg->ty->IsClassLike());
    // Get `getSchedulerHandle` method from spawn argument, whose signature is `()->CPointer<Unit>`.
    auto classLikeTy = StaticCast<ClassLikeTy*>(arg->ty);
    auto retTy = typeManager.GetPointerTy(TypeManager::GetPrimitiveTy(TypeKind::TYPE_UNIT));
    auto funcTy = typeManager.GetFunctionTy({}, retTy);
    const auto fieldName = "getSchedulerHandle";
    auto decls = FieldLookup(ctx, classLikeTy->commonDecl, fieldName, {.file = se.curFile});
    CODEC_ASSERT(decls.size() == 1); // `getSchedulerHandle(): CPointer<Unit>` is the private method.
    auto decl = decls.front();
    CODEC_ASSERT(Ty::IsTyCorrect(decl->ty) && decl->ty->IsFunc() && typeManager.IsSubtype(decl->ty, funcTy));
    auto ma = CreateMemberAccess(ASTCloner::Clone(arg), fieldName);
    CopyBasicInfo(arg, ma.get());
    auto ce = CreateCallExpr(std::move(ma), {});
    CopyBasicInfo(arg, ce.get());
    ce->callKind = CallKind::CALL_DECLARED_FUNCTION;
    ce->resolvedFunction = StaticCast<FuncDecl*>(decl);
    ce->ty = retTy;
    AddCurFile(*ce, se.curFile);
    arg->desugarExpr = std::move(ce);
}

// The `futureObj` of a `SpawnExpr` is `VarDecl` of the form `let futureObj = Future(task)`
// NOTE: This syntax sugar is stored in `futureObj` rather than `desugarExpr`.
void TypeChecker::TypeCheckerImpl::DesugarSpawnExpr(const ASTContext& ctx, AST::SpawnExpr& se)
{
    if (!Ty::IsTyCorrect(se.ty) || se.futureObj) {
        return;
    }
    Ptr<Expr> task = se.task.get();
    CODEC_ASSERT(task && Ty::IsTyCorrect(task->ty) && task->ty->IsFunc());
    Ptr<FuncTy> taskTy = RawStaticCast<FuncTy*>(task->ty);
    CODEC_ASSERT(se.ty->IsClass());
    Ptr<ClassDecl> futureClass = RawStaticCast<ClassTy*>(se.ty)->declPtr;
    CODEC_ASSERT(Ty::IsTyCorrect(futureClass->ty) && futureClass->ty->typeArgs.size() == 1);
    std::vector<Ptr<FuncDecl>> inits;
    for (auto& decl : futureClass->GetMemberDecls()) {
        CODEC_NULLPTR_CHECK(decl);
        if (decl->astKind == ASTKind::FUNC_DECL && decl.get()->TestAttr(Attribute::CONSTRUCTOR)) {
            inits.emplace_back(StaticCast<FuncDecl*>(decl.get()));
        }
    }
    CODEC_ASSERT(inits.size() == 1); // `init(fn: ()->T)` is the only constructor for `Future`
    auto initDecl = inits.front();
    CODEC_ASSERT(Ty::IsTyCorrect(initDecl->ty) && initDecl->ty->IsFunc());
    // Prepare the `baseFunc` of the `Future` function call.
    auto re = CreateRefExprInCore("Future");
    re->isAlone = false;
    re->ref.target = initDecl;
    re->instTys.emplace_back(taskTy->retTy);
    re->ty = typeManager.GetInstantiatedTy(
        initDecl->ty, {{StaticCast<GenericsTy*>(futureClass->ty->typeArgs.front()), taskTy->retTy}});
    CopyBasicInfo(task, re.get());
    // Prepare the arguments of the `CallExpr`.
    std::vector<OwnedPtr<FuncArg>> callArgs;
    auto fa = CreateFuncArg(std::move(se.task));
    CopyBasicInfo(task, fa.get());
    fa->ty = task->ty;
    callArgs.emplace_back(std::move(fa));
    // Create the `CallExpr`.
    auto ce = CreateCallExpr(std::move(re), std::move(callArgs));
    CopyBasicInfo(task, ce.get());
    ce->callKind = CallKind::CALL_OBJECT_CREATION;
    ce->resolvedFunction = initDecl;
    ce->ty = se.ty;

    auto futureObj = CreateVarDecl("futureObj", std::move(ce));
    CopyBasicInfo(task, futureObj.get());
    AddCurFile(*futureObj, se.curFile);
    se.futureObj = std::move(futureObj);

    if (!se.arg) {
        return;
    }
    DesugarSpawnArgExpr(ctx, se);
}
