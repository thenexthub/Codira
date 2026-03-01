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

#include "plugins/ets/stdlib/native/core/stdlib_ani_helpers.h"
#include "IntlCommon.h"
#include "IntlState.h"
#include "IntlNumberFormatters.h"
#include "IntlNumberFormat.h"
#include <unicode/formattedvalue.h>
#include <unicode/numsys.h>
#include <algorithm>
#include <optional>
#include <cstring>
#include <cstdlib>
#include <array>
#include <utility>

namespace ark::ets::stdlib::intl {

namespace refs {
ani_class g_resolvedOptionsClass;

ani_field g_locale_field;
ani_field g_compactDisplay_field;
ani_field g_currencySign_field;
ani_field g_currency_field;
ani_field g_currencyDisplay_field;
ani_field g_minFractionDigits_field;
ani_field g_maxFractionDigits_field;
ani_field g_minSignificantDigits_field;
ani_field g_maxSignificantDigits_field;
ani_field g_minIntegerDigits_field;
ani_field g_notation_field;
ani_field g_numberingSystem_field;
ani_field g_signDisplay_field;
ani_field g_style_field;
ani_field g_unit_field;
ani_field g_unitDisplay_field;
ani_field g_useGrouping_field;

ani_field g_numberFormat_options_field;
}  // namespace refs

constexpr std::string_view INTEGER_FIELD = "integer";
constexpr std::string_view FRACTION_FIELD = "fraction";
constexpr std::string_view DECIMAL_SEPARATOR_FIELD = "decimal";
constexpr std::string_view EXPONENT_SYMBOL_FIELD = "exponentSeparator";
constexpr std::string_view GROUPING_SEPARATOR_FIELD = "group";
constexpr std::string_view CURRENCY_FIELD = "currency";
constexpr std::string_view PERCENT_FIELD = "percentSign";
constexpr std::string_view PERMILL_FIELD = "perMille";
constexpr std::string_view PLUS_SIGN = "plusSign";
constexpr std::string_view MINUS_SIGN = "minusSign";
constexpr std::string_view EXPONENT_MINUS_SIGN = "exponentMinusSign";
constexpr std::string_view EXPONENT_INTEGER = "exponentInteger";
constexpr std::string_view COMPACT_FIELD = "compact";
constexpr std::string_view MEASURE_UNIT_FIELD = "unit";
constexpr std::string_view APPROXIMATELY_SIGN_FIELD = "approximatelySign";

/// Fallback
constexpr std::string_view LITERAL_FIELD = "literal";

/// Sources
constexpr std::string_view START_RANGE_SOURCE = "startRange";
constexpr std::string_view END_RANGE_SOURCE = "endRange";
constexpr std::string_view SHARED_SOURCE = "shared";

constexpr int32_t INTL_NUMBER_RANGE_START = 0;
constexpr int32_t INTL_NUMBER_RANGE_END = 1;

class NumberFormatPart {
public:
    std::string type;
    std::string value;
    std::string source;
};

/*
 * FieldSpan class, used in formatToParts/formatRangeToParts
 *
 * Example:
 * ------------------------------------------------
 * Consider the formatted string: "-5678.9"
    0: "sign",     "-"
    1: "integer",  "5"
    2: "group",    ","
    3: "integer",  "678"
    4: "decimal",  "."
    5: "fraction", "9"
 */
class FieldSpan {
public:
    FieldSpan(int32_t fieldType, int32_t startIndex, int32_t endIndex)
        : fieldType_(fieldType), startIndex_(startIndex), endIndex_(endIndex)
    {
    }

