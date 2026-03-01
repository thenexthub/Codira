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
 *  file implements functions to mark extend box attribute for ast.
 */
#include "ExtendBoxMarker.h"

#include "TypeCheckUtil.h"

#include "Codira/AST/Match.h"
#include "Codira/AST/Node.h"
#include "Codira/AST/Walker.h"
#include "Codira/Sema/TypeManager.h"
#include "Codira/Utils/Utils.h"

using namespace Codira;
using namespace AST;

TypeManager* ExtendBoxMarker::typeManager = nullptr;
std::mutex ExtendBoxMarker::mtx;

std::function<VisitAction(Ptr<Node>)> ExtendBoxMarker::GetMarkExtendBoxFunc(TypeManager& typeMgr)
{
    typeManager = &typeMgr;
    std::function<VisitAction(Ptr<Node>)> markerFunc = [](auto node) -> VisitAction {
        CODEC_ASSERT(node);
        switch (node->astKind) {
            case ASTKind::VAR_DECL:
                return MarkBoxPointHandleVarDecl(*RawStaticCast<VarDecl*>(node));
            case ASTKind::ASSIGN_EXPR:
                return MarkBoxPointHandleAssignExpr(*RawStaticCast<AssignExpr*>(node));
            case ASTKind::CALL_EXPR:
                return MarkBoxPointHandleCallExpr(*RawStaticCast<CallExpr*>(node));
            case ASTKind::IF_EXPR:
                return MarkBoxPointHandleIfExpr(*RawStaticCast<IfExpr*>(node));
            case ASTKind::RETURN_EXPR:
                return MarkBoxPointHandleReturnExpr(*RawStaticCast<ReturnExpr*>(node));
            case ASTKind::MATCH_EXPR:
                return MarkBoxPointHandleMatchExpr(*RawStaticCast<MatchExpr*>(node));
            case ASTKind::TRY_EXPR:
                return MarkBoxPointHandleTryExpr(*RawStaticCast<TryExpr*>(node));
            case ASTKind::ARRAY_EXPR:
                return MarkBoxPointHandleArrayExpr(*RawStaticCast<ArrayExpr*>(node));
            case ASTKind::ARRAY_LIT:
                return MarkBoxPointHandleArrayLit(*RawStaticCast<ArrayLit*>(node));
            case ASTKind::TUPLE_LIT:
                return MarkBoxPointHandleTupleLit(*RawStaticCast<TupleLit*>(node));
            case ASTKind::WHILE_EXPR:
                return MarkBoxPointHandleWhileExpr(StaticCast<WhileExpr>(*node));
            default:
                return VisitAction::WALK_CHILDREN;
        }
    };
    return markerFunc;
}

bool ExtendBoxMarker::NeedAutoBox(Ptr<Ty> child, Ptr<Ty> interface, bool isUpcast)
{
    CODEC_ASSERT(typeManager);
    CODEC_NULLPTR_CHECK(child);
    CODEC_NULLPTR_CHECK(interface);
    auto target = interface;
    // If the 'target' has more option box than child, then get it's typeArg for box checking.
    // NOTE: The 'while' should only be entered during instantiation checking.
    //       After instantiation, the option box is happened before extend box.
    //       So this 'while' should not be entered during extend box 'AutoBoxing' step.
    while (TypeCheckUtil::CountOptionNestedLevel(*child) < TypeCheckUtil::CountOptionNestedLevel(*target)) {
        target = target->typeArgs[0];
    }
    bool isExtended = target && typeManager->HasExtensionRelation(*child, *target);
    if (isExtended && isUpcast) {
        typeManager->RecordUsedExtend(*child, *target);
    }
    return isExtended;
}

void ExtendBoxMarker::CheckBlockNeedBox(const Block& block, Ty& ty, Node& nodeToCheck)
{
    auto lastExpr = block.GetLastExprOrDecl();
    Ptr<Ty> lastTy = TypeManager::GetPrimitiveTy(TypeKind::TYPE_UNIT);
    if (auto expr = DynamicCast<Expr*>(lastExpr); expr) {
        lastTy = expr->ty;
    }
    if (NeedAutoBox(lastTy, &ty)) {
        nodeToCheck.EnableAttr(Attribute::NEED_AUTO_BOX);
    }
}

