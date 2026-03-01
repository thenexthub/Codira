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
 * This file implements the Desugar functions used after instantiation step.
 */

#include "DesugarInTypeCheck.h"

#include <atomic>
#include <memory>
#include <set>
#include <utility>
#include <vector>
#include <fstream>

#include "AutoBoxing.h"
#include "ExtendBoxMarker.h"
#include "TypeCheckUtil.h"
#include "TypeCheckerImpl.h"

#include "Codira/AST/Clone.h"
#include "Codira/AST/Create.h"
#include "Codira/AST/Match.h"
#include "Codira/AST/Types.h"
#include "Codira/AST/Utils.h"
#include "Codira/AST/Walker.h"
#include "Codira/Basic/Match.h"
#include "Codira/Modules/ImportManager.h"
#include "Codira/Sema/TypeManager.h"
#include "Codira/Utils/CheckUtils.h"
#include "Codira/Utils/FileUtil.h"
#include "Codira/Utils/Utils.h"

using namespace Codira;
using namespace AST;
using namespace Meta;
using namespace TypeCheckUtil;

// Perform desugar after generic instantiation.
void TypeChecker::PerformDesugarAfterInstantiation(ASTContext& ctx, Package& pkg) const
{
    impl->PerformDesugarAfterInstantiation(ctx, pkg);
}

namespace {
void UpdateDeclAttributes(Package& pkg, bool exportForTest)
{
    Walker(&pkg, [&exportForTest](auto node) {
        if (auto vd = DynamicCast<VarDecl*>(node); vd && vd->initializer) {
            vd->EnableAttr(Attribute::DEFAULT);
        }
        if (exportForTest) {
            if (auto fd = DynamicCast<FuncDecl*>(node); fd && !fd->TestAttr(Attribute::PRIVATE)) {
                auto isExtend = Is<ExtendDecl>(fd->outerDecl);
                auto isForeignFunc = fd->TestAttr(Attribute::FOREIGN);
                if (!isExtend && !isForeignFunc) {
                    return VisitAction::WALK_CHILDREN;
                }
                // Skip declarations added by the compiler because they wouldn't be accessible in tests anyway
                if (isExtend && fd->outerDecl->TestAttr(Attribute::COMPILER_ADD)) {
                    return VisitAction::WALK_CHILDREN;
                }
                fd->linkage = Linkage::EXTERNAL;
                if (fd->propDecl) {
                    fd->propDecl->linkage = Linkage::EXTERNAL;
                }
                if (isExtend) {
                    fd->outerDecl->EnableAttr(Attribute::FOR_TEST);
                } else {
                    fd->EnableAttr(Attribute::FOR_TEST);
                }
            }
        }
        return VisitAction::WALK_CHILDREN;
    }).Walk();
}

/**
 * For compiled with the `--coverage` option,
 * Clear line info for:
 *   - Compiler add by generic instantiation.
 */
void ClearLineInfoAfterSema(Package& pkg)
{
    std::function<VisitAction(Ptr<Node>)> clearGenericInst = [](Ptr<Node> node) -> VisitAction {
        node->begin.line = 0;
        node->begin.column = 0;
        node->begin.Mark(PositionStatus::IGNORE);
        return VisitAction::WALK_CHILDREN;
    };
    for (auto& decl : pkg.genericInstantiatedDecls) {
        Walker(decl, clearGenericInst).Walk();
    }
}
} // namespace

void TypeChecker::TypeCheckerImpl::PerformDesugarAfterInstantiation([[maybe_unused]] ASTContext& ctx, Package& pkg)
{
    if (pkg.files.empty()) {
        return;
    }
#ifdef CODIRA_CODEGEN_CODENATIVE_BACKEND
    if (!ci->invocation.globalOptions.disableReflection) {
        PerformToAnyInsertion(pkg);
    }
#endif
    PerformRecursiveTypesElimination();
    UpdateDeclAttributes(pkg, ci->invocation.globalOptions.exportForTest);
    if (ci->invocation.globalOptions.enableCoverage) {
        ClearLineInfoAfterSema(pkg);
    }
}

