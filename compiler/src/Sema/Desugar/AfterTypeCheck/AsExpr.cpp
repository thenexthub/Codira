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
using namespace AST;
using namespace TypeCheckUtil;

namespace {
OwnedPtr<CallExpr> CreateAsExprSomeCall(
    FuncDecl& someDecl, Type& theAsType, VarDecl& varDecl, FuncTy& someCtorTy)
{
    CODEC_ASSERT(someCtorTy.paramTys.size() == 1);
    auto some = MakeOwnedNode<CallExpr>();
    auto someRef = CreateRefExpr(someDecl);
    auto theAsTy = someCtorTy.paramTys[0];
    auto optionTy = someCtorTy.retTy;
    someRef->typeArguments.emplace_back(ASTCloner::Clone(Ptr(&theAsType)));
    someRef->instTys.emplace_back(theAsTy);
    someRef->ref.targets.emplace_back(&someDecl);
    someRef->isAlone = false;
    someRef->ty = &someCtorTy;
    someRef->callOrPattern = some.get();
    auto newVarRef = CreateRefExpr(varDecl);
    newVarRef->ty = theAsTy;
    auto someArg = MakeOwnedNode<FuncArg>();
    someArg->expr = std::move(newVarRef);
    someArg->ty = theAsTy;
    some->baseFunc = std::move(someRef);
    some->args.emplace_back(std::move(someArg));
    some->resolvedFunction = &someDecl;
    some->callKind = CallKind::CALL_DECLARED_FUNCTION;
    some->ty = optionTy;
    return some;
}
} // namespace

namespace Codira::Sema::Desugar::AfterTypeCheck {
/**
 * Desugar AsExpr to TypePattern of MatchExpr.
 * *************** before desugar ****************
 * e as T
 * *************** after desugar ****************
 * match (e) {
 *     case newVar : T => Some(newVar)
 *     case _ => None
 * }
 * */
void DesugarAsExpr(TypeManager& typeManager, AsExpr& ae)
{
    if (!Ty::IsTyCorrect(ae.ty) || !ae.ty->IsCoreOptionType() || ae.desugarExpr) {
        return;
    }
    CODEC_NULLPTR_CHECK(ae.leftExpr);
    CODEC_NULLPTR_CHECK(ae.asType);
    CODEC_NULLPTR_CHECK(ae.leftExpr->ty);
    CODEC_NULLPTR_CHECK(ae.asType->ty);
    auto optionTy = ae.ty;
    auto selectorTy = ae.leftExpr->ty;
    auto theAsType = ASTCloner::Clone(ae.asType.get());
    auto theAsTy = ae.asType->ty;
    auto optionDecl = StaticCast<EnumTy*>(optionTy)->decl;
    CODEC_NULLPTR_CHECK(optionDecl);
    auto someDecl = StaticCast<FuncDecl*>(LookupEnumMember(optionDecl, OPTION_VALUE_CTOR));
    CODEC_NULLPTR_CHECK(someDecl);
    std::vector<OwnedPtr<MatchCase>> matchCases;
    auto varPattern = CreateVarPattern(V_COMPILER, theAsTy);
    varPattern->begin = ae.asPos;
    varPattern->end = ae.asPos;
    auto varDecl = varPattern->varDecl.get();
    varDecl->fullPackageName = ae.GetFullPackageName();
    matchCases.emplace_back(
        CreateMatchCase(
            CreateRuntimePreparedTypePattern(typeManager, std::move(varPattern), std::move(ae.asType), *ae.leftExpr),
            CreateAsExprSomeCall(*someDecl, *theAsType, *varDecl, *typeManager.GetFunctionTy({theAsTy}, optionTy))));
    auto wildcard = MakeOwnedNode<WildcardPattern>();
    wildcard->ty = selectorTy;
    auto noneDecl = LookupEnumMember(optionDecl, OPTION_NONE_CTOR);
    CODEC_NULLPTR_CHECK(noneDecl);
    auto none = CreateRefExpr(*noneDecl);
    CopyBasicInfo(&ae, none.get());
    none->typeArguments.emplace_back(std::move(theAsType));
    none->instTys.emplace_back(theAsTy);
    none->ty = optionTy;
    matchCases.emplace_back(CreateMatchCase(std::move(wildcard), std::move(none)));
    ae.desugarExpr = CreateMatchExpr(std::move(ae.leftExpr), std::move(matchCases), optionTy, Expr::SugarKind::AS);
    CopyBasicInfo(&ae, ae.desugarExpr.get());
    AddCurFile(*ae.desugarExpr, ae.curFile);
}
} // namespace Codira::Sema::Desugar::AfterTypeCheck