    // Accessors
    int32_t GetFieldType() const
    {
        return fieldType_;
    }
    int32_t GetStartIndex() const
    {
        return startIndex_;
    }
    int32_t GetEndIndex() const
    {
        return endIndex_;
    }
    std::tuple<int32_t, int32_t> GetStartEndPositionsTuple() const
    {
        return std::make_tuple(startIndex_, endIndex_);
    }

private:
    /// Indicates the type of formatting (e.g., integer, fraction, etc.).
    int32_t fieldType_;
    /// The starting index (inclusive) of the span in the formatted string.
    int32_t startIndex_;
    /// The ending index (exclusive) of the span.
    int32_t endIndex_;
};

class SpanFolder {
public:
    explicit SpanFolder(std::vector<FieldSpan> spans) : spans_ {std::move(spans)}, currentActiveSpan_ {spans_.at(0)}
    {
        activeSpanIndexStack_.push_back(0);
    }
    std::vector<FieldSpan> Fold()
    {
        if (spans_.size() <= 1) {
            return spans_;
        }
        std::sort(spans_.begin(), spans_.end(), [](const auto &a, const auto &b) {
            const auto [aStart, aEnd] = a.GetStartEndPositionsTuple();
            const auto [bStart, bEnd] = b.GetStartEndPositionsTuple();
            if (aStart < bStart) {
                return true;
            }
            if (aStart > bStart) {
                return false;
            }
            if (aEnd < bEnd) {
                return false;
            }
            if (aEnd > bEnd) {
                return true;
            }
            return a.GetFieldType() < b.GetFieldType();
        });
        std::vector<FieldSpan> nonOverlappingSegments;
        // Stack to track the indexes of spans that are currently active/overlapping
        size_t nextSpanIndex = 1;
        // Total length to process (end of the first span, which should cover the entire string)
        const int32_t totalStringLength = spans_.at(0).GetEndIndex();
        while (currentPosition_ < totalStringLength) {
            // Determine where the next segment starts
            const int32_t nextSegmentStartPosition =
                (nextSpanIndex < spans_.size()) ? spans_.at(nextSpanIndex).GetStartIndex() : totalStringLength;
            if (currentPosition_ < nextSegmentStartPosition) {
                if (ProcessSpansUntilNextSegment(nextSegmentStartPosition, nonOverlappingSegments)) {
                    return nonOverlappingSegments;
                }
            }
            if (nextSpanIndex < spans_.size()) {
                activeSpanIndexStack_.push_back(nextSpanIndex++);
                currentActiveSpan_ = spans_.at(activeSpanIndexStack_.back());
            }
        }
        return nonOverlappingSegments;
    }

private:
    /// The spans to process
    std::vector<FieldSpan> spans_;
    /// The current active span
    FieldSpan currentActiveSpan_;
    /// The stack of active span indexes
    std::vector<size_t> activeSpanIndexStack_;
    /// The current position
    int32_t currentPosition_ {0};

    // Adds a segment to the result vector
    static void AddSegment(std::vector<FieldSpan> &segments, int32_t fieldType, int32_t start, int32_t end)
    {
        segments.emplace_back(fieldType, start, end);
    }

    bool ProcessSpansUntilNextSegment(int32_t nextSegmentStartPosition, std::vector<FieldSpan> &nonOverlappingSegments)
    {
        while (currentActiveSpan_.GetEndIndex() < nextSegmentStartPosition) {
            if (currentPosition_ < currentActiveSpan_.GetEndIndex()) {
                AddSegment(nonOverlappingSegments, currentActiveSpan_.GetFieldType(), currentPosition_,
                           currentActiveSpan_.GetEndIndex());

                currentPosition_ = currentActiveSpan_.GetEndIndex();
            }
            activeSpanIndexStack_.pop_back();
            if (activeSpanIndexStack_.empty()) {
                return true;
            }
            currentActiveSpan_ = spans_.at(activeSpanIndexStack_.back());
        }
        if (currentPosition_ < nextSegmentStartPosition) {
            AddSegment(nonOverlappingSegments, currentActiveSpan_.GetFieldType(), currentPosition_,
                       nextSegmentStartPosition);
            currentPosition_ = nextSegmentStartPosition;
        }
        return false;
    }
};

std::string_view GetFieldTypeString(const FieldSpan &field, const icu::UnicodeString &text)
{
    switch (static_cast<UNumberFormatFields>(field.GetFieldType())) {
        case UNUM_INTEGER_FIELD:
            return INTEGER_FIELD;
        case UNUM_FRACTION_FIELD:
            return FRACTION_FIELD;
        case UNUM_DECIMAL_SEPARATOR_FIELD:
            return DECIMAL_SEPARATOR_FIELD;
        case UNUM_EXPONENT_SYMBOL_FIELD:
            return EXPONENT_SYMBOL_FIELD;
        case UNUM_EXPONENT_SIGN_FIELD:
            return EXPONENT_MINUS_SIGN;
        case UNUM_EXPONENT_FIELD:
            return EXPONENT_INTEGER;
        case UNUM_GROUPING_SEPARATOR_FIELD:
            return GROUPING_SEPARATOR_FIELD;
        case UNUM_CURRENCY_FIELD:
            return CURRENCY_FIELD;
        case UNUM_PERCENT_FIELD:
            return PERCENT_FIELD;
        case UNUM_SIGN_FIELD:
            return (text.charAt(field.GetStartIndex()) == '+') ? PLUS_SIGN : MINUS_SIGN;
        case UNUM_PERMILL_FIELD:
            return PERMILL_FIELD;
        case UNUM_COMPACT_FIELD:
            return COMPACT_FIELD;
        case UNUM_MEASURE_UNIT_FIELD:
            return MEASURE_UNIT_FIELD;
        case UNUM_APPROXIMATELY_SIGN_FIELD:
            return APPROXIMATELY_SIGN_FIELD;
        default:
            UNREACHABLE();
    }
}

static ani_double NormalizeIfNaN(ani_double value)
{
    return std::isnan(value) ? std::numeric_limits<ani_double>::quiet_NaN() : value;
}

ani_string IcuFormatDouble(ani_env *env, ani_object self, ani_double value)
{
    ParsedOptions options;
    ParseOptions(env, self, options);

    ani_status err;
    LocNumFmt &formatter = g_intlState->fmtsCache.NumFmtsCacheInvalidation(env, options, err);
    if (err == ANI_OK) {
        UErrorCode status = U_ZERO_ERROR;
        auto fmtNumber = formatter.formatDouble(NormalizeIfNaN(value), status);
        if (UNLIKELY(U_FAILURE(status))) {
            std::string message = "Icu formatter format failed " + std::string(u_errorName(status));
            ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, message.c_str(), CTOR_SIGNATURE_STR);
            return nullptr;
        }
        icu::UnicodeString ustr = fmtNumber.toString(status);
        return UnicodeToAniStr(env, ustr);
    }
    return nullptr;
}

