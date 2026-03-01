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

#include <cstdint>
#include <limits>
#include "include/mem/panda_string.h"
#include "intrinsics.h"
#include "plugins/ets/runtime/types/ets_string.h"
#include "plugins/ets/runtime/intrinsics/helpers/ets_intrinsics_helpers.h"
#include "plugins/ets/runtime/intrinsics/helpers/ets_to_string_cache.h"
#include "unicode/locid.h"
#include "unicode/coll.h"
#include "unicode/numberformatter.h"
#include "unicode/unistr.h"
#include "libarkbase/utils/utf.h"
#include "plugins/ets/runtime/intrinsics/helpers/dtoa_helper.h"
#include "plugins/ets/runtime/ets_exceptions.h"

namespace ark::ets::intrinsics {

namespace {

double ParseFloat(EtsString *s, const uint32_t flags)
{
    PandaVector<uint8_t> tree8Buf;
    PandaVector<uint16_t> tree16Buf;

    if (UNLIKELY(s->IsUtf16())) {
        size_t len = s->IsTreeString()
                         ? utf::Utf16ToUtf8Size(s->GetTreeStringDataUtf16(tree16Buf), s->GetUtf16Length()) - 1
                         : utf::Utf16ToUtf8Size(s->GetDataUtf16(), s->GetUtf16Length()) - 1;
        PandaVector<uint8_t> buf(len);
        len = s->IsTreeString() ? utf::ConvertRegionUtf16ToUtf8(tree16Buf.data(), buf.data(), s->GetLength(), len, 0)
                                : utf::ConvertRegionUtf16ToUtf8(s->GetDataUtf16(), buf.data(), s->GetLength(), len, 0);

        Span<uint8_t> str = Span<uint8_t>(buf.data(), len);
        return helpers::StringToDouble(str.begin(), str.end(), 0, flags);
    }

    Span<uint8_t> str = s->IsTreeString() ? Span<uint8_t>(s->GetTreeStringDataMUtf8(tree8Buf), s->GetMUtf8Length() - 1)
                                          : Span<uint8_t>(s->GetDataMUtf8(), s->GetMUtf8Length() - 1);
    return helpers::StringToDouble(str.begin(), str.end(), 0, flags);
}

}  // namespace

EtsString *StdCoreDoubleToString(double number, int radix)
{
    auto *cache = PandaEtsVM::GetCurrent()->GetDoubleToStringCache();
    if (UNLIKELY(radix != helpers::DECIMAL || cache == nullptr)) {
        return helpers::FpToString(number, radix);
    }
    return cache->GetOrCache(EtsCoroutine::GetCurrent(), number);
}

bool IsNegativeNan(double x)
{
    return std::isnan(x) && std::signbit(x);
}

double StdCoreDoubleParseFloat(EtsString *s)
{
    return ParseFloat(s, helpers::flags::IGNORE_TRAILING);
}

double StdCoreDoubleParseInt(EtsString *str, int32_t radix)
{
    auto ps = str->GetMutf8();
    auto start = reinterpret_cast<const uint8_t *>(ps.c_str());
    auto end = start + ps.size();
    return std::trunc(helpers::StringToDoubleWithRadix(start, end, radix));
}

EtsString *StdCoreDoubleToExponential(ObjectHeader *obj, double d)
{
    EtsDouble objValue = EtsBoxPrimitive<EtsDouble>::Unbox(EtsObject::FromCoreType(obj));
    // If x is NaN, return the String "NaN".
    if (std::isnan(objValue)) {
        return EtsString::CreateFromMUtf8("NaN");
    }
    // If x < 0, then
    //    a. Let s be "-".
    //    b. Let x = –x.
    // If x = +infinity, then
    //    a. Return the concatenation of the Strings s and "Infinity".
    if (!std::isfinite(objValue)) {
        if (objValue < 0) {
            return EtsString::CreateFromMUtf8("-Infinity");
        }
        return EtsString::CreateFromMUtf8("Infinity");
    }

    // truncate the arg val
    double digit = std::isnan(d) ? 0 : d;
    digit = (digit >= 0) ? std::floor(digit) : std::ceil(digit);
    // Check range
    if (UNLIKELY(digit > helpers::MAX_FRACTION || digit < helpers::MIN_FRACTION)) {
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::ARGUMENT_OUT_OF_RANGE_ERROR,
                          "toExponential argument must be between 0 and 100");
        return nullptr;
    }

