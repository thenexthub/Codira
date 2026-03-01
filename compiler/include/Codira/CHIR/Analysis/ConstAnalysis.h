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

#ifndef CODIRA_CHIR_ANALYSIS_CONST_ANALYSIS_H
#define CODIRA_CHIR_ANALYSIS_CONST_ANALYSIS_H

#include "Codira/CHIR/Analysis/ValueAnalysis.h"
#include "Codira/CHIR/DiagAdapter.h"
#include "Codira/CHIR/OverflowChecking.h"
#include "Codira/CHIR/Utils.h"
#include "Codira/CHIR/Value.h"

namespace Codira::CHIR {

struct ConstValue {
    enum class ConstKind : uint8_t { UINT, INT, FLOAT, RUNE, BOOL, STRING };

    ConstValue() = delete;

    ConstValue(ConstKind kind);

    virtual ~ConstValue();

    /// join two state of const value, return nullopt if no change happened.
    virtual std::optional<std::unique_ptr<ConstValue>> Join(const ConstValue& rhs) const = 0;

    /// output to string of const value.
    virtual std::string ToString() const = 0;

    /// clone a const value.
    virtual std::unique_ptr<ConstValue> Clone() const = 0;

    /// get const value kind, such as bool, int, float.
    ConstKind GetConstKind() const;

protected:
    /// value kind for const value, every subtype has a type kind.
    ConstKind kind;
};

struct ConstBoolVal final : ConstValue {
    ConstBoolVal(bool val) : ConstValue(ConstKind::BOOL), val(val)
    {
    }
    ~ConstBoolVal() final = default;

    /// join two state of const value, return nullopt if no change happened.
    std::optional<std::unique_ptr<ConstValue>> Join(const ConstValue& rhs) const override;

    std::string ToString() const override;

    std::unique_ptr<ConstValue> Clone() const override;

    bool GetVal() const;

private:
    bool val;
};

struct ConstRuneVal final : ConstValue {
    ConstRuneVal(char32_t val) : ConstValue(ConstKind::RUNE), val(val)
    {
    }
    ~ConstRuneVal() final = default;

    /// join two state of const value, return nullopt if no change happened.
    std::optional<std::unique_ptr<ConstValue>> Join(const ConstValue& rhs) const override;

    std::string ToString() const override;

    std::unique_ptr<ConstValue> Clone() const override;

    char32_t GetVal() const;

private:
    char32_t val;
};

struct ConstStrVal final : ConstValue {
    ConstStrVal(std::string val) : ConstValue(ConstKind::STRING), val(val)
    {
    }
    ~ConstStrVal() final = default;

    /// join two state of const value, return nullopt if no change happened.
    std::optional<std::unique_ptr<ConstValue>> Join(const ConstValue& rhs) const override;

    std::string ToString() const override;

    std::unique_ptr<ConstValue> Clone() const override;

    std::string GetVal() const;

private:
    std::string val;
};

struct ConstUIntVal final : ConstValue {
    ConstUIntVal(uint64_t val) : ConstValue(ConstKind::UINT), val(val)
    {
    }
    ~ConstUIntVal() final = default;

    /// join two state of const value, return nullopt if no change happened.
    std::optional<std::unique_ptr<ConstValue>> Join(const ConstValue& rhs) const override;

    std::string ToString() const override;

    std::unique_ptr<ConstValue> Clone() const override;

    uint64_t GetVal() const;

private:
    uint64_t val;
};

struct ConstIntVal final : ConstValue {
    ConstIntVal(int64_t val) : ConstValue(ConstKind::INT), val(val)
    {
    }
    ~ConstIntVal() final = default;

    /// join two state of const value, return nullopt if no change happened.
    std::optional<std::unique_ptr<ConstValue>> Join(const ConstValue& rhs) const override;

    std::string ToString() const override;

    std::unique_ptr<ConstValue> Clone() const override;

    int64_t GetVal() const;

private:
    int64_t val;
};

struct ConstFloatVal final : ConstValue {
    ConstFloatVal(double val) : ConstValue(ConstKind::FLOAT), val(val)
    {
    }
    ~ConstFloatVal() final = default;

    /// join two state of const value, return nullopt if no change happened.
    std::optional<std::unique_ptr<ConstValue>> Join(const ConstValue& rhs) const override;

    std::string ToString() const override;

    std::unique_ptr<ConstValue> Clone() const override;

    double GetVal() const;

private:
    double val;
};

/**
 * @brief the abstract value domain of const value
 */
using ConstValueDomain = ValueDomain<ConstValue>;
/**
 * @brief the state of const value domain
 */
using ConstDomain = State<ConstValueDomain>;
/**
 * @brief partially specialized analysis import value.
 */
template <> const std::string Analysis<ConstDomain>::name;
template <> const std::optional<unsigned> Analysis<ConstDomain>::blockLimit;
template <> ConstDomain::ChildrenMap ValueAnalysis<ConstValueDomain>::globalChildrenMap;
template <> ConstDomain::AllocatedRefMap ValueAnalysis<ConstValueDomain>::globalAllocatedRefMap;
template <> ConstDomain::AllocatedObjMap ValueAnalysis<ConstValueDomain>::globalAllocatedObjMap;
template <> std::vector<std::unique_ptr<Ref>> ValueAnalysis<ConstValueDomain>::globalRefPool;
template <> std::vector<std::unique_ptr<AbstractObject>> ValueAnalysis<ConstValueDomain>::globalAbsObjPool;
template <> ConstDomain ValueAnalysis<ConstValueDomain>::globalState;

/**
 * @brief check whether global var need const analysis.
 * @param gv global var to check.
 * @return flag global var need analyse
 */
template <> bool IsTrackedGV<ValueDomain<ConstValue>>(const GlobalVar& gv);

/**
 * @brief literal value analysis function
 * @param literalValue input literal value to analyse
 * @return const value literalValue is.
 */
template <>
ValueDomain<ConstValue> HandleNonNullLiteralValue<ValueDomain<ConstValue>>(const LiteralValue* literalValue);

/**
 * @brief constant value analysis for CHIR IR.
 */
class ConstAnalysis final : public ValueAnalysis<ConstValueDomain> {
public:
    ConstAnalysis() = delete;
    /**
     * @brief const analysis constructor
     * @param func function to analyse
     * @param builder CHIR builder for generating IR.
     * @param isDebug flag whether print debug log.
     * @param diag reporter to report warning or error.
     */
    ConstAnalysis(const Func* func, CHIRBuilder& builder, bool isDebug, DiagAdapter* diag);

