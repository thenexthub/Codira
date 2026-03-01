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

#include <climits>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <random>
#include <cmath>

#include "libarkbase/utils/bit_utils.h"
#include "libarkbase/macros.h"
#include "plugins/ets/runtime/ets_coroutine.h"
#include "plugins/ets/runtime/ets_vm.h"

namespace ark::ets::intrinsics {

namespace {
constexpr int INT_MAX_SIZE = 63;
constexpr double ROUND_BIAS = 0.5;
constexpr uint32_t RIGHT_12 = 12;
constexpr uint32_t LEFT_25 = 25;
constexpr uint32_t RIGHT_27 = 27;
constexpr uint64_t RANDOM_MULTIPLY = 0x2545F4914F6CDD1D;
constexpr uint64_t DOUBLE_MASK = 0x3FF0000000000000;

union Uint64ToDouble {
    double to;
    uint64_t from;
};

int32_t ToInt32(double x)
{
    if (std::isinf(x)) {
        return 0;
    }

    if ((std::isfinite(x)) && (x <= INT_MAX) && (x >= INT_MIN)) {
        return static_cast<int32_t>(x);
    }

    double intPart = 0.0;
    std::modf(x, &intPart);
    double int64Max = std::pow(2, INT_MAX_SIZE);
    intPart = std::fmod(intPart, int64Max);
    return static_cast<int32_t>(static_cast<int64_t>(intPart));
}

uint64_t XorShift64(uint64_t *ptr)
{
    uint64_t val = *ptr;
    val ^= val >> RIGHT_12;
    val ^= val << LEFT_25;
    val ^= val >> RIGHT_27;
    *ptr = val;
    return val * RANDOM_MULTIPLY;
}

uint32_t ToUint32(double x)
{
    return static_cast<uint32_t>(ToInt32(x));
}
}  // namespace

extern "C" double StdMathRandom()
{
    uint64_t threadRandom = XorShift64(EtsCoroutine::GetCurrent()->GetThreadRandomState());
    uint64_t random = (threadRandom >> RIGHT_12) | DOUBLE_MASK;
    // The use of security functions 'memcpy_s' here will have a greater impact on performance
    Uint64ToDouble data {};
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    data.from = random;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-union-access)
    return data.to - 1;
}

extern "C" double StdMathAcos(double val)
{
    return std::acos(val);
}

extern "C" float StdMathAcosFloat(float val)
{
    return std::acos(val);
}

extern "C" double StdMathAcosh(double val)
{
    return std::acosh(val);
}

extern "C" float StdMathAcoshFloat(float val)
{
    return std::acosh(val);
}

extern "C" double StdMathAsin(double val)
{
    return std::asin(val);
}

extern "C" float StdMathAsinFloat(float val)
{
    return std::asin(val);
}

extern "C" double StdMathAsinh(double val)
{
    return std::asinh(val);
}

extern "C" float StdMathAsinhFloat(float val)
{
    return std::asinh(val);
}

extern "C" double StdMathAtan2(double val1, double val2)
{
    return std::atan2(val1, val2);
}

extern "C" float StdMathAtan2Float(float val1, float val2)
{
    return std::atan2(val1, val2);
}

extern "C" double StdMathAtanh(double val)
{
    return std::atanh(val);
}

extern "C" float StdMathAtanhFloat(float val)
{
    return std::atanh(val);
}

extern "C" double StdMathAtan(double val)
{
    return std::atan(val);
}

extern "C" float StdMathAtanFloat(float val)
{
    return std::atan(val);
}

extern "C" double StdMathSinh(double val)
{
    return std::sinh(val);
}

extern "C" float StdMathSinhFloat(float val)
{
    return std::sinh(val);
}

extern "C" double StdMathCosh(double val)
{
    return std::cosh(val);
}

extern "C" float StdMathCoshFloat(float val)
{
    return std::cosh(val);
}

extern "C" double StdMathFloor(double val)
{
    return std::floor(val);
}

extern "C" double StdMathRound(double val)
{
    if (val < 0 && val >= -ROUND_BIAS) {
        return -0.0;
    }
    if (val > 0 && val < ROUND_BIAS) {
        return 0.0;
    }
    double res = std::ceil(val);
    if (res - val > ROUND_BIAS) {
        res -= 1.0;
    }
    return res;
}

extern "C" double StdMathTrunc(double val)
{
    return std::trunc(val);
}

extern "C" double StdMathCbrt(double val)
{
    return std::cbrt(val);
}

extern "C" float StdMathCbrtFloat(float val)
{
    return std::cbrt(val);
}

extern "C" double StdMathTan(double val)
{
    return std::tan(val);
}

extern "C" float StdMathTanFloat(float val)
{
    return std::tan(val);
}

extern "C" double StdMathTanh(double val)
{
    return std::tanh(val);
}

extern "C" float StdMathTanhFloat(float val)
{
    return std::tanh(val);
}

extern "C" double StdMathExp(double val)
{
    return std::exp(val);
}

extern "C" float StdMathExpFloat(float val)
{
    return std::exp(val);
}

extern "C" double StdMathLog10(double val)
{
    return std::log10(val);
}

extern "C" float StdMathLog10Float(float val)
{
    return std::log10(val);
}

extern "C" double StdMathExpm1(double val)
{
    return std::expm1(val);
}

extern "C" float StdMathExpm1Float(float val)
{
    return std::expm1(val);
}

extern "C" double StdMathCeil(double val)
{
    return std::ceil(val);
}

extern "C" int32_t StdMathClz64(int64_t val)
{
    if (val != 0) {
        return Clz(static_cast<uint64_t>(val));
    }
    return std::numeric_limits<uint64_t>::digits;
}

extern "C" int32_t StdMathClz32(int32_t val)
{
    if (val != 0) {
        return Clz(static_cast<uint32_t>(val));
    }
    return std::numeric_limits<uint32_t>::digits;
}

extern "C" double StdMathClz32Double(double val)
{
    auto intValue = ToUint32(val);
    if (intValue != 0) {
        return static_cast<double>(Clz(intValue));
    }
    return std::numeric_limits<uint32_t>::digits;
}

extern "C" double StdMathLog(double val)
{
    return std::log(val);
}

extern "C" float StdMathLogFloat(float val)
{
    return std::log(val);
}

extern "C" double StdMathRem(double val, double val2)
{
    return std::remainder(val, val2);
}

extern "C" double StdMathMod(double val, double val2)
{
    return std::fmod(val, val2);
}

extern "C" bool StdMathSignbit(double val)
{
    return std::signbit(val);
}

extern "C" int StdMathImul(double val, double val2)
{
    if (!std::isfinite(val) || !std::isfinite(val2)) {
        return 0;
    }
    return static_cast<int32_t>(static_cast<int64_t>(val) * static_cast<int64_t>(val2));
}

extern "C" double StdMathFround(double val)
{
    if (std::isnan(val)) {
        return std::numeric_limits<float>::quiet_NaN();
    }

    return static_cast<float>(val);
}

extern "C" double StdMathHypot(double val1, double val2)
{
    return std::hypot(val1, val2);
}

extern "C" float StdMathHypotFloat(float val1, float val2)
{
    return std::hypot(val1, val2);
}

}  // namespace ark::ets::intrinsics