VisitAction ExtendBoxMarker::MarkBoxPointHandleArrayExpr(ArrayExpr& ae)
{
    CODEC_ASSERT(typeManager);
    bool ignored = !Ty::IsTyCorrect(ae.ty) || ae.args.empty() ||
        (ae.initFunc && ae.initFunc->identifier != "arrayInitByCollection");
    if (ignored) {
        return VisitAction::WALK_CHILDREN;
    }
    // For 'VArray<...>(repeat: T)' constructor may need auto box.
    if (ae.isValueArray) {
        if (NeedAutoBox(ae.args[0]->ty, typeManager->GetTypeArgs(*ae.ty)[0])) {
            ae.EnableAttr(Attribute::NEED_AUTO_BOX);
        }
        return VisitAction::WALK_CHILDREN;
    }
    // For 'RawArray(size, item: T)' constructor may need auto box.
    if (!ae.initFunc) {
        if (NeedAutoBox(ae.args[1]->ty, typeManager->GetTypeArgs(*ae.ty)[0])) {
            ae.EnableAttr(Attribute::NEED_AUTO_BOX);
        }
        return VisitAction::WALK_CHILDREN;
    }
    // For 'RawArray(Collection)' constructor may need auto box.
    // 'initFunc' may be generic, we need to instantiated it's type before boxing check.
    auto initFuncTy = DynamicCast<FuncTy*>(ae.initFunc->ty);
    auto generic = ae.initFunc->GetGeneric();
    auto instTys = typeManager->GetTypeArgs(*ae.ty);
    bool invalid = !initFuncTy || initFuncTy->paramTys.size() != 2 || // 'arrayInitByCollection' has 2 parameters.
        !generic || generic->typeParameters.size() != instTys.size();
    if (invalid) {
        return VisitAction::WALK_CHILDREN;
    }
    TypeSubst typeMapping = TypeCheckUtil::GenerateTypeMapping(*ae.initFunc, instTys);
    initFuncTy = RawStaticCast<FuncTy*>(typeManager->GetInstantiatedTy(initFuncTy, typeMapping));
    if (NeedAutoBox(ae.args[0]->ty, initFuncTy->paramTys[1])) {
        ae.EnableAttr(Attribute::NEED_AUTO_BOX);
    }
    return VisitAction::WALK_CHILDREN;
}

/* In TryExpr:
 * If the context does explicitly require a certain type, the tryBlock and
 * each of the catchBlocks (if present) are required to be a subtype of the required type.
 * If the context does not explicitly require a certain type, the tryBlock and
 * all catchBlocks(if present) are required to have their least common super type,
 * which is also the type of the entire tryExpr.
 * When the entire tryExpr is an Interface type, and one of the tryBlock or catchBlocks is
 * an type which extend the Interface, or non-class type which implements the Interface,
 * we need to box it.
 * */
VisitAction ExtendBoxMarker::MarkBoxPointHandleTryExpr(TryExpr& te)
{
    if (!Ty::IsTyCorrect(te.ty)) {
        return VisitAction::WALK_CHILDREN;
    }
    if (te.tryBlock) {
        CheckBlockNeedBox(*te.tryBlock, *te.ty, te);
    }

    for (auto& cb : te.catchBlocks) {
        CODEC_NULLPTR_CHECK(cb);
        CheckBlockNeedBox(*cb, *te.ty, te);
    }
    return VisitAction::WALK_CHILDREN;
}

bool ExtendBoxMarker::IsTypePatternNeedBox(Ptr<Pattern> pattern, Ty& selectorTy)
{
    if (selectorTy.IsNothing() || pattern == nullptr) {
        return false;
    }
    // NOTE: we need to collect all boxed types, so DO NOT interrupt loop early.
    bool boxOrUnbox = false;
    switch (pattern->astKind) {
        case ASTKind::TYPE_PATTERN: {
            auto typePattern = RawStaticCast<TypePattern*>(pattern);
            CODEC_ASSERT(typePattern->type && typePattern->ty && typePattern->type->ty == typePattern->ty);
            // Downcast or Upcast.
            bool cond = NeedAutoBox(typePattern->ty, &selectorTy, false) || NeedAutoBox(&selectorTy, typePattern->ty);
            boxOrUnbox = cond || MustUnboxDownCast(selectorTy, *typePattern->ty);
            break;
        }
        case ASTKind::TUPLE_PATTERN: {
            auto tuplePattern = StaticCast<TuplePattern*>(pattern);
            auto tupleTy = StaticCast<TupleTy*>(&selectorTy);
            for (size_t i = 0; i < tuplePattern->patterns.size(); i++) {
                CODEC_ASSERT(tupleTy->typeArgs[i]);
                bool cond = IsTypePatternNeedBox(tuplePattern->patterns[i].get(), *tupleTy->typeArgs[i]);
                boxOrUnbox = boxOrUnbox || cond;
            }
            break;
        }
        case ASTKind::ENUM_PATTERN: {
            auto enumPattern = StaticCast<EnumPattern*>(pattern);
            CODEC_ASSERT(enumPattern->constructor && enumPattern->constructor->ty);
            auto constructorTy = DynamicCast<FuncTy*>(enumPattern->constructor->ty);
            if (!constructorTy) { // Enum pattern may without param.
                break;
            }
            for (size_t i = 0; i < enumPattern->patterns.size(); i++) {
                auto paramTy = constructorTy->paramTys[i];
                CODEC_ASSERT(paramTy);
                bool cond = IsTypePatternNeedBox(enumPattern->patterns[i].get(), *paramTy);
                boxOrUnbox = boxOrUnbox || cond;
            }
            break;
        }
        default:
            break;
    }
    return boxOrUnbox;
}

