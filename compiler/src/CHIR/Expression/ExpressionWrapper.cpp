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

#include "Codira/CHIR/Expression/ExpressionWrapper.h"

#include "Codira/CHIR/CHIRCasting.h"
#include "Codira/Utils/CastingTemplate.h"

using namespace Codira::CHIR;

ExpressionBase::ExpressionBase(const Expression* e) : expr(e)
{
    CODEC_NULLPTR_CHECK(e);
}

const Expression* ExpressionBase::GetRawExpr() const
{
    return expr;
}

LocalVar* ExpressionBase::GetResult() const
{
    return expr->GetResult();
}

std::vector<Value*> ExpressionBase::GetOperands() const
{
    return expr->GetOperands();
}

FuncCallBase::FuncCallBase(const Expression* e) : ExpressionBase(e)
{
    CODEC_NULLPTR_CHECK(e);
    if (auto funcCall = Codira::DynamicCast<const FuncCall*>(e)) {
        expr = funcCall;
        exprE = nullptr;
    } else {
        expr = nullptr;
        exprE = Codira::StaticCast<const FuncCallWithException*>(e);
    }
}

FuncCallBase::FuncCallBase(const FuncCall* expr) : ExpressionBase(expr), expr(expr), exprE(nullptr)
{
    CODEC_NULLPTR_CHECK(expr);
}

FuncCallBase::FuncCallBase(const FuncCallWithException* exprE) : ExpressionBase(exprE), expr(nullptr), exprE(exprE)
{
    CODEC_NULLPTR_CHECK(exprE);
}

std::vector<Value*> FuncCallBase::GetArgs() const
{
    return expr ? expr->GetArgs()
                : exprE->GetArgs();
}

Type* FuncCallBase::GetThisType() const
{
    return expr ? expr->GetThisType()
                : exprE->GetThisType();
}

std::vector<Type*> FuncCallBase::GetInstantiatedTypeArgs() const
{
    return expr ? expr->GetInstantiatedTypeArgs()
                : exprE->GetInstantiatedTypeArgs();
}

ApplyBase::ApplyBase(const Expression* e) : FuncCallBase(e)
{
    CODEC_NULLPTR_CHECK(e);
    if (e->GetExprKind() == ExprKind::APPLY) {
        expr = Codira::StaticCast<const Apply*>(e);
        exprE = nullptr;
    } else {
        expr = nullptr;
        exprE = Codira::StaticCast<const ApplyWithException*>(e);
    }
}

ApplyBase::ApplyBase(const Apply* expr) : FuncCallBase(expr), expr(expr), exprE(nullptr)
{
    CODEC_NULLPTR_CHECK(expr);
}

ApplyBase::ApplyBase(const ApplyWithException* exprE) : FuncCallBase(exprE), expr(nullptr), exprE(exprE)
{
    CODEC_NULLPTR_CHECK(exprE);
}

Value* ApplyBase::GetCallee() const
{
    return expr ? expr->GetCallee()
                : exprE->GetCallee();
}

Type* ApplyBase::GetInstParentCustomTyOfCallee(CHIRBuilder& builder) const
{
    return expr ? expr->GetInstParentCustomTyOfCallee(builder)
                : exprE->GetInstParentCustomTyOfCallee(builder);
}

DynamicDispatchBase::DynamicDispatchBase(const Expression* e) : FuncCallBase(e)
{
    CODEC_NULLPTR_CHECK(e);
    if (auto funcCall = Codira::DynamicCast<const DynamicDispatch*>(e)) {
        expr = funcCall;
        exprE = nullptr;
    } else {
        expr = nullptr;
        exprE = Codira::StaticCast<const DynamicDispatchWithException*>(e);
    }
}

DynamicDispatchBase::DynamicDispatchBase(const DynamicDispatch* expr) : FuncCallBase(expr), expr(expr), exprE(nullptr)
{
    CODEC_NULLPTR_CHECK(expr);
}

DynamicDispatchBase::DynamicDispatchBase(const DynamicDispatchWithException* exprE)
    : FuncCallBase(exprE), expr(nullptr), exprE(exprE)
{
    CODEC_NULLPTR_CHECK(exprE);
}

std::vector<GenericType*> DynamicDispatchBase::GetGenericTypeParams() const
{
    return expr ? expr->GetGenericTypeParams()
                : exprE->GetGenericTypeParams();
}

std::string DynamicDispatchBase::GetMethodName() const
{
    return expr ? expr->GetMethodName()
                : exprE->GetMethodName();
}

FuncType* DynamicDispatchBase::GetMethodType() const
{
    return expr ? expr->GetMethodType()
                : exprE->GetMethodType();
}

size_t DynamicDispatchBase::GetVirtualMethodOffset() const
{
    return expr ? expr->GetVirtualMethodOffset()
                : exprE->GetVirtualMethodOffset();
}

ClassType* DynamicDispatchBase::GetInstSrcParentCustomTypeOfMethod(CHIRBuilder& builder) const
{
    return expr ? expr->GetInstSrcParentCustomTypeOfMethod(builder)
                : exprE->GetInstSrcParentCustomTypeOfMethod(builder);
}