    return helpers::DoubleToExponential(objValue, static_cast<int>(digit));
}

EtsString *StdCoreDoubleToExponentialWithNoDigit(ObjectHeader *obj)
{
    EtsDouble objValue = EtsBoxPrimitive<EtsDouble>::Unbox(EtsObject::FromCoreType(obj));
    // If x is NaN, return the String "NaN".
    if (std::isnan(objValue)) {
        return EtsString::CreateFromMUtf8("NaN");
    }
    // If x < 0, then
    //    a. Let s be "-".
    //    b. Let x = –x.
    // If x = +infinity, then
    //    a. Return the concatenation of the Strings s and "Infinity".
    if (!std::isfinite(objValue)) {
        if (objValue < 0) {
            return EtsString::CreateFromMUtf8("-Infinity");
        }
        return EtsString::CreateFromMUtf8("Infinity");
    }

    if (objValue == 0.0) {
        return EtsString::CreateFromMUtf8("0e+0");
    }
    std::string res;
    if (objValue < 0) {
        res += "-";
        objValue = -objValue;
    }

    char tmpbuf[helpers::BUF_SIZE] = {0};
    helpers::DtoaHelper dtoa {tmpbuf};
    dtoa.Dtoa(objValue);
    int n = dtoa.GetPoint();
    int k = dtoa.GetDigits();

    std::string base = tmpbuf;
    base.erase(1, k - 1);
    if (k != 1) {
        base += std::string(".") + std::string(&tmpbuf[1]);
    }
    base += "e" + (n >= 1 ? std::string("+") : "") + std::to_string(n - 1);
    res += base;
    return EtsString::CreateFromMUtf8(res.c_str());
}

EtsString *StdCoreDoubleToPrecision(ObjectHeader *obj, double d)
{
    EtsDouble objValue = EtsBoxPrimitive<EtsDouble>::Unbox(EtsObject::FromCoreType(obj));
    // If x is NaN, return the String "NaN".
    if (std::isnan(objValue)) {
        return EtsString::CreateFromMUtf8("NaN");
    }
    // If x < 0, then
    //    a. Let s be "-".
    //    b. Let x = –x.
    // If x = +infinity, then
    //    a. Return the concatenation of the Strings s and "Infinity".
    if (!std::isfinite(objValue)) {
        if (objValue < 0) {
            return EtsString::CreateFromMUtf8("-Infinity");
        }
        return EtsString::CreateFromMUtf8("Infinity");
    }

    // truncate the arg val
    double digitAbs = std::isnan(d) ? 0 : d;
    digitAbs = std::abs((digitAbs >= 0) ? std::floor(digitAbs) : std::ceil(digitAbs));
    // Check range
    if (UNLIKELY(digitAbs > helpers::MAX_FRACTION || digitAbs < helpers::MIN_FRACTION + 1)) {
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::ARGUMENT_OUT_OF_RANGE_ERROR,
                          "toPrecision argument must be between 1 and 100");
        return nullptr;
    }

    return helpers::DoubleToPrecision(objValue, static_cast<int>(digitAbs));
}

