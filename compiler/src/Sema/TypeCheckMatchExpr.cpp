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

/**
 * @file
 *
 * This file implements typecheck apis for MatchExpr.
 */

#include <algorithm>
#include <memory>
#include <set>
#include <variant>
#include <vector>

#include "Diags.h"
#include "JoinAndMeet.h"
#include "PatternUsefulness.h"
#include "TypeCheckerImpl.h"

#include "Codira/AST/Node.h"
#include "Codira/AST/Walker.h"
#include "Codira/Basic/DiagnosticEngine.h"
#include "Codira/Sema/TypeManager.h"
#include "Codira/Utils/CheckUtils.h"
#include "Codira/Utils/Utils.h"

using namespace Codira;
using namespace AST;
using namespace Sema;

namespace {
const std::unordered_map<ASTKind, std::string> PATTERN_ASTKIND_TO_STRING_MAP = {
    {ASTKind::CONST_PATTERN, "constant"},
    {ASTKind::WILDCARD_PATTERN, "wildcard"},
    {ASTKind::VAR_PATTERN, "binding"},
    {ASTKind::TUPLE_PATTERN, "tuple"},
    {ASTKind::TYPE_PATTERN, "type"},
    {ASTKind::ENUM_PATTERN, "enum"},
};

void CheckMatchExprSetTy(MatchExpr& me, Ptr<Ty> target, TypeManager& typeManager, ImportManager& impMgr,
    const std::set<Ptr<Ty>>& matchCaseTys)
{
    // If any of branch equal to the target type, do not join branches' types (avoiding unexpected common supertype).
    if (matchCaseTys.find(target) != matchCaseTys.end()) {
        me.ty = target;
        return;
    }
    auto joinAndMeet = JoinAndMeet(typeManager, matchCaseTys, {}, &impMgr, me.curFile);
    auto joinRes = joinAndMeet.JoinAsVisibleTy();
    me.ty = std::get_if<Ptr<Ty>>(&joinRes) ? std::get<Ptr<Ty>>(joinRes) : target;
}
} // namespace

