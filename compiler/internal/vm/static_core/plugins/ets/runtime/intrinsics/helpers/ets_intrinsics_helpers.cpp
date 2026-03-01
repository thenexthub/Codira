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

#include <cstdlib>
#include "dtoa_helper.h"
#include "ets_intrinsics_helpers.h"
#include "ets_coroutine.h"
#include "ets_panda_file_items.h"
#include "ets_stubs.h"
#include "ets_stubs-inl.h"
#include "ets_exceptions.h"
#include "ets_platform_types.h"
#include "include/class.h"
#include "include/class_root.h"
#include "include/mem/panda_string.h"
#include "include/panda_vm.h"
#include "types/ets_field.h"
#include "types/ets_primitives.h"
#include "types/ets_string.h"
#include "types/ets_type.h"
#include "plugins/ets/runtime/types/ets_method.h"

namespace ark::ets::intrinsics::helpers {

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define RETURN_IF_CONVERSION_END(p, end, result) \
    if ((p) == (end)) {                          \
        return (result);                         \
    }

namespace parse_helpers {

template <typename ResultType>
struct ParseResult {
    ResultType value;
    uint8_t *pointerPosition = nullptr;
    bool isSuccess = false;
};

ParseResult<int32_t> ParseExponent(const uint8_t *start, const uint8_t *end, const uint8_t radix, const uint32_t flags)
{
    constexpr int32_t MAX_EXPONENT = INT32_MAX / 2;
    auto p = const_cast<uint8_t *>(start);
    if (radix == 0) {
        return {0, p, false};
    }

    char exponentSign = '+';
    int32_t additionalExponent = 0;
    bool undefinedExponent = false;
    ++p;
    if (p == end) {
        undefinedExponent = true;
    }

    if (!undefinedExponent && (*p == '+' || *p == '-')) {
        exponentSign = static_cast<char>(*p);
        ++p;
        if (p == end) {
            undefinedExponent = true;
        }
    }
    if (!undefinedExponent) {
        uint8_t digit;
        while ((digit = ToDigit(*p)) < radix) {
            if (additionalExponent > MAX_EXPONENT / radix) {
                additionalExponent = MAX_EXPONENT;
            } else {
                additionalExponent = additionalExponent * static_cast<int32_t>(radix) + static_cast<int32_t>(digit);
            }
            if (++p == end) {
                break;
            }
        }
    } else if ((flags & flags::ERROR_IN_EXPONENT_IS_NAN) != 0) {
        return {0, p, false};
    }
    if (exponentSign == '-') {
        return {-additionalExponent, p, true};
    }
    return {additionalExponent, p, true};
}

}  // namespace parse_helpers

static Sign ReadSign(uint16_t ch)
{
    if (ch == '-') {
        return Sign::NEG;
    }
    if (ch == '+') {
        return Sign::POS;
    }
    return Sign::NONE;
}

static bool SkipHexPrefix(const PandaString &str, PandaString::size_type &index)
{
    namespace tags = panda_file_items::class_descriptors;

    if (str[index] != '0') {
        return true;
    }
    auto zeroPos = index;
    ++index;
    if (index >= str.size()) {
        index = zeroPos;
        return true;
    }
    auto ch = str[index];
    if (ch != 'x' && ch != 'X') {
        ThrowEtsException(EtsCoroutine::GetCurrent(), tags::FORMAT_ERROR, "Invalid hex prefix");
        return false;
    }
    ++index;
    return true;
}

template <typename T>
T StringToInt(const PandaString &str, PandaString::size_type startIndex, uint8_t radix)
{
    namespace tags = panda_file_items::class_descriptors;

    if (radix == 0) {
        radix = DECIMAL;
    }
    if (radix < MIN_RADIX || radix > MAX_RADIX) {
        ThrowEtsException(EtsCoroutine::GetCurrent(), tags::ARGUMENT_OUT_OF_RANGE_ERROR, "Invalid radix");
        return 0;
    }
    if (ToDigit(str[startIndex]) >= radix) {
        ThrowEtsException(EtsCoroutine::GetCurrent(), tags::FORMAT_ERROR, "String does not contain a valid number");
        return 0;
    }
    T result = 0;
    for (auto index = startIndex; index < str.size(); ++index) {
        auto digit = static_cast<T>(ToDigit(str[index]));
        if (digit >= radix) {
            break;
        }
        if (result < (std::numeric_limits<T>::min() + digit) / radix) {
            ThrowEtsException(EtsCoroutine::GetCurrent(), tags::FORMAT_ERROR, "Value exceeds integer limits");
            return 0;
        }
        result = result * radix - digit;
    }
    return result;
}

template <typename T>
T StringToInt(EtsString *str, uint8_t radix)
{
    namespace tags = panda_file_items::class_descriptors;

    auto ps = str->TrimLeft()->GetMutf8();
    PandaString::size_type index = 0;
    if (index >= ps.size()) {
        ThrowEtsException(EtsCoroutine::GetCurrent(), tags::FORMAT_ERROR, "String does not contain a valid number");
        return 0;
    }
    auto sign = ReadSign(ps[index]);
    if (sign != Sign::NONE) {
        ++index;
    }
    if (index >= ps.size()) {
        ThrowEtsException(EtsCoroutine::GetCurrent(), tags::FORMAT_ERROR, "String does not contain a valid number");
        return 0;
    }
    if (radix == HEXADECIMAL && !SkipHexPrefix(ps, index)) {
        return 0;
    }
    auto result = StringToInt<T>(ps, index, radix);
    if (sign == Sign::NEG) {
        return result;
    }
    if (result < -std::numeric_limits<T>::max()) {
        ThrowEtsException(EtsCoroutine::GetCurrent(), tags::FORMAT_ERROR, "Value exceeds integer limits");
        return 0;
    }
    return -result;
}

int8_t StringToInt8(EtsString *str, uint8_t radix)
{
    return StringToInt<int8_t>(str, radix);
}

int16_t StringToInt16(EtsString *str, uint8_t radix)
{
    return StringToInt<int16_t>(str, radix);
}

int32_t StringToInt32(EtsString *str, uint8_t radix)
{
    return StringToInt<int32_t>(str, radix);
}

int64_t StringToInt64(EtsString *str, uint8_t radix)
{
    return StringToInt<int64_t>(str, radix);
}

// CC-OFFNXT(G.FUN.01-CPP,huge_cyclomatic_complexity[C++],huge_method[C++]) solid logic
// CC-OFFNXT(huge_cca_cyclomatic_complexity[C++]) solid logic
double StringToDouble(const uint8_t *start, const uint8_t *end, uint8_t radix, uint32_t flags)
{
    // 1. skip space and line terminal
    if (IsEmptyString(start, end)) {
        if ((flags & flags::EMPTY_IS_ZERO) != 0) {
            return 0.0;
        }
        return NAN_VALUE;
    }

    radix = 0;
    auto p = const_cast<uint8_t *>(start);

    GotoNonspace(&p, end);

    // 2. get number sign
    Sign sign = Sign::NONE;
    if (*p == '+') {
        RETURN_IF_CONVERSION_END(++p, end, NAN_VALUE);
        sign = Sign::POS;
    } else if (*p == '-') {
        RETURN_IF_CONVERSION_END(++p, end, NAN_VALUE);
        sign = Sign::NEG;
    }
    bool ignoreTrailing = (flags & flags::IGNORE_TRAILING) != 0;

    // 3. judge Infinity
    static const char INF[] = "Infinity";  // NOLINT(modernize-avoid-c-arrays)
    if (*p == INF[0]) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        for (const char *i = &INF[1]; *i != '\0'; ++i) {
            if (++p == end || *p != *i) {
                return NAN_VALUE;
            }
        }
        ++p;
        if (!ignoreTrailing && GotoNonspace(&p, end)) {
            return NAN_VALUE;
        }
        return sign == Sign::NEG ? -POSITIVE_INFINITY : POSITIVE_INFINITY;
    }