void ExtendBoxMarker::MarkBoxPointHandleCondition(Expr& e)
{
    // record outermost condition to mark as box
    auto& target = e;
    std::stack<Expr*> st;
    st.push(&e);
    while (!st.empty()) {
        auto expr = st.top();
        st.pop();
        if (!expr || !Ty::IsTyCorrect(expr->ty)) {
            continue;
        }
        if (auto let = DynamicCast<LetPatternDestructor>(expr)) {
            if (!let->initializer || !Ty::IsTyCorrect(let->initializer->ty)) {
                continue;
            }
            for (auto& pat : std::as_const(let->patterns)) {
                if (IsTypePatternNeedBox(pat.get(), *let->initializer->ty)) {
                    target.EnableAttr(Attribute::NEED_AUTO_BOX);
                    pat->EnableAttr(Attribute::NEED_AUTO_BOX);
                }
            }
        }
        if (auto par = DynamicCast<ParenExpr>(expr)) {
            st.push(&*par->expr);
        }
        if (auto bin = DynamicCast<BinaryExpr>(expr); bin && (bin->op == TokenKind::AND || bin->op == TokenKind::OR)) {
            st.push(&*bin->leftExpr);
            st.push(&*bin->rightExpr);
        }
    }
}

VisitAction ExtendBoxMarker::MarkBoxPointHandleMatchExpr(MatchExpr& me)
{
    if (!Ty::IsTyCorrect(me.ty)) {
        return VisitAction::SKIP_CHILDREN;
    }
    // NOTE: we also need to collect all boxed types, so do not interrupt loop early.
    for (auto& matchCase : me.matchCases) {
        // Primary constructors and their desugared components (such as default params) are not type checked and do
        // not have a correct semantic type field.
        if (!me.selector || !Ty::IsTyCorrect(me.selector->ty)) {
            continue;
        }
        for (auto& pattern : matchCase->patterns) {
            if (IsTypePatternNeedBox(pattern.get(), *me.selector->ty)) {
                // It's possible that childs have different box type, so does not break after match.
                // auto box or unbox.
                me.EnableAttr(Attribute::NEED_AUTO_BOX);
            }
        }

        if (matchCase->exprOrDecls) {
            CheckBlockNeedBox(*matchCase->exprOrDecls, *me.ty, me);
        }
    }
    for (auto& matchCase : me.matchCaseOthers) {
        if (matchCase->exprOrDecls) {
            CheckBlockNeedBox(*matchCase->exprOrDecls, *me.ty, me);
        }
    }
    return VisitAction::WALK_CHILDREN;
}

VisitAction ExtendBoxMarker::MarkBoxPointHandleArrayLit(ArrayLit& lit)
{
    if (Ty::IsTyCorrect(lit.ty) && lit.ty->typeArgs.size() == 1) {
        for (auto& child : lit.children) {
            // It's possible that child 0 and child 1 needs different box type, so does not break after match.
            if (child->ty && NeedAutoBox(child->ty, lit.ty->typeArgs[0])) {
                lit.EnableAttr(Attribute::NEED_AUTO_BOX);
            }
        }
    }
    return VisitAction::WALK_CHILDREN;
}

