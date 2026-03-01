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

#include <array>
#include <vector>
#include <cstring>
#include <regex>

#include "unicode/locid.h"
#include "unicode/unistr.h"
#include "unicode/datefmt.h"
#include "unicode/dtptngen.h"
#include "unicode/smpdtfmt.h"
#include "unicode/msgfmt.h"
#include <unicode/dtitvfmt.h>
#include <unicode/numsys.h>

#include "libarkbase/macros.h"
#include "plugins/ets/stdlib/native/core/IntlDateTimeFormat.h"
#include "plugins/ets/stdlib/native/core/IntlDateTimeFormatCache.h"
#include "plugins/ets/stdlib/native/core/IntlState.h"

#include "stdlib_ani_helpers.h"

namespace ark::ets::stdlib::intl {
constexpr const char *FORMAT_HOUR_SYMBOLS = "hHkK";
constexpr const char *FORMAT_AM_PM_SYMBOLS = "a";

constexpr const char *OPTIONS_FIELD_HOUR_CYCLE = "hourCycle_";
constexpr const char *OPTIONS_PROPERTY_HOUR_CYCLE = "hourCycle";

constexpr const char *LOCALE_KEYWORD_HOUR_CYCLE = "hours";
constexpr const char *LOCALE_KEYWORD_CALENDAR = "calendar";
constexpr const char *HOUR_CYCLE_11 = "h11";
constexpr const char *HOUR_CYCLE_12 = "h12";
constexpr const char *HOUR_CYCLE_23 = "h23";
constexpr const char *HOUR_CYCLE_24 = "h24";

constexpr const char *STYLE_FULL = "full";
constexpr const char *STYLE_LONG = "long";
constexpr const char *STYLE_MEDIUM = "medium";
constexpr const char *STYLE_SHORT = "short";

constexpr const char *ERROR_CTOR_SIGNATURE = "C{std.core.String}:";
constexpr const char *DTF_PART_IMPL_CTOR_SIGNATURE = "C{std.core.String}C{std.core.String}:";
constexpr const char *DTRF_PART_IMPL_CTOR_SIGNATURE = "C{std.core.String}C{std.core.String}C{std.core.String}:";
constexpr const char *RESOLVED_OPTS_CTOR_SIGNATURE =
    "C{std.core.String}C{std.core.String}C{std.core.String}C{std.core.String}:";

constexpr const char *DTF_PART_LITERAL_TYPE = "literal";
constexpr const char *DTRF_PART_SOURCE_SHARED = "shared";

constexpr std::array<const char *, 25> DTF_PART_TYPES = {"era",
                                                         "year",
                                                         "month",
                                                         "day",
                                                         "hourDay1",
                                                         "hourDay0",
                                                         "minute",
                                                         "second",
                                                         "fractionalSecond",
                                                         "weekday",
                                                         "dayOfYear",
                                                         "dayOfWeekInMonth",
                                                         "weekOfYear",
                                                         "weekOfMonth",
                                                         "dayPeriod",
                                                         "hour",
                                                         "hour0",
                                                         "timeZoneName",
                                                         "yearWoy",
                                                         "dowLocal",
                                                         "extYear",
                                                         "julianDay",
                                                         "msInDay",
                                                         "timezone",
                                                         "timezoneGeneric"};

constexpr std::array<const char *, 3> DTRF_PART_SOURCES = {"startRange", "endRange"};

static void ThrowRangeError(ani_env *env, const std::string &message)
{
    ani_class errorClass = nullptr;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.RangeError", &errorClass));

    ThrowNewError(env, errorClass, message, ERROR_CTOR_SIGNATURE);
}

static std::nullptr_t ThrowInternalError(ani_env *env, const std::string &message)
{
    ani_class errorClass = nullptr;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.InternalError", &errorClass));

    ThrowNewError(env, errorClass, message, ERROR_CTOR_SIGNATURE);
    return nullptr;
}

static std::unique_ptr<icu::Locale> ToICULocale(ani_env *env, ani_object self)
{
    ani_ref localeRef = nullptr;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Ref(self, "locale", &localeRef));

    ani_boolean localeIsUndefined = ANI_FALSE;
    ANI_FATAL_IF_ERROR(env->Reference_IsUndefined(localeRef, &localeIsUndefined));
    ANI_FATAL_IF(localeIsUndefined == ANI_TRUE);

    auto locale = static_cast<ani_string>(localeRef);
    ASSERT(locale != nullptr);

    std::string localeStr = ConvertFromAniString(env, locale);

    UErrorCode status = U_ZERO_ERROR;
    auto icuLocale = std::make_unique<icu::Locale>(icu::Locale::forLanguageTag(localeStr.data(), status));
    if (U_FAILURE(status) == TRUE) {
        return ThrowInternalError(env, std::string("failed to create locale by lang tag: ") + u_errorName(status));
    }

    return icuLocale;
}

static icu::DateFormat::EStyle ToICUDateTimeStyle(ani_env *env, ani_string style)
{
    std::string styleStr = ConvertFromAniString(env, style);
    if (styleStr == STYLE_FULL) {
        return icu::DateFormat::FULL;
    }
    if (styleStr == STYLE_LONG) {
        return icu::DateFormat::LONG;
    }
    if (styleStr == STYLE_MEDIUM) {
        return icu::DateFormat::MEDIUM;
    }
    if (styleStr == STYLE_SHORT) {
        return icu::DateFormat::SHORT;
    }

    return icu::DateFormat::NONE;
}