    // 4. get number radix
    bool leadingZero = false;
    bool prefixRadix = false;
    if (*p == '0' && radix == 0) {
        RETURN_IF_CONVERSION_END(++p, end, SignedZero(sign));
        if (*p == 'x' || *p == 'X') {
            if ((flags & flags::ALLOW_HEX) == 0) {
                return ignoreTrailing ? SignedZero(sign) : NAN_VALUE;
            }
            RETURN_IF_CONVERSION_END(++p, end, NAN_VALUE);
            if (sign != Sign::NONE) {
                return NAN_VALUE;
            }
            prefixRadix = true;
            radix = HEXADECIMAL;
        } else if (*p == 'o' || *p == 'O') {
            if ((flags & flags::ALLOW_OCTAL) == 0) {
                return ignoreTrailing ? SignedZero(sign) : NAN_VALUE;
            }
            RETURN_IF_CONVERSION_END(++p, end, NAN_VALUE);
            if (sign != Sign::NONE) {
                return NAN_VALUE;
            }
            prefixRadix = true;
            radix = OCTAL;
        } else if (*p == 'b' || *p == 'B') {
            if ((flags & flags::ALLOW_BINARY) == 0) {
                return ignoreTrailing ? SignedZero(sign) : NAN_VALUE;
            }
            RETURN_IF_CONVERSION_END(++p, end, NAN_VALUE);
            if (sign != Sign::NONE) {
                return NAN_VALUE;
            }
            prefixRadix = true;
            radix = BINARY;
        } else {
            leadingZero = true;
        }
    }