ani_string IcuFormatLong(ani_env *env, ani_object self, ani_long value)
{
    ParsedOptions options;
    ParseOptions(env, self, options);

    ani_status err;
    LocNumFmt &formatter = g_intlState->fmtsCache.NumFmtsCacheInvalidation(env, options, err);
    if (err != ANI_OK) {
        return nullptr;
    }
    UErrorCode status = U_ZERO_ERROR;
    auto fmtNumber = formatter.formatInt(value, status);
    if (UNLIKELY(U_FAILURE(status))) {
        std::string message = "Icu formatter format failed " + std::string(u_errorName(status));
        ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, message.c_str(), CTOR_SIGNATURE_STR);
        return nullptr;
    }
    icu::UnicodeString ustr = fmtNumber.toString(status);
    return UnicodeToAniStr(env, ustr);
}

ani_string IcuFormatDecStr(ani_env *env, ani_object self, ani_string value)
{
    ParsedOptions options;
    ParseOptions(env, self, options);

    ani_status err;
    LocNumFmt &formatter = g_intlState->fmtsCache.NumFmtsCacheInvalidation(env, options, err);
    if (err == ANI_OK) {
        const std::string &valueString = ConvertFromAniString(env, value);
        const icu::StringPiece sp {valueString.data(), static_cast<int32_t>(valueString.size())};
        UErrorCode status = U_ZERO_ERROR;
        const icu::number::FormattedNumber &fmtNumber = formatter.formatDecimal(sp, status);
        if (UNLIKELY(U_FAILURE(status))) {
            std::string message = "Icu formatter format failed " + std::string(u_errorName(status));
            ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, message.c_str(), CTOR_SIGNATURE_STR);
            return nullptr;
        }
        icu::UnicodeString ustr = fmtNumber.toString(status);
        return UnicodeToAniStr(env, ustr);
    }
    return nullptr;
}

ani_string IcuFormatRange(ani_env *env, ani_object self, const icu::Formattable &startFrmtbl,
                          const icu::Formattable &endFrmtbl)
{
    ParsedOptions options;
    ParseOptions(env, self, options);

    ani_status err;
    LocNumRangeFmt &formatter = g_intlState->fmtsCache.NumRangeFmtsCacheInvalidation(env, options, err);
    if (err == ANI_OK) {
        UErrorCode status = U_ZERO_ERROR;
        const icu::number::FormattedNumberRange &fmtRangeNumber =
            formatter.formatFormattableRange(startFrmtbl, endFrmtbl, status);
        if (UNLIKELY(U_FAILURE(status))) {
            std::string message = "Icu range formatter format failed " + std::string(u_errorName(status));
            ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, message.c_str(), CTOR_SIGNATURE_STR);
            return nullptr;
        }
        icu::UnicodeString ustr = fmtRangeNumber.toString(status);
        return UnicodeToAniStr(env, ustr);
    }
    return nullptr;
}

icu::Formattable DoubleToFormattable([[maybe_unused]] ani_env *env, double value)
{
    return icu::Formattable(value);
}

