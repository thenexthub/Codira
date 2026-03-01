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

#include "IntlPluralRules.h"
#include "IntlCommon.h"
#include "libarkbase/macros.h"
#include "stdlib_ani_helpers.h"

#include "unicode/locid.h"
#include "unicode/numberformatter.h"
#include "unicode/plurrule.h"

#include <array>
#include <cassert>
#include <sstream>
#include <string>
#include <tuple>

namespace ark::ets::stdlib::intl {

using SelectOptions = std::tuple<std::string, std::string, double, double, double, double, double>;

namespace {
ani_field g_localeField = nullptr;
ani_field g_typeField = nullptr;
ani_field g_minimumIntegerDigitsField = nullptr;
ani_field g_minimumFractionDigitsField = nullptr;
ani_field g_maximumFractionDigitsField = nullptr;
ani_field g_minimumSignificantDigitsField = nullptr;
ani_field g_maximumSignificantDigitsField = nullptr;
}  // namespace

static void CollectSelectOptionsFields(ani_env *env)
{
    ani_class optionsClass;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.Intl.PluralRulesSelectOptionsImpl", &optionsClass));

    ANI_FATAL_IF_ERROR(env->Class_FindField(optionsClass, "locale_", &g_localeField));
    ANI_FATAL_IF_ERROR(env->Class_FindField(optionsClass, "type_", &g_typeField));
    ANI_FATAL_IF_ERROR(env->Class_FindField(optionsClass, "minimumIntegerDigits_", &g_minimumIntegerDigitsField));
    ANI_FATAL_IF_ERROR(env->Class_FindField(optionsClass, "minimumFractionDigits_", &g_minimumFractionDigitsField));
    ANI_FATAL_IF_ERROR(env->Class_FindField(optionsClass, "maximumFractionDigits_", &g_maximumFractionDigitsField));
    ANI_FATAL_IF_ERROR(
        env->Class_FindField(optionsClass, "minimumSignificantDigits_", &g_minimumSignificantDigitsField));
    ANI_FATAL_IF_ERROR(
        env->Class_FindField(optionsClass, "maximumSignificantDigits_", &g_maximumSignificantDigitsField));
}

template <typename... Args>
static void ThrowRangeError(ani_env *env, Args &&...args)
{
    std::stringstream message;
    (message << ... << args);
    ThrowNewError(env, "std.core.RangeError", message.str().c_str(), "C{std.core.String}:");
}

static icu::Locale GetLocale(const std::string &tag)
{
    auto status = UErrorCode::U_ZERO_ERROR;
    auto locale = icu::Locale::forLanguageTag(tag, status);
    if (UNLIKELY(U_FAILURE(status))) {
        return icu::Locale::getDefault();
    }
    return locale;
}

static std::unique_ptr<icu::PluralRules> GetPluralRules(ani_env *env, const icu::Locale &locale, UPluralType pluralType)
{
    auto status = UErrorCode::U_ZERO_ERROR;
    auto rules = std::unique_ptr<icu::PluralRules>(icu::PluralRules::forLocale(locale, pluralType, status));
    if (UNLIKELY(U_FAILURE(status))) {
        ThrowRangeError(env, "Error creating icu::PluralRules: ", u_errorName(status));
        return nullptr;
    }
    return rules;
}

static SelectOptions ExtractOptions(ani_env *env, ani_object options)
{
    ani_ref localeRef;
    ANI_FATAL_IF_ERROR(env->Object_GetField_Ref(options, g_localeField, &localeRef));
    auto locale = ConvertFromAniString(env, reinterpret_cast<ani_string>(localeRef));

    ani_ref typeRef;
    ANI_FATAL_IF_ERROR(env->Object_GetField_Ref(options, g_typeField, &typeRef));
    auto type = ConvertFromAniString(env, reinterpret_cast<ani_string>(typeRef));

    ani_double minimumIntegerDigits;
    ANI_FATAL_IF_ERROR(env->Object_GetField_Double(options, g_minimumIntegerDigitsField, &minimumIntegerDigits));

    ani_double minimumFractionDigits;
    ANI_FATAL_IF_ERROR(env->Object_GetField_Double(options, g_minimumFractionDigitsField, &minimumFractionDigits));

    ani_double maximumFractionDigits;
    ANI_FATAL_IF_ERROR(env->Object_GetField_Double(options, g_maximumFractionDigitsField, &maximumFractionDigits));

    ani_double minimumSignificantDigits;
    ANI_FATAL_IF_ERROR(
        env->Object_GetField_Double(options, g_minimumSignificantDigitsField, &minimumSignificantDigits));

    ani_double maximumSignificantDigits;
    ANI_FATAL_IF_ERROR(
        env->Object_GetField_Double(options, g_maximumSignificantDigitsField, &maximumSignificantDigits));

    return {locale,
            type,
            minimumIntegerDigits,
            minimumFractionDigits,
            maximumFractionDigits,
            minimumSignificantDigits,
            maximumSignificantDigits};
}

ani_string IcuPluralSelect(ani_env *env, [[maybe_unused]] ani_class klass, ani_double value, ani_object options)
{
    CollectSelectOptionsFields(env);

    const auto [localeStr, typeStr, minimumIntegerDigits, minimumFractionDigits, maximumFractionDigits,
                minimumSignificantDigits, maximumSignificantDigits] = ExtractOptions(env, options);

    auto locale = GetLocale(localeStr);
    auto type = typeStr == "ordinal" ? UPluralType::UPLURAL_TYPE_ORDINAL : UPluralType::UPLURAL_TYPE_CARDINAL;
    auto rules = GetPluralRules(env, locale, type);
    if (UNLIKELY(rules == nullptr)) {
        return nullptr;
    }

    auto fractionPrecision = icu::number::Precision::minMaxFraction(minimumFractionDigits, maximumFractionDigits);
    auto formatter = icu::number::NumberFormatter::withLocale(locale)
                         .precision(fractionPrecision)
                         .integerWidth(icu::number::IntegerWidth::zeroFillTo(minimumIntegerDigits));
    if (minimumSignificantDigits > 0) {
        formatter = formatter.precision(icu::number::Precision::minSignificantDigits(minimumSignificantDigits));
    }
    if (maximumSignificantDigits > 0) {
        formatter = formatter.precision(icu::number::Precision::maxSignificantDigits(maximumSignificantDigits));
    }
    auto status = UErrorCode::U_ZERO_ERROR;
    auto formatted = formatter.formatDouble(value, status);
    if (UNLIKELY(U_FAILURE(status))) {
        ThrowRangeError(env, "Failed to format value: ", u_errorName(status));
        return nullptr;
    }

    auto category = rules->select(formatted, status);
    if (UNLIKELY(U_FAILURE(status))) {
        ThrowRangeError(env, "Failed to select plural rules: ", u_errorName(status));
        return nullptr;
    }

    return UnicodeToAniStr(env, category);
}

ani_object IcuGetPluralCategories(ani_env *env, [[maybe_unused]] ani_class klass, ani_string localeTag,
                                  ani_string pluralType)
{
    auto tag = ConvertFromAniString(env, localeTag);
    auto locale = GetLocale(tag);
    auto type = ConvertFromAniString(env, pluralType) == "ordinal" ? UPluralType::UPLURAL_TYPE_ORDINAL
                                                                   : UPluralType::UPLURAL_TYPE_CARDINAL;
    auto rules = GetPluralRules(env, locale, type);
    if (UNLIKELY(rules == nullptr)) {
        return nullptr;
    }

    auto status = UErrorCode::U_ZERO_ERROR;
    auto enumCategories = std::unique_ptr<icu::StringEnumeration>(rules->getKeywords(status));
    if (UNLIKELY(U_FAILURE(status))) {
        ThrowRangeError(env, "Failed to get plural categories: ", u_errorName(status));
        return nullptr;
    }

    std::vector<std::string> categories;

    auto category = enumCategories->snext(status);
    if (UNLIKELY(U_FAILURE(status))) {
        ThrowRangeError(env, "Failed to iterate plural category: ", u_errorName(status));
        return nullptr;
    }
    while (category != nullptr) {
        std::string str;
        category->toUTF8String(str);
        categories.push_back(str);

        category = enumCategories->snext(status);
        if (UNLIKELY(U_FAILURE(status))) {
            ThrowRangeError(env, "Failed to iterate plural category: ", u_errorName(status));
            return nullptr;
        }
    }

    if (UNLIKELY(categories.empty())) {
        ThrowRangeError(env, "No plural categories found");
        return nullptr;
    }

    auto first = StdStrToAni(env, categories[0]);
    ani_class stringClass;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.String", &stringClass));
    ani_fixedarray_ref array;
    ANI_FATAL_IF_ERROR(env->FixedArray_New_Ref(stringClass, categories.size(), first, &array));
    for (size_t i = 1; i < categories.size(); ++i) {
        auto item = StdStrToAni(env, categories[i]);
        ANI_FATAL_IF_ERROR(env->FixedArray_Set_Ref(array, i, item));
    }
    return array;
}

ani_status RegisterIntlPluralRules(ani_env *env)
{
    std::array methods = {ani_native_function {"selectDouble", "dC{std.core.Object}:C{std.core.String}",
                                               reinterpret_cast<void *>(IcuPluralSelect)},
                          ani_native_function {"getPluralCategories",
                                               "C{std.core.String}C{std.core.String}:C{std.core.Object}",
                                               reinterpret_cast<void *>(IcuGetPluralCategories)}};

    ani_class pluralRulesClass;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.Intl.PluralRules", &pluralRulesClass));

    return env->Class_BindStaticNativeMethods(pluralRulesClass, methods.data(), methods.size());
}

}  // namespace ark::ets::stdlib::intl