InvokeBase::InvokeBase(const Expression* e) : DynamicDispatchBase(e)
{
    CODEC_NULLPTR_CHECK(e);
    if (e->GetExprKind() == ExprKind::INVOKE) {
        expr = Codira::StaticCast<const Invoke*>(e);
        exprE = nullptr;
    } else {
        expr = nullptr;
        exprE = Codira::StaticCast<const InvokeWithException*>(e);
    }
}

InvokeBase::InvokeBase(const Invoke* expr) : DynamicDispatchBase(expr), expr(expr), exprE(nullptr)
{
    CODEC_NULLPTR_CHECK(expr);
}

InvokeBase::InvokeBase(const InvokeWithException* exprE) : DynamicDispatchBase(exprE), expr(nullptr), exprE(exprE)
{
    CODEC_NULLPTR_CHECK(exprE);
}

Value* InvokeBase::GetObject() const
{
    return expr ? expr->GetObject()
                : exprE->GetObject();
}

InvokeStaticBase::InvokeStaticBase(const Expression* e) : DynamicDispatchBase(e)
{
    CODEC_NULLPTR_CHECK(e);
    if (e->GetExprKind() == ExprKind::INVOKESTATIC) {
        expr = Codira::StaticCast<const InvokeStatic*>(e);
        exprE = nullptr;
    } else {
        expr = nullptr;
        exprE = Codira::StaticCast<const InvokeStaticWithException*>(e);
    }
}

InvokeStaticBase::InvokeStaticBase(const InvokeStatic* expr) : DynamicDispatchBase(expr), expr(expr), exprE(nullptr)
{
    CODEC_NULLPTR_CHECK(expr);
}

InvokeStaticBase::InvokeStaticBase(const InvokeStaticWithException* exprE)
    : DynamicDispatchBase(exprE), expr(nullptr), exprE(exprE)
{
    CODEC_NULLPTR_CHECK(exprE);
}

Value* InvokeStaticBase::GetRTTIValue() const
{
    return expr ? expr->GetRTTIValue()
                : exprE->GetRTTIValue();
}

UnaryExprBase::UnaryExprBase(const Expression* e) : ExpressionBase(e)
{
    CODEC_NULLPTR_CHECK(e);
    if (e->GetExprMajorKind() == ExprMajorKind::UNARY_EXPR) {
        expr = Codira::StaticCast<const UnaryExpression*>(e);
        exprE = nullptr;
    } else {
        expr = nullptr;
        exprE = Codira::StaticCast<const IntOpWithException*>(e);
    }
}

UnaryExprBase::UnaryExprBase(const UnaryExpression* expr) : ExpressionBase(expr), expr(expr), exprE(nullptr)
{
    CODEC_NULLPTR_CHECK(expr);
}

UnaryExprBase::UnaryExprBase(const IntOpWithException* exprE) : ExpressionBase(exprE), expr(nullptr), exprE(exprE)
{
    CODEC_NULLPTR_CHECK(exprE);
}

ExprKind UnaryExprBase::GetOpKind() const
{
    return expr ? expr->GetExprKind()
                : exprE->GetOpKind();
}

std::string UnaryExprBase::GetExprKindName() const
{
    return expr ? expr->GetExprKindName()
                : exprE->GetOpKindName();
}

Value* UnaryExprBase::GetOperand() const
{
    return expr ? expr->GetOperand()
                : exprE->GetLHSOperand();
}

Codira::OverflowStrategy UnaryExprBase::GetOverflowStrategy() const
{
    return expr ? expr->GetOverflowStrategy()
                : exprE->GetOverflowStrategy();
}

BinaryExprBase::BinaryExprBase(const Expression* e) : ExpressionBase(e)
{
    CODEC_NULLPTR_CHECK(e);
    if (e->GetExprMajorKind() == ExprMajorKind::BINARY_EXPR) {
        expr = Codira::StaticCast<const BinaryExpression*>(e);
        exprE = nullptr;
    } else {
        expr = nullptr;
        exprE = Codira::StaticCast<const IntOpWithException*>(e);
    }
}

BinaryExprBase::BinaryExprBase(const BinaryExpression* expr) : ExpressionBase(expr), expr(expr), exprE(nullptr)
{
    CODEC_NULLPTR_CHECK(expr);
}

BinaryExprBase::BinaryExprBase(const IntOpWithException* exprE) : ExpressionBase(exprE), expr(nullptr), exprE(exprE)
{
    CODEC_NULLPTR_CHECK(exprE);
}

ExprKind BinaryExprBase::GetOpKind() const
{
    return expr ? expr->GetExprKind()
                : exprE->GetOpKind();
}

std::string BinaryExprBase::GetExprKindName() const
{
    return expr ? expr->GetExprKindName()
                : exprE->GetOpKindName();
}

Value* BinaryExprBase::GetLHSOperand() const
{
    return expr ? expr->GetLHSOperand()
                : exprE->GetLHSOperand();
}