icu::Formattable StrToFormattable(ani_env *env, ani_string value)
{
    UErrorCode status = U_ZERO_ERROR;
    const std::string &str = ConvertFromAniString(env, value);
    const icu::StringPiece sp {str.data(), static_cast<int32_t>(str.size())};
    icu::Formattable ret(sp, status);
    if (UNLIKELY(U_FAILURE(status))) {
        std::string message = "StrToToFormattable failed " + std::string(u_errorName(status));
        ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, message.c_str(), CTOR_SIGNATURE_STR);
        return icu::Formattable();
    }
    return ret;
}

template <typename T>
struct GetICUFormattedNumber {
    static std::optional<icu::number::FormattedNumber> GetFormattedNumber(
        const icu::number::LocalizedNumberFormatter &formatter, T value);
};

template <>
struct GetICUFormattedNumber<ani_double> {
    static std::optional<icu::number::FormattedNumber> GetFormattedNumber(
        const icu::number::LocalizedNumberFormatter &formatter, ani_double value)
    {
        UErrorCode status = U_ZERO_ERROR;
        auto formatted = formatter.formatDouble(value, status);
        if (U_FAILURE(status) != 0) {
            return std::nullopt;
        }
        return formatted;
    }
};

template <>
struct GetICUFormattedNumber<std::string> {
    static std::optional<icu::number::FormattedNumber> GetFormattedNumber(
        const icu::number::LocalizedNumberFormatter &formatter, const std::string &value)
    {
        UErrorCode status = U_ZERO_ERROR;
        const icu::StringPiece sp {value.data(), static_cast<int32_t>(value.size())};
        auto formatted = formatter.formatDecimal(sp, status);
        if (U_FAILURE(status) != 0) {
            return std::nullopt;
        }
        return formatted;
    }
};

std::vector<NumberFormatPart> ProcessSpansToFormatParts(const std::vector<FieldSpan> &flatParts,
                                                        const icu::UnicodeString &formattedString,
                                                        const std::pair<int32_t, int32_t> &startRangeSource,
                                                        const std::pair<int32_t, int32_t> &endRangeSource,
                                                        bool calculateSources)
{
    std::vector<NumberFormatPart> parts;
    auto contains = [](std::pair<int32_t, int32_t> gold, std::pair<int32_t, int32_t> request) {
        return gold.first <= request.first && gold.second >= request.second;
    };

    for (const auto &part : flatParts) {
        std::string_view ty = LITERAL_FIELD;
        std::string_view source = SHARED_SOURCE;

        icu::UnicodeString sub;
        formattedString.extractBetween(part.GetStartIndex(), part.GetEndIndex(), sub);
        std::string value;
        sub.toUTF8String(value);

        if (part.GetFieldType() != -1) {
            ty = GetFieldTypeString(part, formattedString);
        }

        if (calculateSources) {
            auto request = std::make_pair(part.GetStartIndex(), part.GetEndIndex());
            if (contains(startRangeSource, request)) {
                source = START_RANGE_SOURCE;
            } else if (contains(endRangeSource, request)) {
                source = END_RANGE_SOURCE;
            }
        }
        parts.push_back({std::string(ty), value, std::string(source)});
    }
    return parts;
}

std::vector<NumberFormatPart> ExtractParts(ani_env *env, const icu::FormattedValue &formatted, bool calculateSources)
{
    UErrorCode status = U_ZERO_ERROR;
    std::vector<NumberFormatPart> parts;
    icu::UnicodeString formattedString = formatted.toString(status);

    if (UNLIKELY(U_FAILURE(status) != 0)) {
        std::string message = "ExtractParts. Unable to convert formatted value to string";
        ThrowNewError(env, "std.core.RuntimeError", message.c_str(), "C{std.core.String}:");
        return parts;
    }

    std::pair<int32_t, int32_t> startRangeSource = std::make_pair(0, 0);
    std::pair<int32_t, int32_t> endRangeSource = std::make_pair(0, 0);

    std::vector<FieldSpan> spans;

    // Add a default span for the entire string
    spans.emplace_back(-1, 0, formattedString.length());
    // Source (sep like "-", "~") for range to parts formatting looks like
    // span is a representation of semi-open interval [start, end)
    // For example "start: 0.123, end: 0.124"
    // formatted: "-0.123-0.124"
    //  - 0 . 1 2 3 - 0 . 1 2 4
    // |<--------->|
    //             |-|
    //             |<--------->|
    //  ^^^^^^^^^^^
    //  startRange               <------------------ [0, 6)
    //              ^
    //           shared          <------------------ [6, 7)
    //
    //              ^^^^^^^^^^^^
    //              endRange     <------------------ [7, 12)
    icu::ConstrainedFieldPosition cfpos;
    while (formatted.nextPosition(cfpos, status) != 0) {
        if (UNLIKELY(U_FAILURE(status) != 0)) {
            std::string message = "ExtractParts. Error during field iteration";
            ThrowNewError(env, "std.core.RuntimeError", message.c_str(), "C{std.core.String}:");
            return parts;
        }
        int32_t cat = cfpos.getCategory();
        int32_t field = cfpos.getField();
        int32_t start = cfpos.getStart();
        int32_t limit = cfpos.getLimit();
        if (cat == UFIELD_CATEGORY_NUMBER_RANGE_SPAN) {
            switch (field) {
                case INTL_NUMBER_RANGE_START:
                    startRangeSource.first = start;
                    startRangeSource.second = limit;
                    break;
                case INTL_NUMBER_RANGE_END:
                    endRangeSource.first = start;
                    endRangeSource.second = limit;
                    break;
                default:
                    UNREACHABLE();
            }
        } else {
            spans.emplace_back(field, start, limit);
        }
    }
    return ProcessSpansToFormatParts(SpanFolder(std::move(spans)).Fold(), formattedString, startRangeSource,
                                     endRangeSource, calculateSources);
}

