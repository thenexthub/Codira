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

#include "Codira/CHIR/AST2CHIR/TranslateASTNode/Translator.h"

using namespace Codira::CHIR;
using namespace Codira;

Ptr<Value> Translator::Visit(const AST::BinaryExpr& binaryExpr)
{
    if (!binaryExpr.TestAttr(AST::Attribute::SIDE_EFFECT)) {
        return ProcessBinaryExpr(binaryExpr);
    }
    /* The following codira code may have side effect, and sema will set attribute SIDE_EFFECT on binaryExpr.
        test(1) == (true, true, true))
        desugar to:
        test(1)[0] == true && test(1)[1] == true && test(1)[2] == true
                                                | |=========================> has the Attribute: SIDE_EFFECT
        |     | ============================================================> the callExpr has mapExpr.

    */
    if (auto rightBinaryExpr = DynamicCast<AST::BinaryExpr*>(binaryExpr.rightExpr.get()); rightBinaryExpr) {
        AST::SubscriptExpr* subscriptExpr = nullptr;
        if (rightBinaryExpr->leftExpr && rightBinaryExpr->leftExpr->astKind == AST::ASTKind::SUBSCRIPT_EXPR) {
            subscriptExpr = StaticCast<AST::SubscriptExpr*>(rightBinaryExpr->leftExpr.get());
        } else if (rightBinaryExpr->rightExpr && rightBinaryExpr->rightExpr->astKind == AST::ASTKind::SUBSCRIPT_EXPR) {
            subscriptExpr = StaticCast<AST::SubscriptExpr*>(rightBinaryExpr->rightExpr.get());
        }
        if (subscriptExpr) {
            if (auto mapExpr = GetMapExpr(*subscriptExpr->baseExpr)) {
                if (!exprValueTable.Has(*mapExpr)) {
                    auto chirNode = TranslateExprArg(*subscriptExpr->baseExpr);
                    exprValueTable.Set(*mapExpr, *chirNode);
                }
            }
        }
    }
    return ProcessBinaryExpr(binaryExpr);
}

Ptr<Value> Translator::ProcessBinaryExpr(const AST::BinaryExpr& binaryExpr)
{
    // BinaryExpression init func is (ExprKind, Value*, Value*, OverflowStrategy, Block*)
    static const std::unordered_map<Codira::TokenKind, ExprKind> OP2_EXPR_KIND = {
        {Codira::TokenKind::ADD, ExprKind::ADD},
        {Codira::TokenKind::SUB, ExprKind::SUB},
        {Codira::TokenKind::MUL, ExprKind::MUL},
        {Codira::TokenKind::DIV, ExprKind::DIV},
        {Codira::TokenKind::MOD, ExprKind::MOD},
        {Codira::TokenKind::EXP, ExprKind::EXP},
        {Codira::TokenKind::AND, ExprKind::AND},
        {Codira::TokenKind::OR, ExprKind::OR},
        {Codira::TokenKind::BITAND, ExprKind::BITAND},
        {Codira::TokenKind::BITOR, ExprKind::BITOR},
        {Codira::TokenKind::BITXOR, ExprKind::BITXOR},
        {Codira::TokenKind::LSHIFT, ExprKind::LSHIFT},
        {Codira::TokenKind::RSHIFT, ExprKind::RSHIFT},
        {Codira::TokenKind::LT, ExprKind::LT},
        {Codira::TokenKind::GT, ExprKind::GT},
        {Codira::TokenKind::LE, ExprKind::LE},
        {Codira::TokenKind::GE, ExprKind::GE},
        {Codira::TokenKind::NOTEQ, ExprKind::NOTEQUAL},
        {Codira::TokenKind::EQUAL, ExprKind::EQUAL},
    };
    const auto chirType = TranslateType(*binaryExpr.ty);
    const auto& loc = TranslateLocation(binaryExpr);
    auto it = OP2_EXPR_KIND.find(binaryExpr.op);
    CODEC_ASSERT(it != OP2_EXPR_KIND.end());
    ExprKind kd = it->second;
    auto lhs = TranslateExprArg(*binaryExpr.leftExpr);
    CODEC_NULLPTR_CHECK(lhs);
    if (kd == ExprKind::AND) {
        return TransShortCircuitAnd(lhs, *binaryExpr.rightExpr, loc, binaryExpr.TestAttr(AST::Attribute::COMPILER_ADD));
    }
    if (kd == ExprKind::OR) {
        return TransShortCircuitOr(lhs, *binaryExpr.rightExpr, loc, binaryExpr.TestAttr(AST::Attribute::COMPILER_ADD));
    }
    auto rightExpr = TranslateExprArg(*binaryExpr.rightExpr);
    bool mayHaveException = OverloadableExprMayThrowException(binaryExpr, *chirType);
    if (binaryExpr.leftExpr->ty->IsNothing()) {
        const auto& rightExprLoc = TranslateLocation(*binaryExpr.rightExpr);
        return TryCreateWithOV<BinaryExpression>(currentBlock, mayHaveException, binaryExpr.overflowStrategy,
            rightExprLoc, loc, chirType, kd, lhs, rightExpr)
            ->GetResult();
    } else {
        const auto& operatorLoc = GetOperatorLoc(binaryExpr);
        return TryCreateWithOV<BinaryExpression>(
            currentBlock, mayHaveException, binaryExpr.overflowStrategy, operatorLoc, loc, chirType, kd, lhs, rightExpr)
            ->GetResult();
    }
}