EtsString *StdCoreDoubleToFixed(ObjectHeader *obj, double d)
{
    // truncate the arg val
    double digitAbs = std::isnan(d) ? 0 : d;
    digitAbs = std::abs((digitAbs >= 0) ? std::floor(digitAbs) : std::ceil(digitAbs));
    // Check range
    if (UNLIKELY(digitAbs > helpers::MAX_FRACTION || digitAbs < helpers::MIN_FRACTION)) {
        ThrowEtsException(EtsCoroutine::GetCurrent(), panda_file_items::class_descriptors::RANGE_ERROR,
                          "toFixed argument must be between 0 and 100");
        return nullptr;
    }

    EtsDouble objValue = EtsBoxPrimitive<EtsDouble>::Unbox(EtsObject::FromCoreType(obj));
    // If x is NaN, return the String "NaN".
    if (std::isnan(objValue)) {
        return EtsString::CreateFromMUtf8("NaN");
    }
    // If x < 0, then
    //    a. Let s be "-".
    //    b. Let x = –x.
    // If x = +infinity, then
    //    a. Return the concatenation of the Strings s and "Infinity".
    if (!std::isfinite(objValue)) {
        if (objValue < 0) {
            return EtsString::CreateFromMUtf8("-Infinity");
        }
        return EtsString::CreateFromMUtf8("Infinity");
    }

    if (std::fabs(objValue) >= helpers::SCIENTIFIC_NOTATION_THRESHOLD) {
        return StdCoreDoubleToString(objValue, static_cast<int>(helpers::DECIMAL));
    }
    return helpers::DoubleToFixed(objValue, static_cast<int>(digitAbs));
}

extern "C" EtsBoolean StdCoreDoubleIsNan(double v)
{
    return ToEtsBoolean(v != v);
}

extern "C" EtsBoolean StdCoreDoubleIsFinite(double v)
{
    static const double POSITIVE_INFINITY = 1.0 / 0.0;
    static const double NEGATIVE_INFINITY = -1.0 / 0.0;

    return ToEtsBoolean(v == v && v != POSITIVE_INFINITY && v != NEGATIVE_INFINITY);
}

extern "C" EtsDouble StdCoreDoubleBitCastFromLong(EtsLong i)
{
    return bit_cast<EtsDouble>(i);
}

extern "C" EtsLong StdCoreDoubleBitCastToLong(EtsDouble f)
{
    return bit_cast<EtsLong>(f);
}

static inline bool IsInteger(double v)
{
    return std::isfinite(v) && (std::fabs(v - std::trunc(v)) == 0.0);
}

extern "C" EtsBoolean StdCoreDoubleIsInteger(double v)
{
    return ToEtsBoolean(IsInteger(v));
}

/*
 * In ETS Double.isSafeInteger returns (Double.isInteger(v) && (abs(v) <= Double.MAX_SAFE_INTEGER)).
 * MAX_SAFE_INTEGER is a max integer value that can be used as a double without losing precision.
 */
extern "C" EtsBoolean StdCoreDoubleIsSafeInteger(double v)
{
    return ToEtsBoolean(IsInteger(v) && (std::fabs(v) <= helpers::MaxSafeInteger<double>()));
}

double StdCoreDoubleNumberFromString(EtsString *s)
{
    uint32_t flags = 0;
    flags |= helpers::flags::ALLOW_BINARY;
    flags |= helpers::flags::ALLOW_OCTAL;
    flags |= helpers::flags::ALLOW_HEX;
    flags |= helpers::flags::EMPTY_IS_ZERO;
    flags |= helpers::flags::ERROR_IN_EXPONENT_IS_NAN;
    return ParseFloat(s, flags);
}

EtsShort StdCoreDoubleToShort(EtsDouble val)
{
    // CC-OFFNXT(G.NAM.03) false positive
    int intVal = CastFloatToInt<EtsDouble, EtsInt>(val);
    return static_cast<int16_t>(intVal);
}

EtsByte StdCoreDoubleToByte(EtsDouble val)
{
    // CC-OFFNXT(G.NAM.03) false positive
    int intVal = CastFloatToInt<EtsDouble, EtsInt>(val);
    return static_cast<int8_t>(intVal);
}

EtsInt StdCoreDoubleToInt(EtsDouble val)
{
    return CastFloatToInt<EtsDouble, EtsInt>(val);
}

EtsLong StdCoreDoubleToLong(EtsDouble val)
{
    return CastFloatToInt<EtsDouble, EtsLong>(val);
}

EtsFloat StdCoreDoubleToFloat(EtsDouble val)
{
    return static_cast<float>(val);
}

}  // namespace ark::ets::intrinsics