bool AutoBoxing::NeedBoxOption(Ty& child, Ty& target)
{
    if (Ty::IsInitialTy(&child) || Ty::IsInitialTy(&target) ||
        (typeManager.CheckTypeCompatibility(&child, &target, false, target.IsGeneric()) !=
            TypeCompatibility::INCOMPATIBLE) ||
        child.kind == TypeKind::TYPE_NOTHING || target.kind != TypeKind::TYPE_ENUM) {
        return false;
    }
    auto lCnt = CountOptionNestedLevel(child);
    auto rCnt = CountOptionNestedLevel(target);
    // If type contains generic ty, current is node inside @Java class. Otherwise, incompatible types need to be boxed.
    if (lCnt == rCnt && child.HasGeneric()) {
        return false;
    }
    auto enumTy = RawStaticCast<EnumTy*>(&target);
    if (enumTy->declPtr->fullPackageName != CORE_PACKAGE_NAME || enumTy->declPtr->identifier != STD_LIB_OPTION) {
        return false;
    }
    return true;
}

// Option Box happens twice before and after instantiation, and must before extend box.
void AutoBoxing::TryOptionBox(EnumTy& target, Expr& expr)
{
    if (expr.ty && target.typeArgs[0] && NeedBoxOption(*expr.ty, *target.typeArgs[0])) {
        TryOptionBox(*StaticCast<EnumTy*>(target.typeArgs[0]), expr);
    }
    auto ed = target.decl;
    Ptr<FuncDecl> optionDecl = nullptr;
    for (auto& it : ed->constructors) {
        if (it->identifier == OPTION_VALUE_CTOR) {
            optionDecl = StaticCast<FuncDecl*>(it.get());
            break;
        }
    }
    if (optionDecl == nullptr) {
        return;
    }

    auto baseFunc = CreateRefExpr(OPTION_VALUE_CTOR);
    baseFunc->EnableAttr(Attribute::IMPLICIT_ADD);
    baseFunc->ref.target = optionDecl;
    baseFunc->ty = typeManager.GetInstantiatedTy(optionDecl->ty, GenerateTypeMapping(*ed, target.typeArgs));

    std::vector<OwnedPtr<FuncArg>> arg;
    if (expr.desugarExpr) {
        arg.emplace_back(CreateFuncArg(std::move(expr.desugarExpr)));
    } else {
        arg.emplace_back(CreateFuncArg(ASTCloner::Clone(Ptr(&expr))));
    }

    auto ce = CreateCallExpr(std::move(baseFunc), std::move(arg));
    ce->callKind = AST::CallKind::CALL_DECLARED_FUNCTION;
    ce->ty = &target;
    ce->resolvedFunction = optionDecl;
    if (expr.astKind == ASTKind::BLOCK) {
        // For correct deserialization, we need to keep type of block.
        auto b = MakeOwnedNode<Block>();
        b->ty = ce->ty;
        b->body.emplace_back(std::move(ce));
        expr.desugarExpr = std::move(b);
    } else {
        expr.desugarExpr = std::move(ce);
    }
    AddCurFile(*expr.desugarExpr, expr.curFile);
    expr.ty = expr.desugarExpr->ty;
}

/**
 * Option Box happens before type check finished with no errors.
 * All nodes and sema types should be valid.
 */
void AutoBoxing::AddOptionBox(Package& pkg)
{
    std::function<VisitAction(Ptr<Node>)> preVisit = [this](Ptr<Node> node) -> VisitAction {
        return match(*node)([this](const VarDecl& vd) { return AddOptionBoxHandleVarDecl(vd); },
            [this](const AssignExpr& ae) { return AddOptionBoxHandleAssignExpr(ae); },
            [this](CallExpr& ce) { return AddOptionBoxHandleCallExpr(ce); },
            [this](const IfExpr& ie) { return AddOptionBoxHandleIfExpr(ie); },
            [this](TryExpr& te) { return AddOptionBoxHandleTryExpr(te); },
            [this](const ReturnExpr& re) { return AddOptionBoxHandleReturnExpr(re); },
            [this](ArrayLit& lit) { return AddOptionBoxHandleArrayLit(lit); },
            [this](MatchExpr& me) { return AddOptionBoxHandleMatchExpr(me); },
            [this](const TupleLit& tl) { return AddOptionBoxHandleTupleList(tl); },
            [this](ArrayExpr& ae) { return AddOptionBoxHandleArrayExpr(ae); },
            []() { return VisitAction::WALK_CHILDREN; });
    };
    Walker walker(&pkg, preVisit);
    walker.Walk();
}