    ~ConstAnalysis() final;

private:
    void PrintDebugMessage(const Expression* expr, const ConstValue* absVal) const;

    void MarkExpressionAsMustNotOverflow(Expression& expr) const;

    // ======== Transfer functions for normal expressions based on ExprMajorKind ======== //

    void HandleNormalExpressionEffect(ConstDomain& state, const Expression* expression) override;

    enum class ExceptionKind : uint8_t { SUCCESS, FAIL, NA };

    void HandleUnaryExpr(ConstDomain& state, const UnaryExpression* unaryExpr, ExceptionKind& exceptionKind);

    void HandleBinaryExpr(ConstDomain& state, const BinaryExpression* binaryExpr, ExceptionKind& exceptionKind);

    void HandleOthersExpr(ConstDomain& state, const Expression* expression, ExceptionKind& exceptionKind);

    // ======================= Transfer functions for terminators ======================= //

    std::optional<Block*> HandleTerminatorEffect(ConstDomain& state, const Terminator* terminator) override;

    std::optional<Block*> HandleBranchTerminator(const ConstDomain& state, const Branch* branch) const;

    std::optional<Block*> HandleMultiBranchTerminator(const ConstDomain& state, const MultiBranch* multi) const;

    ExceptionKind HandleIntOpWithExcepTerminator(ConstDomain& state, const IntOpWithException* intOp);

    // ============= Helper functions for Unary/BinaryExpression ============= //

    template <typename T, typename TUnary>
    ExceptionKind HandleNegOpOfInt(ConstDomain& state, const TUnary* expr, const ConstValue* constVal)
    {
        auto dest = expr->GetResult();
        auto os = expr->GetOverflowStrategy();

        const T* absVal = static_cast<const T*>(constVal);
        using ValType = decltype(absVal->GetVal());
        ValType res = 0;
        ValType val = absVal->GetVal();
        bool isOverflow = false;
        if constexpr (std::is_same_v<ValType, int64_t>) {
            isOverflow =
                OverflowChecker::IsIntOverflow(dest->GetType()->GetTypeKind(), ExprKind::NEG, 0, val, os, &res);
        } else if constexpr (std::is_same_v<ValType, uint64_t>) {
            isOverflow =
                OverflowChecker::IsUIntOverflow(dest->GetType()->GetTypeKind(), ExprKind::NEG, 0, val, os, &res);
        } else {
            CODEC_ABORT();
        }

        if (isOverflow && os == OverflowStrategy::THROWING) {
            if (this->isStable) {
                auto builder = diag->DiagnoseRefactor(
                    DiagKindRefactor::chir_arithmetic_operator_overflow, ToRange(expr->GetDebugLocation()), "-");
                std::string hint = "-" + dest->GetType()->ToString() + "(" + absVal->ToString() + ")";
                builder.AddMainHintArguments(hint);
                builder.AddNote(GenerateTypeRangePrompt(expr->GetResult()->GetType()));
            }
            state.SetToBound(dest, /* isTop = */ true);
            return ExceptionKind::FAIL;
        } else {
            state.Update(dest, std::make_unique<T>(res));
            return ExceptionKind::SUCCESS;
        }
    }

    // (a+b), (a-b), (a*b), (a/b), (a%b)
    template <typename TBinary>
    ExceptionKind HandleArithmeticOp(ConstDomain& state, const TBinary* binary, ExprKind kind)
    {
        auto lhs = binary->GetLHSOperand();
        auto rhs = binary->GetRHSOperand();
        CODEC_ASSERT(lhs->GetType() == rhs->GetType());
        const ConstValue* lhsAbsVal = state.CheckAbstractValue(lhs);
        const ConstValue* rhsAbsVal = state.CheckAbstractValue(rhs);

        if (!lhsAbsVal && !rhsAbsVal) {
            state.SetToBound(binary->GetResult(), /* isTop = */ true);
            return ExceptionKind::NA;
        }

        if (lhs->GetType()->IsInteger()) {
            auto intTy = StaticCast<IntType*>(lhs->GetType());
            if (intTy->IsSigned()) {
                return HandleArithmeticOpOfInt<ConstIntVal>(state, binary, kind, lhsAbsVal, rhsAbsVal);
            } else {
                return HandleArithmeticOpOfInt<ConstUIntVal>(state, binary, kind, lhsAbsVal, rhsAbsVal);
            }
        } else if constexpr (std::is_same_v<BinaryExpression, TBinary>) {
            HandleArithmeticOpOfFloat(state, binary, lhsAbsVal, rhsAbsVal);
        }
        return ExceptionKind::NA;
    }

    /**
     * This function handles constant folding on the arithmetic operations whose operands
     * are integers, which include:
     * a) BinaryExpression: ADD, SUB, MUL, DIV, MOD
     *
     * It will try to calculate the result of the arithmetic operations base on the known
     * constant information. It will first check if this arithmetic operation is trivial
     * (e.g. `a * 0` is a trivial arithmetic operation as its result is always zero).
     * Then, if we have the constant information of all the operands, we will calculate the
     * result. If an overflow has occurred, an error may be raised depends on the overflow
     * strategy. If we don't have the enough constant information, the result will be set
     * to a Top.
     *
     * ** note **:
     * 1. We don't handle EXP binary operations in this funciton. The reason is the type of
     * its parameter is different. The types of all the other binary expressions' parameters
     * are the same. See @fn HandleExpOp for details.
     */
    template <typename T, typename TBinary>
    ExceptionKind HandleArithmeticOpOfInt(
        ConstDomain& state, const TBinary* expr, ExprKind kind, const ConstValue* lhs, const ConstValue* rhs)
    {
        auto dest = expr->GetResult();
        auto os = expr->GetOverflowStrategy();

        const T* left = static_cast<const T*>(lhs);
        const T* right = static_cast<const T*>(rhs);

        auto isTrivial = HandleTrivialArithmeticOp(state, expr, kind, left, right);
        if (isTrivial != ExceptionKind::NA) {
            return isTrivial;
        }
        if (expr->GetLHSOperand() == expr->GetRHSOperand() && kind == ExprKind::SUB) {
            // `a - a` => 0
            state.Update(dest, std::make_unique<T>(0));
            return ExceptionKind::SUCCESS;
        }

        if (!left || !right) {
            state.SetToBound(expr->GetResult(), /* isTop = */ true);
            return ExceptionKind::NA;
        }

        using ValType = decltype(left->GetVal());
        ValType res = 0;
        ValType x = left->GetVal();
        ValType y = right->GetVal();
        bool isOverflow = false;
        if constexpr (std::is_same_v<ValType, int64_t>) {
            isOverflow = OverflowChecker::IsIntOverflow(dest->GetType()->GetTypeKind(), kind, x, y, os, &res);
        } else if constexpr (std::is_same_v<ValType, uint64_t>) {
            isOverflow = OverflowChecker::IsUIntOverflow(dest->GetType()->GetTypeKind(), kind, x, y, os, &res);
        } else {
            CODEC_ABORT();
        }

        if (isOverflow && os == OverflowStrategy::THROWING) {
            RaiseArithmeticOverflowError(expr, kind, left, right);
            state.SetToBound(dest, /* isTop = */ true);
            return ExceptionKind::FAIL;
        } else {
            state.Update(dest, std::make_unique<T>(res));
            return ExceptionKind::SUCCESS;
        }
    }