static std::unique_ptr<icu::UnicodeString> UnicodeStringFromAniString(ani_env *env, ani_string str)
{
    ani_boolean strUndef = ANI_FALSE;
    ANI_FATAL_IF_ERROR(env->Reference_IsUndefined(str, &strUndef));

    if (strUndef == ANI_TRUE) {
        return nullptr;
    }

    ASSERT(str != nullptr);

    ani_size strSize = 0;
    ANI_FATAL_IF_ERROR(env->String_GetUTF16Size(str, &strSize));

    std::vector<uint16_t> strBuf(strSize + 1);

    ani_size charsCount = 0;
    ANI_FATAL_IF_ERROR(env->String_GetUTF16(str, strBuf.data(), strBuf.size(), &charsCount));
    ASSERT(charsCount == strSize);

    return std::make_unique<icu::UnicodeString>(strBuf.data(), strSize);
}

static ani_status DateFormatSetTimeZone(ani_env *env, icu::DateFormat *dateFormat, ani_object options)
{
    ani_ref timeZoneRef = nullptr;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Ref(options, "timeZone_", &timeZoneRef));

    auto timeZone = static_cast<ani_string>(timeZoneRef);
    std::unique_ptr<icu::UnicodeString> timeZoneId = UnicodeStringFromAniString(env, timeZone);
    if (!timeZoneId) {
        return ANI_OK;
    }

    std::unique_ptr<icu::TimeZone> formatTimeZone(icu::TimeZone::createTimeZone(*timeZoneId));
    if (*formatTimeZone == icu::TimeZone::getUnknown()) {
        std::string invalidTimeZoneId;
        timeZoneId->toUTF8String(invalidTimeZoneId);

        ThrowRangeError(env, "Invalid time zone specified: " + invalidTimeZoneId);
        return ANI_PENDING_ERROR;
    }

    dateFormat->adoptTimeZone(formatTimeZone.release());

    return ANI_OK;
}

static ani_string GetLocaleHourCycle(ani_env *env, ani_object self)
{
    ani_ref localeRef = nullptr;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Ref(self, "locale", &localeRef));
    std::string localeStr = ConvertFromAniString(env, static_cast<ani_string>(localeRef));
    {
        os::memory::LockHolder lh(g_intlState->hourCycleMutex);
        auto it = g_intlState->hourCycleCache.find(localeStr);
        if (it != g_intlState->hourCycleCache.end()) {
            const std::string &cachedHourCycle = it->second;
            return CreateUtf8String(env, cachedHourCycle.c_str(), cachedHourCycle.length());
        }
    }

    std::unique_ptr<icu::Locale> icuLocale = ToICULocale(env, self);
    if (!icuLocale) {
        return nullptr;
    }

    UErrorCode icuStatus = U_ZERO_ERROR;
    std::unique_ptr<icu::DateTimePatternGenerator> hourCycleResolver(
        icu::DateTimePatternGenerator::createInstance(*icuLocale, icuStatus));

    if (U_FAILURE(icuStatus) == TRUE) {
        return ThrowInternalError(env, "Locale hour cycle resolver instance creation failed");
    }

    UDateFormatHourCycle localeHourCycle = hourCycleResolver->getDefaultHourCycle(icuStatus);
    if (U_FAILURE(icuStatus) == TRUE) {
        return ThrowInternalError(env, "Failed to resolve locale hour cycle");
    }

    const char *resolvedHourCycle = nullptr;
    switch (localeHourCycle) {
        case UDAT_HOUR_CYCLE_11:
            resolvedHourCycle = HOUR_CYCLE_11;
            break;
        case UDAT_HOUR_CYCLE_12:
            resolvedHourCycle = HOUR_CYCLE_12;
            break;
        case UDAT_HOUR_CYCLE_23:
            resolvedHourCycle = HOUR_CYCLE_23;
            break;
        case UDAT_HOUR_CYCLE_24:
            resolvedHourCycle = HOUR_CYCLE_24;
            break;
    }

    if (resolvedHourCycle == nullptr) {
        return ThrowInternalError(env, "Failed to resolve locale hour cycle");
    }

    {
        os::memory::LockHolder lh(g_intlState->hourCycleMutex);
        g_intlState->hourCycleCache[localeStr] = resolvedHourCycle;
    }

    return CreateUtf8String(env, resolvedHourCycle, strlen(resolvedHourCycle));
}

static std::unique_ptr<icu::DateFormat> CreateSkeletonBasedDateFormat(ani_env *env, const icu::Locale &locale,
                                                                      const icu::UnicodeString &skeleton)
{
    UErrorCode icuStatus = U_ZERO_ERROR;

    std::unique_ptr<icu::DateTimePatternGenerator> dateTimePatternGen(
        icu::DateTimePatternGenerator::createInstance(locale, icuStatus));

    if (U_FAILURE(icuStatus) == TRUE) {
        return ThrowInternalError(env, "DateTimePatternGenerator instance creation failed");
    }

    icu::UnicodeString datePattern = dateTimePatternGen->getBestPattern(
        skeleton, UDateTimePatternMatchOptions::UDATPG_MATCH_HOUR_FIELD_LENGTH, icuStatus);

    if (U_FAILURE(icuStatus) == TRUE) {
        return ThrowInternalError(env, "DateFormat pattern creation failed");
    }

    auto icuDateFormat = std::make_unique<icu::SimpleDateFormat>(datePattern, locale, icuStatus);
    if (U_FAILURE(icuStatus) == TRUE) {
        return ThrowInternalError(env, "DateFormat instance creation failed");
    }

    return icuDateFormat;
}