ani_object GetNumberFormatPart(ani_env *env, ani_string ty, ani_string value)
{
    ani_class numberFormatPartClass;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.Intl.NumberFormatPart", &numberFormatPartClass));

    ani_method constructorMethod;
    ANI_FATAL_IF_ERROR(env->Class_FindMethod(numberFormatPartClass, "<ctor>", ":", &constructorMethod));

    ani_object numberFormatPartObj;

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_FATAL_IF_ERROR(env->Object_New(numberFormatPartClass, constructorMethod, &numberFormatPartObj));

    ani_field typeField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(numberFormatPartClass, "type", &typeField));

    ani_field valueField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(numberFormatPartClass, "value", &valueField));

    ANI_FATAL_IF_ERROR(env->Object_SetField_Ref(numberFormatPartObj, typeField, ty));
    ANI_FATAL_IF_ERROR(env->Object_SetField_Ref(numberFormatPartObj, valueField, value));

    return numberFormatPartObj;
}

ani_object GetNumberFormatRangePart(ani_env *env, ani_string ty, ani_string value, ani_string source)
{
    ani_class numberRangeFormatPartClass;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.Intl.NumberRangeFormatPart", &numberRangeFormatPartClass));

    ani_method constructorMethod;
    ANI_FATAL_IF_ERROR(env->Class_FindMethod(numberRangeFormatPartClass, "<ctor>", ":", &constructorMethod));

    ani_object numberRangeFormatPartObj;

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_FATAL_IF_ERROR(env->Object_New(numberRangeFormatPartClass, constructorMethod, &numberRangeFormatPartObj));

    ani_field typeField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(numberRangeFormatPartClass, "type", &typeField));

    ani_field valueField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(numberRangeFormatPartClass, "value", &valueField));

    ani_field sourceField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(numberRangeFormatPartClass, "source", &sourceField));

    ANI_FATAL_IF_ERROR(env->Object_SetField_Ref(numberRangeFormatPartObj, typeField, ty));
    ANI_FATAL_IF_ERROR(env->Object_SetField_Ref(numberRangeFormatPartObj, valueField, value));
    ANI_FATAL_IF_ERROR(env->Object_SetField_Ref(numberRangeFormatPartObj, sourceField, source));
    return numberRangeFormatPartObj;
}

template <typename T>
ani_array IcuFormatToParts(ani_env *env, [[maybe_unused]] ani_object self, [[maybe_unused]] T value)
{
    ani_array resultArray;
    ParsedOptions options;
    ParseOptions(env, self, options);

    ani_status err;
    LocNumFmt &formatter = g_intlState->fmtsCache.NumFmtsCacheInvalidation(env, options, err);
    if (err != ANI_OK) {
        return nullptr;
    }

    auto fmtNumber = GetICUFormattedNumber<T>::GetFormattedNumber(formatter, value);
    if (!fmtNumber) {
        return nullptr;
    }

    std::vector<ani_object> resultParts;
    UErrorCode status = U_ZERO_ERROR;
    icu::UnicodeString ustr = fmtNumber->toString(status);

    auto parts = ExtractParts(env, static_cast<const icu::FormattedValue &>(*fmtNumber), false);
    for (const auto &part : parts) {
        auto utype = CreateUtf8String(env, part.type.c_str(), part.type.size());
        auto uvalue = CreateUtf8String(env, part.value.c_str(), part.value.size());
        auto partObj = GetNumberFormatPart(env, utype, uvalue);
        resultParts.push_back(partObj);
    }

    ani_ref undefined;
    ANI_FATAL_IF_ERROR(env->GetUndefined(&undefined));
    ANI_FATAL_IF_ERROR(env->Array_New(resultParts.size(), undefined, &resultArray));
    for (size_t i = 0; i < resultParts.size(); ++i) {
        ANI_FATAL_IF_ERROR(env->Array_Set(resultArray, i, resultParts[i]));
    }

    return resultArray;
}

