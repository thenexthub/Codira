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

#include <unicode/unistr.h>
#include <unicode/locid.h>
#include <unicode/reldatefmt.h>
#include <unicode/udat.h>
#include <unicode/localematcher.h>
#include <unicode/numfmt.h>
#include <unicode/utypes.h>
#include <set>
#include <algorithm>
#include <array>
#include "libarkbase/macros.h"
#include "plugins/ets/stdlib/native/core/IntlRelativeTimeFormat.h"
#include "plugins/ets/stdlib/native/core/stdlib_ani_helpers.h"
#include "plugins/ets/stdlib/native/core/IntlCommon.h"
#include "plugins/ets/stdlib/native/core/IntlLocaleMatch.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "ani/ani_checkers.h"
#include "plugins/ets/stdlib/native/core/IntlState.h"
#include "plugins/ets/stdlib/native/core/IntlRelativeTimeFormatCache.h"

namespace ark::ets::stdlib::intl {

typedef enum { AUTO, ALWAYS } NumericOption;

static bool IsUndefined(ani_env *env, ani_ref ref)
{
    ani_boolean isUndefined = ANI_FALSE;
    [[maybe_unused]] auto status = env->Reference_IsUndefined(ref, &isUndefined);
    ASSERT(status == ANI_OK);
    return isUndefined == ANI_TRUE;
}

static ani_status ThrowError(ani_env *env, const char *errorClassName, const std::string &message)
{
    ani_status aniStatus = ANI_OK;

    ani_class errorClass = nullptr;
    aniStatus = env->FindClass(errorClassName, &errorClass);
    ANI_CHECK_RETURN_IF_NE(aniStatus, ANI_OK, aniStatus);

    ani_method errorCtor;
    aniStatus = env->Class_FindMethod(errorClass, "<ctor>", "C{std.core.String}:", &errorCtor);
    ANI_CHECK_RETURN_IF_NE(aniStatus, ANI_OK, aniStatus);

    ani_string errorMsg;
    aniStatus = env->String_NewUTF8(message.data(), message.size(), &errorMsg);
    ANI_CHECK_RETURN_IF_NE(aniStatus, ANI_OK, aniStatus);

    ani_object errorObj;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    aniStatus = env->Object_New(errorClass, errorCtor, &errorObj, errorMsg);
    ANI_CHECK_RETURN_IF_NE(aniStatus, ANI_OK, aniStatus);

    return env->ThrowError(static_cast<ani_error>(errorObj));
}

static ani_status ThrowRangeError(ani_env *env, const std::string &message)
{
    ani_class errorClass = nullptr;
    if (env->FindClass("std.core.RangeError", &errorClass) != ANI_OK) {
        return ANI_PENDING_ERROR;
    }
    ani_method ctor;
    if (env->Class_FindMethod(errorClass, "<ctor>", "C{std.core.String}:", &ctor) != ANI_OK) {
        return ANI_PENDING_ERROR;
    }
    ani_string msgStr;
    if (env->String_NewUTF8(message.c_str(), message.length(), &msgStr) != ANI_OK) {
        return ANI_PENDING_ERROR;
    }
    ani_object err;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    if (env->Object_New(errorClass, ctor, &err, msgStr) != ANI_OK) {
        return ANI_PENDING_ERROR;
    }
    return env->ThrowError(static_cast<ani_error>(err));
}

static ani_status ThrowInternalError(ani_env *env, const std::string &message)
{
    return ThrowError(env, "std.core.InternalError", message);
}

static std::unique_ptr<icu::Locale> ToIcuLocale(ani_env *env, ani_object self)
{
    ani_ref localeRef = nullptr;
    ani_status aniStatus = env->Object_GetFieldByName_Ref(self, "locale", &localeRef);
    if (aniStatus != ANI_OK || IsUndefined(env, localeRef)) {
        return std::make_unique<icu::Locale>(icu::Locale::getDefault());
    }

    auto localeStr = static_cast<ani_string>(localeRef);

    ani_boolean isUndefined = ANI_FALSE;
    aniStatus = env->Reference_IsUndefined(localeStr, &isUndefined);
    if (aniStatus != ANI_OK || isUndefined == ANI_TRUE) {
        return std::make_unique<icu::Locale>(icu::Locale::getDefault());
    }

    ani_size strSize = 0;
    env->String_GetUTF8Size(localeStr, &strSize);
    std::vector<char> buf(strSize + 1);
    ani_size copied = 0;
    env->String_GetUTF8(localeStr, buf.data(), buf.size(), &copied);

    icu::Locale locale(buf.data());
    if (locale.isBogus() != 0) {
        ThrowRangeError(env, "Invalid locale: " + std::string(buf.data()));
        return nullptr;
    }

    return std::make_unique<icu::Locale>(locale);
}

static URelativeDateTimeUnit ToICUUnitOrThrow(ani_env *env, const std::string &unitStr)
{
    std::string unit = unitStr;
    std::transform(unit.begin(), unit.end(), unit.begin(), ::tolower);

    if (!unit.empty() && unit.back() == 's') {
        unit.pop_back();
    }

    if (unit == "second") {
        return UDAT_REL_UNIT_SECOND;
    }
    if (unit == "minute") {
        return UDAT_REL_UNIT_MINUTE;
    }
    if (unit == "hour") {
        return UDAT_REL_UNIT_HOUR;
    }
    if (unit == "day") {
        return UDAT_REL_UNIT_DAY;
    }
    if (unit == "week") {
        return UDAT_REL_UNIT_WEEK;
    }
    if (unit == "month") {
        return UDAT_REL_UNIT_MONTH;
    }
    if (unit == "quarter") {
        return UDAT_REL_UNIT_QUARTER;
    }
    if (unit == "year") {
        return UDAT_REL_UNIT_YEAR;
    }
    ThrowNewError(env, "std.core.RuntimeError", ("Invalid unit: " + unitStr).c_str(), "C{std.core.String}:");
    return static_cast<URelativeDateTimeUnit>(-1);
}

static ani_string StdCoreIntlGetLocale(ani_env *env, ani_object self)
{
    std::unique_ptr<icu::Locale> icuLocale = ToIcuLocale(env, self);
    if (!icuLocale) {
        return nullptr;
    }
    UErrorCode status = U_ZERO_ERROR;
    auto langTagStr = icuLocale->toLanguageTag<std::string>(status);
    if (U_FAILURE(status) != 0) {
        ThrowInternalError(env, "failed to get locale lang tag: " + std::string(u_errorName(status)));
        return nullptr;
    }
    return CreateUtf8String(env, langTagStr.c_str(), langTagStr.length());
}

static UDateRelativeDateTimeFormatterStyle ToRelativeTimeFormatStyle(ani_env *env, ani_object self)
{
    ani_ref optionRef = nullptr;
    ani_status aniOptionStatus = env->Object_GetFieldByName_Ref(self, "options", &optionRef);
    if (aniOptionStatus != ANI_OK || IsUndefined(env, optionRef)) {
        return UDAT_STYLE_LONG;
    }
    ani_ref styleRef = nullptr;
    ani_object optionObject = static_cast<ani_object>(optionRef);
    ani_status aniStyle = env->Object_GetFieldByName_Ref(optionObject, "_style", &styleRef);
    UDateRelativeDateTimeFormatterStyle uStyle;
    if (aniStyle != ANI_OK || IsUndefined(env, styleRef)) {
        uStyle = UDAT_STYLE_LONG;
    } else {
        ani_string styleStr = static_cast<ani_string>(styleRef);
        ani_size styleSize = 0;
        env->String_GetUTF8Size(styleStr, &styleSize);
        std::vector<char> styleBuf(styleSize + 1);
        ani_size copiedChars = 0;
        env->String_GetUTF8(styleStr, styleBuf.data(), styleBuf.size(), &copiedChars);
        std::string styleOptionStr(styleBuf.data());
        std::transform(styleOptionStr.begin(), styleOptionStr.end(), styleOptionStr.begin(), ::tolower);
        if (styleOptionStr == "long") {
            uStyle = UDAT_STYLE_LONG;
        } else if (styleOptionStr == "short") {
            uStyle = UDAT_STYLE_SHORT;
        } else if (styleOptionStr == "narrow") {
            uStyle = UDAT_STYLE_NARROW;
        } else {
            uStyle = UDAT_STYLE_LONG;
        }
    }
    return uStyle;
}

static NumericOption GetNumericOption(ani_env *env, ani_object self)
{
    ani_ref optionRef = nullptr;
    ani_status aniOptionStatus = env->Object_GetFieldByName_Ref(self, "options", &optionRef);
    if (aniOptionStatus != ANI_OK || IsUndefined(env, optionRef)) {
        return NumericOption::ALWAYS;
    }
    ani_ref numericRef = nullptr;
    ani_object optionObject = static_cast<ani_object>(optionRef);
    ani_status aniNumeric = env->Object_GetFieldByName_Ref(optionObject, "_numeric", &numericRef);
    if (aniNumeric != ANI_OK || IsUndefined(env, numericRef)) {
        return NumericOption::ALWAYS;
    }
    ani_string str = static_cast<ani_string>(numericRef);
    ani_size strSize = 0;
    env->String_GetUTF8Size(str, &strSize);
    std::vector<char> buf(strSize + 1);
    ani_size copiedChars = 0;
    env->String_GetUTF8(str, buf.data(), buf.size(), &copiedChars);
    std::string numericOptionStr(buf.data());
    std::transform(numericOptionStr.begin(), numericOptionStr.end(), numericOptionStr.begin(), ::tolower);
    if (numericOptionStr == "auto") {
        return NumericOption::AUTO;
    } else if (numericOptionStr == "always") {
        return NumericOption::ALWAYS;
    } else {
        // throw RangeError for invalid numeric option
        ThrowRangeError(env, "Invalid numeric option: " + numericOptionStr);
        return static_cast<NumericOption>(-1);
    }
}

static ani_string StdCoreIntlRelativeTimeFormatFormatImpl(ani_env *env, ani_object self, ani_double value,
                                                          ani_string unit)
{
    std::unique_ptr<icu::Locale> icuLocale = ToIcuLocale(env, self);
    if (!icuLocale) {
        return nullptr;
    }
    UDateRelativeDateTimeFormatterStyle uStyle = ToRelativeTimeFormatStyle(env, self);

    // Generate Cache Key
    std::string styleStr = (uStyle == UDAT_STYLE_LONG) ? "long" : (uStyle == UDAT_STYLE_SHORT) ? "short" : "narrow";
    std::string cacheKey = std::string(icuLocale->getName()) + ";" + styleStr;

    icu::RelativeDateTimeFormatter *formatter =
        g_intlState->relativeTimeFormatCache.GetOrCreateFormatter(cacheKey, *icuLocale, uStyle);

    if (formatter == nullptr) {
        ThrowRangeError(env, "Failed to get RelativeDateTimeFormatter from cache");
        return nullptr;
    }

    // Use the obtained formatter to perform formatting
    UErrorCode status = U_ZERO_ERROR;
    std::string unitStr = ConvertFromAniString(env, unit);
    URelativeDateTimeUnit icuUnit = ToICUUnitOrThrow(env, unitStr);
    if (icuUnit == static_cast<URelativeDateTimeUnit>(-1)) {
        return nullptr;
    }

    NumericOption numericOption = GetNumericOption(env, self);
    if (numericOption == static_cast<NumericOption>(-1)) {
        return nullptr;
    }

    icu::FormattedRelativeDateTime formatted;
    if (numericOption == NumericOption::AUTO) {
        formatted = formatter->formatToValue(value, icuUnit, status);
    } else {
        formatted = formatter->formatNumericToValue(value, icuUnit, status);
    }

    if (U_FAILURE(status)) {
        ThrowRangeError(env, "RelativeDateTimeFormatter format failed");
        return nullptr;
    }

    icu::UnicodeString result = formatted.toString(status);
    if (U_FAILURE(status)) {
        ThrowRangeError(env, "FormattedRelativeDateTime toString failed");
        return nullptr;
    }

    ani_string output;
    env->String_NewUTF16(reinterpret_cast<const uint16_t *>(result.getBuffer()), result.length(), &output);
    return output;
}

ani_status RegisterIntlRelativeTimeFormatMethods(ani_env *env)
{
    std::array methods = {
        ani_native_function {"formatImpl", "dC{std.core.String}:C{std.core.String}",
                             reinterpret_cast<void *>(StdCoreIntlRelativeTimeFormatFormatImpl)},
        ani_native_function {"getLocale", ":C{std.core.String}", reinterpret_cast<void *>(StdCoreIntlGetLocale)}};
    ani_class rtfClass;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.Intl.RelativeTimeFormat", &rtfClass));

    return env->Class_BindNativeMethods(rtfClass, methods.data(), methods.size());
}

}  // namespace ark::ets::stdlib::intl