    /**
     * This function handles constant folding on the trivial arithmetic operation, which means the result
     * of it should be a constant. This function also detects the DIV_BY_ZERO error.
     * This function will return a true if it's a trivial arithmetic operation; otherwise it will return a
     * false.
     *
     * note: `a + 0` is *not* a trivial arithmetic operation, as its result `a` is not a constant;
     *       `a * 0` is a trivial arithmetic operation, as its result is always zero.
     *
     * Here is a list of the trivial arithmetic operation this function handles.
     * a) ADD: none.
     * b) SUB: none. (We handle the `a - a` in @fn HandleArithmeticOpOfInt.)
     * c) MUL: `a * 0 = 0` and `0 * a = 0`
     * d) DIV: `a \ 0` and `0 \ a = 0`
     * e) MOD: `a % 0`, `0 % a = 0` and `a % 1 == 0`
     * f) EXP: We don't handle this kind of operations in this funciton. See @fn HandleExpOp.
     */
    template <typename T, typename TBinary>
    ExceptionKind HandleTrivialArithmeticOp(
        ConstDomain& state, const TBinary* expr, ExprKind kind, const T* left, const T* right)
    {
        auto dest = expr->GetResult();
        if (right) {
            if (right->GetVal() == 0) {
                if (kind == ExprKind::DIV || kind == ExprKind::MOD) {
                    // `a / 0` or `a % 0` => error
                    RaiseDivByZeroError(expr, kind);
                    state.SetToBound(dest, /* isTop = */ true);
                    return ExceptionKind::FAIL;
                } else if (kind == ExprKind::MUL) {
                    // `a * 0` => 0
                    state.Update(dest, std::make_unique<T>(0));
                    return ExceptionKind::SUCCESS;
                }
            }
            if (right->GetVal() == 1 && kind == ExprKind::MOD) {
                // `a % 1` => 0
                state.Update(dest, std::make_unique<T>(0));
                return ExceptionKind::SUCCESS;
            }
        }
        if (left && left->GetVal() == 0) {
            if (kind == ExprKind::MUL || kind == ExprKind::DIV || kind == ExprKind::MOD) {
                // `0 * a`, `0 \ a` or `0 % a`
                state.Update(dest, std::make_unique<T>(0));
                return ExceptionKind::SUCCESS;
            }
        }
        return ExceptionKind::NA;
    }

    // a**b
    template <typename TBinary> ExceptionKind HandleExpOp(ConstDomain& state, const TBinary* binary)
    {
        auto dest = binary->GetResult();
        if (!dest->GetType()->IsInteger()) {
            state.SetToBound(dest, /* isTop = */ true);
            return ExceptionKind::NA;
        }
        auto lhs = binary->GetLHSOperand();
        auto rhs = binary->GetRHSOperand();
        const ConstIntVal* lhsAbsVal = static_cast<const ConstIntVal*>(state.CheckAbstractValue(lhs));
        const ConstUIntVal* rhsAbsVal = static_cast<const ConstUIntVal*>(state.CheckAbstractValue(rhs));
        if (rhsAbsVal) {
            // `a ** 0 = 1` has a higher priority than `0 ** a = 0` as `0 ** 0 = 1`
            if (rhsAbsVal->GetVal() == 0) {
                // `a ** 0 = 1`
                state.Update(dest, std::make_unique<ConstIntVal>(1));
                return ExceptionKind::SUCCESS;
            }
        }
        if (lhsAbsVal) {
            auto val = lhsAbsVal->GetVal();
            if (val == 0 || val == 1) {
                // `0 ** a = 0` and `1 ** a = 1`
                state.Update(dest, std::make_unique<ConstIntVal>(val));
                return ExceptionKind::SUCCESS;
            }
        }
        if (!lhsAbsVal || !rhsAbsVal) {
            state.SetToBound(dest, /* isTop = */ true);
            return ExceptionKind::NA;
        }
        auto os = binary->GetOverflowStrategy();
        int64_t res = 0;
        bool isOverflow = OverflowChecker::IsExpOverflow(lhsAbsVal->GetVal(), rhsAbsVal->GetVal(), os, &res);
        if (isOverflow && os == OverflowStrategy::THROWING) {
            RaiseArithmeticOverflowError(binary, ExprKind::EXP, lhsAbsVal, rhsAbsVal);
            state.SetToBound(dest, /* isTop = */ true);
            return ExceptionKind::FAIL;
        } else {
            state.Update(dest, std::make_unique<ConstIntVal>(res));
            return ExceptionKind::SUCCESS;
        }
    }

    /**
     * This function handles constant folding on the arithmetic operations whose operands
     * are floats, which include:
     * a) BinaryExpression: ADD, SUB, MUL, DIV
     *
     * It will try to calculate the result of the arithmetic operations base on the known
     * constant information. If we have the constant information of all the operands, we
     * will calculate the result. Otherwise, or if the result is an NaN or an Inf, the
     * state of the result will be set to Top.
     *
     * ** note **:
     * 1. We don't handle EXP binary operations whose operands are floats. See @fn HandleExpOp.
     */
    void HandleArithmeticOpOfFloat(
        ConstDomain& state, const BinaryExpression* binaryExpr, const ConstValue* lhs, const ConstValue* rhs) const;

