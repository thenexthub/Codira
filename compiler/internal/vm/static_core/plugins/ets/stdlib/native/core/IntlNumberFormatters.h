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

#ifndef PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLNUMBERFORMATTERS_H
#define PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLNUMBERFORMATTERS_H

#include <ani.h>
#include <string>
#include "unicode/numberformatter.h"
#include "unicode/numberrangeformatter.h"
#include "unicode/locid.h"
#include <set>

namespace ark::ets::stdlib::intl {

namespace refs {
extern ani_class g_resolvedOptionsClass;

extern ani_field g_locale_field;
extern ani_field g_compactDisplay_field;
extern ani_field g_currencySign_field;
extern ani_field g_currency_field;
extern ani_field g_currencyDisplay_field;
extern ani_field g_minFractionDigits_field;
extern ani_field g_maxFractionDigits_field;
extern ani_field g_minSignificantDigits_field;
extern ani_field g_maxSignificantDigits_field;
extern ani_field g_minIntegerDigits_field;
extern ani_field g_notation_field;
extern ani_field g_numberingSystem_field;
extern ani_field g_signDisplay_field;
extern ani_field g_style_field;
extern ani_field g_unit_field;
extern ani_field g_unitDisplay_field;
extern ani_field g_useGrouping_field;
extern ani_field g_numberFormat_options_field;
}  // namespace refs

constexpr std::string_view COMPACT_DISPLAY_SHORT = "short";
constexpr std::string_view COMPACT_DISPLAY_LONG = "long";
constexpr std::string_view CURRENCY_SIGN_ACCOUNTING = "accounting";
constexpr std::string_view CURRENCY_SIGN_STANDARD = "standard";
constexpr std::string_view CURRENCY_DISPLAY_CODE = "code";
constexpr std::string_view CURRENCY_DISPLAY_SYMBOL = "symbol";
constexpr std::string_view CURRENCY_DISPLAY_NARROWSYMBOL = "narrowSymbol";
constexpr std::string_view CURRENCY_DISPLAY_NAME = "name";
constexpr std::string_view NOTATION_STANDARD = "standard";
constexpr std::string_view NOTATION_SCIENTIFIC = "scientific";
constexpr std::string_view NOTATION_ENGINEERING = "engineering";
constexpr std::string_view NOTATION_COMPACT = "compact";
constexpr std::string_view SIGN_DISPLAY_AUTO = "auto";
constexpr std::string_view SIGN_DISPLAY_NEVER = "never";
constexpr std::string_view SIGN_DISPLAY_ALWAYS = "always";
constexpr std::string_view SIGN_DISPLAY_EXCEPTZERO = "exceptZero";
constexpr std::string_view STYLE_DECIMAL = "decimal";
constexpr std::string_view STYLE_PERCENT = "percent";
constexpr std::string_view STYLE_CURRENCY = "currency";
constexpr std::string_view STYLE_UNIT = "unit";
constexpr std::string_view UNIT_DISPLAY_SHORT = "short";
constexpr std::string_view UNIT_DISPLAY_LONG = "long";
constexpr std::string_view UNIT_DISPLAY_NARROW = "narrow";
constexpr std::string_view USE_GROUPING_TRUE = "true";
constexpr std::string_view USE_GROUPING_FALSE = "false";
constexpr std::string_view USE_GROUPING_MIN2 = "min2";
constexpr int32_t STYLE_PERCENT_SCALE_FACTOR = 2;
constexpr int32_t PER_UNIT_STR_SIZE = 5;
constexpr const char *UNDEFINED_STR = "_";

inline std::string IsUndefinedStr(const std::string &str)
{
    return str.empty() ? UNDEFINED_STR : str;
}

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
const std::set<std::string> REFERENCE_UNITS({"acre",       "bit",        "byte",
                                             "celsius",    "centimeter", "day",
                                             "degree",     "fahrenheit", "fluid-ounce",
                                             "foot",       "gallon",     "gigabit",
                                             "gigabyte",   "gram",       "hectare",
                                             "hour",       "inch",       "kilobit",
                                             "kilobyte",   "kilogram",   "kilometer",
                                             "liter",      "megabit",    "megabyte",
                                             "meter",      "mile",       "mile-scandinavian",
                                             "millimeter", "milliliter", "millisecond",
                                             "minute",     "month",      "ounce",
                                             "percent",    "petabyte",   "pound",
                                             "second",     "stone",      "terabit",
                                             "terabyte",   "week",       "yard",
                                             "year"});

// NOLINTBEGIN(misc-non-private-member-variables-in-classes)
struct ParsedOptions {
    std::string compactDisplay;
    std::string currencySign;
    std::string currency;
    std::string currencyDisplay;
    std::string locale;
    std::string minFractionDigits;
    std::string maxFractionDigits;
    std::string minSignificantDigits;
    std::string maxSignificantDigits;
    std::string minIntegerDigits;
    std::string notation;
    std::string numberingSystem;
    std::string signDisplay;
    std::string style;
    std::string unit;
    std::string unitDisplay;
    std::string useGrouping;