    if (radix == 0) {
        radix = DECIMAL;
    }
    auto pStart = p;
    // 5. skip leading '0'
    while (*p == '0') {
        RETURN_IF_CONVERSION_END(++p, end, SignedZero(sign));
        leadingZero = true;
    }
    // 6. parse to number
    uint64_t intNumber = 0;
    uint64_t numberMax = (UINT64_MAX - (radix - 1)) / radix;
    int digits = 0;
    int exponent = 0;
    do {
        uint8_t c = ToDigit(*p);
        if (c >= radix) {
            if (!prefixRadix || ignoreTrailing || (pStart != p && !GotoNonspace(&p, end))) {
                break;
            }
            // "0b" "0x1.2" "0b1e2" ...
            return NAN_VALUE;
        }
        ++digits;
        if (intNumber < numberMax) {
            intNumber = intNumber * radix + c;
        } else {
            ++exponent;
        }
    } while (++p != end);

    auto number = static_cast<double>(intNumber);
    if (sign == Sign::NEG) {
        if (number == 0) {
            number = -0.0;
        } else {
            number = -number;
        }
    }

    // 7. deal with other radix except DECIMAL
    if (p == end || radix != DECIMAL) {
        if ((digits == 0 && !leadingZero) || (p != end && !ignoreTrailing && GotoNonspace(&p, end))) {
            // no digits there, like "0x", "0xh", or error trailing of "0x3q"
            return NAN_VALUE;
        }
        return number * std::pow(radix, exponent);
    }

    // 8. parse '.'
    if (*p == '.') {
        RETURN_IF_CONVERSION_END(++p, end, (digits > 0) ? (number * std::pow(radix, exponent)) : NAN_VALUE);
        exponent = 0;
        while (ToDigit(*p) < radix) {
            --exponent;
            ++digits;
            if (++p == end) {
                break;
            }
        }
    } else {
        exponent = 0;
    }
    if (digits == 0 && !leadingZero) {
        // no digits there, like ".", "sss", or ".e1"
        return NAN_VALUE;
    }
    auto pEnd = p;

    // 9. parse 'e/E' with '+/-'
    if (radix == DECIMAL && (p != end && (*p == 'e' || *p == 'E'))) {
        auto parseExponentResult = parse_helpers::ParseExponent(p, end, radix, flags);
        if (!parseExponentResult.isSuccess) {
            return NAN_VALUE;
        }
        p = parseExponentResult.pointerPosition;
        exponent += parseExponentResult.value;
    }
    if (!ignoreTrailing && GotoNonspace(&p, end)) {
        return NAN_VALUE;
    }