static ani_status ConfigureLocaleCalendar(ani_env *env, icu::Locale *locale, ani_object options)
{
    ani_ref calendarRef = nullptr;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Ref(options, "calendar_", &calendarRef));

    ani_boolean calendarUndefined = ANI_FALSE;
    ANI_FATAL_IF_ERROR(env->Reference_IsUndefined(calendarRef, &calendarUndefined));
    if (calendarUndefined == TRUE) {
        return ANI_OK;
    }

    auto calendar = static_cast<ani_string>(calendarRef);
    std::string calendarStr = ConvertFromAniString(env, calendar);

    UErrorCode icuStatus = U_ZERO_ERROR;
    locale->setKeywordValue(LOCALE_KEYWORD_CALENDAR, calendarStr.data(), icuStatus);
    if (U_FAILURE(icuStatus) == TRUE) {
        ThrowInternalError(env, std::string("failed to set locale 'calendar' keyword: ") + u_errorName(icuStatus));
        return ANI_PENDING_ERROR;
    }

    return ANI_OK;
}

static ani_status ConfigureLocaleOptions(ani_env *env, icu::Locale *locale, ani_object options)
{
    ani_status aniStatus = ConfigureLocaleCalendar(env, locale, options);
    if (aniStatus != ANI_OK) {
        if (aniStatus != ANI_PENDING_ERROR) {
            ThrowInternalError(env, "failed to configure locale calendar!");
        }

        return ANI_PENDING_ERROR;
    }

    ani_ref hourCycleRef = nullptr;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Ref(options, OPTIONS_FIELD_HOUR_CYCLE, &hourCycleRef));

    ani_boolean hourCycleUndefined = ANI_FALSE;
    ANI_FATAL_IF_ERROR(env->Reference_IsUndefined(hourCycleRef, &hourCycleUndefined));

    UErrorCode icuStatus = U_ZERO_ERROR;
    if (hourCycleUndefined == ANI_TRUE) {
        if (strcmp(locale->getLanguage(), icu::Locale::getChinese().getLanguage()) != 0) {
            return ANI_OK;
        }

        // applying chinese locale hourCycle fix: h12 -> h23
        locale->setKeywordValue(LOCALE_KEYWORD_HOUR_CYCLE, HOUR_CYCLE_23, icuStatus);
        if (U_FAILURE(icuStatus) == TRUE) {
            ThrowInternalError(env, std::string("failed to fix chinese locale hourCycle: ") + u_errorName(icuStatus));
            return ANI_PENDING_ERROR;
        }

        return ANI_OK;
    }

    ASSERT(hourCycleRef != nullptr);

    auto hourCycle = static_cast<ani_string>(hourCycleRef);
    std::string hourCycleStr = ConvertFromAniString(env, hourCycle);

    locale->setKeywordValue(LOCALE_KEYWORD_HOUR_CYCLE, hourCycleStr.data(), icuStatus);
    if (U_FAILURE(icuStatus) == TRUE) {
        ThrowInternalError(env, std::string("failed to set locale 'hours' keyword: ") + u_errorName(icuStatus));
        return ANI_PENDING_ERROR;
    }

    return ANI_OK;
}

static icu::UnicodeString GetLocaleDateTimeFormat(const icu::Locale &locale,
                                                  const icu::DateTimePatternGenerator &patternGenerator,
                                                  icu::DateFormat::EStyle dateStyle, UErrorCode &icuStatus)
{
    if (locale == icu::Locale::getUS()) {
        // en-LR locale uses 'correct' ( 'at' instead of ',' ) date-time separator
        icu::Locale fixLocale("en-LR");
        std::unique_ptr<icu::DateTimePatternGenerator> fixPatternGen(
            icu::DateTimePatternGenerator::createInstance(fixLocale, icuStatus));
        if (U_FAILURE(icuStatus) == TRUE) {
            return icu::UnicodeString();
        }

        return fixPatternGen->getDateTimeFormat(static_cast<UDateFormatStyle>(dateStyle), icuStatus);
    }

    return patternGenerator.getDateTimeFormat(static_cast<UDateFormatStyle>(dateStyle), icuStatus);
}

static bool RemoveExplicitHourCycleSymbolsFromTimeFormatSkeleton(icu::UnicodeString *skeleton)
{
    std::string skeletonStr;
    skeleton->toUTF8String(skeletonStr);

    bool skeletonUpdated = false;

    // removing AM/PM related symbols from time format skeleton
    std::size_t amPmStartIdx = skeletonStr.find_first_of(FORMAT_AM_PM_SYMBOLS);
    std::size_t amPmEndIdx = skeletonStr.find_last_of(FORMAT_AM_PM_SYMBOLS);
    if (amPmStartIdx != std::string::npos && amPmEndIdx != std::string::npos) {
        skeletonStr.erase(amPmStartIdx, amPmEndIdx - amPmStartIdx + 1);
        skeletonUpdated = true;
    }

    // replacing strict hour format symbols with locale aware 'j'
    std::size_t hourStartIdx = skeletonStr.find_first_of(FORMAT_HOUR_SYMBOLS);
    std::size_t hourEndIdx = skeletonStr.find_last_of(FORMAT_HOUR_SYMBOLS);
    if (hourStartIdx != std::string::npos && hourEndIdx != std::string::npos) {
        auto hourPatternSize = hourEndIdx - hourStartIdx + 1;
        skeletonStr.replace(hourStartIdx, hourPatternSize, hourPatternSize, 'j');
        skeletonUpdated = true;
    }

    if (skeletonUpdated) {
        *skeleton = icu::UnicodeString(skeletonStr.data());
    }

    return skeletonUpdated;
}