VisitAction ExtendBoxMarker::MarkBoxPointHandleReturnExpr(ReturnExpr& re)
{
    if (re.expr && re.refFuncBody && re.refFuncBody->ty && re.refFuncBody->ty->kind == TypeKind::TYPE_FUNC) {
        auto funcTy = RawStaticCast<FuncTy*>(re.refFuncBody->ty);
        if (NeedAutoBox(re.expr->ty, funcTy->retTy)) {
            re.EnableAttr(Attribute::NEED_AUTO_BOX);
        }
    }
    return VisitAction::WALK_CHILDREN;
}

VisitAction ExtendBoxMarker::MarkBoxPointHandleIfExpr(IfExpr& ie)
{
    if (Ty::IsTyCorrect(ie.ty) && (ie.condExpr && Ty::IsTyCorrect(ie.condExpr->ty))) {
        MarkBoxPointHandleCondition(*ie.condExpr);
    }
    if (Ty::IsTyCorrect(ie.ty) && !ie.ty->IsUnitOrNothing()) {
        if (ie.thenBody) {
            CheckBlockNeedBox(*ie.thenBody, *ie.ty, ie);
        }
        if (!ie.hasElse || !ie.elseBody) {
            return VisitAction::WALK_CHILDREN;
        }
        if (auto block = DynamicCast<Block*>(ie.elseBody.get()); block) {
            CheckBlockNeedBox(*block, *ie.ty, ie);
        }
    }
    return VisitAction::WALK_CHILDREN;
}

VisitAction ExtendBoxMarker::MarkBoxPointHandleWhileExpr(const AST::WhileExpr& we)
{
    if (Ty::IsTyCorrect(we.ty) && we.condExpr && Ty::IsTyCorrect(we.condExpr->ty)) {
        MarkBoxPointHandleCondition(*we.condExpr);
    }
    return VisitAction::WALK_CHILDREN;
}

VisitAction ExtendBoxMarker::MarkBoxPointHandleCallExpr(CallExpr& ce)
{
    if (!ce.baseFunc || !ce.baseFunc->ty || ce.baseFunc->ty->kind != TypeKind::TYPE_FUNC) {
        return VisitAction::WALK_CHILDREN;
    }
    auto funcTy = RawStaticCast<FuncTy*>(ce.baseFunc->ty);
    unsigned count = 0;
    auto callCheck = [&count, &funcTy, &ce](auto begin, auto end) {
        for (auto it = begin; it != end; ++it) {
            if (count >= funcTy->paramTys.size()) {
                break;
            }
            auto& paramTy = funcTy->paramTys[count];
            // It's possible that childs have different box type, so does not break after match.
            if ((*it)->expr && NeedAutoBox((*it)->expr->ty, paramTy)) {
                ce.EnableAttr(Attribute::NEED_AUTO_BOX);
            }
            count = count + 1;
        }
    };
    if (ce.desugarArgs.has_value()) {
        callCheck(ce.desugarArgs->begin(), ce.desugarArgs->end());
    } else {
        callCheck(ce.args.begin(), ce.args.end());
    }
    return VisitAction::WALK_CHILDREN;
}

VisitAction ExtendBoxMarker::MarkBoxPointHandleAssignExpr(AssignExpr& ae)
{
    // Desugared assign expression will be skipped.
    if (!ae.desugarExpr && ae.rightExpr && ae.leftValue && NeedAutoBox(ae.rightExpr->ty, ae.leftValue->ty)) {
        ae.EnableAttr(Attribute::NEED_AUTO_BOX);
    }
    return VisitAction::WALK_CHILDREN;
}

VisitAction ExtendBoxMarker::MarkBoxPointHandleVarDecl(VarDecl& vd)
{
    if (vd.initializer && NeedAutoBox(vd.initializer->ty, vd.ty)) {
        vd.EnableAttr(Attribute::NEED_AUTO_BOX);
    }
    return VisitAction::WALK_CHILDREN;
}

VisitAction ExtendBoxMarker::MarkBoxPointHandleTupleLit(TupleLit& tl)
{ // Tuple literal allows element been boxed.
    auto tupleTy = DynamicCast<TupleTy*>(tl.ty);
    if (tupleTy == nullptr) {
        return VisitAction::WALK_CHILDREN;
    }
    auto typeArgs = tupleTy->typeArgs;
    for (size_t i = 0; i < typeArgs.size(); ++i) {
        if (NeedAutoBox(tl.children[i]->ty, typeArgs[i])) {
            tl.EnableAttr(Attribute::NEED_AUTO_BOX);
        }
    }
    return VisitAction::WALK_CHILDREN;
}
