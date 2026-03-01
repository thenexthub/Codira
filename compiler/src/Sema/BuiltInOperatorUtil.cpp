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
 * This file implements the builtin operator's type helper functions for TypeCheck.
 */

#include "TypeCheckUtil.h"

#define GENERAL_ARITHMIC_INT_TYPE_MAP                                                                                  \
    {TypeKind::TYPE_INT8, TypeKind::TYPE_INT8}, {TypeKind::TYPE_INT16, TypeKind::TYPE_INT16},      \
    {TypeKind::TYPE_INT32, TypeKind::TYPE_INT32},                                                            \
    {TypeKind::TYPE_INT64, TypeKind::TYPE_INT64},                                                            \
    {TypeKind::TYPE_INT_NATIVE, TypeKind::TYPE_INT_NATIVE},                                                  \
    {TypeKind::TYPE_UINT8, TypeKind::TYPE_UINT8},                                                            \
    {TypeKind::TYPE_UINT16, TypeKind::TYPE_UINT16},                                                          \
    {TypeKind::TYPE_UINT32, TypeKind::TYPE_UINT32},                                                          \
    {TypeKind::TYPE_UINT64, TypeKind::TYPE_UINT64},                                                          \
    {TypeKind::TYPE_UINT_NATIVE, TypeKind::TYPE_UINT_NATIVE},                                                \
    {TypeKind::TYPE_IDEAL_INT, TypeKind::TYPE_IDEAL_INT}

#define GENERAL_ARITHMIC_TYPE_MAP                                                                                      \
    GENERAL_ARITHMIC_INT_TYPE_MAP,                                                                                     \
    {TypeKind::TYPE_FLOAT16, TypeKind::TYPE_FLOAT16},                                                        \
    {TypeKind::TYPE_FLOAT32, TypeKind::TYPE_FLOAT32},                                                        \
    {TypeKind::TYPE_FLOAT64, TypeKind::TYPE_FLOAT64},                                                        \
    {TypeKind::TYPE_IDEAL_FLOAT, TypeKind::TYPE_IDEAL_FLOAT}

#define GENERAL_RELATION_NUMERIC_TYPE_MAP                                                                              \
    {TypeKind::TYPE_INT8, TypeKind::TYPE_BOOLEAN},                                                           \
    {TypeKind::TYPE_INT16, TypeKind::TYPE_BOOLEAN},                                                          \
    {TypeKind::TYPE_INT32, TypeKind::TYPE_BOOLEAN},                                                          \
    {TypeKind::TYPE_INT64, TypeKind::TYPE_BOOLEAN},                                                          \
    {TypeKind::TYPE_INT_NATIVE, TypeKind::TYPE_BOOLEAN},                                                     \
    {TypeKind::TYPE_UINT8, TypeKind::TYPE_BOOLEAN},                                                          \
    {TypeKind::TYPE_UINT16, TypeKind::TYPE_BOOLEAN},                                                         \
    {TypeKind::TYPE_UINT32, TypeKind::TYPE_BOOLEAN},                                                         \
    {TypeKind::TYPE_UINT64, TypeKind::TYPE_BOOLEAN},                                                         \
    {TypeKind::TYPE_UINT_NATIVE, TypeKind::TYPE_BOOLEAN},                                                    \
    {TypeKind::TYPE_FLOAT16, TypeKind::TYPE_BOOLEAN},                                                        \
    {TypeKind::TYPE_FLOAT32, TypeKind::TYPE_BOOLEAN},                                                        \
    {TypeKind::TYPE_FLOAT64, TypeKind::TYPE_BOOLEAN},                                                        \
    {TypeKind::TYPE_IDEAL_INT, TypeKind::TYPE_BOOLEAN},                                                      \
    {TypeKind::TYPE_IDEAL_FLOAT, TypeKind::TYPE_BOOLEAN},                                                    \
    {TypeKind::TYPE_RUNE, TypeKind::TYPE_BOOLEAN}