static std::unique_ptr<icu::UnicodeString> CreateDateTimeFormatPattern(ani_env *env,
                                                                       const icu::UnicodeString &datePattern,
                                                                       const icu::UnicodeString &timePattern,
                                                                       const icu::UnicodeString &localeDateTimeFormat,
                                                                       const icu::Locale &locale)
{
    std::array formatPatterns {icu::Formattable(timePattern), icu::Formattable(datePattern)};

    UErrorCode icuStatus = U_ZERO_ERROR;
    icu::MessageFormat messageFormat(localeDateTimeFormat, locale, icuStatus);
    if (U_FAILURE(icuStatus) == TRUE) {
        return ThrowInternalError(env, "locale aligned date+time format creation failed");
    }

    // creating combined date + time format pattern
    auto dateTimeFormatPattern = std::make_unique<icu::UnicodeString>();
    icu::FieldPosition fieldPos = 0;
    messageFormat.format(formatPatterns.data(), formatPatterns.size(), *dateTimeFormatPattern, fieldPos, icuStatus);
    if (U_FAILURE(icuStatus) == TRUE) {
        return ThrowInternalError(env, "locale aligned date+time format pattern creation failed");
    }

    return dateTimeFormatPattern;
}

static std::unique_ptr<icu::DateFormat> CreateStyleBasedDateFormatAlignedWithLocale(ani_env *env,
                                                                                    icu::DateFormat::EStyle dateStyle,
                                                                                    icu::DateFormat::EStyle timeStyle,
                                                                                    const icu::Locale &locale)
{
    if (timeStyle == icu::DateFormat::NONE && dateStyle == icu::DateFormat::NONE) {
        return std::unique_ptr<icu::DateFormat>(
            icu::DateFormat::createDateTimeInstance(icu::DateFormat::kShort, icu::DateFormat::kMedium, locale));
    }

    if (timeStyle == icu::DateFormat::NONE) {
        return std::unique_ptr<icu::DateFormat>(icu::DateFormat::createDateInstance(dateStyle, locale));
    }

    std::unique_ptr<icu::DateFormat> timeFormat(icu::DateFormat::createTimeInstance(timeStyle, locale));

    icu::UnicodeString timeFormatPattern;
    static_cast<icu::SimpleDateFormat *>(timeFormat.get())->toPattern(timeFormatPattern);

    UErrorCode icuStatus = U_ZERO_ERROR;

    // transforming time format pattern to more generic time format skeleton
    icu::UnicodeString timeFormatSkeleton =
        icu::DateTimePatternGenerator::staticGetSkeleton(timeFormatPattern, icuStatus);
    if (U_FAILURE(icuStatus) == TRUE) {
        return ThrowInternalError(env, "style format pattern -> style format skeleton transformation failed");
    }

    RemoveExplicitHourCycleSymbolsFromTimeFormatSkeleton(&timeFormatSkeleton);

    // generating updated time format pattern based on updated time format skeleton
    std::unique_ptr<icu::DateTimePatternGenerator> patternGen(
        icu::DateTimePatternGenerator::createInstance(locale, icuStatus));
    if (U_FAILURE(icuStatus) == TRUE) {
        return ThrowInternalError(env, "DateTimePatternGenerator instance creation failed");
    }

    UDateTimePatternMatchOptions matchOpts = UDateTimePatternMatchOptions::UDATPG_MATCH_HOUR_FIELD_LENGTH;
    icu::UnicodeString alignedTimeFormatPattern = patternGen->getBestPattern(timeFormatSkeleton, matchOpts, icuStatus);
    if (U_FAILURE(icuStatus) == TRUE) {
        return ThrowInternalError(env, "locale aligned style based format pattern generation failed");
    }

    if (dateStyle == icu::DateFormat::NONE) {
        // dateStyle is not specified, so returning time pattern only based format
        auto alignedTimeFormat = std::make_unique<icu::SimpleDateFormat>(alignedTimeFormatPattern, locale, icuStatus);
        if (U_FAILURE(icuStatus) == TRUE) {
            return ThrowInternalError(env, "locale aligned style based time formatter creation failed");
        }

        return alignedTimeFormat;
    }

    std::unique_ptr<icu::DateFormat> dateFormat(icu::DateFormat::createDateInstance(dateStyle, locale));

    icu::UnicodeString dateFormatPattern;
    static_cast<icu::SimpleDateFormat *>(dateFormat.get())->toPattern(dateFormatPattern);

    // resolving locale specific date + time format
    const icu::UnicodeString localeDateTimeFormat = GetLocaleDateTimeFormat(locale, *patternGen, dateStyle, icuStatus);
    if (U_FAILURE(icuStatus) == TRUE) {
        return ThrowInternalError(env, "failed to resolve locale specific date+time format");
    }

    std::unique_ptr<icu::UnicodeString> alignedFormatPattern =
        CreateDateTimeFormatPattern(env, dateFormatPattern, alignedTimeFormatPattern, localeDateTimeFormat, locale);
    if (!alignedFormatPattern) {
        return nullptr;
    }

    auto alignedDateTimeFormat = std::make_unique<icu::SimpleDateFormat>(*alignedFormatPattern, locale, icuStatus);
    if (U_FAILURE(icuStatus) == TRUE) {
        return ThrowInternalError(env, "locale aligned style based date+time formatter creation failed");
    }

    return alignedDateTimeFormat;
}