Value* BinaryExprBase::GetRHSOperand() const
{
    return expr ? expr->GetRHSOperand()
                : exprE->GetRHSOperand();
}

Codira::OverflowStrategy BinaryExprBase::GetOverflowStrategy() const
{
    return expr ? expr->GetOverflowStrategy()
                : exprE->GetOverflowStrategy();
}

SpawnBase::SpawnBase(const Expression* e) : ExpressionBase(e)
{
    CODEC_NULLPTR_CHECK(e);
    if (e->GetExprKind() == ExprKind::SPAWN) {
        expr = Codira::StaticCast<const Spawn*>(e);
        exprE = nullptr;
    } else {
        expr = nullptr;
        exprE = Codira::StaticCast<const SpawnWithException*>(e);
    }
}

SpawnBase::SpawnBase(const Spawn* expr) : ExpressionBase(expr), expr(expr), exprE(nullptr)
{
    CODEC_NULLPTR_CHECK(expr);
}

SpawnBase::SpawnBase(const SpawnWithException* exprE) : ExpressionBase(exprE), expr(nullptr), exprE(exprE)
{
    CODEC_NULLPTR_CHECK(exprE);
}

Value* SpawnBase::GetObject() const
{
    return expr ? expr->GetOperands()[0]
                : exprE->GetOperands()[0];
}

bool SpawnBase::IsExecuteClosure() const
{
    return expr ? expr->IsExecuteClosure()
                : exprE->IsExecuteClosure();
}

IntrinsicBase::IntrinsicBase(const Expression* e) : ExpressionBase(e)
{
    CODEC_NULLPTR_CHECK(e);
    if (e->GetExprKind() == ExprKind::INTRINSIC) {
        expr = Codira::StaticCast<const Intrinsic*>(e);
        exprE = nullptr;
    } else {
        expr = nullptr;
        exprE = Codira::StaticCast<const IntrinsicWithException*>(e);
    }
}

IntrinsicBase::IntrinsicBase(const Intrinsic* expr) : ExpressionBase(expr), expr(expr), exprE(nullptr)
{
    CODEC_NULLPTR_CHECK(expr);
}

IntrinsicBase::IntrinsicBase(const IntrinsicWithException* exprE) : ExpressionBase(exprE), expr(nullptr), exprE(exprE)
{
    CODEC_NULLPTR_CHECK(exprE);
}

IntrinsicKind IntrinsicBase::GetIntrinsicKind() const
{
    return expr ? expr->GetIntrinsicKind()
                : exprE->GetIntrinsicKind();
}

std::vector<Type*> IntrinsicBase::GetInstantiatedTypeArgs() const
{
    return expr ? expr->GetInstantiatedTypeArgs()
                : exprE->GetInstantiatedTypeArgs();
}

AllocateBase::AllocateBase(const Expression* e) : ExpressionBase(e)
{
    CODEC_NULLPTR_CHECK(e);
    if (e->GetExprKind() == ExprKind::ALLOCATE) {
        expr = Codira::StaticCast<const Allocate*>(e);
        exprE = nullptr;
    } else {
        expr = nullptr;
        exprE = Codira::StaticCast<const AllocateWithException*>(e);
    }
}

AllocateBase::AllocateBase(const Allocate* expr) : ExpressionBase(expr), expr(expr), exprE(nullptr)
{
    CODEC_NULLPTR_CHECK(expr);
}

AllocateBase::AllocateBase(const AllocateWithException* exprE) : ExpressionBase(exprE), expr(nullptr), exprE(exprE)
{
    CODEC_NULLPTR_CHECK(exprE);
}

Type* AllocateBase::GetType() const
{
    return expr ? expr->GetType()
                : exprE->GetType();
}

RawArrayAllocateBase::RawArrayAllocateBase(const Expression* e) : ExpressionBase(e)
{
    CODEC_NULLPTR_CHECK(e);
    if (e->GetExprKind() == ExprKind::RAW_ARRAY_ALLOCATE) {
        expr = Codira::StaticCast<const RawArrayAllocate*>(e);
        exprE = nullptr;
    } else {
        expr = nullptr;
        exprE = Codira::StaticCast<const RawArrayAllocateWithException*>(e);
    }
}

RawArrayAllocateBase::RawArrayAllocateBase(const RawArrayAllocate* expr)
    : ExpressionBase(expr), expr(expr), exprE(nullptr)
{
    CODEC_NULLPTR_CHECK(expr);
}

RawArrayAllocateBase::RawArrayAllocateBase(const RawArrayAllocateWithException* exprE)
    : ExpressionBase(exprE), expr(nullptr), exprE(exprE)
{
    CODEC_NULLPTR_CHECK(exprE);
}

Type* RawArrayAllocateBase::GetElementType() const
{
    return expr ? expr->GetElementType()
                : exprE->GetElementType();
}

Value* RawArrayAllocateBase::GetSize() const
{
    return expr ? expr->GetSize()
                : exprE->GetSize();
}