ani_array IcuFormatToRangeParts(ani_env *env, [[maybe_unused]] ani_object self,
                                [[maybe_unused]] const icu::Formattable &startFrmtbl,
                                [[maybe_unused]] const icu::Formattable &endFrmtbl)
{
    ani_array resultArray;

    ParsedOptions options;
    ParseOptions(env, self, options);

    ani_status err;
    LocNumRangeFmt &formatter = g_intlState->fmtsCache.NumRangeFmtsCacheInvalidation(env, options, err);
    if (err != ANI_OK) {
        return nullptr;
    }

    UErrorCode status = U_ZERO_ERROR;
    icu::number::FormattedNumberRange formatted = formatter.formatFormattableRange(startFrmtbl, endFrmtbl, status);

    if (U_FAILURE(status) != 0) {
        return nullptr;
    }

    std::vector<NumberFormatPart> parts = ExtractParts(env, formatted, true);

    ani_ref undefined;
    ANI_FATAL_IF_ERROR(env->GetUndefined(&undefined));
    ANI_FATAL_IF_ERROR(env->Array_New(parts.size(), undefined, &resultArray));

    for (size_t i = 0; i < parts.size(); ++i) {
        auto typeString = CreateUtf8String(env, parts[i].type.c_str(), parts[i].type.size());
        auto valueString = CreateUtf8String(env, parts[i].value.c_str(), parts[i].value.size());
        auto sourceString = CreateUtf8String(env, parts[i].source.c_str(), parts[i].source.size());
        auto part = GetNumberFormatRangePart(env, typeString, valueString, sourceString);
        ANI_FATAL_IF_ERROR(env->Array_Set(resultArray, i, part));
    }

    return resultArray;
}

ani_string IcuFormatRangeDoubleDouble(ani_env *env, ani_object self, ani_double startValue, ani_double endValue)
{
    return IcuFormatRange(env, self, DoubleToFormattable(env, startValue), DoubleToFormattable(env, endValue));
}

ani_string IcuFormatRangeDoubleDecStr(ani_env *env, ani_object self, ani_double startValue, ani_string endValue)
{
    return IcuFormatRange(env, self, DoubleToFormattable(env, startValue), StrToFormattable(env, endValue));
}

ani_string IcuFormatRangeDecStrDouble(ani_env *env, ani_object self, ani_string startValue, ani_double endValue)
{
    return IcuFormatRange(env, self, StrToFormattable(env, startValue), DoubleToFormattable(env, endValue));
}

ani_string IcuFormatRangeDecStrDecStr(ani_env *env, ani_object self, ani_string startValue, ani_string endValue)
{
    return IcuFormatRange(env, self, StrToFormattable(env, startValue), StrToFormattable(env, endValue));
}

ani_array IcuFormatToPartsDouble(ani_env *env, [[maybe_unused]] ani_object self, [[maybe_unused]] ani_double value)
{
    return IcuFormatToParts<ani_double>(env, self, value);
}

ani_array IcuFormatToPartsDecStr(ani_env *env, [[maybe_unused]] ani_object self, [[maybe_unused]] ani_string value)
{
    return IcuFormatToParts<std::string>(env, self, ConvertFromAniString(env, value));
}

ani_array IcuFormatToRangePartsDoubleDouble(ani_env *env, [[maybe_unused]] ani_object self,
                                            [[maybe_unused]] ani_double startValue,
                                            [[maybe_unused]] ani_double endValue)
{
    return IcuFormatToRangeParts(env, self, DoubleToFormattable(env, startValue), DoubleToFormattable(env, endValue));
}

ani_array IcuFormatToRangePartsDoubleDecStr(ani_env *env, [[maybe_unused]] ani_object self,
                                            [[maybe_unused]] ani_double startValue,
                                            [[maybe_unused]] ani_string endValue)
{
    return IcuFormatToRangeParts(env, self, DoubleToFormattable(env, startValue), StrToFormattable(env, endValue));
}

ani_array IcuFormatToRangePartsDecStrDouble(ani_env *env, [[maybe_unused]] ani_object self,
                                            [[maybe_unused]] ani_string startValue,
                                            [[maybe_unused]] ani_double endValue)
{
    return IcuFormatToRangeParts(env, self, StrToFormattable(env, startValue), DoubleToFormattable(env, endValue));
}