static std::unique_ptr<icu::DateFormat> CreateStyleBasedDateFormat(ani_env *env, const icu::Locale &locale,
                                                                   ani_object options)
{
    ani_ref dateStyleRef = nullptr;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Ref(options, "dateStyle_", &dateStyleRef));

    ani_ref timeStyleRef = nullptr;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Ref(options, "timeStyle_", &timeStyleRef));

    ani_boolean dateStyleUndefined = ANI_FALSE;
    ANI_FATAL_IF_ERROR(env->Reference_IsUndefined(dateStyleRef, &dateStyleUndefined));

    icu::DateFormat::EStyle icuDateStyle = icu::DateFormat::NONE;
    if (dateStyleUndefined != ANI_TRUE) {
        ASSERT(dateStyleRef != nullptr);

        auto dateStyle = static_cast<ani_string>(dateStyleRef);
        icuDateStyle = ToICUDateTimeStyle(env, dateStyle);
    }

    ani_boolean timeStyleUndefined = ANI_FALSE;
    ANI_FATAL_IF_ERROR(env->Reference_IsUndefined(timeStyleRef, &timeStyleUndefined));

    icu::DateFormat::EStyle icuTimeStyle = icu::DateFormat::NONE;
    if (timeStyleUndefined != ANI_TRUE) {
        ASSERT(timeStyleRef != nullptr);

        auto timeStyle = static_cast<ani_string>(timeStyleRef);
        icuTimeStyle = ToICUDateTimeStyle(env, timeStyle);
    }

    return CreateStyleBasedDateFormatAlignedWithLocale(env, icuDateStyle, icuTimeStyle, locale);
}

static ani_object DateTimeFormatGetOptions(ani_env *env, ani_object self)
{
    ani_ref optionsRef = nullptr;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Ref(self, "options", &optionsRef));
    ASSERT(optionsRef != nullptr);

    return static_cast<ani_object>(optionsRef);
}

static std::unique_ptr<icu::UnicodeString> DateTimeFormatGetPatternSkeleton(ani_env *env, ani_object self)
{
    ani_ref patternRef;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Ref(self, "pattern", &patternRef));

    auto pattern = static_cast<ani_string>(patternRef);

    ani_size patternSize = 0;
    ANI_FATAL_IF_ERROR(env->String_GetUTF16Size(pattern, &patternSize));

    ani_size copiedCharsCount = 0;
    std::vector<uint16_t> patternBuf(patternSize + 1);

    ANI_FATAL_IF_ERROR(env->String_GetUTF16(pattern, patternBuf.data(), patternBuf.size(), &copiedCharsCount));
    ANI_FATAL_IF(copiedCharsCount != patternSize);

    return std::make_unique<icu::UnicodeString>(patternBuf.data(), patternSize);
}

std::unique_ptr<icu::DateFormat> CreateICUDateFormat(ani_env *env, ani_object self)
{
    std::unique_ptr<icu::Locale> icuLocale = ToICULocale(env, self);
    if (!icuLocale) {
        return nullptr;
    }

    std::unique_ptr<icu::UnicodeString> patternSkeleton = DateTimeFormatGetPatternSkeleton(env, self);
    if (!patternSkeleton) {
        return nullptr;
    }

    ani_object options = DateTimeFormatGetOptions(env, self);
    ani_status aniStatus = ConfigureLocaleOptions(env, icuLocale.get(), options);
    if (aniStatus != ANI_OK) {
        return nullptr;
    }

    std::unique_ptr<icu::DateFormat> icuDateFormat;
    if (patternSkeleton->isEmpty() == FALSE) {
        icuDateFormat = CreateSkeletonBasedDateFormat(env, *icuLocale, *patternSkeleton);
    } else {
        icuDateFormat = CreateStyleBasedDateFormat(env, *icuLocale, options);
    }

    if (!icuDateFormat) {
        // DateFormat creation failed
        return nullptr;
    }

    aniStatus = DateFormatSetTimeZone(env, icuDateFormat.get(), options);
    if (aniStatus != ANI_OK) {
        if (aniStatus != ANI_PENDING_ERROR) {
            ThrowInternalError(env, "DateFormat time zone initialization failed");
        }
        return nullptr;
    }

    return icuDateFormat;
}

static ani_string FormatImpl(ani_env *env, ani_object self, ani_double timestamp, ani_string aniCacheKey)
{
    std::string cacheKey = ConvertFromAniString(env, aniCacheKey);
    icu::DateFormat *icuDateFormat = g_intlState->dateTimeFormatCache.GetOrCreateDateFormat(env, self, cacheKey);
    if (!icuDateFormat) {
        return nullptr;
    }

    icu::UnicodeString icuFormattedDate;
    icuDateFormat->format(timestamp, icuFormattedDate);

    auto formattedDateChars = reinterpret_cast<const uint16_t *>(icuFormattedDate.getBuffer());
    return CreateUtf16String(env, formattedDateChars, icuFormattedDate.length());
}