    template <typename TBinary>
    ExceptionKind HandleBitwiseOp(ConstDomain& state, const TBinary* binaryExpr, ExprKind kind)
    {
        auto lhs = binaryExpr->GetLHSOperand();
        auto rhs = binaryExpr->GetRHSOperand();
        auto isLSigned = StaticCast<IntType*>(lhs->GetType())->IsSigned();
        auto isRSigned = StaticCast<IntType*>(rhs->GetType())->IsSigned();
        const ConstValue* lhsAbsVal = state.CheckAbstractValue(lhs);
        const ConstValue* rhsAbsVal = state.CheckAbstractValue(rhs);
        if (isLSigned && isRSigned) {
            return HandleBitwiseOpOfType<ConstIntVal, ConstIntVal>(state, binaryExpr, kind, lhsAbsVal, rhsAbsVal);
        } else if (isLSigned) {
            return HandleBitwiseOpOfType<ConstIntVal, ConstUIntVal>(state, binaryExpr, kind, lhsAbsVal, rhsAbsVal);
        } else if (isRSigned) {
            return HandleBitwiseOpOfType<ConstUIntVal, ConstIntVal>(state, binaryExpr, kind, lhsAbsVal, rhsAbsVal);
        } else {
            return HandleBitwiseOpOfType<ConstUIntVal, ConstUIntVal>(state, binaryExpr, kind, lhsAbsVal, rhsAbsVal);
        }
    }

    /**
     * This function handles constant folding on the bitwise operations, which include:
     * a) BinaryExpression: LSHIFT, RSHIFT, BITAND, BITXOR, BITOR
     *
     * It will try to calculate the result of the bitwise operations base on the known
     * constant information. First, we will do some checking. For bitwise left/right
     * shift operation, an error will be raised in the following two cases:
     * a) if the right operand is known to be negative, an error will be raise.
     * b) if the value of the right operand is GE than the length of the left operand,
     *    an error.
     * Then we will update the state  the result of the operations based on our known
     * constant information.
     *
     * ** note **:
     * 1. Both parameter @p lhs and @p rhs are required to be non-null values.
     */
    template <typename L, typename R, typename TBinary>
    ExceptionKind HandleBitwiseOpOfType(
        ConstDomain& state, const TBinary* binaryExpr, ExprKind kind, const ConstValue* lhs, const ConstValue* rhs)
    {
        const L* left = static_cast<const L*>(lhs);
        const R* right = static_cast<const R*>(rhs);

        auto dest = binaryExpr->GetResult();
        bool isShiftOp = kind == ExprKind::LSHIFT || kind == ExprKind::RSHIFT;

        if (!right) {
            state.SetToBound(dest, /* isTop = */ true);
            return ExceptionKind::NA;
        }
        auto rightVal = right->GetVal();
        if constexpr (std::is_same_v<int64_t, decltype(rightVal)>) {
            if (isShiftOp && rightVal < 0) {
                RaiseNegativeShiftError(binaryExpr, rightVal);
                state.SetToBound(dest, /* isTop = */ true);
                return ExceptionKind::FAIL;
            }
        }
        auto lhsOperandType = binaryExpr->GetLHSOperand()->GetType();
        if (isShiftOp) {
            auto lhsValBit = StaticCast<NumericType*>(lhsOperandType)->GetBitness();
            if (static_cast<uint64_t>(rightVal) >= lhsValBit) {
                RaiseOvershiftError(binaryExpr, rightVal, lhsValBit);
                state.SetToBound(dest, /* isTop = */ true);
                return ExceptionKind::FAIL;
            }
        }
        if (!left) {
            state.SetToBound(dest, /* isTop = */ true);
            return ExceptionKind::NA;
        }

        using LValType = decltype(left->GetVal());
        using RValType = decltype(right->GetVal());
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"
#endif
        const static std::unordered_map<ExprKind, std::function<LValType(LValType, RValType)>> ops = {
            {ExprKind::LSHIFT, [](LValType x, RValType y) { return x << y; }},
            {ExprKind::RSHIFT, [](LValType x, RValType y) { return x >> y; }},
            {ExprKind::BITAND, [](LValType x, RValType y) { return x & y; }},
            {ExprKind::BITXOR, [](LValType x, RValType y) { return x ^ y; }},
            {ExprKind::BITOR, [](LValType x, RValType y) { return x | y; }}};
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

        auto op = ops.find(kind);
        CODEC_ASSERT(op != ops.end());

        LValType res = op->second(left->GetVal(), rightVal);
        res = CutOffHighBits(res, lhsOperandType->GetTypeKind());
        state.Update(dest, std::make_unique<L>(res));
        return ExceptionKind::SUCCESS;
    }

    void HandleRelationalOp(ConstDomain& state, const BinaryExpression* binaryExpr);

    /**
     * This function handles constant folding on the relational operations, which include:
     * a) BinaryExpression: LT, GT, LE, GE, EQUAL, NOTEQUAL
     *
     * It will try to calculate the result of the relational operations base on the known
     * constant information.
     *
     * ** note **:
     * 1. Both parameter @p lhs and @p rhs are required to be non-null values.
     */
    template <typename T>
    void HandleRelationalOpOfType(
        ConstDomain& state, const BinaryExpression* binaryExpr, const ConstValue* lhs, const ConstValue* rhs)
    {
        const T* left = static_cast<const T*>(lhs);
        const T* right = static_cast<const T*>(rhs);

        using ValType = decltype(left->GetVal());
        const static std::unordered_map<ExprKind, std::function<bool(ValType, ValType)>> ops = {
            {ExprKind::LT, std::less<ValType>{}},
            {ExprKind::LE, std::less_equal<ValType>{}},
            {ExprKind::GT, std::greater<ValType>{}},
            {ExprKind::GE, std::greater_equal<ValType>{}},
            {ExprKind::EQUAL, std::equal_to<ValType>{}},
            {ExprKind::NOTEQUAL, std::not_equal_to<ValType>{}},
        };

        auto op = ops.find(binaryExpr->GetExprKind());
        CODEC_ASSERT(op != ops.end());

        bool res = op->second(left->GetVal(), right->GetVal());
        state.Update(binaryExpr->GetResult(), std::make_unique<ConstBoolVal>(res));
    }

    void HandleLogicalOp(ConstDomain& state, const BinaryExpression* binaryExpr) const;

    // =============== Error reporting functions for DIV_BY_ZERO or OVERFLOW erros  =============== //

    template <typename TBinary> void RaiseDivByZeroError(const TBinary* binary, ExprKind kind)
    {
        if (this->isStable) {
            auto& loc = binary->GetDebugLocation();
            auto prompt = kind == ExprKind::DIV ? "divide" : "modulo";
            auto builder = diag->DiagnoseRefactor(DiagKindRefactor::chir_divisor_is_zero, ToRange(loc), prompt);
            builder.AddMainHintArguments(prompt);
        }
    }