ani_array IcuFormatToRangePartsDecStrDecStr(ani_env *env, [[maybe_unused]] ani_object self,
                                            [[maybe_unused]] ani_string startValue,
                                            [[maybe_unused]] ani_string endValue)
{
    return IcuFormatToRangeParts(env, self, StrToFormattable(env, startValue), StrToFormattable(env, endValue));
}

ani_boolean IsIcuUnitCorrect(ani_env *env, [[maybe_unused]] ani_object self, ani_string unit)
{
    return IsCorrectUnitIdentifier(ConvertFromAniString(env, unit)) ? ANI_TRUE : ANI_FALSE;
}

ani_string IcuNumberingSystem(ani_env *env, [[maybe_unused]] ani_object self, ani_string locale)
{
    icu::Locale icuLocale(icu::Locale::getDefault());
    ani_status err = LocTagToIcuLocale(env, ConvertFromAniString(env, locale), icuLocale);
    if (err == ANI_OK) {
        UErrorCode status = U_ZERO_ERROR;
        std::unique_ptr<icu::NumberingSystem> numSystem(icu::NumberingSystem::createInstance(icuLocale, status));
        if (UNLIKELY(U_FAILURE(status))) {
            std::string message = "NumberingSystem creation failed";
            ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, message.c_str(), CTOR_SIGNATURE_STR);
            return nullptr;
        }
        return CreateUtf8String(env, numSystem->getName(), strlen(numSystem->getName()));
    }
    return nullptr;
}

ani_double IcuCurrencyDigits(ani_env *env, [[maybe_unused]] ani_object self, ani_string currency)
{
    UErrorCode status = U_ZERO_ERROR;
    const std::string currencyStr = ConvertFromAniString(env, currency);
    icu::UnicodeString uCurrency = icu::UnicodeString::fromUTF8(currencyStr);
    int32_t fractionDigits = ucurr_getDefaultFractionDigits(uCurrency.getBuffer(), &status);
    if (U_SUCCESS(status) != 0) {
        return static_cast<double>(fractionDigits);
    }
    // return default fraction as 2 if ucurr_getDefaultFractionDigits failed
    return 2;
}