VisitAction AutoBoxing::AddOptionBoxHandleTupleList(const TupleLit& tl)
{ // Tuple literal allows element been boxed.
    auto tupleTy = DynamicCast<TupleTy*>(tl.ty);
    if (tupleTy == nullptr) {
        return VisitAction::WALK_CHILDREN;
    }
    auto typeArgs = tupleTy->typeArgs;
    for (size_t i = 0; i < typeArgs.size(); ++i) {
        if (tl.children[i]->ty && typeArgs[i] && NeedBoxOption(*tl.children[i]->ty, *typeArgs[i])) {
            TryOptionBox(*StaticCast<EnumTy*>(typeArgs[i]), *tl.children[i]);
        }
    }
    return VisitAction::WALK_CHILDREN;
}

VisitAction AutoBoxing::AddOptionBoxHandleMatchExpr(MatchExpr& me)
{
    for (auto& single : me.matchCases) {
        CODEC_ASSERT(me.ty && single->exprOrDecls);
        AddOptionBoxHandleBlock(*single->exprOrDecls, *me.ty);
    }
    for (auto& caseOther : me.matchCaseOthers) {
        CODEC_ASSERT(me.ty && caseOther->exprOrDecls);
        AddOptionBoxHandleBlock(*caseOther->exprOrDecls, *me.ty);
    }
    return VisitAction::WALK_CHILDREN;
}

VisitAction AutoBoxing::AddOptionBoxHandleArrayLit(ArrayLit& lit)
{
    if (Ty::IsInitialTy(lit.ty) || !lit.ty->IsStructArray()) {
        return VisitAction::WALK_CHILDREN;
    }

    if (lit.ty->typeArgs.size() == 1) {
        auto targetTy = lit.ty->typeArgs[0];
        CODEC_NULLPTR_CHECK(targetTy);
        for (auto& child : lit.children) {
            if (child->ty && NeedBoxOption(*child->ty, *targetTy)) {
                TryOptionBox(*StaticCast<EnumTy*>(targetTy), *child);
            }
        }
    }
    return VisitAction::WALK_CHILDREN;
}

VisitAction AutoBoxing::AddOptionBoxHandleIfExpr(const IfExpr& ie)
{
    if (!Ty::IsTyCorrect(ie.ty) || ie.ty->IsUnitOrNothing() || !ie.thenBody) {
        return VisitAction::WALK_CHILDREN;
    }
    AddOptionBoxHandleBlock(*ie.thenBody, *ie.ty);
    if (ie.hasElse && ie.elseBody) {
        if (auto block = DynamicCast<Block*>(ie.elseBody.get()); block) {
            AddOptionBoxHandleBlock(*block, *ie.ty);
        } else if (auto elseIfExpr = DynamicCast<IfExpr*>(ie.elseBody.get());
            elseIfExpr && Ty::IsTyCorrect(elseIfExpr->ty) && NeedBoxOption(*elseIfExpr->ty, *ie.ty)) {
            TryOptionBox(*StaticCast<EnumTy*>(ie.ty), *elseIfExpr);
            elseIfExpr->ty = ie.ty;
        }
    }
    return VisitAction::WALK_CHILDREN;
}

void AutoBoxing::AddOptionBoxHandleBlock(Block& block, Ty& ty)
{
    // If the block is empty or end with declaration, the last type is 'Unit',
    // otherwise the last type is the type of last expression.
    auto lastExprOrDecl = block.GetLastExprOrDecl();
    Ptr<Ty> lastTy = TypeManager::GetPrimitiveTy(TypeKind::TYPE_UNIT);
    if (auto expr = DynamicCast<Expr*>(lastExprOrDecl)) {
        lastTy = expr->ty;
    }
    if (!lastTy || !NeedBoxOption(*lastTy, ty)) {
        return;
    }
    // If the block is empty or end with declaration, we need to insert a unitExpr for box.
    if (Is<Decl>(lastExprOrDecl) || block.body.empty()) {
        auto unitExpr = CreateUnitExpr(TypeManager::GetPrimitiveTy(TypeKind::TYPE_UNIT));
        unitExpr->curFile = block.curFile;
        lastExprOrDecl = unitExpr.get();
        block.body.emplace_back(std::move(unitExpr));
    }

    if (auto lastExpr = DynamicCast<Expr*>(lastExprOrDecl)) {
        TryOptionBox(StaticCast<EnumTy&>(ty), *lastExpr);
        block.ty = lastExpr->ty;
    }
}