namespace Codira::TypeCheckUtil {
using namespace AST;
namespace {
/**
 * This table encodes the types of the operator and result, for every unary operator.
 */
static const std::map<TokenKind, std::map<TypeKind, TypeKind>> UNARY_EXPR_TYPE_MAP = {
    {TokenKind::SUB, {GENERAL_ARITHMIC_TYPE_MAP}},
    {TokenKind::NOT, {GENERAL_ARITHMIC_INT_TYPE_MAP, {TypeKind::TYPE_BOOLEAN, TypeKind::TYPE_BOOLEAN}}},
    {TokenKind::INCR, {GENERAL_ARITHMIC_INT_TYPE_MAP}},
    {TokenKind::DECR, {GENERAL_ARITHMIC_INT_TYPE_MAP}},
};

/**
 * This table encodes the types of the operator and result, for every binary operator.
 */
static const std::map<TokenKind, std::map<TypeKind, TypeKind>> BINARY_EXPR_TYPE_MAP = {
    {TokenKind::ADD, {GENERAL_ARITHMIC_TYPE_MAP}},
    {TokenKind::SUB, {GENERAL_ARITHMIC_TYPE_MAP}},
    {TokenKind::MUL, {GENERAL_ARITHMIC_TYPE_MAP}},
    {TokenKind::DIV, {GENERAL_ARITHMIC_TYPE_MAP}},
    {TokenKind::MOD, {GENERAL_ARITHMIC_INT_TYPE_MAP}},
    {TokenKind::EXP,
        {{TypeKind::TYPE_INT64, TypeKind::TYPE_INT64},
            {TypeKind::TYPE_FLOAT64, TypeKind::TYPE_FLOAT64},
            {TypeKind::TYPE_IDEAL_INT, TypeKind::TYPE_IDEAL_INT},
            {TypeKind::TYPE_IDEAL_FLOAT, TypeKind::TYPE_IDEAL_FLOAT}}},
    {TokenKind::EQUAL,
        {GENERAL_RELATION_NUMERIC_TYPE_MAP, {TypeKind::TYPE_BOOLEAN, TypeKind::TYPE_BOOLEAN},
            {TypeKind::TYPE_UNIT, TypeKind::TYPE_BOOLEAN}}},
    {TokenKind::NOTEQ,
        {GENERAL_RELATION_NUMERIC_TYPE_MAP, {TypeKind::TYPE_BOOLEAN, TypeKind::TYPE_BOOLEAN},
            {TypeKind::TYPE_UNIT, TypeKind::TYPE_BOOLEAN}}},
    {TokenKind::LT, {GENERAL_RELATION_NUMERIC_TYPE_MAP}},
    {TokenKind::LE, {GENERAL_RELATION_NUMERIC_TYPE_MAP}},
    {TokenKind::GT, {GENERAL_RELATION_NUMERIC_TYPE_MAP}},
    {TokenKind::GE, {GENERAL_RELATION_NUMERIC_TYPE_MAP}},
    {TokenKind::LSHIFT, {GENERAL_ARITHMIC_INT_TYPE_MAP}},
    {TokenKind::RSHIFT, {GENERAL_ARITHMIC_INT_TYPE_MAP}},
    {TokenKind::BITAND, {GENERAL_ARITHMIC_INT_TYPE_MAP}},
    {TokenKind::BITXOR, {GENERAL_ARITHMIC_INT_TYPE_MAP}},
    {TokenKind::BITOR, {GENERAL_ARITHMIC_INT_TYPE_MAP}},
    {TokenKind::AND, {{TypeKind::TYPE_BOOLEAN, TypeKind::TYPE_BOOLEAN}}},
    {TokenKind::OR, {{TypeKind::TYPE_BOOLEAN, TypeKind::TYPE_BOOLEAN}}},
};

const std::map<TypeKind, TypeKind> EMPTY_KIND_MAP;

// '**' only support builtin as Int64 ** UInt64 -> return Int64 or
// Float64 ** Int64/Float64 -> return Float64.
const std::map<TypeKind, std::set<TypeKind>> SEMA_EXP_TYPES = {
    {TypeKind::TYPE_INT64, {TypeKind::TYPE_UINT64}},
    {TypeKind::TYPE_FLOAT64, {TypeKind::TYPE_FLOAT64, TypeKind::TYPE_INT64}},
};
} // namespace

bool IsUnaryOperator(TokenKind op)
{
    return UNARY_EXPR_TYPE_MAP.find(op) != UNARY_EXPR_TYPE_MAP.end();
}

bool IsBinaryOperator(TokenKind op)
{
    return BINARY_EXPR_TYPE_MAP.find(op) != BINARY_EXPR_TYPE_MAP.end();
}

// Find builtin function for unary expressions from operand Ty.
bool IsBuiltinUnaryExpr(TokenKind op, const Ty& ty)
{
    auto candidateMap = UNARY_EXPR_TYPE_MAP.find(op);
    if (candidateMap == UNARY_EXPR_TYPE_MAP.end()) {
        return false;
    }
    std::map<TypeKind, TypeKind> typeCandidate = candidateMap->second;

    if (Ty::IsTyCorrect(&ty)) {
        if (typeCandidate.find(ty.kind) != typeCandidate.end()) {
            return true;
        }
    }
    return false;
}

// Find builtin function for binary expressions from left and right operand Ty.
bool IsBuiltinBinaryExpr(TokenKind op, Ty& leftTy, Ty& rightTy)
{
    if (!Ty::AreTysCorrect(std::set{Ptr(&leftTy), Ptr(&rightTy)})) {
        return false;
    }
    bool ret = false;
    // [] default check.
    if (op == TokenKind::LSQUARE) {
        return leftTy.kind == TypeKind::TYPE_ARRAY && rightTy.kind == TypeKind::TYPE_INT64;
    } else if (op == TokenKind::EXP) {
        auto found = SEMA_EXP_TYPES.find(leftTy.kind);
        return found != SEMA_EXP_TYPES.end() ? found->second.count(rightTy.kind) != 0 : false;
    }

    auto candidateMap = BINARY_EXPR_TYPE_MAP.find(op);
    if (candidateMap == BINARY_EXPR_TYPE_MAP.end()) {
        return false;
    }
    auto typeCandidate = candidateMap->second;

    if (op == TokenKind::LSHIFT || op == TokenKind::RSHIFT) {
        // For these two operators, the type of left expression is okay to not equal to the type of right expression.
        std::map<TypeKind, TypeKind>::const_iterator search1 = typeCandidate.find(leftTy.kind);
        std::map<TypeKind, TypeKind>::const_iterator search2 = typeCandidate.find(rightTy.kind);
        if (search1 != typeCandidate.end() && search2 != typeCandidate.end()) {
            ret = true;
        }
    } else if (&leftTy == &rightTy) {
        if (leftTy.kind == TypeKind::TYPE_UNIT && (op == TokenKind::EQUAL || op == TokenKind::NOTEQ)) {
            ret = true;
        }
        std::map<TypeKind, TypeKind>::const_iterator search = typeCandidate.find(leftTy.kind);
        if (search != typeCandidate.end()) {
            ret = true;
        }
    }

    return ret;
}

TypeKind GetBuiltinBinaryExprReturnKind(TokenKind op, TypeKind leftOpType)
{
    if (op == TokenKind::EXP) {
        // Return kind of exp is same as leftTy kind.
        auto found = SEMA_EXP_TYPES.find(leftOpType);
        return found != SEMA_EXP_TYPES.end() ? leftOpType : TypeKind::TYPE_INVALID;
    }

    auto candidateMap = BINARY_EXPR_TYPE_MAP.find(op);
    if (candidateMap != BINARY_EXPR_TYPE_MAP.end()) {
        auto found = candidateMap->second.find(leftOpType);
        if (found != candidateMap->second.end()) {
            return found->second;
        }
    }
    return TypeKind::TYPE_INVALID;
}

TypeKind GetBuiltinUnaryOpReturnKind(TokenKind op, TypeKind opType)
{
    auto found = UNARY_EXPR_TYPE_MAP.find(op);
    if (found == UNARY_EXPR_TYPE_MAP.end()) {
        return TypeKind::TYPE_INVALID;
    }
    auto findType = found->second.find(opType);
    return findType == found->second.end() ? TypeKind::TYPE_INVALID : findType->second;
}

const std::map<TypeKind, TypeKind>& GetBinaryOpTypeCandidates(TokenKind op)
{
    auto found = BINARY_EXPR_TYPE_MAP.find(op);
    return found == BINARY_EXPR_TYPE_MAP.end() ? EMPTY_KIND_MAP : found->second;
}

const std::map<TypeKind, TypeKind>& GetUnaryOpTypeCandidates(TokenKind op)
{
    auto found = UNARY_EXPR_TYPE_MAP.find(op);
    return found == UNARY_EXPR_TYPE_MAP.end() ? EMPTY_KIND_MAP : found->second;
}
} // namespace Codira::TypeCheckUtil
