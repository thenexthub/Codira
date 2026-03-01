/**
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "plugins/ets/stdlib/native/core/stdlib_ani_helpers.h"
#include "IntlCommon.h"
#include "IntlNumberFormatters.h"
#include <cstring>

namespace ark::ets::stdlib::intl {

ANI_EXPORT ani_status LocTagToIcuLocale(ani_env *env, const std::string &localeTag, icu::Locale &locale)
{
    icu::StringPiece sp {localeTag.data(), static_cast<int32_t>(localeTag.size())};
    UErrorCode status = U_ZERO_ERROR;
    locale = icu::Locale::forLanguageTag(sp, status);
    if (UNLIKELY(U_FAILURE(status))) {
        std::string message = "Language tag '" + localeTag + std::string("' is invalid or not supported");
        ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, message.c_str(), CTOR_SIGNATURE_STR);
        return ANI_PENDING_ERROR;
    }
    return ANI_OK;
}

ani_status SetNumberingSystemIntoLocale(ani_env *env, const ParsedOptions &options, icu::Locale &locale)
{
    if (!options.numberingSystem.empty()) {
        UErrorCode status = U_ZERO_ERROR;
        locale.setKeywordValue("nu", options.numberingSystem.c_str(), status);
        if (UNLIKELY(U_FAILURE(status))) {
            std::string message = "Invalid numbering system " + options.numberingSystem;
            ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, message.c_str(), CTOR_SIGNATURE_STR);
            return ANI_PENDING_ERROR;
        }
    }
    return ANI_OK;
}

// Configure minFractionDigits, maxFractionDigits, minSignificantDigits, maxSignificantDigits, minIntegerDigits
template <typename FormatterType>
ani_status ConfigurePrecision([[maybe_unused]] ani_env *env, const ParsedOptions &options, FormatterType &fmt)
{
    fmt = fmt.precision(
        icu::number::Precision::minMaxFraction(stoi(options.minFractionDigits), stoi(options.maxFractionDigits)));
    if (!options.minSignificantDigits.empty()) {
        if (!options.maxSignificantDigits.empty()) {
            if (options.notation == NOTATION_COMPACT) {
                fmt = fmt.precision(icu::number::Precision::minMaxFraction(0, 0).withSignificantDigits(
                    stoi(options.minSignificantDigits), stoi(options.maxSignificantDigits),
                    UNUM_ROUNDING_PRIORITY_RELAXED));
            } else {
                fmt = fmt.precision(icu::number::Precision::minMaxSignificantDigits(
                    stoi(options.minSignificantDigits), stoi(options.maxSignificantDigits)));
            }
        } else {
            fmt = fmt.precision(icu::number::Precision::minSignificantDigits(stoi(options.minSignificantDigits)));
        }
    } else {
        if (!options.maxSignificantDigits.empty()) {
            fmt = fmt.precision(icu::number::Precision::maxSignificantDigits(stoi(options.maxSignificantDigits)));
        }
    }
    fmt = fmt.integerWidth(icu::number::IntegerWidth::zeroFillTo(stoi(options.minIntegerDigits)));
    fmt = fmt.roundingMode(UNUM_ROUND_HALFUP);
    return ANI_OK;
}

template <typename FormatterType>
ani_status ConfigureNotation(ani_env *env, const ParsedOptions &options, FormatterType &fmt)
{
    if (options.notation == NOTATION_STANDARD) {
        fmt = fmt.notation(icu::number::Notation::simple());
    } else if (options.notation == NOTATION_SCIENTIFIC) {
        fmt = fmt.notation(icu::number::Notation::scientific());
    } else if (options.notation == NOTATION_ENGINEERING) {
        fmt = fmt.notation(icu::number::Notation::engineering());
    } else if (options.notation == NOTATION_COMPACT) {
        if (options.compactDisplay == COMPACT_DISPLAY_SHORT) {
            fmt = fmt.notation(icu::number::Notation::compactShort());
        } else if (options.compactDisplay == COMPACT_DISPLAY_LONG) {
            fmt = fmt.notation(icu::number::Notation::compactLong());
        } else {
            if (!options.compactDisplay.empty()) {
                std::string message = "Invalid compactDisplay " + options.compactDisplay;
                ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, message.c_str(), CTOR_SIGNATURE_STR);
                return ANI_PENDING_ERROR;
            }
        }
    } else {
        if (!options.notation.empty()) {
            std::string message = "Invalid compactDisplay " + options.notation;
            ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, message.c_str(), CTOR_SIGNATURE_STR);
            return ANI_PENDING_ERROR;
        }
    }
    return ANI_OK;
}

bool IsCorrectUnitIdentifier(const std::string &unit)
{
    // Quick check
    return REFERENCE_UNITS.find(unit) != REFERENCE_UNITS.end();
}

ani_status StdStrToIcuUnit(ani_env *env, const std::string &str, icu::MeasureUnit &icuUnit)
{
    // Handle MeasureUnit
    UErrorCode status = U_ZERO_ERROR;
    icuUnit = icu::MeasureUnit::forIdentifier(icu::StringPiece(str.c_str()), status);
    if (UNLIKELY(U_FAILURE(status))) {
        std::string message = "Unit input is illegal " + str;
        ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, message, CTOR_SIGNATURE_STR);
        return ANI_PENDING_ERROR;
    }
    return ANI_OK;
}

template <typename FormatterType>
ani_status ConfigureUnit(ani_env *env, const ParsedOptions &options, FormatterType &fmt)
{
    ani_status err;
    icu::MeasureUnit icuUnit;  // Default is empty

    // If no "-per-" inside string unit
    auto pos = options.unit.find("-per-");
    if (IsCorrectUnitIdentifier(options.unit) && pos == std::string::npos) {
        err = StdStrToIcuUnit(env, options.unit, icuUnit);
        if (err != ANI_OK) {
            return err;
        }
        fmt = fmt.unit(icuUnit);
        return ANI_OK;
    }

    // If the substring "-per-" invalid
    size_t afterPos = pos + PER_UNIT_STR_SIZE;
    if (pos == std::string::npos || options.unit.find("-per-", afterPos) != std::string::npos) {
        std::string message = "Unit input is illegal " + options.unit;
        ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, message, CTOR_SIGNATURE_STR);
        return ANI_PENDING_ERROR;
    }

    // First part of unit
    std::string numerator = options.unit.substr(0, pos);
    std::string errMessage = "Not a wellformed " + numerator;
    if (!IsCorrectUnitIdentifier(numerator)) {
        ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, errMessage, CTOR_SIGNATURE_STR);
        return ANI_PENDING_ERROR;
    }
    err = StdStrToIcuUnit(env, numerator, icuUnit);
    if (err != ANI_OK) {
        std::string message = "Unit input is illegal " + options.unit;
        ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, message, CTOR_SIGNATURE_STR);
        return err;
    }

    // Second part of unit is perUnit
    std::string denominator = options.unit.substr(pos + PER_UNIT_STR_SIZE);
    icu::MeasureUnit icuPerUnit = icu::MeasureUnit();
    errMessage = "Not a wellformed  " + denominator;
    if (!IsCorrectUnitIdentifier(denominator)) {
        ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, errMessage, CTOR_SIGNATURE_STR);
        return ANI_PENDING_ERROR;
    }
    err = StdStrToIcuUnit(env, denominator, icuPerUnit);
    if (err != ANI_OK) {
        std::string message = "Unit input is illegal " + options.unit;
        ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, message, CTOR_SIGNATURE_STR);
        return err;
    }

    fmt = fmt.unit(icuUnit);
    fmt = fmt.perUnit(icuPerUnit);
    return ANI_OK;
}

template <typename FormatterType>
ani_status ConfigureStyleUnit(ani_env *env, const ParsedOptions &options, FormatterType &fmt)
{
    ani_status err;

    // Set unit and perUnit
    if (!options.unit.empty()) {
        err = ConfigureUnit<FormatterType>(env, options, fmt);
        if (err != ANI_OK) {
            return err;
        }
    }

    // Set unitDisplay
    if (options.unitDisplay == UNIT_DISPLAY_SHORT) {
        fmt = fmt.unitWidth(UNUM_UNIT_WIDTH_SHORT);
    } else if (options.unitDisplay == UNIT_DISPLAY_LONG) {
        fmt = fmt.unitWidth(UNUM_UNIT_WIDTH_FULL_NAME);
    } else if (options.unitDisplay == UNIT_DISPLAY_NARROW) {
        fmt = fmt.unitWidth(UNUM_UNIT_WIDTH_NARROW);
    } else {
        if (!options.unitDisplay.empty()) {
            std::string message = "Invalid unitDisplay " + options.unitDisplay;
            ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, message.c_str(), CTOR_SIGNATURE_STR);
            return ANI_PENDING_ERROR;
        }
    }
    return ANI_OK;
}

template <typename FormatterType>
ani_status ConfigureStyleCurrency(ani_env *env, const ParsedOptions &options, FormatterType &fmt)
{
    UErrorCode status = U_ZERO_ERROR;
    icu::CurrencyUnit currency(icu::StringPiece(options.currency.c_str()), status);
    if (UNLIKELY(U_FAILURE(status))) {
        std::string message = "Invalid currency " + options.currency;
        ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, message.c_str(), CTOR_SIGNATURE_STR);
        return ANI_PENDING_ERROR;
    }
    fmt = fmt.unit(currency);
    // Set currency display
    if (options.currencyDisplay == CURRENCY_DISPLAY_CODE) {
        fmt = fmt.unitWidth(UNUM_UNIT_WIDTH_ISO_CODE);
    } else if (options.currencyDisplay == CURRENCY_DISPLAY_SYMBOL) {
        fmt = fmt.unitWidth(UNUM_UNIT_WIDTH_SHORT);
    } else if (options.currencyDisplay == CURRENCY_DISPLAY_NARROWSYMBOL) {
        fmt = fmt.unitWidth(UNUM_UNIT_WIDTH_NARROW);
    } else if (options.currencyDisplay == CURRENCY_DISPLAY_NAME) {
        fmt = fmt.unitWidth(UNUM_UNIT_WIDTH_FULL_NAME);
    } else {
        // Default style, no need to set anything
        if (!options.currencyDisplay.empty()) {
            std::string message = "Invalid currencyDisplay " + options.currencyDisplay;
            ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, message.c_str(), CTOR_SIGNATURE_STR);
            return ANI_PENDING_ERROR;
        }
    }
    return ANI_OK;
}

template <typename FormatterType>
ani_status ConfigureStyle(ani_env *env, const ParsedOptions &options, FormatterType &fmt)
{
    if (options.style == STYLE_PERCENT) {
        // Use percent unit
        fmt = fmt.unit(icu::MeasureUnit::getPercent());
        fmt = fmt.scale(icu::number::Scale::powerOfTen(STYLE_PERCENT_SCALE_FACTOR));
    } else if (options.style == STYLE_CURRENCY && !options.currency.empty()) {
        return ConfigureStyleCurrency(env, options, fmt);
    } else if (options.style == STYLE_UNIT) {
        return ConfigureStyleUnit(env, options, fmt);
    } else {
        // Default style, no need to set anything
        if (!options.style.empty() && options.style != STYLE_DECIMAL) {
            std::string message = "Invalid style " + options.style;
            ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, message.c_str(), CTOR_SIGNATURE_STR);
            return ANI_PENDING_ERROR;
        }
    }
    return ANI_OK;
}

template <typename FormatterType>
ani_status ConfigureUseGrouping(ani_env *env, const ParsedOptions &options, FormatterType &fmt)
{
    if (options.useGrouping == USE_GROUPING_TRUE) {
        fmt = fmt.grouping(UNumberGroupingStrategy::UNUM_GROUPING_ON_ALIGNED);
    } else if (options.useGrouping == USE_GROUPING_FALSE) {
        fmt = fmt.grouping(UNumberGroupingStrategy::UNUM_GROUPING_OFF);
    } else if (options.useGrouping == USE_GROUPING_MIN2) {
        fmt = fmt.grouping(UNumberGroupingStrategy::UNUM_GROUPING_MIN2);
    } else {
        // Default is UNumberGroupingStrategy::UNUM_GROUPING_AUTO
        if (!options.useGrouping.empty()) {
            std::string message = "Invalid useGrouping " + options.useGrouping;
            ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, message.c_str(), CTOR_SIGNATURE_STR);
            return ANI_PENDING_ERROR;
        }
    }
    return ANI_OK;
}

template <typename FormatterType>
ani_status ConfigureSignDisplay(ani_env *env, const ParsedOptions &options, FormatterType &fmt)
{
    // Configure signDisplay
    if (options.signDisplay == SIGN_DISPLAY_AUTO) {
        // Handle currency sign
        if (options.currencySign == CURRENCY_SIGN_ACCOUNTING) {
            fmt = fmt.sign(UNumberSignDisplay::UNUM_SIGN_ACCOUNTING);
        } else {
            // CURRENCY_SIGN_STANDARD
            fmt = fmt.sign(UNumberSignDisplay::UNUM_SIGN_AUTO);
        }
    } else if (options.signDisplay == SIGN_DISPLAY_NEVER) {
        fmt = fmt.sign(UNumberSignDisplay::UNUM_SIGN_NEVER);
    } else if (options.signDisplay == SIGN_DISPLAY_ALWAYS) {
        // Handle currency sign
        if (options.currencySign == CURRENCY_SIGN_ACCOUNTING) {
            fmt = fmt.sign(UNumberSignDisplay::UNUM_SIGN_ACCOUNTING_ALWAYS);
        } else {
            // CURRENCY_SIGN_STANDARD AND SIGN_DISPLAY_ALWAYS
            fmt = fmt.sign(UNumberSignDisplay::UNUM_SIGN_ALWAYS);
        }
    } else if (options.signDisplay == SIGN_DISPLAY_EXCEPTZERO) {
        // Handle currency sign
        if (options.currencySign == CURRENCY_SIGN_ACCOUNTING) {
            fmt = fmt.sign(UNumberSignDisplay::UNUM_SIGN_ACCOUNTING_EXCEPT_ZERO);
        } else {
            // CURRENCY_SIGN_STANDARD AND SIGN_DISPLAY_EXCEPTZERO
            fmt = fmt.sign(UNumberSignDisplay::UNUM_SIGN_EXCEPT_ZERO);
        }
    } else {
        if (!options.signDisplay.empty()) {
            ThrowNewError(env, ERR_CLS_RUNTIME_EXCEPTION, "Invalid signDisplay", CTOR_SIGNATURE_STR);
            return ANI_PENDING_ERROR;
        }
    }
    return ANI_OK;
}

ani_status InitUnlocNumFormatter(ani_env *env, const ParsedOptions &options, UnlocNumFmt &fmt)
{
    ani_status err = ConfigureNotation<UnlocNumFmt>(env, options, fmt);
    err = err == ANI_OK ? ConfigurePrecision<UnlocNumFmt>(env, options, fmt) : err;
    err = err == ANI_OK ? ConfigureStyle<UnlocNumFmt>(env, options, fmt) : err;
    err = err == ANI_OK ? ConfigureUseGrouping<UnlocNumFmt>(env, options, fmt) : err;
    err = err == ANI_OK ? ConfigureSignDisplay<UnlocNumFmt>(env, options, fmt) : err;
    return err;
}

ani_status InitNumFormatter(ani_env *env, const ParsedOptions &options, LocNumFmt &fmt)
{
    icu::Locale localeWithNumSystem(icu::Locale::getDefault());
    ani_status err = LocTagToIcuLocale(env, options.locale, localeWithNumSystem);
    if (err != ANI_OK) {
        return err;
    }
    err = SetNumberingSystemIntoLocale(env, options, localeWithNumSystem);
    if (err == ANI_OK) {
        auto unlocFmt = icu::number::NumberFormatter::with();
        err = InitUnlocNumFormatter(env, options, unlocFmt);
        if (err == ANI_OK) {
            fmt = unlocFmt.locale(localeWithNumSystem);
        }
    }
    return err;
}

ani_status InitNumRangeFormatter(ani_env *env, const ParsedOptions &options, LocNumRangeFmt &fmt)
{
    icu::Locale localeWithNumSystem(icu::Locale::getDefault());
    ani_status err = LocTagToIcuLocale(env, options.locale, localeWithNumSystem);
    if (err != ANI_OK) {
        return err;
    }
    err = SetNumberingSystemIntoLocale(env, options, localeWithNumSystem);
    if (err == ANI_OK) {
        auto unlocFmt = icu::number::NumberFormatter::with();
        err = InitUnlocNumFormatter(env, options, unlocFmt);
        if (err == ANI_OK) {
            fmt = icu::number::NumberRangeFormatter::withLocale(localeWithNumSystem);
            fmt = fmt.numberFormatterBoth(unlocFmt);
        }
    }
    return err;
}

static std::string GetFieldStrById(ani_env *env, ani_object obj, ani_field fieldId, bool allowUndefined)
{
    ani_ref ref = nullptr;
    ANI_FATAL_IF_ERROR(env->Object_GetField_Ref(obj, fieldId, &ref));
    if (allowUndefined) {
        ani_boolean isUndefined = ANI_FALSE;
        ANI_FATAL_IF_ERROR(env->Reference_IsUndefined(ref, &isUndefined));
        if (isUndefined) {
            return std::string();
        }
    }

    auto aniStr = static_cast<ani_string>(ref);
    ASSERT(aniStr != nullptr);
    return ConvertFromAniString(env, aniStr);
}

void ParseOptions(ani_env *env, ani_object self, ParsedOptions &options)
{
    ani_ref optionsRef = nullptr;
    ANI_FATAL_IF_ERROR(env->Object_GetField_Ref(self, refs::g_numberFormat_options_field, &optionsRef));
    auto optionsObj = static_cast<ani_object>(optionsRef);
    ASSERT(optionsObj != nullptr);

    options.locale = GetFieldStrById(env, optionsObj, refs::g_locale_field, false);
    options.compactDisplay = GetFieldStrById(env, optionsObj, refs::g_compactDisplay_field, true);
    options.currencySign = GetFieldStrById(env, optionsObj, refs::g_currencySign_field, true);
    options.currency = GetFieldStrById(env, optionsObj, refs::g_currency_field, true);
    options.currencyDisplay = GetFieldStrById(env, optionsObj, refs::g_currencyDisplay_field, true);
    options.minFractionDigits = GetFieldStrById(env, optionsObj, refs::g_minFractionDigits_field, true);
    options.maxFractionDigits = GetFieldStrById(env, optionsObj, refs::g_maxFractionDigits_field, true);
    options.minSignificantDigits = GetFieldStrById(env, optionsObj, refs::g_minSignificantDigits_field, true);
    options.maxSignificantDigits = GetFieldStrById(env, optionsObj, refs::g_maxSignificantDigits_field, true);
    options.minIntegerDigits = GetFieldStrById(env, optionsObj, refs::g_minIntegerDigits_field, true);
    options.notation = GetFieldStrById(env, optionsObj, refs::g_notation_field, true);
    options.numberingSystem = GetFieldStrById(env, optionsObj, refs::g_numberingSystem_field, true);
    options.signDisplay = GetFieldStrById(env, optionsObj, refs::g_signDisplay_field, true);
    options.style = GetFieldStrById(env, optionsObj, refs::g_style_field, true);
    options.unit = GetFieldStrById(env, optionsObj, refs::g_unit_field, true);
    options.unitDisplay = GetFieldStrById(env, optionsObj, refs::g_unitDisplay_field, true);
    options.useGrouping = GetFieldStrById(env, optionsObj, refs::g_useGrouping_field, true);
}

}  // namespace ark::ets::stdlib::intl