    template <typename TBinary, typename T, typename U>
    void RaiseArithmeticOverflowError(const TBinary* expr, ExprKind kind, const T* leftVal, const U* rightVal)
    {
        if (this->isStable) {
            auto& loc = expr->GetDebugLocation();
            auto ty = expr->GetResult()->GetType();
            const static std::unordered_map<ExprKind, std::string> ops = {
                {ExprKind::ADD, "+"},
                {ExprKind::SUB, "-"},
                {ExprKind::MUL, "*"},
                {ExprKind::DIV, "/"},
                {ExprKind::MOD, "%"},
                {ExprKind::EXP, "**"},
            };
            auto token = ops.find(kind);
            CODEC_ASSERT(token != ops.end());
            auto builder = diag->DiagnoseRefactor(
                DiagKindRefactor::chir_arithmetic_operator_overflow, ToRange(loc), token->second);
            std::string hint = ty->ToString() + "(" + leftVal->ToString() + ") " + token->second + " " +
                expr->GetRHSOperand()->GetType()->ToString() + "(" + rightVal->ToString() + ")";
            builder.AddMainHintArguments(hint);
            builder.AddNote(GenerateTypeRangePrompt(expr->GetResult()->GetType()));
        }
    }

    template <typename T, typename TBinary> void RaiseNegativeShiftError(const TBinary* expr, T rightVal)
    {
        if (this->isStable) {
            auto& loc = expr->GetDebugLocation();
            auto builder = diag->DiagnoseRefactor(DiagKindRefactor::chir_shift_length_overflow, ToRange(loc));
            builder.AddMainHintArguments(std::to_string(rightVal), expr->GetLHSOperand()->GetType()->ToString());
            builder.AddNote("right operand can not be negative");
        }
    }

    template <typename T, typename TBinary>
    void RaiseOvershiftError(const TBinary* expr, T rightVal, uint64_t leftValBit)
    {
        if (this->isStable) {
            auto& loc = expr->GetDebugLocation();
            auto lhsTyString = expr->GetLHSOperand()->GetType()->ToString();
            auto builder = diag->DiagnoseRefactor(DiagKindRefactor::chir_shift_length_overflow, ToRange(loc));
            builder.AddMainHintArguments(std::to_string(rightVal), lhsTyString);
            builder.AddNote("the type of left operand is " + lhsTyString +
                ", the most bits that expected to shift are " + std::to_string(leftValBit - 1));
        }
    }

    std::string GenerateTypeRangePrompt(Type* type)
    {
        const static std::unordered_map<Type::TypeKind, std::pair<int64_t, uint64_t>> typeToRange = {
            {Type::TypeKind::TYPE_INT8, {std::numeric_limits<int8_t>::min(), std::numeric_limits<int8_t>::max()}},
            {Type::TypeKind::TYPE_INT16, {std::numeric_limits<int16_t>::min(), std::numeric_limits<int16_t>::max()}},
            {Type::TypeKind::TYPE_INT32, {std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max()}},
            {Type::TypeKind::TYPE_INT64, {std::numeric_limits<int64_t>::min(), std::numeric_limits<int64_t>::max()}},
            {Type::TypeKind::TYPE_INT_NATIVE,
                {std::numeric_limits<ssize_t>::min(), std::numeric_limits<ssize_t>::max()}},
            {Type::TypeKind::TYPE_UINT8, {std::numeric_limits<uint8_t>::min(), std::numeric_limits<uint8_t>::max()}},
            {Type::TypeKind::TYPE_UINT16, {std::numeric_limits<uint16_t>::min(), std::numeric_limits<uint16_t>::max()}},
            {Type::TypeKind::TYPE_UINT32, {std::numeric_limits<uint32_t>::min(), std::numeric_limits<uint32_t>::max()}},
            {Type::TypeKind::TYPE_UINT64, {std::numeric_limits<uint64_t>::min(), std::numeric_limits<uint64_t>::max()}},
            {Type::TypeKind::TYPE_UINT_NATIVE,
                {std::numeric_limits<size_t>::min(), std::numeric_limits<size_t>::max()}}};

        auto [min, max] = typeToRange.at(type->GetTypeKind());
        return "range of " + type->ToString() + " is " + std::to_string(min) + " ~ " + std::to_string(max);
    }

    // =============== Transfer functions for TypeCast expression =============== //

    template <typename TTypeCast> ExceptionKind HandleTypeCast(ConstDomain& state, const TTypeCast* cast)
    {
        auto dest = cast->GetResult();
        auto srcTy = cast->GetSourceTy();
        if (srcTy->IsInteger()) {
            if (auto srcAbsVal = state.CheckAbstractValue(cast->GetSourceValue()); srcAbsVal) {
                return HandleTypecastOfInt(state, cast, srcAbsVal);
            }
        }
        if (srcTy->IsRef() || srcTy->IsClass()) {
            state.Propagate(cast->GetSourceValue(), dest);
            return ExceptionKind::NA;
        }

        state.SetToTopOrTopRef(dest, /* isRef = */ dest->GetType()->IsRef());
        return ExceptionKind::NA;
    }

