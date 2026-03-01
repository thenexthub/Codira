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
#include "TypeCheckUtil.h"
#include "ExtraScopes.h"

using namespace Codira;
using namespace AST;
using namespace Sema;
using namespace TypeCheckUtil;
using namespace Utils;

namespace {
// we should clear the nodes in the block and then recheck the body according to the new `target`.
void ClearLambdaBodyForReCheck(const LambdaExpr& le)
{
    Walker(le.funcBody->body.get(), [](Ptr<Node> node) {
        if (Is<Type>(node) || Is<Generic>(node)) {
            // In the `PreCheck` stage:
            // The `Ty` of `Type` was set by `GetTyFromASTType`.
            // The `Ty` of `GenericParamDecl` was set by `SetDeclTy`.
            return VisitAction::SKIP_CHILDREN;
        } else if (Is<FuncParam>(node) && node->TestAttr(AST::Attribute::HAS_INITIAL)) {
            // The sugar for default parameter was set by `AddDefaultFunction` in the `PrepareTypeCheck` stage.
            return VisitAction::SKIP_CHILDREN;
        } else {
            node->Clear();
            return VisitAction::WALK_CHILDREN;
        }
    }).Walk();
}

bool IsAnyParamTypeOmitted(const LambdaExpr& le)
{
    CODEC_ASSERT(le.funcBody && !le.funcBody->paramLists.empty());
    auto& paramList = le.funcBody->paramLists[0];
    return std::any_of(paramList->params.cbegin(), paramList->params.cend(), [](auto& param) {
        return param->type == nullptr;
    });
}

void ClearCacheForNames(ASTContext& ctx, const LambdaExpr& le, const std::vector<std::string>& paramNames)
{
    std::vector<Ptr<const Node>> parentStack;
    std::unordered_set<Ptr<const Node>> clearedNodes;
    std::unordered_set<std::string> aliases(paramNames.cbegin(), paramNames.cend()); // potential aliases of parameters
    std::unordered_map<Ptr<const Node>, std::vector<std::string>> aliasDecl;
    std::unordered_map<Ptr<const Node>, std::vector<std::string>> aliasShadow; // a simplistic version of symbol table
    auto shadowName = [&aliases, &aliasShadow](const LambdaExpr& le) {
        CODEC_ASSERT(le.funcBody && !le.funcBody->paramLists.empty());
        for (auto& param : le.funcBody->paramLists[0]->params) {
            CODEC_NULLPTR_CHECK(param);
            auto id = param->identifier;
            if (aliases.count(id) > 0) {
                aliasShadow[&le].push_back(id);
                aliases.erase(id);
            }
        }
    };
    auto preAction = [&ctx, &aliases, &clearedNodes, &parentStack, &shadowName](Ptr<Node> node) -> VisitAction {
        parentStack.push_back(node);
        if (auto re = DynamicCast<RefExpr*>(node); re && aliases.count(re->ref.identifier) > 0) {
            for (size_t i = parentStack.size(); i > 0; i--) {
                auto p = parentStack[i - 1];
                if (clearedNodes.count(p) > 0) {
                    break;
                }
                ctx.RemoveTypeCheckCache(*p);
                clearedNodes.insert(p);
            }
        } else if (auto lam = DynamicCast<LambdaExpr*>(node)) {
            shadowName(*lam); // shadow by local var is too complicated to track. skip here
        }
        return aliases.empty() ? VisitAction::SKIP_CHILDREN : VisitAction::WALK_CHILDREN;
    };
    auto postAction = [&aliases, &aliasDecl, &aliasShadow, &parentStack, &clearedNodes](
                          Ptr<const Node> /* node */) -> VisitAction {
        auto top = parentStack.back();
        parentStack.pop_back();
        if (auto decl = DynamicCast<VarDecl*>(top); decl && clearedNodes.count(decl) > 0) {
            aliases.insert(decl->identifier); // VarDecl is cleared ==> it contains alias ==> the var is a new alias
            aliasDecl[parentStack.back()].push_back(decl->identifier);
        }
        for (auto name : aliasShadow[top]) {
            aliases.insert(name);
        }
        for (auto name : aliasDecl[top]) {
            aliases.erase(name);
        }
        return VisitAction::WALK_CHILDREN;
    };
    Walker(le.funcBody->body.get(), preAction, postAction).Walk();
}

void ClearInvalidTypeCheckCache(ASTContext& ctx, const LambdaExpr& le, const Ty& target)
{
    if ((ctx.lastTargetTypeMap.count(&le) == 0 || ctx.lastTargetTypeMap[&le] != &target) &&
        IsAnyParamTypeOmitted(le)) {
        std::vector<std::string> names;
        CODEC_ASSERT(le.funcBody && !le.funcBody->paramLists.empty());
        for (auto& param : le.funcBody->paramLists[0]->params) {
            CODEC_NULLPTR_CHECK(param);
            if (!param->type) {
                names.push_back(param->identifier);
            }
        }
        ClearCacheForNames(ctx, le, names);
    }
}

// Return true if lambda has any mismatched parameter type and any of the parameter's type is omitted.
bool IsLambdaIncompatible(const LambdaExpr& le, bool paramsMismatched)
{
    return paramsMismatched && IsAnyParamTypeOmitted(le);
}

// return: true if error, false if no error
bool DiagInferParamTyFail(DiagnosticEngine& diag, LambdaExpr& le)
{
    for (auto& node : le.funcBody->paramLists[0]->params) {
        if (!node->type) {
            diag.DiagnoseRefactor(DiagKindRefactor::sema_lambdaExpr_must_have_type_annotation, *node);
            le.ty = TypeManager::GetInvalidTy();
            return true;
        }
    }
    return false;
}

// if it's a member func call to a param, return the param's id and possible type decls
// otherwise return empty string and empty set
std::pair<std::string, std::set<Ptr<Decl>>> FindCandidatesFromCall(
    ASTContext& ctx, const CallExpr& ce, const std::map<std::string, Ptr<FuncParam>>& unsolvedParams)
{
    if (auto ma = DynamicCast<MemberAccess*>(ce.baseFunc.get())) {
        if (auto re = DynamicCast<RefExpr*>(ma->baseExpr.get())) {
            if (unsolvedParams.count(re->ref.identifier) > 0) {
                auto id = re->ref.identifier;
                auto sig = MemSig{ma->field, false, ce.args.size(), ma->typeArguments.size()};
                return {id, ctx.Mem2Decls(sig)};
            }
        }
    }
    return {"", {}};
}

// if it's a member access to a param, return the param's id and possible type decls
// otherwise return empty string and empty set
std::pair<std::string, std::set<Ptr<Decl>>> FindCandidatesFromAccess(
    ASTContext& ctx, MemberAccess& ma, const std::map<std::string, Ptr<FuncParam>>& unsolvedParams)
{
    if (auto re = DynamicCast<RefExpr*>(ma.baseExpr.get())) {
        if (unsolvedParams.count(re->ref.identifier) > 0) {
            auto id = re->ref.identifier;
            auto sig = MemSig{ma.field, true};
            return {id, ctx.Mem2Decls(sig)};
        }
    }
    return {"", {}};
}
} // namespace