namespace Codira {
Ptr<Ty> TypeChecker::TypeCheckerImpl::SynMatchExpr(ASTContext& ctx, MatchExpr& me)
{
    if (me.selector) {
        return SynMatchExprHasSelector(ctx, me);
    }
    return SynMatchExprNoSelector(ctx, me);
}

bool TypeChecker::TypeCheckerImpl::ChkMatchExpr(ASTContext& ctx, Ty& target, MatchExpr& me)
{
    if (me.selector) {
        return ChkMatchExprHasSelector(ctx, target, me);
    }
    return ChkMatchExprNoSelector(ctx, target, me);
}

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynMatchExprHasSelector(ASTContext& ctx, MatchExpr& me)
{
    // Synthesize selector's ty.
    CODEC_NULLPTR_CHECK(me.selector);
    Synthesize(ctx, me.selector.get());
    ReplaceIdealTy(*me.selector);

    // Check each case.
    if (me.sugarKind == Expr::SugarKind::QUEST) {
        return SynQuestSugarMatchCaseBody(ctx, me);
    } else {
        // NoSugar, As, Is
        return SynNormalMatchCaseBody(ctx, me);
    }
}

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynNormalMatchCaseBody(ASTContext& ctx, MatchExpr& me)
{
    std::set<Ptr<Ty>> matchCaseTyVec;
    CODEC_NULLPTR_CHECK(me.selector);
    auto selectorTy = me.selector->ty;
    bool isMatchCorrect = !me.matchCases.empty();
    for (auto& mc : me.matchCases) {
        CODEC_NULLPTR_CHECK(mc);
        CODEC_ASSERT(!mc->patterns.empty());
        mc->SetCtxExprForPatterns(me.selector.get());
        if (Ty::IsTyCorrect(selectorTy)) {
            // Check whether pattern is compatible with selectorTy.
            bool isPatOK = ChkMatchCasePatterns(ctx, selectorTy, *mc);
            // Check the actions in a match case anyway.
            bool isGuardOK = ChkMatchCasePatGuard(ctx, *mc);
            bool areActionsOK = ChkMatchCaseActions(ctx, nullptr, *mc);
            if (isPatOK && isGuardOK && areActionsOK) {
                matchCaseTyVec.insert(mc->ty);
            } else {
                isMatchCorrect = false;
            }
        }
    }
    if (!isMatchCorrect) {
        me.ty = TypeManager::GetInvalidTy();
        return me.ty;
    }

    if (me.selector->ty->HasPlaceholder()) {
        me.selector->ty = typeManager.TryGreedySubst(me.selector->ty);
        for (auto& mc : me.matchCases) {
            (void)ChkMatchCasePatterns(ctx, me.selector->ty, *mc);
        }
    }
    // Join match expr's ty.
    auto joinAndMeet = JoinAndMeet(typeManager, matchCaseTyVec, {}, &importManager, me.curFile);
    auto joinRes = joinAndMeet.JoinAsVisibleTy();
    if (auto optErrs = JoinAndMeet::SetJoinedType(me.ty, joinRes)) {
        if (me.sugarKind == Expr::SugarKind::IF_LET) {
            auto builder = diag.Diagnose(me, DiagKind::sema_diag_report_error_message,
                "types " + Ty::ToString(me.matchCases[0]->ty) + " and " + Ty::ToString(me.matchCases[1]->ty) +
                    " of the two branches of this 'if' expression mismatch");
            builder.AddNote(*optErrs);
        } else {
            auto builder = diag.Diagnose(me, DiagKind::sema_type_incompatible, "MatchCase");
            builder.AddNote(*optErrs);
        }
    }

    // Check pattern exhaustiveness and set unreachable attr of match cases.
    // Hotfix: ignore for desugared matchExpr.
    if (Ty::IsTyCorrect(me.ty) && me.sugarKind == Expr::SugarKind::NO_SUGAR &&
        !PatternUsefulness::CheckMatchExprHasSelectorExhaustivenessAndReachability(ctx.diag, typeManager, me)) {
        me.ty = TypeManager::GetInvalidTy();
        return me.ty;
    }
    return me.ty;
}

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynQuestSugarMatchCaseBody(ASTContext& ctx, MatchExpr& me)
{
    auto selectorTy = me.selector->ty;
    if (!Ty::IsTyCorrect(selectorTy)) {
        me.ty = TypeManager::GetInvalidTy();
        return me.ty;
    } else if (!selectorTy->IsCoreOptionType()) {
        auto builder = diag.DiagnoseRefactor(DiagKindRefactor::sema_optional_chain_non_optional, me);
        builder.AddMainHintArguments(selectorTy->String());
        me.ty = TypeManager::GetInvalidTy();
        return me.ty;
    }
    // Match desugared from e? always have 2 cases.
    CODEC_ASSERT(me.matchCases.size() == 2);
    auto& mc0 = me.matchCases[0];
    // Match desugared from e? always have patterns.
    CODEC_ASSERT(mc0->patterns.size() == 1);
    CODEC_NULLPTR_CHECK(mc0->patterns.front());
    mc0->SetCtxExprForPatterns(me.selector.get());
    // Desugared patterns are always compatible with selectorTy. Therefore, no error will be raised if e is correct.
    (void)ChkMatchCasePatterns(ctx, selectorTy, *mc0);
    // Desugared patterns have no pattern guards and hence skip the invocation of ChkMatchCasePatGuard.
    (void)ChkMatchCaseActions(ctx, nullptr, *mc0);

    auto& mc1 = me.matchCases[1];
    CODEC_ASSERT(mc1->patterns.size() == 1);
    CODEC_NULLPTR_CHECK(mc1->patterns.front());
    mc1->SetCtxExprForPatterns(me.selector.get());
    ChkMatchCasePatterns(ctx, selectorTy, *mc1);
    // Case 1 is merely a None constructor, whose type is the SAME as the type of CASE0. Return value can be ignored.
    (void)Check(ctx, mc0->ty, mc1->exprOrDecls.get());
    mc1->ty = mc1->exprOrDecls->ty;

    me.ty = mc0->ty;
    return me.ty;
}

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynMatchExprNoSelector(ASTContext& ctx, MatchExpr& me)
{
    std::set<Ptr<Ty>> matchCaseTyVec;
    bool hasInvalidCase = false;
    for (auto& mco : me.matchCaseOthers) {
        if (!Ty::IsTyCorrect(SynMatchCaseNoSelector(ctx, *mco))) {
            hasInvalidCase = true;
        } else {
            matchCaseTyVec.insert(mco->ty);
        }
    }
    if (hasInvalidCase) {
        me.ty = TypeManager::GetInvalidTy();
        return me.ty;
    }

    // Join match expr's ty.
    auto joinAndMeet = JoinAndMeet(typeManager, matchCaseTyVec, {}, &importManager, me.curFile);
    auto joinRes = joinAndMeet.JoinAsVisibleTy();
    if (auto optErrs = JoinAndMeet::SetJoinedType(me.ty, joinRes)) {
        auto builder = diag.Diagnose(me, DiagKind::sema_type_incompatible, "MatchCase");
        builder.AddNote(*optErrs);
    }

    // Check exhaustiveness and set unreachable attr of match cases.
    if (!CheckMatchExprNoSelectorExhaustiveness(me, false)) {
        me.ty = TypeManager::GetInvalidTy();
        return me.ty;
    }
    return me.ty;
}

bool TypeChecker::TypeCheckerImpl::ChkMatchExprHasSelector(ASTContext& ctx, AST::Ty& target, AST::MatchExpr& me)
{
    CODEC_NULLPTR_CHECK(me.selector);
    std::set<Ptr<Ty>> matchCaseTyVec;
    bool isMatchCorrect = Synthesize(ctx, me.selector.get()) && ReplaceIdealTy(*me.selector);
    auto selectorTy = me.selector->ty;

    for (auto& mc : me.matchCases) {
        CODEC_NULLPTR_CHECK(mc);
        CODEC_ASSERT(!mc->patterns.empty());
        mc->SetCtxExprForPatterns(me.selector.get());
        // Check whether pattern has the sameTy with selectorTy.
        if (Ty::IsTyCorrect(selectorTy)) {
            bool isPatOK = ChkMatchCasePatterns(ctx, selectorTy, *mc);
            bool isGuardOK = ChkMatchCasePatGuard(ctx, *mc);
            bool areActionsOK = ChkMatchCaseActions(ctx, &target, *mc);
            if (isPatOK && isGuardOK && areActionsOK) {
                matchCaseTyVec.insert(mc->ty);
            } else {
                isMatchCorrect = false;
            }
        }
    }

    if (isMatchCorrect) {
        CheckMatchExprSetTy(me, &target, typeManager, importManager, matchCaseTyVec);
        // Check pattern exhaustiveness and set unreachable attr of match cases.
        if (me.sugarKind == Expr::SugarKind::NO_SUGAR &&
            !PatternUsefulness::CheckMatchExprHasSelectorExhaustivenessAndReachability(ctx.diag, typeManager, me)) {
            isMatchCorrect = false;
        }
    }

    me.ty = isMatchCorrect ? me.ty : TypeManager::GetInvalidTy();
    return isMatchCorrect;
}

bool TypeChecker::TypeCheckerImpl::ChkMatchExprNoSelector(ASTContext& ctx, AST::Ty& target, AST::MatchExpr& me)
{
    bool isWellTyped = true;
    std::set<Ptr<Ty>> matchCaseTyVec;
    for (auto& mco : me.matchCaseOthers) {
        if (!ChkMatchCaseNoSelector(ctx, target, *mco)) {
            isWellTyped = false;
        } else {
            matchCaseTyVec.insert(mco->ty);
        }
    }

    if (isWellTyped) {
        CheckMatchExprSetTy(me, &target, typeManager, importManager, matchCaseTyVec);
        // Check pattern exhaustiveness and set unreachable attr of match cases.
        if (!CheckMatchExprNoSelectorExhaustiveness(me, false)) {
            isWellTyped = false;
        }
    }
    me.ty = isWellTyped ? me.ty : TypeManager::GetInvalidTy();
    return isWellTyped;
}

Ptr<Ty> TypeChecker::TypeCheckerImpl::SynMatchCaseNoSelector(ASTContext& ctx, MatchCaseOther& mco)
{
    // Type of patternGuard (matchExpr) is boolean.
    if (Is<WildcardExpr>(mco.matchExpr.get())) {
        mco.matchExpr->ty = TypeManager::GetPrimitiveTy(TypeKind::TYPE_BOOLEAN);
    } else {
        if (!Check(ctx, TypeManager::GetPrimitiveTy(TypeKind::TYPE_BOOLEAN), mco.matchExpr.get())) {
            mco.matchExpr->ty = TypeManager::GetInvalidTy();
        }
    }

    // Synthesize the ty of exprOrDecls of each case.
    mco.ty = Synthesize(ctx, mco.exprOrDecls.get());
    return mco.ty;
}

bool TypeChecker::TypeCheckerImpl::ChkMatchCasePatGuard(ASTContext& ctx, const MatchCase& mc)
{
    bool ret = true;
    if (mc.patternGuard) {
        Ptr<Ty> boolTy = TypeManager::GetPrimitiveTy(TypeKind::TYPE_BOOLEAN);
        ret = Check(ctx, boolTy, mc.patternGuard.get());
    }
    return ret;
}

bool TypeChecker::TypeCheckerImpl::ChkMatchCaseActions(ASTContext& ctx, Ptr<Ty> target, MatchCase& mc)
{
    bool ret = true;
    if (!mc.exprOrDecls) {
        mc.ty = TypeManager::GetInvalidTy();
        return false;
    }
    if (!target) { // Synthesize the ty of exprOrDecls of each case.
        mc.exprOrDecls->ty = Synthesize(ctx, mc.exprOrDecls.get());
        mc.ty = mc.exprOrDecls->ty;
    } else if (Check(ctx, target, mc.exprOrDecls.get())) { // Check whether exprOrDecls->ty has given target ty.
        mc.ty = mc.exprOrDecls->ty;
    } else {
        mc.ty = TypeManager::GetInvalidTy();
        ret = false;
    }
    return ret;
}

bool TypeChecker::TypeCheckerImpl::ChkNoVarPatternInOrPattern(
    const ASTContext& ctx, const std::vector<OwnedPtr<AST::Pattern>>& ps)
{
    CODEC_ASSERT(!ps.empty());
    if (ps.size() == 1) {
        return true;
    }
    std::vector<Ptr<Node>> varPatterns;
    for (auto& pattern : ps) {
        CODEC_NULLPTR_CHECK(pattern);
        Walker(pattern.get(), [&varPatterns, &ctx](Ptr<Node> node) {
            CODEC_NULLPTR_CHECK(node);
            bool isVarPattern = node->astKind == ASTKind::VAR_PATTERN ||
                (node->astKind == ASTKind::VAR_OR_ENUM_PATTERN &&
                    !ctx.IsEnumConstructor(StaticCast<VarOrEnumPattern&>(*node).identifier));
            if (isVarPattern) {
                varPatterns.emplace_back(node);
                return VisitAction::SKIP_CHILDREN;
            }
            return VisitAction::WALK_CHILDREN;
        }).Walk();
    }
    if (varPatterns.empty()) {
        return true;
    }
    auto builder = diag.DiagnoseRefactor(DiagKindRefactor::sema_var_in_or_pattern, *varPatterns.front());
    auto iter = varPatterns.cbegin() + 1; // Skip the first `Var` as it has been reported in main hint.
    while (iter != varPatterns.cend()) {
        builder.AddHint(**iter);
        ++iter;
    }
    return false;
}

bool TypeChecker::TypeCheckerImpl::ChkPatternsSameASTKind(const ASTContext& ctx,
    const std::vector<OwnedPtr<Pattern>>& patterns)
{
    CODEC_ASSERT(!patterns.empty());
    if (patterns.size() == 1) {
        CODEC_NULLPTR_CHECK(patterns.front());
        return true;
    }
    if (std::all_of(patterns.cbegin(), patterns.cend(), [](const OwnedPtr<Pattern>& pattern) {
            CODEC_NULLPTR_CHECK(pattern);
            return pattern->astKind == ASTKind::VAR_OR_ENUM_PATTERN || pattern->astKind == ASTKind::ENUM_PATTERN;
        })) {
        // We don't know the real type of the `VarOrEnumPattern` by far, and there are two possible cases:
        // 1. if it is a `VarPattern`, the `CheckMatchCaseNoVarPatternsInOr` will report errors latter.
        // 2. if it is an `EnumPattern`, it should be compiled correctly, e.g., `Some(_) | None`.
        return true;
    }
    auto patternToString = [&ctx](const Pattern& p) {
        auto kind = p.astKind;
        if (kind == ASTKind::VAR_OR_ENUM_PATTERN) {
            kind = ctx.IsEnumConstructor(StaticCast<const VarOrEnumPattern&>(p).identifier) ? ASTKind::ENUM_PATTERN
                                                                                            : ASTKind::VAR_PATTERN;
        }
        auto iter = PATTERN_ASTKIND_TO_STRING_MAP.find(kind);
        std::string res = iter == PATTERN_ASTKIND_TO_STRING_MAP.cend() ? "invalid" : iter->second;
        return res + " pattern";
    };
    const OwnedPtr<Pattern>& front = patterns.front();
    for (auto iter = patterns.cbegin() + 1; iter != patterns.cend(); ++iter) {
        CODEC_NULLPTR_CHECK(*iter);
        if ((*iter)->astKind != front->astKind) {
            auto builder = diag.DiagnoseRefactor(DiagKindRefactor::sema_different_or_pattern, *front);
            builder.AddMainHintArguments(patternToString(*front));
            builder.AddHint(**iter, patternToString(**iter));
            return false;
        }
    }
    return true;
}

bool TypeChecker::TypeCheckerImpl::ChkMatchCasePatterns(ASTContext& ctx, Ptr<Ty> target, MatchCase& mc)
{
    if (!ChkNoVarPatternInOrPattern(ctx, mc.patterns) || !ChkPatternsSameASTKind(ctx, mc.patterns)) {
        for (auto& pattern : mc.patterns) {
            CODEC_NULLPTR_CHECK(pattern);
            pattern->ty = TypeManager::GetInvalidTy();
        }
        return false;
    }
    bool ret = true;
    for (auto& pattern : mc.patterns) {
        auto safeTarget = target ? target : TypeManager::GetInvalidTy();
        if (!ChkPattern(ctx, *safeTarget, *pattern)) {
            ret = false;
        }
    }
    return ret;
}

bool TypeChecker::TypeCheckerImpl::ChkMatchCaseNoSelector(ASTContext& ctx, Ty& target, MatchCaseOther& mco)
{
    bool ret = true;
    // Type of patternGuard (matchExpr) is boolean.
    if (Is<WildcardExpr>(mco.matchExpr.get())) {
        mco.matchExpr->ty = TypeManager::GetPrimitiveTy(TypeKind::TYPE_BOOLEAN);
    } else {
        if (!Check(ctx, TypeManager::GetPrimitiveTy(TypeKind::TYPE_BOOLEAN), mco.matchExpr.get())) {
            mco.matchExpr->ty = TypeManager::GetInvalidTy();
            ret = false;
        }
    }

    // Check whether exprOrDecls->ty has given target ty.
    if (Check(ctx, &target, mco.exprOrDecls.get())) {
        mco.ty = mco.exprOrDecls->ty;
    } else {
        mco.ty = TypeManager::GetInvalidTy();
        ret = false;
    }
    return ret;
}

bool TypeChecker::TypeCheckerImpl::CheckMatchExprNoSelectorExhaustiveness(MatchExpr& me, bool hasDefault)
{
    bool ret = true;
    size_t defaultCase{0};
    for (size_t i = 0; i < me.matchCaseOthers.size(); ++i) {
        auto matchCaseOther = me.matchCaseOthers[i].get();
        if (!hasDefault && Is<WildcardExpr>(matchCaseOther->matchExpr.get())) {
            hasDefault = true;
            defaultCase = i;
        }
        if (Ty::IsInitialTy(matchCaseOther->ty)) {
            ret = false;
            diag.Diagnose(*matchCaseOther, DiagKind::sema_match_case_has_no_type);
            continue;
        }
    }
    if (!hasDefault) {
        ret = false;
        diag.Diagnose(me, DiagKind::sema_match_case_must_have_default);
    }
    // The cases after default should be set UNREACHABLE.
    for (size_t i = defaultCase + 1; i < me.matchCaseOthers.size(); i++) {
        me.matchCaseOthers[i]->EnableAttr(Attribute::UNREACHABLE);
    }
    return ret;
}

bool TypeChecker::TypeCheckerImpl::IsIrrefutablePattern(const Pattern& pattern)
{
    switch (pattern.astKind) {
        case AST::ASTKind::INVALID_PATTERN:
        case AST::ASTKind::CONST_PATTERN:
        case AST::ASTKind::TYPE_PATTERN:
            return false;
        case AST::ASTKind::WILDCARD_PATTERN:
        case AST::ASTKind::VAR_PATTERN:
            return true;
        case AST::ASTKind::TUPLE_PATTERN: {
            auto& tuplePattern = static_cast<const TuplePattern&>(pattern);
            return std::all_of(tuplePattern.patterns.cbegin(), tuplePattern.patterns.cend(),
                [this](const OwnedPtr<Pattern>& p) { return IsIrrefutablePattern(*p); });
        }
        case AST::ASTKind::ENUM_PATTERN: {
            if (!pattern.ty || !pattern.ty->IsEnum()) {
                return false;
            }
            auto& enumPattern = static_cast<const EnumPattern&>(pattern);
            auto enumTy = RawStaticCast<EnumTy*>(pattern.ty);
            return enumTy && enumTy->declPtr && enumTy->declPtr->constructors.size() == 1 &&
                std::all_of(enumPattern.patterns.cbegin(), enumPattern.patterns.cend(),
                    [this](const OwnedPtr<Pattern>& p) { return IsIrrefutablePattern(*p); });
        }
        default:
            CODEC_ABORT();
            return false;
    }
}
}; // namespace Codira
