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

#include "StructuralRuleGEXP02.h"
#include <cmath>
#include <limits>

using namespace Codira;
using namespace Codira::AST;
using namespace Meta;
using namespace CodeCheck;

/*
 * This rule mainly checks the representation error of floating-point numbers now, that is, floating-point numbers
 * cannot accurately represent the numbers represented by string. For example, if '3.1415926' uses fp16 to represent the
 * sum operation, the result must contain errors.
 */

namespace {
// Define a floating-point type of fp16
struct Float16 {
    uint16_t value;
    explicit Float16(float f)
    {
        uint32_t i = *reinterpret_cast<uint32_t*>(&f);
        uint32_t sign = (i >> 16) & 0x8000;
        int exponent = ((i >> 23) & 0xff) - 112;
        uint32_t mantissa = i & 0x7fffff;
        if (exponent <= 0) {
            value = sign;
            return;
        }
        // If the exponent is greater than 30, the result is set to positive or negative infinity
        if (exponent > 30) {
            value = sign | 0x7c00;
            return;
        }
        // Move the exponent left by 10 bits
        exponent <<= 10;
        // Move the mantissa right by 13 digits
        mantissa >>= 13;
        value = sign | exponent | mantissa;
    }

    float ToFloat() const
    {
        uint32_t sign = (value >> 15) & 1;
        int exponent = (value >> 10) & 0x1f;
        uint32_t mantissa = value & 0x3ff;
        if (exponent == 0) {
            return sign ? -0.0f : 0.0f;
        }
        // If the exponent is 31, handle special cases (NaN or infinity)
        if (exponent == 31) {
            return mantissa ? (std::numeric_limits<float>::quiet_NaN)()
                            : (sign ? -std::numeric_limits<float>::infinity() : std::numeric_limits<float>::infinity());
        }
        // Converts the exponent value of a half-precision floating-point number to the offset value of a
        // single-precision floating-point number, that is, increases the exponent by 112.
        exponent += 112;
        uint32_t i = (sign << 31) | (exponent << 23) | (mantissa << 13);
        float f = *reinterpret_cast<float*>(&i);
        return f;
    }
};

template <typename T> static bool IsWithinFloatPointRange(const std::string& str)
{
    std::istringstream iss(str);
    T value;
    iss >> value;

    if (iss.fail() || !iss.eof()) {
        return false;
    }
    unsigned int maxDigits = std::numeric_limits<T>::digits10;
    if (str.find('.') != std::string::npos) {
        unsigned int lengthBeforeDecimal = str.find('.');
        auto lengthAfterDecimal = str.length() - lengthBeforeDecimal - 1;
        auto totalLength = lengthBeforeDecimal + lengthAfterDecimal;
        if (totalLength > maxDigits + 1) {
            return false;
        }
    } else {
        if (str.length() > maxDigits) {
            return false;
        }
    }
    if (value < std::numeric_limits<T>::lowest() || value > std::numeric_limits<T>::max()) {
        return false;
    }

    std::ostringstream oss;
    oss << std::setprecision(std::numeric_limits<T>::max_digits10) << value;

#ifndef CODIRA_ENABLE_GCOV
    try {
#endif
        T closestValue = std::stod(oss.str());
        T difference = std::abs(value - closestValue);
        return difference <= std::numeric_limits<T>::epsilon();
#ifndef CODIRA_ENABLE_GCOV
    } catch (...) {
        return false;
    }
#endif
}

static bool IsAccuateFloatPointNum(const std::string& str, const AST::TypeKind& floatKind)
{
    size_t pos;
    switch (floatKind) {
        case TypeKind::TYPE_FLOAT16: {
            if (!IsWithinFloatPointRange<float>(str)) {
                return false;
            }
            std::istringstream iss(str);
            float value;
            iss >> value;
            Float16 fp16Value(value);
            return (fp16Value.ToFloat() == value) || (std::isnan(value) && std::isnan(fp16Value.ToFloat()));
        }
        case TypeKind::TYPE_FLOAT32: {
            return IsWithinFloatPointRange<float>(str);
        }
        case TypeKind::TYPE_FLOAT64: {
            return IsWithinFloatPointRange<double>(str);
        }
        default:
            break;
    }
    return true;
}
}

void StructuralRuleGEXP02::CheckLitConstExpr(const Codira::AST::LitConstExpr& litConstExpr)
{
    auto str = litConstExpr.stringValue;
    // Skip null pointer
    if (!litConstExpr.ty) {
        return;
    }
    auto kind = litConstExpr.ty->kind;
    if (!IsAccuateFloatPointNum(str, kind)) {
        Diagnose(litConstExpr.begin, litConstExpr.end, CodeCheckDiagKind::G_EXP_02_not_accurate_float_operation_01,
            litConstExpr.stringValue);
    }
}

void StructuralRuleGEXP02::CheckBinaryExpr(const Codira::AST::BinaryExpr& binaryExpr)
{
    if (binaryExpr.op != TokenKind::EQUAL && binaryExpr.op != TokenKind::NOTEQ) {
        return;
    }
    if (!binaryExpr.leftExpr || !binaryExpr.rightExpr) {
        return;
    }
    if (binaryExpr.leftExpr->ty->IsFloating() && binaryExpr.rightExpr->ty->IsFloating()) {
        Diagnose(binaryExpr.begin, binaryExpr.end, CodeCheckDiagKind::G_EXP_02_not_accurate_float_operation_02,
            TOKENS[static_cast<int>(binaryExpr.op)]);
    }
}
void StructuralRuleGEXP02::FindFloatASTNode(Ptr<Codira::AST::Node> node)
{
    if (node == nullptr) {
        return;
    }
    Walker walker(node, [this](Ptr<Node> node) -> VisitAction {
        return match(*node)(
            [this](const LitConstExpr& litConstExpr) {
                CheckLitConstExpr(litConstExpr);
                return VisitAction::WALK_CHILDREN;
            },
            [this](const BinaryExpr& binaryExpr) {
                CheckBinaryExpr(binaryExpr);
                return VisitAction::WALK_CHILDREN;
            },
            []() { return VisitAction::WALK_CHILDREN; });
    });
    walker.Walk();
}

void StructuralRuleGEXP02::MatchPattern(ASTContext& ctx, Ptr<Node> node)
{
    (void)ctx;
    FindFloatASTNode(node);
}