void TypeChecker::TypeCheckerImpl::ResetLambdaForReinfer(ASTContext& ctx, const AST::LambdaExpr& le)
{
    le.funcBody->retType.reset(nullptr);
    AddRetTypeNode(*le.funcBody);
    le.funcBody->Clear();
    Walker(le.funcBody->body.get(), [](Ptr<Node> node) {
        if ((Is<Decl>(node) || Is<Expr>(node)) && !Ty::IsInitialTy(node->ty)) {
            node->Clear();
        }
        return VisitAction::WALK_CHILDREN;
    }).Walk();
    ctx.ClearTypeCheckCache(*le.funcBody);
}

bool TypeChecker::TypeCheckerImpl::SolveLamExprParamTys(ASTContext& ctx, AST::LambdaExpr& le)
{
    auto tyVars = typeManager.GetInnermostUnsolvedTyVars();
    auto sol = SolveConstraints(typeManager.constraints);
    bool successful = false;
    if (sol && !HasUnsolvedTyVars(*sol, tyVars)) {
        ResetLambdaForReinfer(ctx, le);
        for (auto& node : le.funcBody->paramLists[0]->params) {
            node->ty = typeManager.GetInstantiatedTy(node->ty, *sol);
        }
        if (Ty::IsTyCorrect(Synthesize(ctx, le.funcBody.get())) && le.funcBody->body &&
            Ty::IsTyCorrect(le.funcBody->body->ty)) {
            le.ty = le.funcBody->ty;
            successful = true;
        }
    }
    if (!successful && DiagInferParamTyFail(diag, le)) {
        return false;
    }
    return true;
}