VisitAction AutoBoxing::AddOptionBoxHandleTryExpr(TryExpr& te)
{
    if (!Ty::IsTyCorrect(te.ty)) {
        return VisitAction::WALK_CHILDREN;
    }
    if (te.tryBlock) {
        AddOptionBoxHandleBlock(*te.tryBlock, *te.ty);
    }
    for (auto& ce : te.catchBlocks) {
        AddOptionBoxHandleBlock(*ce, *te.ty);
    }
    return VisitAction::WALK_CHILDREN;
}

VisitAction AutoBoxing::AddOptionBoxHandleArrayExpr(ArrayExpr& ae)
{
    bool ignore = !Ty::IsTyCorrect(ae.ty) || ae.initFunc || ae.args.size() < 1;
    if (ignore) {
        return VisitAction::WALK_CHILDREN;
    }
    auto targetTy = typeManager.GetTypeArgs(*ae.ty)[0];

    Ptr<FuncArg> arg = nullptr;
    if (ae.isValueArray) {
        // For VArray only one argument, and it need option box.
        arg = ae.args[0].get();
    } else if (ae.args.size() > 1) {
        // For RawArray(size, item:T) boxing argIndex is 1, only this case may need option box.
        arg = ae.args[1].get();
    }
    if (arg && arg->expr && arg->expr->ty && targetTy && NeedBoxOption(*arg->expr->ty, *targetTy)) {
        TryOptionBox(*StaticCast<EnumTy*>(targetTy), *arg->expr);
        arg->ty = arg->expr->ty;
    }
    return VisitAction::WALK_CHILDREN;
}

VisitAction AutoBoxing::AddOptionBoxHandleCallExpr(CallExpr& ce)
{
    bool ignored = !ce.baseFunc || !ce.baseFunc->ty || ce.baseFunc->ty->kind != TypeKind::TYPE_FUNC;
    if (ignored) {
        return VisitAction::WALK_CHILDREN;
    }
    auto funcTy = RawStaticCast<FuncTy*>(ce.baseFunc->ty);
    unsigned count = 0;
    auto callCheck = [&count, &funcTy, this](auto begin, auto end) {
        for (auto it = begin; it != end; ++it) {
            if (count >= funcTy->paramTys.size()) {
                break;
            }
            auto paramTy = funcTy->paramTys[count];
            // It's possible that childs have different box type, so does not break after match.
            if ((*it)->expr && (*it)->expr->ty && paramTy && NeedBoxOption(*(*it)->expr->ty, *paramTy)) {
                TryOptionBox(*StaticCast<EnumTy*>(paramTy), *(*it)->expr);
                (*it)->ty = (*it)->expr->ty;
            }
            ++count;
        }
    };
    if (ce.desugarArgs.has_value()) {
        callCheck(ce.desugarArgs->begin(), ce.desugarArgs->end());
    } else {
        callCheck(ce.args.begin(), ce.args.end());
    }
    return VisitAction::WALK_CHILDREN;
}

VisitAction AutoBoxing::AddOptionBoxHandleAssignExpr(const AssignExpr& ae)
{
    if (ae.desugarExpr) {
        return VisitAction::WALK_CHILDREN;
    }
    if (ae.rightExpr->ty && ae.leftValue->ty && NeedBoxOption(*ae.rightExpr->ty, *ae.leftValue->ty)) {
        TryOptionBox(*StaticCast<EnumTy*>(ae.leftValue->ty), *ae.rightExpr);
    }
    return VisitAction::WALK_CHILDREN;
}

VisitAction AutoBoxing::AddOptionBoxHandleVarDecl(const VarDecl& vd)
{
    if (vd.initializer && vd.initializer->ty && vd.ty && NeedBoxOption(*vd.initializer->ty, *vd.ty)) {
        TryOptionBox(*StaticCast<EnumTy*>(vd.ty), *vd.initializer);
    }
    return VisitAction::WALK_CHILDREN;
}

VisitAction AutoBoxing::AddOptionBoxHandleReturnExpr(const ReturnExpr& re)
{
    if (re.expr && re.refFuncBody && re.refFuncBody->ty && re.refFuncBody->ty->kind == TypeKind::TYPE_FUNC) {
        auto funcTy = RawStaticCast<FuncTy*>(re.refFuncBody->ty);
        if (re.expr->ty && funcTy->retTy && NeedBoxOption(*re.expr->ty, *funcTy->retTy)) {
            auto expr = re.desugarExpr ? re.desugarExpr.get() : re.expr.get();
            TryOptionBox(*StaticCast<EnumTy*>(funcTy->retTy), *expr);
        }
    }
    return VisitAction::WALK_CHILDREN;
}