    [[maybe_unused]] std::string ToString() const
    {
        std::string str;
        str.append("compactDisplay: " + compactDisplay);
        str.append(", currencySign: " + currencySign);
        str.append(", currency: " + currency);
        str.append(", currencyDisplay: " + currencyDisplay);
        str.append(", locale: " + locale);
        str.append(", maxFractionDigits: " + maxFractionDigits);
        str.append(", maxSignificantDigits: " + maxSignificantDigits);
        str.append(", minFractionDigits: " + minFractionDigits);
        str.append(", minIntegerDigits: " + minIntegerDigits);
        str.append(", minSignificantDigits: " + minSignificantDigits);
        str.append(", notation: " + notation);
        str.append(", numberingSystem: " + numberingSystem);
        str.append(", signDisplay: " + signDisplay);
        str.append(", style: " + style);
        str.append(", unit: " + unit);
        str.append(", unitDisplay: " + unitDisplay);
        str.append(", useGrouping: " + useGrouping);
        return str;
    }

    std::string TagString() const
    {
        std::string tag;
        tag.append(IsUndefinedStr(compactDisplay));
        tag.append(IsUndefinedStr(currencySign));
        tag.append(IsUndefinedStr(currency));
        tag.append(IsUndefinedStr(currencyDisplay));
        tag.append(IsUndefinedStr(locale));
        tag.append(IsUndefinedStr(minFractionDigits));
        tag.append(IsUndefinedStr(maxFractionDigits));
        tag.append(IsUndefinedStr(minSignificantDigits));
        tag.append(IsUndefinedStr(maxSignificantDigits));
        tag.append(IsUndefinedStr(minIntegerDigits));
        tag.append(IsUndefinedStr(notation));
        tag.append(IsUndefinedStr(numberingSystem));
        tag.append(IsUndefinedStr(signDisplay));
        tag.append(IsUndefinedStr(style));
        tag.append(IsUndefinedStr(unit));
        tag.append(IsUndefinedStr(unitDisplay));
        tag.append(IsUndefinedStr(useGrouping));
        return tag;
    }
};
// NOLINTEND(misc-non-private-member-variables-in-classes)

using LocNumFmt = icu::number::LocalizedNumberFormatter;
using UnlocNumFmt = icu::number::UnlocalizedNumberFormatter;
using LocNumRangeFmt = icu::number::LocalizedNumberRangeFormatter;

ANI_EXPORT ani_status LocTagToIcuLocale(ani_env *env, const std::string &localeTag, icu::Locale &locale);
ANI_EXPORT ani_status InitNumFormatter(ani_env *env, const ParsedOptions &options, LocNumFmt &fmt);
ANI_EXPORT ani_status InitNumRangeFormatter(ani_env *env, const ParsedOptions &options, LocNumRangeFmt &fmt);
ANI_EXPORT bool IsCorrectUnitIdentifier(const std::string &unit);
ANI_EXPORT void ParseOptions(ani_env *env, ani_object self, ParsedOptions &options);

}  // namespace ark::ets::stdlib::intl

#endif  //  PANDA_PLUGINS_ETS_STDLIB_NATIVE_CORE_INTLNUMBERFORMATTERS_H