void TypeChecker::TypeCheckerImpl::TryInferFromSyntaxInfo(ASTContext& ctx, const AST::LambdaExpr& le)
{
    if (typeManager.GetInnermostUnsolvedTyVars().empty()) {
        return;
    }
    std::map<std::string, Ptr<FuncParam>> unsolvedParams;
    std::map<std::string, std::set<Ptr<Decl>>> candidates;
    for (auto& param : le.funcBody->paramLists[0]->params) {
        if (param->ty->IsPlaceholder()) {
            unsolvedParams[param->identifier] = param;
        }
    }
    if (unsolvedParams.empty()) {
        return;
    }

    auto candiHandler = [&candidates](const std::pair<std::string, std::set<Ptr<Decl>>>& candi) {
        if (candi.first.empty()) {
            return false;
        }
        if (candidates.count(candi.first) == 0) {
            candidates[candi.first] = candi.second;
        } else {
            EraseIf(candidates[candi.first], [&candi](Ptr<Decl> d) { return candi.second.count(d) == 0; });
        }
        return true;
    };
    std::function<VisitAction(Ptr<Node>)> memberScanner = [&ctx, &unsolvedParams, &candiHandler, &memberScanner](
                                                              Ptr<Node> n) -> VisitAction {
        if (auto ce = DynamicCast<CallExpr*>(n)) {
            if (candiHandler(FindCandidatesFromCall(ctx, *ce, unsolvedParams))) {
                // need to skip baseFunc to avoid handling it again as an access to var/prop
                for (auto& arg : ce->args) {
                    Walker(arg.get(), memberScanner).Walk();
                }
                return VisitAction::SKIP_CHILDREN;
            }
        } else if (auto ma = DynamicCast<MemberAccess*>(n)) {
            candiHandler(FindCandidatesFromAccess(ctx, *ma, unsolvedParams));
        }
        return VisitAction::WALK_CHILDREN;
    };
    Walker(le.funcBody.get(), memberScanner).Walk();

    for (auto& [id, decls] : candidates) {
        TryEnforceCandidate(*StaticCast<TyVar*>(unsolvedParams[id]->ty), decls, typeManager);
    }
}

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynLamExpr(ASTContext& ctx, LambdaExpr& le)
{
    if (le.funcBody == nullptr || le.funcBody->paramLists.empty()) {
        return TypeManager::GetInvalidTy();
    }
    // Lambda's return type is added by compiler, so reset it here to allow re-checking.
    le.funcBody->retType.reset(nullptr);
    AddRetTypeNode(*le.funcBody);
    le.funcBody->retType->begin = le.begin;
    le.funcBody->retType->end = le.end;
    CODEC_ASSERT(le.funcBody && !le.funcBody->paramLists.empty());
    TyVarScope sc(typeManager);
    for (auto& node : le.funcBody->paramLists[0]->params) {
        CODEC_NULLPTR_CHECK(node);
        Ptr<Ty> ty = Synthesize(ctx, node->type.get());
        if (Ty::IsTyCorrect(ty)) {
            node->ty = ty;
        } else if (ctx.funcArgReachable.count(&le) == 0 && !node->type) {
            node->ty = typeManager.AllocTyVar("T-Lam", true);
        } else {
            diag.DiagnoseRefactor(DiagKindRefactor::sema_lambdaExpr_must_have_type_annotation, *node);
            le.ty = TypeManager::GetInvalidTy();
            return le.ty;
        }
    }
    TryInferFromSyntaxInfo(ctx, le);
    bool skipSolving = typeManager.GetInnermostUnsolvedTyVars().empty();
    Ptr<Ty> leTy = nullptr;
    {
        DiagSuppressor ds(diag);
        leTy = Synthesize(ctx, le.funcBody.get());
        if (skipSolving) {
            ds.ReportDiag();
        }
    }
    // `funcBody` containing invalid expressions can have valid types.
    // We have to use `funcBody->body` to determine if there are errors.
    if (!Ty::IsTyCorrect(leTy) || !le.funcBody->body ||
        !Ty::IsTyCorrect(le.funcBody->body->ty)) {
        le.ty = TypeManager::GetInvalidTy();
        if (DiagInferParamTyFail(diag, le)) {
            return le.ty;
        }
    } else {
        if (skipSolving) {
            le.ty = le.funcBody->ty;
        } else {
            if (!SolveLamExprParamTys(ctx, le)) {
                return le.ty;
            }
        }
    }
    if (auto ft = DynamicCast<FuncTy*>(le.ty)) {
        le.funcBody->retType->ty = ft->retTy;
    } else {
        le.funcBody->retType->ty = TypeManager::GetInvalidTy();
    }
    return le.ty;
}