    template <typename TTypeCast>
    ExceptionKind HandleTypecastOfInt(ConstDomain& state, const TTypeCast* cast, const ConstValue* srcAbsVal)
    {
        switch (cast->GetSourceTy()->GetTypeKind()) {
            case Type::TypeKind::TYPE_INT8: {
                auto constVal = StaticCast<const ConstIntVal*>(srcAbsVal)->GetVal();
                return HandleTypecastOfIntDispatcher(state, cast, static_cast<int8_t>(constVal));
            }
            case Type::TypeKind::TYPE_INT16: {
                auto constVal = StaticCast<const ConstIntVal*>(srcAbsVal)->GetVal();
                return HandleTypecastOfIntDispatcher(state, cast, static_cast<int16_t>(constVal));
            }
            case Type::TypeKind::TYPE_INT32: {
                auto constVal = StaticCast<const ConstIntVal*>(srcAbsVal)->GetVal();
                return HandleTypecastOfIntDispatcher(state, cast, static_cast<int32_t>(constVal));
            }
            case Type::TypeKind::TYPE_INT64: {
                auto constVal = StaticCast<const ConstIntVal*>(srcAbsVal)->GetVal();
                return HandleTypecastOfIntDispatcher(state, cast, static_cast<int64_t>(constVal));
            }
            case Type::TypeKind::TYPE_INT_NATIVE: {
                auto constVal = StaticCast<const ConstIntVal*>(srcAbsVal)->GetVal();
                return HandleTypecastOfIntDispatcher(state, cast, static_cast<ssize_t>(constVal));
            }
            case Type::TypeKind::TYPE_UINT8: {
                auto constVal = StaticCast<const ConstUIntVal*>(srcAbsVal)->GetVal();
                return HandleTypecastOfIntDispatcher(state, cast, static_cast<uint8_t>(constVal));
            }
            case Type::TypeKind::TYPE_UINT16: {
                auto constVal = StaticCast<const ConstUIntVal*>(srcAbsVal)->GetVal();
                return HandleTypecastOfIntDispatcher(state, cast, static_cast<uint16_t>(constVal));
            }
            case Type::TypeKind::TYPE_UINT32: {
                auto constVal = StaticCast<const ConstUIntVal*>(srcAbsVal)->GetVal();
                return HandleTypecastOfIntDispatcher(state, cast, static_cast<uint32_t>(constVal));
            }
            case Type::TypeKind::TYPE_UINT64: {
                auto constVal = StaticCast<const ConstUIntVal*>(srcAbsVal)->GetVal();
                return HandleTypecastOfIntDispatcher(state, cast, static_cast<uint64_t>(constVal));
            }
            case Type::TypeKind::TYPE_UINT_NATIVE: {
                auto constVal = StaticCast<const ConstUIntVal*>(srcAbsVal)->GetVal();
                return HandleTypecastOfIntDispatcher(state, cast, static_cast<size_t>(constVal));
            }
            default:
                // Currently, we haven't suppport constant folding on a cast operation from an
                // integer to a float.
                state.SetToBound(cast->GetResult(), /* isTop = */ true);
                return ExceptionKind::NA;
        }
    }

    template <typename SrcTy, typename TTypeCast>
    ExceptionKind HandleTypecastOfIntDispatcher(ConstDomain& state, const TTypeCast* cast, SrcTy val)
    {
        auto targetTyKind = cast->GetTargetTy()->GetTypeKind();
        switch (targetTyKind) {
            case Type::TypeKind::TYPE_INT8:
                return CastOrRaiseExceptionForInt<SrcTy, int8_t>(state, cast, val);
            case Type::TypeKind::TYPE_INT16:
                return CastOrRaiseExceptionForInt<SrcTy, int16_t>(state, cast, val);
            case Type::TypeKind::TYPE_INT32:
                return CastOrRaiseExceptionForInt<SrcTy, int32_t>(state, cast, val);
            case Type::TypeKind::TYPE_INT64:
                return CastOrRaiseExceptionForInt<SrcTy, int64_t>(state, cast, val);
            case Type::TypeKind::TYPE_INT_NATIVE:
                return CastOrRaiseExceptionForInt<SrcTy, ssize_t>(state, cast, val);
            case Type::TypeKind::TYPE_UINT8:
                return CastOrRaiseExceptionForInt<SrcTy, uint8_t>(state, cast, val);
            case Type::TypeKind::TYPE_UINT16:
                return CastOrRaiseExceptionForInt<SrcTy, uint16_t>(state, cast, val);
            case Type::TypeKind::TYPE_UINT32:
                return CastOrRaiseExceptionForInt<SrcTy, uint32_t>(state, cast, val);
            case Type::TypeKind::TYPE_UINT64:
                return CastOrRaiseExceptionForInt<SrcTy, uint64_t>(state, cast, val);
            case Type::TypeKind::TYPE_UINT_NATIVE:
                return CastOrRaiseExceptionForInt<SrcTy, size_t>(state, cast, val);
            default:
                state.SetToBound(cast->GetResult(), /* isTop = */ true);
                return ExceptionKind::NA;
        }
    }

    template <typename SrcTy, typename TargetTy, typename TTypeCast>
    ExceptionKind CastOrRaiseExceptionForInt(ConstDomain& state, const TTypeCast* cast, SrcTy val)
    {
        auto os = cast->GetOverflowStrategy();
        TargetTy res = 0;
        bool isOverflow = OverflowChecker::IsTypecastOverflowForInt<SrcTy, TargetTy>(val, &res, os);
        if (isOverflow && os == OverflowStrategy::THROWING) {
            RaiseTypeCastOverflowError(cast, val);
            state.SetToBound(cast->GetResult(), /* isTop = */ true);
            return ExceptionKind::FAIL;
        } else {
            using ConstVal = std::conditional_t<std::is_signed_v<TargetTy>, ConstIntVal, ConstUIntVal>;
            state.Update(cast->GetResult(), std::make_unique<ConstVal>(res));
            return ExceptionKind::SUCCESS;
        }
    }

    template <typename T, typename TTypeCast> void RaiseTypeCastOverflowError(const TTypeCast* cast, T srcVal)
    {
        if (this->isStable) {
            auto& loc = cast->GetDebugLocation();
            auto builder = diag->DiagnoseRefactor(DiagKindRefactor::chir_typecast_overflow, ToRange(loc));
            std::string srcValStr = cast->GetSourceTy()->ToString() + "(" + std::to_string(srcVal) + ")";
            builder.AddMainHintArguments(srcValStr, cast->GetTargetTy()->ToString());
            builder.AddNote(GenerateTypeRangePrompt(cast->GetTargetTy()));
        }
    }

    // =============== Helper functions for Array/VArrayOutOfBounds Check =============== //

    const FuncInfo boxInitFunc = FuncInfo("init", "$BOX_RNat5Array", {ANY_TYPE, ANY_TYPE}, NOT_CARE, NOT_CARE);
    const FuncInfo arrayInitFunc = FuncInfo("init", "Array", {NOT_CARE}, NOT_CARE, "std.core");
    const FuncInfo arraySliceFunc = FuncInfo("slice", "Array", {ANY_TYPE, ANY_TYPE, ANY_TYPE}, ANY_TYPE, "std.core");
    const FuncInfo arrayBracketsFunc = FuncInfo("[]", "Array", {NOT_CARE}, ANY_TYPE, "std.core");
    const FuncInfo arrayGetFunc = FuncInfo("get", "Array", {ANY_TYPE, ANY_TYPE}, ANY_TYPE, "std.core");
    const FuncInfo arraySetFunc = FuncInfo("set", "Array", {ANY_TYPE, ANY_TYPE, ANY_TYPE}, ANY_TYPE, "std.core");
    const FuncInfo arraySizeGet = FuncInfo("$sizeget", "Array", {ANY_TYPE}, ANY_TYPE, "std.core");
    const FuncInfo rangeInitFunc = FuncInfo("init", "Range", {NOT_CARE}, NOT_CARE, "std.core");

