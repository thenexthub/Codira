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
 * This file implements the literal value related class in CHIR.
 */
#include "Codira/CHIR/LiteralValue.h"
#include <iomanip>
#include <iostream>
#include <sstream>

#include "Codira/Basic/StringConvertor.h"

using namespace Codira::CHIR;

LiteralValue::LiteralValue(Type* ty, ConstantValueKind literalKind)
    : Value(ty, "", ValueKind::KIND_LITERAL), literalKind(literalKind)
{
    CODEC_ASSERT(literalKind != ConstantValueKind::KIND_FUNC);
}

bool LiteralValue::IsNullLiteral() const
{
    return literalKind == ConstantValueKind::KIND_NULL;
}

bool LiteralValue::IsBoolLiteral() const
{
    return literalKind == ConstantValueKind::KIND_BOOL;
}

bool LiteralValue::IsRuneLiteral() const
{
    return literalKind == ConstantValueKind::KIND_RUNE;
}

bool LiteralValue::IsStringLiteral() const
{
    return literalKind == ConstantValueKind::KIND_STRING;
}

bool LiteralValue::IsIntLiteral() const
{
    return literalKind == ConstantValueKind::KIND_INT;
}

bool LiteralValue::IsFloatLiteral() const
{
    return literalKind == ConstantValueKind::KIND_FLOAT;
}

bool LiteralValue::IsUnitLiteral() const
{
    return literalKind == ConstantValueKind::KIND_UNIT;
}

ConstantValueKind LiteralValue::GetConstantValueKind() const
{
    return literalKind;
}

BoolLiteral::BoolLiteral(Type* ty, bool val)
    : LiteralValue(ty, ConstantValueKind::KIND_BOOL), val(val)
{
    CODEC_ASSERT(ty->IsBoolean());
}

bool BoolLiteral::GetVal() const
{
    return val;
}

std::string BoolLiteral::ToString() const
{
    std::stringstream ss;
    ss << std::boolalpha << val;
    return ss.str();
}

RuneLiteral::RuneLiteral(Type* ty, char32_t val)
    : LiteralValue(ty, ConstantValueKind::KIND_RUNE), val(val)
{
    CODEC_ASSERT(ty->IsRune());
}

char32_t RuneLiteral::GetVal() const
{
    return val;
}

std::string RuneLiteral::ToString() const
{
    std::stringstream ss;
    ss << '\'' << val << '\'';
    return ss.str();
}

StringLiteral::StringLiteral(Type* ty, std::string val)
    : LiteralValue(ty, ConstantValueKind::KIND_STRING), val(val)
{
    CODEC_ASSERT(ty->IsString());
}

std::string StringLiteral::GetVal() const&
{
    return val;
}
std::string StringLiteral::GetVal() &&
{
    return std::move(val);
}

std::string StringLiteral::ToString() const
{
    std::stringstream ss;
    ss << '"' << StringConvertor::Normalize(val) << '"';
    return ss.str();
}

IntLiteral::IntLiteral(Type* ty, uint64_t val)
    : LiteralValue(ty, ConstantValueKind::KIND_INT), val(val)
{
    CODEC_ASSERT(ty->IsInteger());
}

int64_t IntLiteral::GetSignedVal() const
{
    return static_cast<int64_t>(val);
}

uint64_t IntLiteral::GetUnsignedVal() const
{
    return val;
}

bool IntLiteral::IsSigned() const
{
    return static_cast<IntType*>(ty)->IsSigned();
}

std::string IntLiteral::ToString() const
{
    std::stringstream ss;
    if (IsSigned()) {
        ss << GetSignedVal();
        ss << 'i';
    } else {
        ss << GetUnsignedVal();
        ss << 'u';
    }
    return ss.str();
}

FloatLiteral::FloatLiteral(Type* ty, double val)
    : LiteralValue(ty, ConstantValueKind::KIND_FLOAT), val(val)
{
}

double FloatLiteral::GetVal() const
{
    return val;
}

std::string FloatLiteral::ToString() const
{
    std::stringstream ss;
    ss << std::fixed << val << 'f';
    return ss.str();
}

UnitLiteral::UnitLiteral(Type* ty)
    : LiteralValue(ty, ConstantValueKind::KIND_UNIT)
{
}

std::string UnitLiteral::ToString() const
{
    std::stringstream ss;
    ss << "unit";
    return ss.str();
}

NullLiteral::NullLiteral(Type* ty)
    : LiteralValue(ty, ConstantValueKind::KIND_NULL)
{
}

std::string NullLiteral::ToString() const
{
    std::stringstream ss;
    ss << "null";
    return ss.str();
}
