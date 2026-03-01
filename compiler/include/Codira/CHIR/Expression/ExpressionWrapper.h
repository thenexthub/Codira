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
 * Many Expressions have their xxxWithException version, we wrap them with a new class instead of using a base class
 */

#ifndef CODIRA_CHIR_EXPRESSION_WRAPPER_H
#define CODIRA_CHIR_EXPRESSION_WRAPPER_H

#include "Codira/CHIR/Expression/Terminator.h"

namespace Codira {
namespace CHIR {

class ExpressionBase {
public:
    const Expression* GetRawExpr() const;
    LocalVar* GetResult() const;
    std::vector<Value*> GetOperands() const;

protected:
    explicit ExpressionBase(const Expression* e);

private:
    const Expression* expr;
};

class FuncCallBase : public ExpressionBase {
public:
    explicit FuncCallBase(const Expression* e);
    explicit FuncCallBase(const FuncCall* expr);
    explicit FuncCallBase(const FuncCallWithException* exprE);

    std::vector<Value*> GetArgs() const;
    Type* GetThisType() const;
    std::vector<Type*> GetInstantiatedTypeArgs() const;

private:
    const FuncCall* expr;
    const FuncCallWithException* exprE;
};

class ApplyBase : public FuncCallBase {
public:
    explicit ApplyBase(const Expression* e);
    explicit ApplyBase(const Apply* expr);
    explicit ApplyBase(const ApplyWithException* exprE);

    Value* GetCallee() const;
    Type* GetInstParentCustomTyOfCallee(CHIRBuilder& builder) const;

private:
    const Apply* expr;
    const ApplyWithException* exprE;
};

class DynamicDispatchBase : public FuncCallBase {
public:
    explicit DynamicDispatchBase(const Expression* e);
    explicit DynamicDispatchBase(const DynamicDispatch* expr);
    explicit DynamicDispatchBase(const DynamicDispatchWithException* exprE);

    std::vector<GenericType*> GetGenericTypeParams() const;
    std::string GetMethodName() const;
    FuncType* GetMethodType() const;
    size_t GetVirtualMethodOffset() const;
    ClassType* GetInstSrcParentCustomTypeOfMethod(CHIRBuilder& builder) const;

private:
    const DynamicDispatch* expr;
    const DynamicDispatchWithException* exprE;
};

class InvokeBase : public DynamicDispatchBase {
public:
    explicit InvokeBase(const Expression* e);
    explicit InvokeBase(const Invoke* expr);
    explicit InvokeBase(const InvokeWithException* exprE);

    Value* GetObject() const;

private:
    const Invoke* expr;
    const InvokeWithException* exprE;
};

class InvokeStaticBase : public DynamicDispatchBase {
public:
    explicit InvokeStaticBase(const Expression* e);
    explicit InvokeStaticBase(const InvokeStatic* expr);
    explicit InvokeStaticBase(const InvokeStaticWithException* exprE);

    Value* GetRTTIValue() const;

private:
    const InvokeStatic* expr;
    const InvokeStaticWithException* exprE;
};

class UnaryExprBase : public ExpressionBase {
public:
    explicit UnaryExprBase(const Expression* e);
    explicit UnaryExprBase(const UnaryExpression* expr);
    explicit UnaryExprBase(const IntOpWithException* exprE);

    ExprKind GetOpKind() const;
    std::string GetExprKindName() const;
    Value* GetOperand() const;
    OverflowStrategy GetOverflowStrategy() const;

private:
    const UnaryExpression* expr;
    const IntOpWithException* exprE;
};

class BinaryExprBase : public ExpressionBase {
public:
    explicit BinaryExprBase(const Expression* e);
    explicit BinaryExprBase(const BinaryExpression* expr);
    explicit BinaryExprBase(const IntOpWithException* exprE);

    ExprKind GetOpKind() const;
    std::string GetExprKindName() const;
    Value* GetLHSOperand() const;
    Value* GetRHSOperand() const;
    OverflowStrategy GetOverflowStrategy() const;
    
private:
    const BinaryExpression* expr;
    const IntOpWithException* exprE;
};

class SpawnBase : public ExpressionBase {
public:
    explicit SpawnBase(const Expression* e);
    explicit SpawnBase(const Spawn* expr);
    explicit SpawnBase(const SpawnWithException* exprE);

    Value* GetObject() const;
    bool IsExecuteClosure() const;
private:
    const Spawn* expr;
    const SpawnWithException* exprE;
};

class IntrinsicBase : public ExpressionBase {
public:
    explicit IntrinsicBase(const Expression* e);
    explicit IntrinsicBase(const Intrinsic* expr);
    explicit IntrinsicBase(const IntrinsicWithException* exprE);

    IntrinsicKind GetIntrinsicKind() const;
    std::vector<Type*> GetInstantiatedTypeArgs() const;
    
private:
    const Intrinsic* expr;
    const IntrinsicWithException* exprE;
};

class AllocateBase : public ExpressionBase {
public:
    explicit AllocateBase(const Expression* e);
    explicit AllocateBase(const Allocate* expr);
    explicit AllocateBase(const AllocateWithException* exprE);

    Type* GetType() const;
private:
    const Allocate* expr;
    const AllocateWithException* exprE;
};

class RawArrayAllocateBase : public ExpressionBase {
public:
    explicit RawArrayAllocateBase(const Expression* e);
    explicit RawArrayAllocateBase(const RawArrayAllocate* expr);
    explicit RawArrayAllocateBase(const RawArrayAllocateWithException* exprE);

    Type* GetElementType() const;
    Value* GetSize() const;
private:
    const RawArrayAllocate* expr;
    const RawArrayAllocateWithException* exprE;
};
}
}

#endif