ani_status RegisterIntlNumberFormatNativeMethods(ani_env *env)
{
    ani_class numberFormatClass;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.Intl.NumberFormat", &numberFormatClass));

    const auto methods = std::array {
        ani_native_function {"formatDouble", "d:C{std.core.String}", reinterpret_cast<void *>(IcuFormatDouble)},
        ani_native_function {"formatLong", "l:C{std.core.String}", reinterpret_cast<void *>(IcuFormatLong)},
        ani_native_function {"formatDecStr", "C{std.core.String}:C{std.core.String}",
                             reinterpret_cast<void *>(IcuFormatDecStr)},
        ani_native_function {"formatRangeDoubleDouble", "dd:C{std.core.String}",
                             reinterpret_cast<void *>(IcuFormatRangeDoubleDouble)},
        ani_native_function {"formatRangeDoubleDecStr", "dC{std.core.String}:C{std.core.String}",
                             reinterpret_cast<void *>(IcuFormatRangeDoubleDecStr)},
        ani_native_function {"formatRangeDecStrDouble", "C{std.core.String}d:C{std.core.String}",
                             reinterpret_cast<void *>(IcuFormatRangeDecStrDouble)},
        ani_native_function {"formatRangeDecStrDecStr", "C{std.core.String}C{std.core.String}:C{std.core.String}",
                             reinterpret_cast<void *>(IcuFormatRangeDecStrDecStr)},
        ani_native_function {"formatToPartsDouble", "d:C{std.core.Array}",
                             reinterpret_cast<void *>(IcuFormatToPartsDouble)},
        ani_native_function {"formatToPartsDecStr", "C{std.core.String}:C{std.core.Array}",
                             reinterpret_cast<void *>(IcuFormatToPartsDecStr)},
        ani_native_function {"formatToRangePartsDoubleDouble", "dd:C{std.core.Array}",
                             reinterpret_cast<void *>(IcuFormatToRangePartsDoubleDouble)},
        ani_native_function {"formatToRangePartsDoubleDecStr", "dC{std.core.String}:C{std.core.Array}",
                             reinterpret_cast<void *>(IcuFormatToRangePartsDoubleDecStr)},
        ani_native_function {"formatToRangePartsDecStrDouble", "C{std.core.String}d:C{std.core.Array}",
                             reinterpret_cast<void *>(IcuFormatToRangePartsDecStrDouble)},
        ani_native_function {"formatToRangePartsDecStrDecStr", "C{std.core.String}C{std.core.String}:C{std.core.Array}",
                             reinterpret_cast<void *>(IcuFormatToRangePartsDecStrDecStr)},
        ani_native_function {"currencyDigits", "C{std.core.String}:d", reinterpret_cast<void *>(IcuCurrencyDigits)},
    };
    ani_status status = env->Class_BindNativeMethods(numberFormatClass, methods.data(), methods.size());
    if (!(status == ANI_OK || status == ANI_ALREADY_BINDED)) {
        return status;
    }

    const auto staticMethods = std::array {
        ani_native_function {"getNumberingSystem", "C{std.core.String}:C{std.core.String}",
                             reinterpret_cast<void *>(IcuNumberingSystem)},
        ani_native_function {"isUnitCorrect", "C{std.core.String}:z", reinterpret_cast<void *>(IsIcuUnitCorrect)},
    };
    status = env->Class_BindStaticNativeMethods(numberFormatClass, staticMethods.data(), staticMethods.size());
    if (!(status == ANI_OK || status == ANI_ALREADY_BINDED)) {
        return status;
    }

    ANI_FATAL_IF_ERROR(env->Class_FindField(numberFormatClass, "options", &refs::g_numberFormat_options_field));

    ani_class resolvedOptionsClassLocal;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.Intl.ResolvedNumberFormatOptionsImpl", &resolvedOptionsClassLocal));
    ANI_FATAL_IF_ERROR(env->GlobalReference_Create(resolvedOptionsClassLocal,
                                                   reinterpret_cast<ani_ref *>(&refs::g_resolvedOptionsClass)));

    ANI_FATAL_IF_ERROR(env->Class_FindField(refs::g_resolvedOptionsClass, "_locale", &refs::g_locale_field));
    ANI_FATAL_IF_ERROR(
        env->Class_FindField(refs::g_resolvedOptionsClass, "_compactDisplay", &refs::g_compactDisplay_field));
    ANI_FATAL_IF_ERROR(
        env->Class_FindField(refs::g_resolvedOptionsClass, "_currencySign", &refs::g_currencySign_field));
    ANI_FATAL_IF_ERROR(env->Class_FindField(refs::g_resolvedOptionsClass, "_currency", &refs::g_currency_field));
    ANI_FATAL_IF_ERROR(
        env->Class_FindField(refs::g_resolvedOptionsClass, "_currencyDisplay", &refs::g_currencyDisplay_field));
    ANI_FATAL_IF_ERROR(
        env->Class_FindField(refs::g_resolvedOptionsClass, "minFracStr", &refs::g_minFractionDigits_field));
    ANI_FATAL_IF_ERROR(
        env->Class_FindField(refs::g_resolvedOptionsClass, "maxFracStr", &refs::g_maxFractionDigits_field));
    ANI_FATAL_IF_ERROR(
        env->Class_FindField(refs::g_resolvedOptionsClass, "minSignStr", &refs::g_minSignificantDigits_field));
    ANI_FATAL_IF_ERROR(
        env->Class_FindField(refs::g_resolvedOptionsClass, "maxSignStr", &refs::g_maxSignificantDigits_field));
    ANI_FATAL_IF_ERROR(
        env->Class_FindField(refs::g_resolvedOptionsClass, "minIntStr", &refs::g_minIntegerDigits_field));
    ANI_FATAL_IF_ERROR(env->Class_FindField(refs::g_resolvedOptionsClass, "_notation", &refs::g_notation_field));
    ANI_FATAL_IF_ERROR(
        env->Class_FindField(refs::g_resolvedOptionsClass, "_numberingSystem", &refs::g_numberingSystem_field));
    ANI_FATAL_IF_ERROR(env->Class_FindField(refs::g_resolvedOptionsClass, "_signDisplay", &refs::g_signDisplay_field));
    ANI_FATAL_IF_ERROR(env->Class_FindField(refs::g_resolvedOptionsClass, "_style", &refs::g_style_field));
    ANI_FATAL_IF_ERROR(env->Class_FindField(refs::g_resolvedOptionsClass, "_unit", &refs::g_unit_field));
    ANI_FATAL_IF_ERROR(env->Class_FindField(refs::g_resolvedOptionsClass, "_unitDisplay", &refs::g_unitDisplay_field));
    ANI_FATAL_IF_ERROR(
        env->Class_FindField(refs::g_resolvedOptionsClass, "useGroupingStr", &refs::g_useGrouping_field));

    return ANI_OK;
}

}  // namespace ark::ets::stdlib::intl
