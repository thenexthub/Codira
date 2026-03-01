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
 * This file implements the overflow checking APIs in CHIR.
 */

#include "Codira/CHIR/OverflowChecking.h"
#include "Codira/CHIR/Value.h"

using namespace Codira::CHIR;

bool OverflowChecker::IsIntOverflow(const Type::TypeKind& typeKind, const ExprKind& exprKind, int64_t x, int64_t y,
    const OverflowStrategy& strategy, int64_t* res)
{
    bool isOverflow = true;
    switch (typeKind) {
        case Type::TypeKind::TYPE_INT8: {
            int8_t tmpRes = 0;
            isOverflow =
                IsOverflow<int8_t>(static_cast<int8_t>(x), static_cast<int8_t>(y), exprKind, strategy, &tmpRes);
            *res = tmpRes;
            break;
        }
        case Type::TypeKind::TYPE_INT16: {
            int16_t tmpRes = 0;
            isOverflow =
                IsOverflow<int16_t>(static_cast<int16_t>(x), static_cast<int16_t>(y), exprKind, strategy, &tmpRes);
            *res = tmpRes;
            break;
        }
        case Type::TypeKind::TYPE_INT32: {
            int32_t tmpRes = 0;
            isOverflow =
                IsOverflow<int32_t>(static_cast<int32_t>(x), static_cast<int32_t>(y), exprKind, strategy, &tmpRes);
            *res = tmpRes;
            break;
        }
        case Type::TypeKind::TYPE_INT64: {
            int64_t tmpRes = 0;
            isOverflow =
                IsOverflow<int64_t>(static_cast<int64_t>(x), static_cast<int64_t>(y), exprKind, strategy, &tmpRes);
            *res = tmpRes;
            break;
        }
        case Type::TypeKind::TYPE_INT_NATIVE: {
            ssize_t tmpRes = 0;
            isOverflow =
                IsOverflow<ssize_t>(static_cast<ssize_t>(x), static_cast<ssize_t>(y), exprKind, strategy, &tmpRes);
            *res = tmpRes;
            break;
        }
        default:
            CODEC_ABORT();
    }
    return isOverflow;
}

bool OverflowChecker::IsUIntOverflow(const Type::TypeKind& typeKind, const ExprKind& exprKind, uint64_t x, uint64_t y,
    const OverflowStrategy& strategy, uint64_t* res)
{
    bool isOverflow = true;
    switch (typeKind) {
        case Type::TypeKind::TYPE_UINT8: {
            uint8_t tmpRes = 0;
            isOverflow =
                IsOverflow<uint8_t>(static_cast<uint8_t>(x), static_cast<uint8_t>(y), exprKind, strategy, &tmpRes);
            *res = tmpRes;
            break;
        }
        case Type::TypeKind::TYPE_UINT16: {
            uint16_t tmpRes = 0;
            isOverflow =
                IsOverflow<uint16_t>(static_cast<uint16_t>(x), static_cast<uint16_t>(y), exprKind, strategy, &tmpRes);
            *res = tmpRes;
            break;
        }
        case Type::TypeKind::TYPE_UINT32: {
            uint32_t tmpRes = 0;
            isOverflow =
                IsOverflow<uint32_t>(static_cast<uint32_t>(x), static_cast<uint32_t>(y), exprKind, strategy, &tmpRes);
            *res = tmpRes;
            break;
        }
        case Type::TypeKind::TYPE_UINT64: {
            uint64_t tmpRes = 0;
            isOverflow =
                IsOverflow<uint64_t>(static_cast<uint64_t>(x), static_cast<uint64_t>(y), exprKind, strategy, &tmpRes);
            *res = tmpRes;
            break;
        }
        case Type::TypeKind::TYPE_UINT_NATIVE: {
            size_t tmpRes = 0;
            isOverflow =
                IsOverflow<size_t>(static_cast<size_t>(x), static_cast<size_t>(y), exprKind, strategy, &tmpRes);
            *res = tmpRes;
            break;
        }
        default:
            CODEC_ABORT();
    }
    return isOverflow;
}

template <typename T>
bool OverflowChecker::IsOverflow(T x, T y, ExprKind kind, const OverflowStrategy& strategy, T* res)
{
    *res = 0;
    bool isOverflow = true;
    switch (kind) {
        case ExprKind::ADD: {
            isOverflow = IsOverflowAfterAdd<T>(x, y, strategy, res);
            break;
        }
        case ExprKind::NEG: {
            static_assert(!std::is_same_v<double, T> && !std::is_same_v<float, T>);
            isOverflow = IsOverflowAfterSub<T>(0, y, strategy, res);
            break;
        }
        case ExprKind::SUB: {
            isOverflow = IsOverflowAfterSub<T>(x, y, strategy, res);
            break;
        }
        case ExprKind::MUL: {
            isOverflow = IsOverflowAfterMul<T>(x, y, strategy, res);
            break;
        }
        case ExprKind::DIV: {
            isOverflow = IsOverflowAfterDiv<T>(x, y, strategy, res);
            break;
        }
        case ExprKind::MOD: {
            isOverflow = IsOverflowAfterMod<T>(x, y, res);
            break;
        }
        default:
            CODEC_ABORT();
    }
    return isOverflow;
}

namespace {
bool Binpow(int64_t a, uint64_t b, int64_t* res)
{
    *res = 1;
    if (b == 0) {
        return false;
    }
    bool isOverflow = false;
    while (b > 1) {
        if ((b & 1) == 1) {
            if (__builtin_mul_overflow(*res, a, res) && !isOverflow) {
                isOverflow = true;
            }
        }
        if (__builtin_mul_overflow(a, a, &a) && !isOverflow) {
            isOverflow = true;
        }
        b >>= 1;
    }
    if (__builtin_mul_overflow(*res, a, res) && !isOverflow) {
        isOverflow = true;
    }
    return isOverflow;
}
} // namespace

bool OverflowChecker::IsExpOverflow(int64_t x, uint64_t y, OverflowStrategy strategy, int64_t* res)
{
    bool isOverflow = Binpow(x, y, res);
    if (isOverflow && strategy == OverflowStrategy::SATURATING) {
        uint64_t magicNumber = 2;
        if (x < 0 && (y % magicNumber == 0)) {
            *res = std::numeric_limits<int64_t>::max();
        } else if (x < 0 && (y % magicNumber == 1)) {
            *res = std::numeric_limits<int64_t>::min();
        } else {
            *res = std::numeric_limits<int64_t>::max();
        }
    }
    return isOverflow;
}