    static constexpr size_t thisArgIndex = 0;
    static constexpr size_t lenFieldIndex = 2;

    void HandleApplyExpr(ConstDomain& state, const Apply* apply, Value* refObj) override;

    std::optional<Block*> HandleApplyWithExceptionTerminator(
        ConstDomain& state, const ApplyWithException* apply, Value* refObj) override;

    template <typename TApply> ExceptionKind HandleApply(ConstDomain& state, const TApply* apply, Value* /* refObj */)
    {
        auto calleeFunc = DynamicCast<FuncBase*>(apply->GetCallee());
        if (!calleeFunc) {
            return ExceptionKind::NA;
        }

        if (IsExpectedFunction(*calleeFunc, boxInitFunc)) {
            HandleBoxedArrayInit(state, apply);
        } else if (IsExpectedFunction(*calleeFunc, arrayInitFunc)) {
            HandleArrayInit(state, apply);
        } else if (IsExpectedFunction(*calleeFunc, arraySliceFunc)) {
            HandleArraySlice(state, apply);
        } else if (IsExpectedFunction(*calleeFunc, arraySizeGet)) {
            HandleArraySizeGet(state, apply);
        } else if (IsExpectedFunction(*calleeFunc, arrayBracketsFunc) ||
            IsExpectedFunction(*calleeFunc, arrayGetFunc) || IsExpectedFunction(*calleeFunc, arraySetFunc)) {
            return HandleArrayAccess(state, apply);
        } else if (IsExpectedFunction(*calleeFunc, rangeInitFunc)) {
            return HandleRangeInit(state, apply);
        }
        return ExceptionKind::NA;
    }

    template <typename TApply> void HandleBoxedArrayInit(ConstDomain& state, const TApply* apply)
    {
        /**
         * func init(this: Class-_CN7default27$BOX_RNat5ArrayIlEE&, array: Struct-_CNat5ArrayIlE<Int64>)
         */
        auto args = apply->GetArgs();
        CODEC_ASSERT(args.size() == 2U);
        constexpr size_t boxedValueIndex = 0;
        auto toBeInitedArray = state.GetChild(args[thisArgIndex], boxedValueIndex);
        CODEC_ASSERT(toBeInitedArray);
        constexpr size_t initArrayArgIndex = 1;
        auto initArg = args[initArrayArgIndex];
        state.Propagate(initArg, toBeInitedArray);
    }

    /**
     * public struct Array<T> {
     *  let rawptr: RawArray<T>
     *  let start: Int64
     *  let len: Int64
     *  ...
     * }
     * The index of the field `len` is 2.
     */
    template <typename TApply> void HandleArrayInit(ConstDomain& state, const TApply* apply)
    {
        auto args = apply->GetArgs();
        CODEC_ASSERT(args.size() > 0);
        auto lenChild = state.GetChild(args[thisArgIndex], lenFieldIndex);

        /**
         * struct Array<T> {
         *     init() { ... }
         *     init(size: Int64, item!: T) { ... }
         *     init(elements: Collection<T>) { ... }
         *     init(size: Int64, initElement: (Int64) -> T) { ... }
         *     init(data: RawArray<T>, start: Int64, len: Int64) { ... }
         * }
         * note: Each function has an implicit `this` parameter.
         */

        constexpr size_t initWithOneParameter = 1;
        constexpr size_t initWithTwoParameter = 2;
        constexpr size_t initWithThreeParameter = 3;
        constexpr size_t initWithFourParamter = 4;

        switch (args.size()) {
            case initWithOneParameter: {
                // init() { ... }
                return state.Update(lenChild, std::make_unique<ConstIntVal>(0));
            }
            case initWithTwoParameter: {
                // init(elements: Collection<T>) { ... }
                constexpr size_t collectionParamterIndex = 1;
                constexpr size_t boxedValIndex = 0; // class $Box_Array { let $value : Array }
                if (auto boxedVal = state.GetChild(args[collectionParamterIndex], boxedValIndex); boxedVal) {
                    if (auto collectionLen = state.GetChild(boxedVal, lenFieldIndex); collectionLen) {
                        state.Propagate(collectionLen, lenChild);
                    }
                }
                return;
            }
            case initWithThreeParameter: {
                // init(size: Int64, item!: T) { ... }
                // init(size: Int64, initElement: (Int64) -> T) { ... }
                constexpr size_t sizeParamterIndex = 1;
                return state.Propagate(args[sizeParamterIndex], lenChild);
            }
            case initWithFourParamter: {
                // init(data: RawArray<T>, start: Int64, len: Int64) { ... }
                constexpr size_t lenParamterIndex = 3;
                return state.Propagate(args[lenParamterIndex], lenChild);
            }
            default:
                InternalError("Unsupported array init func");
                return;
        }
    }

    template <typename TApply> void HandleArraySlice(ConstDomain& state, const TApply* apply)
    {
        /**
         *  func slice(start: Int64, len: Int64): Array<T>
         */
        auto args = apply->GetArgs();
        CODEC_ASSERT(args.size() == 3U);
        auto lenChild = state.GetChild(apply->GetResult(), lenFieldIndex);
        CODEC_ASSERT(lenChild);
        constexpr size_t lenParameterIndex = 2;
        state.Propagate(args[lenParameterIndex], lenChild);
    }

    template <typename TApply> void HandleArraySizeGet(ConstDomain& state, const TApply* apply)
    {
        /**
         *  $sizeget: (Class-$BOX_RNat5ArrayIlE) -> Int64
         */
        auto args = apply->GetArgs();
        CODEC_ASSERT(args.size() == 1);
        if (auto lenChild = state.GetChild(args[thisArgIndex], lenFieldIndex); lenChild) {
            state.Propagate(lenChild, apply->GetResult());
        }
    }