Ptr<Value> Translator::TransShortCircuitAnd(
    const Ptr<Value> leftValue, const AST::Expr& rightExpr, const DebugLocation& loc, bool isImplicitAdd)
{
    auto expr = CreateAndAppendExpression<Allocate>(
        loc, builder.GetType<RefType>(builder.GetBoolTy()), builder.GetBoolTy(), currentBlock);
    Ptr<Value> alloca = expr->GetResult();
    Ptr<Block> thenBlock = CreateBlock();
    Ptr<Block> elseBlock = CreateBlock();
    Ptr<Block> endBlock = CreateBlock();
    CODEC_ASSERT(
        leftValue->GetType()->IsBoolean() || leftValue->GetType()->IsNothing() || leftValue->GetType()->IsGeneric());
    const auto& rightLoc = TranslateLocation(rightExpr);
    CreateWrappedBranch(SourceExpr::BINARY, loc, leftValue, thenBlock, elseBlock, currentBlock);
    // Create Then Block:
    currentBlock = thenBlock;
    if (!isImplicitAdd) {
        currentBlock->SetDebugLocation(rightLoc);
    }
    TranslateSubExprToLoc(rightExpr, alloca, rightLoc);
    CreateAndAppendTerminator<GoTo>(loc, endBlock, currentBlock);
    // Create Else Block:
    auto boolLiteral =
        CreateAndAppendConstantExpression<BoolLiteral>(loc, builder.GetBoolTy(), *elseBlock, false)->GetResult();
    CreateWrappedStore(loc, boolLiteral, alloca, elseBlock);
    CreateAndAppendTerminator<GoTo>(loc, endBlock, elseBlock);
    // Update 'currentBlock' at last.
    currentBlock = endBlock;
    return CreateAndAppendExpression<Load>(loc, builder.GetBoolTy(), alloca, endBlock)->GetResult();
}

Ptr<Value> Translator::TransShortCircuitOr(
    const Ptr<Value> leftValue, const AST::Expr& rightExpr, const DebugLocation& loc, bool isImplicitAdd)
{
    Ptr<Value> alloca = CreateAndAppendExpression<Allocate>(
        loc, builder.GetType<RefType>(builder.GetBoolTy()), builder.GetBoolTy(), currentBlock)
                            ->GetResult();
    Ptr<Block> thenBlock = CreateBlock();
    Ptr<Block> elseBlock = CreateBlock();
    Ptr<Block> endBlock = CreateBlock();
    CODEC_ASSERT(
        leftValue->GetType()->IsBoolean() || leftValue->GetType()->IsNothing() || leftValue->GetType()->IsGeneric());
    const auto& rightLoc = TranslateLocation(rightExpr);
    CreateWrappedBranch(SourceExpr::BINARY, loc, leftValue, thenBlock, elseBlock, currentBlock);
    // Create Then Block:
    auto boolLiteral =
        CreateAndAppendConstantExpression<BoolLiteral>(builder.GetBoolTy(), *thenBlock, true)->GetResult();
    CreateWrappedStore(rightLoc, boolLiteral, alloca, thenBlock);
    CreateAndAppendTerminator<GoTo>(rightLoc, endBlock, thenBlock);
    // Create Else Block:
    currentBlock = elseBlock;
    if (!isImplicitAdd) {
        currentBlock->SetDebugLocation(rightLoc);
    }
    TranslateSubExprToLoc(rightExpr, alloca, rightLoc);
    CreateAndAppendTerminator<GoTo>(rightLoc, endBlock, currentBlock);
    // Update 'currentBlock' at last.
    this->currentBlock = endBlock;
    return CreateAndAppendExpression<Load>(loc, builder.GetBoolTy(), alloca, endBlock)->GetResult();
}
