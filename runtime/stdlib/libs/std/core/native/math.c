/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2025. All rights reserved.
 * This source file is part of the Codira project, licensed under Apache-2.0
 * with Runtime Library Exception.
 *
 * See https://cangjie-lang.cn/pages/LICENSE for license information.
 */

#include <math.h>
#include <stdint.h>
#include <stdbool.h>

extern double CODE_CORE_FastPowerDoubleInt64(double x, int64_t y)
{
    if (y == 0) {
        return 1.0;
    }
    if (isnan(x)) {
        return 0.0 / 0.0; // nan
    }
    bool isPositiveZero = fpclassify(x) == FP_ZERO && signbit(x) == 0;
    bool isNegativeZero = fpclassify(x) == FP_ZERO && signbit(x) != 0;
    // x == +0.0 || (x == -0.0 && y is even)
    if (isPositiveZero || (isNegativeZero && ((uint64_t)y & 1) == 0)) {
        return y < 0 ? 1 / 0.0 : 0.0;
    }
    // x == -0.0 && y is odd
    if (isNegativeZero && ((uint64_t)y & 1) != 0) { // -0.0
        return y < 0 ? -1 / 0.0 : -0.0;
    }
    bool isPositiveInf = isinf(x) && signbit(x) == 0;
    bool isNegativeInf = isinf(x) && signbit(x) != 0;
    // x == +Inf || (x == -Inf && y is even)
    if (isPositiveInf || (isNegativeInf && ((uint64_t)y & 1) == 0)) { // +inf
        return (y > 0) ? 1 / 0.0 : 0.0;
    }
    // x == -Inf && y is odd
    if (isNegativeInf && ((uint64_t)y & 1) != 0) { // -inf
        return (y > 0) ? -1 / 0.0 : -0.0;
    }
    double temp = CODE_CORE_FastPowerDoubleInt64(x, y / 2);
    temp *= temp;
    if (((uint64_t)y & 1) == 0) {
        return temp;
    } else {
        return y > 0 ? x * temp : temp / x;
    }
}

extern float CODE_CORE_FastPowerFloatInt32(float x, int32_t y)
{
    if (y == 0) {
        return 1.0;
    }
    if (isnan(x)) {
        return 0.0 / 0.0; // nan
    }
    bool isPositiveZero = fpclassify(x) == FP_ZERO && signbit(x) == 0;
    bool isNegativeZero = fpclassify(x) == FP_ZERO && signbit(x) != 0;
    // x == +0.0 || (x == -0.0 && y is even)
    if (isPositiveZero || (isNegativeZero && ((uint32_t)y & 1) == 0)) {
        return y < 0 ? 1 / 0.0 : 0.0;
    }
    // x == -0.0 && y is odd
    if (isNegativeZero && ((uint32_t)y & 1) != 0) { // -0.0
        return y < 0 ? -1 / 0.0 : -0.0;
    }
    bool isPositiveInf = isinf(x) && signbit(x) == 0;
    bool isNegativeInf = isinf(x) && signbit(x) != 0;
    // x == +Inf || (x == -Inf && y is even)
    if (isPositiveInf || (isNegativeInf && ((uint32_t)y & 1) == 0)) { // +inf
        return (y > 0) ? 1 / 0.0 : 0.0;
    }
    // x == -Inf && y is odd
    if (isNegativeInf && ((uint32_t)y & 1) != 0) { // -inf
        return (y > 0) ? -1 / 0.0 : -0.0;
    }
    float temp = CODE_CORE_FastPowerFloatInt32(x, y / 2);
    if (((uint32_t)y & 1) == 0) {
        return temp * temp;
    } else {
        return y > 0 ? x * temp * temp : (temp * temp) / x;
    }
}

extern double CODE_CORE_CPow(double x, double y)
{
    if (isfinite(x) && x < 0.0 && isfinite(y) && (ceil(y) > floor(y) || ceil(y) < floor(y))) {
        return 0.0 / 0.0; // nan
    }
    return pow(x, y);
}

extern float CODE_CORE_CPowf(float x, float y)
{
    if (isfinite(x) && x < 0.0 && isfinite(y) && (ceil(y) > floor(y) || ceil(y) < floor(y))) {
        return 0.0 / 0.0; // nan
    }
    return powf(x, y);
}

extern int64_t CODE_CORE_Float64ToHash(double x)
{
    return *(int64_t*)&x;
}

extern int64_t CODE_CORE_Float32ToHash(float x)
{
    return (int64_t)(*(int32_t*)&x);
}