    // 10. build StringNumericLiteral string
    PandaString buffer;
    if (sign == Sign::NEG) {
        buffer += "-";
    }
    for (uint8_t *i = pStart; i < pEnd; ++i) {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (*i != static_cast<uint8_t>('.')) {
            buffer += *i;
        }
    }

    // 11. convert none-prefix radix string
    return Strtod(buffer.c_str(), exponent, radix);
}

// CC-OFFNXT(G.FUN.01-CPP,huge_cyclomatic_complexity[C++],huge_method[C++]) solid logic
// CC-OFFNXT(huge_cca_cyclomatic_complexity[C++]) solid logic
double StringToDoubleWithRadix(const uint8_t *start, const uint8_t *end, int radix)
{
    auto p = const_cast<uint8_t *>(start);
    // 1. skip space and line terminal
    if (!GotoNonspace(&p, end)) {
        return NAN_VALUE;
    }

    // 2. sign bit
    bool negative = false;
    if (*p == '-') {
        negative = true;
        RETURN_IF_CONVERSION_END(++p, end, NAN_VALUE);
    } else if (*p == '+') {
        RETURN_IF_CONVERSION_END(++p, end, NAN_VALUE);
    }
    // 3. 0x or 0X
    bool stripPrefix = true;
    // 4. If R != 0, then
    //     a. If R < 2 or R > 36, return NaN.
    //     b. If R != 16, let stripPrefix be false.
    if (radix != 0) {
        if (radix < MIN_RADIX || radix > MAX_RADIX) {
            return NAN_VALUE;
        }
        if (radix != HEXADECIMAL) {
            stripPrefix = false;
        }
    } else {
        radix = DECIMAL;
    }
    int size = 0;
    if (stripPrefix) {
        if (*p == '0') {
            size++;
            if (++p != end && (*p == 'x' || *p == 'X')) {
                RETURN_IF_CONVERSION_END(++p, end, NAN_VALUE);
                radix = HEXADECIMAL;
            }
        }
    }

    double result = 0;
    bool isDone = false;
    do {
        double part = 0;
        uint32_t multiplier = 1;
        for (; p != end; ++p) {
            // The maximum value to ensure that uint32_t will not overflow
            const uint32_t maxMultiper = 0xffffffffU / 36U;
            uint32_t m = multiplier * static_cast<uint32_t>(radix);
            if (m > maxMultiper) {
                break;
            }

            int currentBit = static_cast<int>(ToDigit(*p));
            if (currentBit >= radix) {
                isDone = true;
                break;
            }
            size++;
            part = part * radix + currentBit;
            multiplier = m;
        }
        result = result * multiplier + part;
        if (isDone) {
            break;
        }
    } while (p != end);

    if (size == 0) {
        return NAN_VALUE;
    }

    return negative ? -result : result;
}

EtsString *DoubleToExponential(double number, int digit)
{
    PandaStringStream ss;
    if (digit < 0) {
        ss << std::setiosflags(std::ios::scientific) << std::setprecision(MAX_PRECISION) << number;
    } else {
        ss << std::setiosflags(std::ios::scientific) << std::setprecision(digit) << number;
    }
    PandaString result = ss.str();
    size_t found = result.find_last_of('e');
    if (found != PandaString::npos && found < result.size() - 2U && result[found + 2U] == '0') {
        result.erase(found + 2U, 1);  // 2:offset of e
    }
    if (digit < 0) {
        size_t end = found;
        while (--found > 0) {
            if (result[found] != '0') {
                break;
            }
        }
        if (result[found] == '.') {
            found--;
        }
        if (found < end - 1) {
            result.erase(found + 1, end - found - 1);
        }
    }
    return EtsString::CreateFromMUtf8(result.c_str());
}

