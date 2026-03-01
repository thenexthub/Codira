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
 * This file instantiate functions for CHIR pass Function Inline and Devirtualization
 */

#include "Codira/CHIR/Transformation/BlockGroupCopyHelper.h"

#include "Codira/CHIR/CHIRCasting.h"
#include "Codira/CHIR/Type/Type.h"
#include "Codira/CHIR/Utils.h"
#include "Codira/CHIR/Visitor/Visitor.h"
#include "Codira/CHIR/Type/ExtendDef.h"
#include "Codira/CHIR/Type/PrivateTypeConverter.h"

namespace Codira::CHIR {
std::pair<BlockGroup*, LocalVar*> BlockGroupCopyHelper::CloneBlockGroup(
    const BlockGroup& other, Func& parentFunc)
{
    auto newGroup = other.Clone(builder, parentFunc);
    InstBlockGroup(newGroup);
    // including local var and param
    std::unordered_map<Value*, Value*> valueMap;
    std::unordered_set<Expression*> newDebugs;
    CollectValueMap(other, *newGroup, valueMap, newDebugs);
    ReplaceExprOperands(*newGroup, valueMap);
    for (auto dbg : newDebugs) {
        dbg->RemoveSelfFromBlock();
    }

    LocalVar* newBlockGroupRetValue = nullptr;
    LocalVar* oldFuncRetValue = nullptr;
    if (auto func = other.GetOwnerFunc()) {
        oldFuncRetValue = func->GetReturnValue();
    } else if (auto lambda = DynamicCast<Lambda*>(other.GetOwnerExpression())) {
        oldFuncRetValue = lambda->GetReturnValue();
    }
    // some functions don't have return value, such as `init`
    if (oldFuncRetValue != nullptr) {
        CODEC_ASSERT(oldFuncRetValue->IsLocalVar());
        auto it = valueMap.find(oldFuncRetValue);
        CODEC_ASSERT(it != valueMap.end());
        newBlockGroupRetValue = StaticCast<LocalVar*>(it->second);
    }
    return {newGroup, newBlockGroupRetValue};
}

void BlockGroupCopyHelper::SubstituteValue(Ptr<BlockGroup> block, std::unordered_map<Value*, Value*>& valueMap)
{
    ReplaceExprOperands(*block, valueMap);
}

void BlockGroupCopyHelper::CollectValueMap(const Lambda& oldLambda, const Lambda& newLambda,
    std::unordered_map<Value*, Value*>& valueMap, std::unordered_set<Expression*>& newDebugs)
{
    auto oldParams = oldLambda.GetParams();
    auto newParams = newLambda.GetParams();
    CODEC_ASSERT(oldParams.size() == newParams.size());
    for (size_t i = 0; i < oldParams.size(); ++i) {
        valueMap.emplace(oldParams[i], newParams[i]);
    }
    CollectValueMap(*oldLambda.GetBody(), *newLambda.GetBody(), valueMap, newDebugs);
}

void BlockGroupCopyHelper::CollectValueMap(const Block& oldBlk, const Block& newBlk,
    std::unordered_map<Value*, Value*>& valueMap, std::unordered_set<Expression*>& newDebugs)
{
    auto oldExprs = oldBlk.GetExpressions();
    auto newExprs = newBlk.GetExpressions();
    CODEC_ASSERT(oldExprs.size() == newExprs.size());
    for (size_t i = 0; i < oldExprs.size(); ++i) {
        auto oldExpr = oldExprs[i];
        auto newExpr = newExprs[i];
        CODEC_ASSERT(oldExpr->GetExprKind() == newExpr->GetExprKind());
        if (oldExpr->GetResult() == nullptr) {
            CODEC_ASSERT(newExpr->GetResult() == nullptr);
            continue;
        }
        CODEC_ASSERT(newExpr->GetResult() != nullptr);
        valueMap.emplace(oldExpr->GetResult(), newExpr->GetResult());
        if (oldExpr->GetExprKind() == ExprKind::DEBUGEXPR) {
            newDebugs.emplace(newExpr);
        }
        if (oldExpr->GetExprKind() == ExprKind::LAMBDA) {
            CollectValueMap(*StaticCast<Lambda*>(oldExpr), *StaticCast<Lambda*>(newExpr), valueMap, newDebugs);
        }
    }
}

void BlockGroupCopyHelper::CollectValueMap(const BlockGroup& oldBG, const BlockGroup& newBG,
    std::unordered_map<Value*, Value*>& valueMap, std::unordered_set<Expression*>& newDebugs)
{
    auto oldBlocks = oldBG.GetBlocks();
    auto newBlocks = newBG.GetBlocks();
    CODEC_ASSERT(oldBlocks.size() == newBlocks.size());
    for (size_t i = 0; i < oldBlocks.size(); ++i) {
        CollectValueMap(*oldBlocks[i], *newBlocks[i], valueMap, newDebugs);
    }
}

void BlockGroupCopyHelper::ReplaceExprOperands(const Block& block, const std::unordered_map<Value*, Value*>& valueMap)
{
    for (auto expr : block.GetExpressions()) {
        // note: a hack here, remove later
        if (expr->GetExprKind() == ExprKind::DEBUGEXPR) {
            continue;
        }
        for (auto op : expr->GetOperands()) {
            auto it = valueMap.find(op);
            if (it == valueMap.end()) {
                continue;
            }
            expr->ReplaceOperand(op, it->second);
        }
        if (expr->GetExprKind() == ExprKind::LAMBDA) {
            auto lambda = StaticCast<Lambda*>(expr);
            ReplaceExprOperands(*lambda->GetBody(), valueMap);
        }
    }
}

void BlockGroupCopyHelper::ReplaceExprOperands(
    const BlockGroup& bg, const std::unordered_map<Value*, Value*>& valueMap)
{
    for (auto block : bg.GetBlocks()) {
        ReplaceExprOperands(*block, valueMap);
    }
}

void BlockGroupCopyHelper::GetInstMapFromApply(const Apply& apply, const FuncBase* newBodyOuterFunction)
{
    if (apply.GetCallee()->IsLocalVar()) {
        auto lambda = DynamicCast<Lambda*>(StaticCast<LocalVar*>(apply.GetCallee())->GetExpr());
        CODEC_NULLPTR_CHECK(lambda);
        // get inst map from function
        size_t index = 0;
        for (auto& genericType : lambda->GetGenericTypeParams()) {
            instMap.emplace(genericType, apply.GetInstantiatedTypeArgs()[index]);
            ++index;
        }
        thisType = builder.GetType<ThisType>();
    } else {
        auto func = VirtualCast<FuncBase*>(apply.GetCallee());
        auto customDef = func->GetParentCustomTypeDef();
        if (customDef && customDef->IsGenericDef()) {
            // 1. get customType where function in.
            auto instParentCustomType = apply.GetInstParentCustomTyOfCallee(builder);
            if (instParentCustomType == nullptr) {
                instParentCustomType =
                    customDef->IsExtend() ? StaticCast<ExtendDef*>(customDef)->GetExtendedType() : customDef->GetType();
            }
            instParentCustomType = instParentCustomType->StripAllRefs();
            // 2. get inst map from custom type
            if (auto exDef = DynamicCast<const ExtendDef*>(customDef)) {
                auto newMap = exDef->GetExtendedType()->CalculateGenericTyMapping(*instParentCustomType);
                CODEC_ASSERT(newMap.first);
                instMap.merge(newMap.second);
            } else {
                instMap = GetInstMapFromCurDefToCurType(StaticCast<CustomType&>(*instParentCustomType));
            }
        }
        // 3. get inst map from function
        size_t index = 0;
        for (auto& genericType : func->GetGenericTypeParams()) {
            instMap.emplace(genericType, apply.GetInstantiatedTypeArgs()[index]);
            ++index;
        }
        // 4. set this type if needed
        auto outerFunc = newBodyOuterFunction ? newBodyOuterFunction : apply.GetTopLevelFunc();
        auto outerDef = outerFunc->GetParentCustomTypeDef();
        if (customDef && outerDef && outerDef == customDef && outerFunc->TestAttr(Attribute::STATIC)) {
            // keep this type if same custom type between callee and caller
            thisType = builder.GetType<ThisType>();
        } else {
            thisType = apply.GetThisType();
        }
    }
}

void BlockGroupCopyHelper::InstBlockGroup(Ptr<BlockGroup> group)
{
    GenericTypeConvertor gConverter(instMap, builder);
    ConvertTypeFunc convertFunc = [&gConverter, this](Type& type) {
        auto res = ReplaceThisTypeToConcreteType(type, *thisType, builder);
        if (res != &type) {
            return res;
        } else {
            return gConverter.ConvertToInstantiatedType(type);
        }
    };
    PrivateTypeConverterNoInvokeOriginal converter(convertFunc, builder);
    auto postVisit = [&converter](Expression& e) {
        converter.VisitExpr(e);
        return VisitResult::CONTINUE;
    };
    Visitor::Visit(*group, [](Expression&) { return VisitResult::CONTINUE; }, postVisit);
}

void FixCastProblemAfterInst(Ptr<BlockGroup> group, CHIRBuilder& builder)
{
    auto postVisit = [&builder](Expression& e) {
        if (e.GetExprKind() == ExprKind::LAMBDA) {
            auto lambda = StaticCast<Lambda*>(&e);
            FixCastProblemAfterInst(lambda->GetBody(), builder);
        } else if (e.GetExprKind() == ExprKind::INSTANCEOF) {
            // fix instanceOf problem
            auto instance = StaticCast<InstanceOf*>(&e);
            auto objType = instance->GetObject()->GetType();
            if (objType->IsGenericRelated() ||
                (objType->IsRef() && StaticCast<RefType*>(objType)->GetBaseType()->IsClass()) ||
                instance->GetType()->IsGenericRelated()) {
                return VisitResult::CONTINUE;
            }
            if (objType->IsEqualOrSubTypeOf(*instance->GetType(), builder)) {
                auto trueExpr =
                    builder.CreateConstantExpression<BoolLiteral>(builder.GetBoolTy(), e.GetParentBlock(), true);
                e.ReplaceWith(*trueExpr);
            } else {
                auto falseExpr =
                    builder.CreateConstantExpression<BoolLiteral>(builder.GetBoolTy(), e.GetParentBlock(), false);
                e.ReplaceWith(*falseExpr);
            }
        } else if (e.GetExprKind() == ExprKind::TRANSFORM_TO_CONCRETE) {
            // change transformToConcrete to box/unbox/typecast
            auto& cast = StaticCast<TransformToConcrete&>(e);
            if (!cast.GetSourceTy()->IsGenericRelated()) {
                auto newCast = TypeCastOrBoxIfNeeded(
                    *cast.GetSourceValue(), *cast.GetTargetTy(), builder, *e.GetParentBlock(), e.GetDebugLocation());
                if (newCast == cast.GetSourceValue()) {
                    for (auto user : e.GetResult()->GetUsers()) {
                        user->ReplaceOperand(e.GetResult(), cast.GetSourceValue());
                    }
                    e.RemoveSelfFromBlock();
                    return VisitResult::CONTINUE;
                }
                for (auto user : e.GetResult()->GetUsers()) {
                    user->ReplaceOperand(e.GetResult(), newCast);
                }
                StaticCast<LocalVar*>(newCast)->GetExpr()->MoveBefore(&e);
                e.RemoveSelfFromBlock();
            }
        } else if (e.GetExprKind() == ExprKind::TRANSFORM_TO_GENERIC) {
            // change TransformToGeneric to box/unbox/typecast
            auto cast = StaticCast<TransformToGeneric*>(&e);
            if (!cast->GetTargetTy()->IsGenericRelated()) {
                auto newCast =
                    TypeCastOrBoxIfNeeded(*cast->GetSourceValue(), *cast->GetTargetTy(), builder, *e.GetParentBlock(),
                        e.GetDebugLocation());
                if (newCast == cast->GetSourceValue()) {
                    for (auto user : e.GetResult()->GetUsers()) {
                        user->ReplaceOperand(e.GetResult(), cast->GetSourceValue());
                    }
                    e.RemoveSelfFromBlock();
                    return VisitResult::CONTINUE;
                }
                for (auto user : e.GetResult()->GetUsers()) {
                    user->ReplaceOperand(e.GetResult(), newCast);
                }
                StaticCast<LocalVar*>(newCast)->GetExpr()->MoveBefore(&e);
                e.RemoveSelfFromBlock();
            }
        } else if (e.GetExprKind() == ExprKind::TYPECAST) {
            /* change typecast to unbox/box
             * func foo<T>(a: T) {
             *   let b: CA<T> = TypeCast(a, CA<T>)   // typecast to CA<T>
             * }
             *
             * when foo inline to other function and inst to int64 type, the typecast would change to:
             *   (a: Int64)
             *   let b: CA<Int64> = Box(a, CA<Int64>) */
            auto cast = StaticCast<TypeCast*>(&e);
            if (cast->GetSourceValue()->GetType()->IsGenericRelated()) {
                return VisitResult::CONTINUE;
            }
            auto newCastRes =
                TypeCastOrBoxIfNeeded(*cast->GetSourceValue(), *e.GetResult()->GetType(), builder, *e.GetParentBlock(),
                    e.GetDebugLocation());
            if (newCastRes == cast->GetSourceValue()) {
                return VisitResult::CONTINUE;
            }
            auto newCast = StaticCast<LocalVar*>(newCastRes);
            if (newCast->GetExpr()->GetExprKind() == ExprKind::TYPECAST) {
                newCast->GetExpr()->RemoveSelfFromBlock();
                return VisitResult::CONTINUE;
            }
            for (auto user : e.GetResult()->GetUsers()) {
                user->ReplaceOperand(e.GetResult(), newCast);
            }
            newCast->GetExpr()->MoveBefore(&e);
            e.RemoveSelfFromBlock();
        }
        return VisitResult::CONTINUE;
    };
    Visitor::Visit(*group, [](Expression&) { return VisitResult::CONTINUE; }, postVisit);
}
}  // namespace Codira::CHIR