    template <typename TApply> ExceptionKind HandleArrayAccess(ConstDomain& state, const TApply* apply)
    {
        /**
         * This function handles the following four approaches to accessing an array.
         *  struct Array<T> {
         *     operator func [](index: Int64): T { ... }
         *     operator func [](index: Int64, value!: T): Unit { ... }
         *     func get(index: Int64): Option<T> { ... }
         *     func set(index: Int64, element: T): Unit { ... }
         *  }
         */
        auto args = apply->GetArgs();
        CODEC_ASSERT(args.size() >= 2U);
        auto lenNode = state.GetChild(args[0U], 2U);
        if (!lenNode) {
            return ExceptionKind::NA;
        }
        auto lenVal = state.CheckAbstractValue(lenNode);
        if (!lenVal) {
            return ExceptionKind::NA;
        }
        CODEC_ASSERT(lenVal->GetConstKind() == ConstValue::ConstKind::INT);
        constexpr size_t indexParameterIndex = 1;
        auto indexVal = state.CheckAbstractValue(args[indexParameterIndex]);
        if (!indexVal || indexVal->GetConstKind() != ConstValue::ConstKind::INT) {
            return ExceptionKind::NA;
        }
        auto len = StaticCast<const ConstIntVal*>(lenVal)->GetVal();
        auto index = StaticCast<const ConstIntVal*>(indexVal)->GetVal();
        return RaiseOutOfBoundError(apply, static_cast<uint64_t>(len), index);
    }

    template <typename TApply> ExceptionKind HandleRangeInit(ConstDomain& state, const TApply* apply)
    {
        /**
         * This function handles the following four approaches to accessing an array.
         *  struct Range<T> {
         *     init(start: T, end: T, step: Int64, hasStart: Bool, hasEnd: Bool, isClosed: Bool) { ... }
         *  }
         */
        auto args = apply->GetArgs();
        CODEC_ASSERT(args.size() == 7U);
        constexpr size_t stepParameterIndex = 3;
        auto stepNode = args[stepParameterIndex];
        auto stepVal = state.CheckAbstractValue(stepNode);
        if (!stepVal) {
            return ExceptionKind::NA;
        }
        CODEC_ASSERT(stepVal->GetConstKind() == ConstValue::ConstKind::INT);
        auto step = StaticCast<const ConstIntVal*>(stepVal)->GetVal();
        if (step != 0) {
            return ExceptionKind::SUCCESS;
        }
        if (this->isStable) {
            auto builder =
                diag->DiagnoseRefactor(DiagKindRefactor::chir_step_non_zero_range, ToRange(apply->GetDebugLocation()));
            return ExceptionKind::FAIL;
        }
        return ExceptionKind::NA;
    }

    template <typename TIntrinsic>
    ConstAnalysis::ExceptionKind HandleIntrinsic(ConstDomain& state, const TIntrinsic* intrinsic)
    {
        auto dest = intrinsic->GetResult();
        state.SetToTopOrTopRef(dest, /* isRef = */ dest->GetType()->IsRef());

        if (intrinsic->GetIntrinsicKind() == CHIR::IntrinsicKind::VARRAY_GET) {
            return HandleVArrayGet(state, intrinsic);
        }

        if (intrinsic->GetIntrinsicKind() == CHIR::IntrinsicKind::VARRAY_SET) {
            return HandleVArraySet(state, intrinsic);
        }

        return ExceptionKind::NA;
    }

    template <typename TIntrinsic> ExceptionKind HandleVArrayGet(const ConstDomain& state, const TIntrinsic* intrinsic)
    {
        // Intrinsic/varrayGet(arr, indexes...)
        constexpr size_t varrayOperandIndex = 0;
        if (intrinsic->GetNumOfOperands() != 2U) {
            return ExceptionKind::NA;
        }
        auto arrNode = intrinsic->GetOperand(varrayOperandIndex);
        constexpr size_t indexOperandIndex = 1;
        auto indexNode = intrinsic->GetOperand(indexOperandIndex);
        auto indexVal = state.CheckAbstractValue(indexNode);
        if (!indexVal) {
            return ExceptionKind::NA;
        }
        CODEC_ASSERT(indexVal->GetConstKind() == ConstValue::ConstKind::INT);
        auto index = StaticCast<const ConstIntVal*>(indexVal)->GetVal();
        CODEC_ASSERT(arrNode->GetType()->IsVArray());
        auto len = StaticCast<VArrayType*>(arrNode->GetType())->GetSize();
        return RaiseOutOfBoundError(intrinsic, len, index);
    }

    template <typename TIntrinsic> ExceptionKind HandleVArraySet(const ConstDomain& state, const TIntrinsic* intrinsic)
    {
        // Intrinsic/varraySet(arr, value, index)
        constexpr size_t varrayOperandIndex = 0;
        CODEC_ASSERT(intrinsic->GetNumOfOperands() == 3U);
        auto arrRefNode = intrinsic->GetOperand(varrayOperandIndex);
        constexpr size_t indexOperandIndex = 2;
        auto indexNode = intrinsic->GetOperand(indexOperandIndex);
        auto indexVal = state.CheckAbstractValue(indexNode);
        if (!indexVal) {
            return ExceptionKind::NA;
        }
        CODEC_ASSERT(indexVal->GetConstKind() == ConstValue::ConstKind::INT);
        auto index = StaticCast<const ConstIntVal*>(indexVal)->GetVal();
        CODEC_ASSERT(arrRefNode->GetType()->IsRef());
        auto arrType = StaticCast<RefType*>(arrRefNode->GetType())->GetBaseType();
        CODEC_ASSERT(arrType->IsVArray());
        auto len = StaticCast<VArrayType*>(arrType)->GetSize();
        auto res = RaiseOutOfBoundError(intrinsic, len, index);
        if (res == ExceptionKind::SUCCESS) {
            const_cast<TIntrinsic*>(intrinsic)->template Set<NeedCheckArrayBound>(false);
        }
        return res;
    }

    template <typename TExpr> ExceptionKind RaiseOutOfBoundError(const TExpr* expr, size_t len, int64_t index)
    {
        if (index < 0) {
            if (this->isStable) {
                auto builder =
                    diag->DiagnoseRefactor(DiagKindRefactor::chir_idx_out_of_bounds, ToRange(expr->GetDebugLocation()));
                builder.AddMainHintArguments("array index can not be negative");
            }
            return ExceptionKind::FAIL;
        } else if (static_cast<size_t>(index) >= len) {
            if (this->isStable) {
                auto builder =
                    diag->DiagnoseRefactor(DiagKindRefactor::chir_idx_out_of_bounds, ToRange(expr->GetDebugLocation()));
                auto hint = "array index " + std::to_string(index) + " is past the end of array (which contains " +
                    std::to_string(len) + " elements)";
                builder.AddMainHintArguments(hint);
            }

            return ExceptionKind::FAIL;
        }
        return ExceptionKind::SUCCESS;
    }

    DiagAdapter* diag;
};

} // namespace Codira::CHIR

#endif