static ani_array FormatToPartsImpl(ani_env *env, ani_object self, ani_double timestamp, ani_string aniCacheKey)
{
    std::string cacheKey = ConvertFromAniString(env, aniCacheKey);
    icu::DateFormat *dateFormat = g_intlState->dateTimeFormatCache.GetOrCreateDateFormat(env, self, cacheKey);
    if (!dateFormat) {
        // DateFormat creation failed ( check for pending error )
        return nullptr;
    }

    UErrorCode status = U_ZERO_ERROR;

    icu::UnicodeString formattedDate;
    icu::FieldPositionIterator fieldPosIter;

    dateFormat->format(timestamp, formattedDate, &fieldPosIter, status);
    if (U_FAILURE(status) == TRUE) {
        return ThrowInternalError(env, std::string("DateFormat.format() failed: ") + u_errorName(status));
    }

    int32_t prevFieldEndIdx = 0;
    const icu::UnicodeString literalType(DTF_PART_LITERAL_TYPE);

    icu::FieldPosition fieldPos;
    std::vector<std::pair<icu::UnicodeString, icu::UnicodeString>> parts;
    while (fieldPosIter.next(fieldPos) == TRUE) {
        if (fieldPos.getBeginIndex() > prevFieldEndIdx) {
            icu::UnicodeString literalValue;
            formattedDate.extractBetween(prevFieldEndIdx, fieldPos.getBeginIndex(), literalValue);

            parts.emplace_back(literalType, literalValue);
        }

        const char *partType = DTF_PART_TYPES.at(fieldPos.getField());

        icu::UnicodeString fieldValue;
        formattedDate.extractBetween(fieldPos.getBeginIndex(), fieldPos.getEndIndex(), fieldValue);

        parts.emplace_back(partType, fieldValue);

        prevFieldEndIdx = fieldPos.getEndIndex();
    }

    ani_class fmtPartImplCls = nullptr;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.Intl.DateTimeFormatPartImpl", &fmtPartImplCls));

    ani_method fmtPartImplCtor;
    ANI_FATAL_IF_ERROR(env->Class_FindMethod(fmtPartImplCls, "<ctor>", DTF_PART_IMPL_CTOR_SIGNATURE, &fmtPartImplCtor));

    ani_ref undefined {};
    ANI_FATAL_IF_ERROR(env->GetUndefined(&undefined));
    ani_array formattedDateParts {};
    ANI_FATAL_IF_ERROR(env->Array_New(parts.size(), undefined, &formattedDateParts));

    for (size_t partIdx = 0; partIdx < parts.size(); partIdx++) {
        const auto &part = parts[partIdx];

        auto partTypeChars = reinterpret_cast<const uint16_t *>(part.first.getBuffer());
        ani_string partType = CreateUtf16String(env, partTypeChars, part.first.length());

        auto partValueChars = reinterpret_cast<const uint16_t *>(part.second.getBuffer());
        ani_string partValue = CreateUtf16String(env, partValueChars, part.second.length());

        ani_object fmtPartImpl = nullptr;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        ANI_FATAL_IF_ERROR(env->Object_New(fmtPartImplCls, fmtPartImplCtor, &fmtPartImpl, partType, partValue));
        ANI_FATAL_IF_ERROR(env->Array_Set(formattedDateParts, partIdx, fmtPartImpl));
    }

    return formattedDateParts;
}

using UStr = icu::UnicodeString;

static ani_object FormatResolvedOptionsImpl(ani_env *env, ani_object self, ani_string aniCacheKey)
{
    std::string cacheKey = ConvertFromAniString(env, aniCacheKey);
    icu::DateFormat *dateFormat = g_intlState->dateTimeFormatCache.GetOrCreateDateFormat(env, self, cacheKey);
    if (!dateFormat) {
        return nullptr;
    }

    auto dateFormatImpl = static_cast<icu::SimpleDateFormat *>(dateFormat);
    const icu::Locale &locale = dateFormatImpl->getSmpFmtLocale();

    UErrorCode status = U_ZERO_ERROR;

    auto langTagStr = locale.toLanguageTag<std::string>(status);
    if (U_FAILURE(status) == TRUE) {
        return ThrowInternalError(env, std::string("failed to get locale lang tag: ") + u_errorName(status));
    }
    ani_string langTag = CreateUtf8String(env, langTagStr.data(), langTagStr.size());
    std::string icuCalendar = dateFormat->getCalendar()->getType();
    ani_string calendarStr = CreateUtf8String(env, icuCalendar.c_str(), icuCalendar.length());
    if (icuCalendar == "gregorian") {
        calendarStr = CreateUtf8String(env, "gregory", strlen("gregory"));
    } else if (icuCalendar == "ethiopic-amete-alem") {
        calendarStr = CreateUtf8String(env, "ethioaa", strlen("ethioaa"));
    }

    std::unique_ptr<icu::NumberingSystem> numSys(icu::NumberingSystem::createInstance(locale, status));
    if (U_FAILURE(status) == TRUE) {
        return ThrowInternalError(env, std::string("NumberingSystem creation failed: ") + u_errorName(status));
    }

    ani_string numSysName = CreateUtf8String(env, numSys->getName(), strlen(numSys->getName()));

    icu::UnicodeString timeZoneId;
    dateFormat->getTimeZone().getID(timeZoneId);

    auto timeZoneIdChars = reinterpret_cast<const uint16_t *>(timeZoneId.getBuffer());
    ani_string timeZone = CreateUtf16String(env, timeZoneIdChars, timeZoneId.length());

    ani_class optsClass = nullptr;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.Intl.ResolvedDateTimeFormatOptionsImpl", &optsClass));

    ani_method optsCtor = nullptr;
    ANI_FATAL_IF_ERROR(env->Class_FindMethod(optsClass, "<ctor>", RESOLVED_OPTS_CTOR_SIGNATURE, &optsCtor));

    ani_object resolvedOpts = nullptr;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    env->Object_New(optsClass, optsCtor, &resolvedOpts, langTag, calendarStr, numSysName, timeZone);

    auto localeHours = locale.getKeywordValue<std::string>(LOCALE_KEYWORD_HOUR_CYCLE, status);
    if (U_FAILURE(status) == TRUE) {
        return ThrowInternalError(env, std::string("failed to get locale 'hours' keyword: ") + u_errorName(status));
    }

    if (!localeHours.empty()) {
        ani_string hourCycle = CreateUtf8String(env, localeHours.data(), localeHours.size());

        ani_method setter = nullptr;
        ANI_FATAL_IF_ERROR(env->Class_FindSetter(optsClass, OPTIONS_PROPERTY_HOUR_CYCLE, &setter));

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        ANI_FATAL_IF_ERROR(env->Object_CallMethod_Void(resolvedOpts, setter, hourCycle));
    }

    return resolvedOpts;
}