EtsString *DoubleToFixed(double number, int digit)
{
    PandaStringStream ss;
    if (std::fabs(number) < SCIENTIFIC_NOTATION_THRESHOLD) {
        ss << std::setiosflags(std::ios::fixed) << std::setprecision(digit) << number;
        return EtsString::CreateFromMUtf8(ss.str().c_str());
    }

    ss << std::defaultfloat << std::setprecision(digit) << number;
    auto str = ss.str();
    auto ePos = str.find('e');
    if (ePos == PandaString::npos) {
        return EtsString::CreateFromMUtf8(str.c_str());
    }

    auto dotPos = str.find('.');
    if (dotPos == PandaString::npos || dotPos >= ePos) {
        return EtsString::CreateFromMUtf8(str.c_str());
    }

    bool allZero = true;
    for (size_t i = dotPos + 1; i < ePos; ++i) {
        if (str[i] != '0') {
            allZero = false;
            break;
        }
    }

    const size_t digitsAfterDotToKeep = 2;
    if (allZero) {
        str.erase(dotPos + digitsAfterDotToKeep, ePos - dotPos - digitsAfterDotToKeep);
    }

    return EtsString::CreateFromMUtf8(str.c_str());
}

EtsString *DoubleToPrecision(double number, int digit)
{
    if (number == 0.0) {
        return DoubleToFixed(number, digit - 1);
    }
    PandaStringStream ss;
    double positiveNumber = number > 0 ? number : -number;
    int logDigit = std::floor(log10(positiveNumber));
    int radixDigit = digit - logDigit - 1;
    const int minExponentDigit = -6;
    if (logDigit >= minExponentDigit && logDigit < digit) {
        return DoubleToFixed(number, std::abs(radixDigit));
    }
    return DoubleToExponential(number, digit - 1);
}

// NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
template <typename FpType, std::enable_if_t<std::is_floating_point_v<FpType>, bool> = true>
void GetBase(FpType d, int digits, int *decpt, Span<char> buf)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    auto ret = snprintf_s(buf.begin(), buf.size(), buf.size() - 1, "%.*e", digits - 1, d);
    if (ret == -1) {
        LOG(FATAL, ETS) << "snprintf_s failed";
        UNREACHABLE();
    }
    char *end = buf.begin() + ret;
    ASSERT(*end == 0);
    const size_t positive = (digits > 1) ? 1 : 0;
    char *ePos = buf.begin() + digits + positive;
    ASSERT(*ePos == 'e');
    char *signPos = ePos + 1;
    char *from = signPos + 1;
    // exponent
    if (std::from_chars(from, end, *decpt).ec != std::errc()) {
        UNREACHABLE();
    }
    if (*signPos == '-') {
        *decpt *= -1;
    }
    ++*decpt;
}

constexpr bool USE_GET_BASE_FAST =
#ifdef __cpp_lib_to_chars
    true;
#else
    false;
#endif

template <typename FpType, std::enable_if_t<std::is_floating_point_v<FpType>, bool> = true>
int GetBaseFast([[maybe_unused]] FpType d, int *decpt, Span<char> buf)
{
    ASSERT(d >= 0);
    char *end;
#ifdef __cpp_lib_to_chars
    auto ret = std::to_chars(buf.begin(), buf.end(), d, std::chars_format::scientific);
    if (ret.ec != std::errc()) {
        LOG(FATAL, ETS) << "to_chars failed";
        UNREACHABLE();
    }
    end = ret.ptr;
#else
    UNREACHABLE();
#endif
    *end = '\0';
    // exponent
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto from = std::find(buf.begin(), end, 'e');
    auto digits = from - buf.begin();
    if (digits > 1) {
        digits--;
    }
    ASSERT(from != end);
    if (std::from_chars(from + 2U, end, *decpt).ec != std::errc()) {
        UNREACHABLE();
    }
    if (from[1] == '-') {
        *decpt *= -1;
    }
    ++*decpt;
    return digits;
}