bool TypeChecker::TypeCheckerImpl::ChkLamExpr(ASTContext& ctx, Ty& target, LambdaExpr& le)
{
    CODEC_ASSERT(le.funcBody != nullptr && !le.funcBody->paramLists.empty());
    ClearInvalidTypeCheckCache(ctx, le, target);
    ctx.lastTargetTypeMap[&le] = &target;
    if (target.IsAny() || (le.TestAttr(AST::Attribute::C) && target.IsCType())) {
        le.ty = Synthesize(ctx, &le);
        (void)ReplaceIdealTy(le);
        return Ty::IsTyCorrect(le.ty);
    }
    Ptr<Ty> targetTy = TypeCheckUtil::UnboxOptionType(&target);
    if (!Ty::IsTyCorrect(targetTy) || !targetTy->IsFunc()) {
        auto ds = DiagSuppressor(diag);
        auto synTy = Synthesize(ctx, &le);
        le.ty = TypeManager::GetInvalidTy();
        if (!ds.HasError() && Ty::IsTyCorrect(synTy)) { // Only report type mismatch when no error happens.
            DiagMismatchedTypesWithFoundTy(diag, le, target, *synTy);
        }
        ds.ReportDiag();
        return false;
    }

    { // Suppress errors if check lambda failed. Only report when type check succeed.
        auto ds = DiagSuppressor(diag);
        auto tgtTy = StaticCast<FuncTy*>(targetTy);
        std::vector<Ptr<Ty>> lamParamTys;
        // Check arguments' types against parameters' types.
        bool paramsMatched = ChkLamParamTys(ctx, le, tgtTy->paramTys, lamParamTys);
        // In the check mode, a lambda's return type is explicitly given.
        // To ease the following check, we create a retType node from the given target type.
        le.funcBody->retType = MakeOwned<RefType>();
        le.funcBody->retType->begin = le.begin;
        le.funcBody->retType->end = le.end;
        le.funcBody->retType->ty = tgtTy->retTy;
        le.funcBody->retType->EnableAttr(AST::Attribute::COMPILER_ADD);
        ClearLambdaBodyForReCheck(le);

        // Should not be short-circuited.
        if (ChkLamBody(ctx, *le.funcBody) && paramsMatched) {
            ds.ReportDiag();
            // The call to GetFunctionTy is necessary to create (cached) CPointer types if necessary.
            le.funcBody->ty = typeManager.GetFunctionTy(lamParamTys, StaticCast<FuncTy*>(le.funcBody->ty)->retTy,
                {tgtTy->isC, tgtTy->isClosureTy, tgtTy->hasVariableLenArg});
            le.ty = le.funcBody->ty;
            return true;
        } else if (IsLambdaIncompatible(le, !paramsMatched)) {
            // User omitted parameter's type. We should quit early.
            ds.ReportDiag();
            return false;
        }
    }
    // In the LSP that uses macros, the ast before the macro expansion needs to save the complete ty information to
    // prevent 'Synthesize' skipping child nodes in the lambda body due to cache.
    if (!ci->invocation.globalOptions.enableMacroInLSP) {
        ClearLambdaBodyForReCheck(le);
    }
    le.funcBody->retType.reset(nullptr);
    auto bodyTy = Synthesize(ctx, le.funcBody.get());
    if (Ty::IsTyCorrect(bodyTy) && !typeManager.IsSubtype(bodyTy, targetTy)) {
        DiagMismatchedTypesWithFoundTy(diag, le, *targetTy, *bodyTy);
    }
    le.funcBody->ty = TypeManager::GetInvalidTy();
    le.ty = le.funcBody->ty;
    return false;
}

bool TypeChecker::TypeCheckerImpl::ChkLamParamTys(
    ASTContext& ctx, LambdaExpr& le, const std::vector<Ptr<AST::Ty>>& tgtParamTys, std::vector<Ptr<Ty>>& lamParamTys)
{
    CODEC_ASSERT(le.funcBody && !le.funcBody->paramLists.empty());
    if (le.funcBody->paramLists[0]->params.size() != tgtParamTys.size()) {
        auto& paramList = le.funcBody->paramLists[0];
        auto builder = diag.DiagnoseRefactor(DiagKindRefactor::sema_param_miss_match, *paramList);
        builder.AddMainHintArguments(std::to_string(tgtParamTys.size()), std::to_string(paramList->params.size()));
        return false;
    }
    size_t i = 0;
    for (auto& node : le.funcBody->paramLists[0]->params) {
        CODEC_NULLPTR_CHECK(node);
        Ptr<Ty> paramTy = tgtParamTys[i]; // Caller guarantees the 'paramTy' is valid.
        if (!Check(ctx, paramTy, node.get())) {
            le.ty = TypeManager::GetInvalidTy();
            return false;
        }
        // Lambda's parameter type not support auto box.
        // NOTE: lambda should not report parameter type mismatch, only report lambda type mismatchs.
        if (!typeManager.IsSubtype(paramTy, node->ty, false, false)) {
            le.ty = TypeManager::GetInvalidTy();
            return false;
        }
        lamParamTys.push_back(node->ty);
        i++;
    }
    return true;
}

bool TypeChecker::TypeCheckerImpl::ChkLamBody(ASTContext& ctx, FuncBody& lamFb)
{
    if (CheckFuncBody(ctx, lamFb) && Ty::IsTyCorrect(lamFb.ty) && Ty::IsTyCorrect(lamFb.body->ty)) {
        return true;
    }
    // Since the return type of lambda body is added in 'ChkLamExpr', all type mismatching errors are reported before.
    lamFb.ty = TypeManager::GetInvalidTy();
    return false;
}