static void FillDateTimeRangeFormatPartArray(ani_env *env, ani_array partsArr,
                                             const std::vector<std::tuple<UStr, UStr, UStr>> &parts)
{
    ani_class partImplCls = nullptr;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.Intl.DateTimeRangeFormatPartImpl", &partImplCls));

    ani_method partImplCtor = nullptr;
    ANI_FATAL_IF_ERROR(env->Class_FindMethod(partImplCls, "<ctor>", DTRF_PART_IMPL_CTOR_SIGNATURE, &partImplCtor));

    for (size_t partIdx = 0; partIdx < parts.size(); partIdx++) {
        const auto &[partType, partValue, partSource] = parts[partIdx];

        auto partTypeChars = reinterpret_cast<const uint16_t *>(partType.getBuffer());
        ani_string partTypeStr = CreateUtf16String(env, partTypeChars, partType.length());

        auto partValueChars = reinterpret_cast<const uint16_t *>(partValue.getBuffer());
        ani_string partValStr = CreateUtf16String(env, partValueChars, partValue.length());

        auto partSourceChars = reinterpret_cast<const uint16_t *>(partSource.getBuffer());
        ani_string partSrcStr = CreateUtf16String(env, partSourceChars, partSource.length());

        ani_object partImpl = nullptr;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        ANI_FATAL_IF_ERROR(env->Object_New(partImplCls, partImplCtor, &partImpl, partTypeStr, partValStr, partSrcStr));
        ANI_FATAL_IF_ERROR(env->Array_Set(partsArr, partIdx, partImpl));
    }
}

static ani_status DateIntervalFormatSetTimeZone(ani_env *env, icu::DateIntervalFormat *format, ani_object options)
{
    ani_ref timeZoneRef = nullptr;
    ANI_FATAL_IF_ERROR(env->Object_GetFieldByName_Ref(options, "timeZone_", &timeZoneRef));

    auto timeZone = static_cast<ani_string>(timeZoneRef);
    std::unique_ptr<icu::UnicodeString> timeZoneId = UnicodeStringFromAniString(env, timeZone);
    if (!timeZoneId) {
        return ANI_OK;
    }

    std::unique_ptr<icu::TimeZone> formatTimeZone(icu::TimeZone::createTimeZone(*timeZoneId));
    if (*formatTimeZone == icu::TimeZone::getUnknown()) {
        std::string invalidTimeZoneId;
        timeZoneId->toUTF8String(invalidTimeZoneId);

        ThrowRangeError(env, "Invalid time zone specified: " + invalidTimeZoneId);
        return ANI_PENDING_ERROR;
    }
    format->adoptTimeZone(formatTimeZone.release());

    return ANI_OK;
}

static std::unique_ptr<icu::FormattedDateInterval> FormatDateInterval(ani_env *env, ani_object self, ani_double start,
                                                                      ani_double end, const std::string &cacheKey)
{
    icu::DateFormat *dateFmt = g_intlState->dateTimeFormatCache.GetOrCreateDateFormat(env, self, cacheKey);
    if (!dateFmt) {
        return nullptr;
    }

    auto dateFmtImpl = static_cast<icu::SimpleDateFormat *>(dateFmt);

    UStr pattern;
    dateFmtImpl->toPattern(pattern);

    UErrorCode status = U_ZERO_ERROR;
    UStr skeleton = icu::DateTimePatternGenerator::staticGetSkeleton(pattern, status);
    if (U_FAILURE(status) == TRUE) {
        return ThrowInternalError(env, std::string("Failed to get skeleton: ") + u_errorName(status));
    }

    const icu::Locale &locale = dateFmtImpl->getSmpFmtLocale();
    std::unique_ptr<icu::DateIntervalFormat> intervalFmt(
        icu::DateIntervalFormat::createInstance(skeleton, locale, status));
    if (U_FAILURE(status) == TRUE) {
        return ThrowInternalError(env, std::string("Failed to create DateIntervalFormat: ") + u_errorName(status));
    }

    ani_object options = DateTimeFormatGetOptions(env, self);
    DateIntervalFormatSetTimeZone(env, intervalFmt.get(), options);

    auto interval = std::make_unique<icu::DateInterval>(start, end);

    icu::FormattedDateInterval formattedIntervalVal = intervalFmt->formatToValue(*interval, status);
    if (U_FAILURE(status) == TRUE) {
        return ThrowInternalError(env, std::string("DateIntervalFormat::formatToValue failed: ") + u_errorName(status));
    }

    return std::make_unique<icu::FormattedDateInterval>(std::move(formattedIntervalVal));
}

enum DateIntervalPartSource { OUT_OF_RANGE = -1, START_RANGE = 0, END_RANGE = 1 };