template <typename FpType>
int GetBaseBinarySearch(FpType d, int *decpt, Span<char> buf)
{
    // find the minimum amount of digits
    int minDigits = 1;
    int maxDigits = std::is_same_v<FpType, double> ? DOUBLE_MAX_PRECISION : FLOAT_MAX_PRECISION;
    int digits;

    while (minDigits < maxDigits) {
        digits = (minDigits + maxDigits) / 2_I;
        GetBase(d, digits, decpt, buf);

        bool same = StrToFp<FpType>(buf.begin(), nullptr) == d;
        if (same) {
            // no need to keep the trailing zeros
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            while (digits >= 2_I && buf[digits] == '0') {  // 2 means ignore the integer and point
                digits--;
            }
            maxDigits = digits;
        } else {
            minDigits = digits + 1;
        }
    }
    digits = maxDigits;
    GetBase(d, digits, decpt, buf);
    return digits;
}

template <typename FpType>
[[maybe_unused]] static bool RecheckGetMinimumDigits(FpType d, Span<char> buf)
{
    ASSERT(StrToFp<FpType>(buf.begin(), nullptr) == d);
    std::string str(buf.begin());
    auto pos = str.find('e');
    std::copy(str.begin() + pos, str.end(), str.begin() + pos - 1);
    str[str.size() - 1] = '\0';
    return StrToFp<FpType>(str.data(), nullptr) != d;
}

// result is written starting with buf[1]
template <typename FpType>
int GetMinimumDigits(FpType d, int *decpt, Span<char> buf)
{
    if (std::is_same_v<FpType, double>) {
        DtoaHelper helper(buf.begin() + 1);
        helper.Dtoa(d);
        *decpt = helper.GetPoint();
        return helper.GetDigits();
    }
    int digits;
    if constexpr (USE_GET_BASE_FAST) {
        digits = GetBaseFast(d, decpt, buf);
    } else {
        digits = GetBaseBinarySearch(d, decpt, buf);
    }
    ASSERT(RecheckGetMinimumDigits(d, buf));
    ASSERT(digits == 1 || buf[1] == '.');
    buf[1] = buf[0];
    return digits;
}

template <typename FpType>
char *SmallFpToString(FpType number, bool negative, char *buffer)
{
    using SignedInt = typename ark::helpers::TypeHelperT<sizeof(FpType) * CHAR_BIT, true>;
    if (negative) {
        *(buffer++) = '-';
    }
    *(buffer++) = '0';
    *(buffer++) = '.';
    SignedInt power = TEN;
    SignedInt s = 0;
    int maxDigits = std::is_same_v<FpType, double> ? DOUBLE_MAX_PRECISION : FLOAT_MAX_PRECISION;
    int digits = maxDigits;
    for (int k = 1; k <= maxDigits; ++k) {
        s = static_cast<SignedInt>(number * power);
        if (k == maxDigits || s / static_cast<FpType>(power) == number) {  // s * (10 ** -k)
            digits = k;
            break;
        }
        power *= TEN;
    }
    for (int k = digits - 1; k >= 0; k--) {
        auto digit = s % TEN;
        s /= TEN;
        *(buffer + k) = '0' + digit;
    }
    return buffer + digits;
}