static ani_string FormatRangeImpl(ani_env *env, ani_object self, ani_double start, ani_double end,
                                  ani_string aniCacheKey)
{
    std::string cacheKey = ConvertFromAniString(env, aniCacheKey);
    auto formattedIntervalVal = FormatDateInterval(env, self, start, end, cacheKey);
    if (UNLIKELY(formattedIntervalVal == nullptr)) {
        return ThrowInternalError(env, std::string("FormattedDateInterval failed"));
    }

    UErrorCode status = U_ZERO_ERROR;
    UStr formattedInterval = formattedIntervalVal->toString(status);
    if (U_FAILURE(status) == TRUE) {
        return ThrowInternalError(env, std::string("FormattedDateInterval::toString failed: ") + u_errorName(status));
    }

    auto formattedIntervalChars = reinterpret_cast<const uint16_t *>(formattedInterval.getBuffer());
    return CreateUtf16String(env, formattedIntervalChars, formattedInterval.length());
}

static ani_array FormatRangeToPartsImpl(ani_env *env, ani_object self, ani_double start, ani_double end,
                                        ani_string aniCacheKey)
{
    std::string cacheKey = ConvertFromAniString(env, aniCacheKey);
    auto formattedIntervalVal = FormatDateInterval(env, self, start, end, cacheKey);
    if (UNLIKELY(formattedIntervalVal == nullptr)) {
        return ThrowInternalError(env, std::string("FormattedDateInterval failed"));
    }
    UErrorCode status = U_ZERO_ERROR;
    UStr formattedInterval = formattedIntervalVal->toString(status);
    if (U_FAILURE(status) == TRUE) {
        return ThrowInternalError(env, std::string("FormattedDateInterval::toString failed: ") + u_errorName(status));
    }
    std::vector<std::tuple<UStr, UStr, UStr>> parts;
    icu::ConstrainedFieldPosition partPos;
    DateIntervalPartSource partSource = OUT_OF_RANGE;
    int32_t prevPartEndIdx = 0;
    int32_t prevSpanEndIdx = 0;
    const UStr partLiteralType = DTF_PART_LITERAL_TYPE;
    while (formattedIntervalVal->nextPosition(partPos, status) == TRUE) {
        if (U_FAILURE(status) == TRUE) {
            return ThrowInternalError(env, std::string("FormattedDateInterval::nextPos failed:") + u_errorName(status));
        }
        if (partPos.getCategory() == UFIELD_CATEGORY_DATE_INTERVAL_SPAN) {
            partSource = static_cast<DateIntervalPartSource>(partSource + 1);

            if (partPos.getStart() > prevSpanEndIdx) {
                UStr literalValue;
                formattedInterval.extractBetween(prevSpanEndIdx, partPos.getStart(), literalValue);

                parts.emplace_back(partLiteralType, literalValue, DTRF_PART_SOURCE_SHARED);
            }

            prevSpanEndIdx = partPos.getLimit(), prevPartEndIdx = partPos.getStart();
        } else if (partPos.getCategory() == UFIELD_CATEGORY_DATE) {
            const char *partSourceName = DTRF_PART_SOURCES.at(partSource);
            if (partPos.getStart() > prevPartEndIdx) {
                UStr literalValue;
                formattedInterval.extractBetween(prevPartEndIdx, partPos.getStart(), literalValue);

                parts.emplace_back(partLiteralType, literalValue, UStr(partSourceName));
            }

            UStr partValue;
            formattedInterval.extractBetween(partPos.getStart(), partPos.getLimit(), partValue);

            parts.emplace_back(UStr(DTF_PART_TYPES.at(partPos.getField())), partValue, UStr(partSourceName));

            prevPartEndIdx = partPos.getLimit();
        } else {
            UNREACHABLE();
        }
    }
    ani_ref undefined {};
    ANI_FATAL_IF_ERROR(env->GetUndefined(&undefined));
    ani_array partsArr {};
    ANI_FATAL_IF_ERROR(env->Array_New(parts.size(), undefined, &partsArr));
    FillDateTimeRangeFormatPartArray(env, partsArr, parts);
    return partsArr;
}

ani_status RegisterIntlDateTimeFormatMethods(ani_env *env)
{
    ani_class dtfClass;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.Intl.DateTimeFormat", &dtfClass));

    std::array dtfMethods {
        ani_native_function {"formatImpl", "dC{std.core.String}:C{std.core.String}",
                             reinterpret_cast<void *>(FormatImpl)},
        ani_native_function {"formatToPartsImpl", "dC{std.core.String}:C{std.core.Array}",
                             reinterpret_cast<void *>(FormatToPartsImpl)},
        ani_native_function {"formatRangeImpl", "ddC{std.core.String}:C{std.core.String}",
                             reinterpret_cast<void *>(FormatRangeImpl)},
        ani_native_function {"formatRangeToPartsImpl", "ddC{std.core.String}:C{std.core.Array}",
                             reinterpret_cast<void *>(FormatRangeToPartsImpl)},
        ani_native_function {"resolvedOptionsImpl", "C{std.core.String}:C{std.core.Intl.ResolvedDateTimeFormatOptions}",
                             reinterpret_cast<void *>(FormatResolvedOptionsImpl)},
        ani_native_function {"getLocaleHourCycle", ":C{std.core.String}", reinterpret_cast<void *>(GetLocaleHourCycle)},
    };

    ani_status status = env->Class_BindNativeMethods(dtfClass, dtfMethods.data(), dtfMethods.size());
    if (!(status == ANI_OK || status == ANI_ALREADY_BINDED)) {
        return status;
    }

    return ANI_OK;
}
}  // namespace ark::ets::stdlib::intl