template <typename FpType>
Span<char> FpToStringDecimalRadixMainCase(FpType number, bool negative, Span<char> buffer)
{
    auto bufferStart = buffer.begin() + 2U;
    ASSERT(number > 0);
    int n = 0;
    int k = intrinsics::helpers::GetMinimumDigits(number, &n, buffer.SubSpan(1));
    auto bufferEnd = bufferStart + k;

    if (0 < n && n <= 21_I) {  // NOLINT(readability-magic-numbers)
        if (k <= n) {
            // 6. If k ≤ n ≤ 21, return the String consisting of the code units of the k digits of the decimal
            // representation of s (in order, with no leading zeroes), followed by n−k occurrences of the code unit
            // 0x0030 (DIGIT ZERO).
            std::fill_n(bufferEnd, n - k, '0');
            bufferEnd += n - k;
        } else {
            // 7. If 0 < n ≤ 21, return the String consisting of the code units of the most significant n digits of the
            // decimal representation of s, followed by the code unit 0x002E (FULL STOP), followed by the code units of
            // the remaining k−n digits of the decimal representation of s.
            auto fracStart = bufferStart + n;
            if (memmove_s(fracStart + 1, buffer.end() - (fracStart + 1), fracStart, k - n) != EOK) {
                UNREACHABLE();
            }
            *fracStart = '.';
            bufferEnd++;
        }
    } else if (-6_I < n && n <= 0) {  // NOLINT(readability-magic-numbers)
        // 8. If −6 < n ≤ 0, return the String consisting of the code unit 0x0030 (DIGIT ZERO), followed by the code
        // unit 0x002E (FULL STOP), followed by −n occurrences of the code unit 0x0030 (DIGIT ZERO), followed by the
        // code units of the k digits of the decimal representation of s.
        auto length = -n + 2U;
        auto fracStart = bufferStart + length;
        if (memmove_s(fracStart, buffer.end() - fracStart, bufferStart, k) != EOK) {
            UNREACHABLE();
        }
        std::fill_n(bufferStart, length, '0');
        bufferStart[1] = '.';
        bufferEnd += length;
    } else {
        if (k == 1) {
            // 9. Otherwise, if k = 1, return the String consisting of the code unit of the single digit of s
            ASSERT(bufferEnd == bufferStart + 1);
        } else {
            *(bufferStart - 1) = *bufferStart;
            *(bufferStart--) = '.';
        }
        // followed by code unit 0x0065 (LATIN SMALL LETTER E), followed by the code unit 0x002B (PLUS SIGN) or the code
        // unit 0x002D (HYPHEN-MINUS) according to whether n−1 is positive or negative, followed by the code units of
        // the decimal representation of the integer abs(n−1) (with no leading zeroes).
        *(bufferEnd++) = 'e';
        if (n >= 1) {
            *(bufferEnd++) = '+';
        }
        bufferEnd = std::to_chars(bufferEnd, buffer.end(), n - 1).ptr;
    }
    if (negative) {
        *--bufferStart = '-';
    }
    return {bufferStart, bufferEnd};
}
// NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

template char *SmallFpToString<double>(double number, bool negative, char *buffer);
template char *SmallFpToString<float>(float number, bool negative, char *buffer);

template Span<char> FpToStringDecimalRadixMainCase<double>(double number, bool negative, Span<char> buffer);
template Span<char> FpToStringDecimalRadixMainCase<float>(float number, bool negative, Span<char> buffer);

namespace {
template <typename T>
bool SameValueZeroFloatingPoint(EtsObject *a, EtsObject *b)
{
    auto aVal = EtsBoxPrimitive<T>::FromCoreType(a)->GetValue();
    auto bVal = EtsBoxPrimitive<T>::FromCoreType(b)->GetValue();
    if (std::isnan(aVal) && std::isnan(bVal)) {
        return true;
    }
    return aVal == bVal;
}
}  // namespace

bool SameValueZero(EtsCoroutine *coro, EtsObject *a, EtsObject *b)
{
    if (UNLIKELY(a == b)) {
        return true;
    }

    if (EtsReferenceNullish(coro, a) || EtsReferenceNullish(coro, b)) {
        // a == b is already handled
        return false;
    }

    EtsClass *aKlass = a->GetClass();
    EtsClass *bKlass = b->GetClass();
    auto ptypes = PlatformTypes(coro);
    if (aKlass == ptypes->coreDouble && bKlass == ptypes->coreDouble) {
        return SameValueZeroFloatingPoint<EtsDouble>(a, b);
    }
    if (aKlass == ptypes->coreFloat && bKlass == ptypes->coreFloat) {
        return SameValueZeroFloatingPoint<EtsFloat>(a, b);
    }

    return EtsReferenceEquals(coro, a, b);
}

}  // namespace ark::ets::intrinsics::helpers
