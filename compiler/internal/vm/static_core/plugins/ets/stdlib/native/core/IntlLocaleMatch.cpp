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

#include "plugins/ets/stdlib/native/core/IntlLocaleMatch.h"
#include "plugins/ets/stdlib/native/core/IntlCommon.h"
#include "plugins/ets/stdlib/native/core/IntlLanguageTag.h"
#include "plugins/ets/stdlib/native/core/IntlLocale.h"
#include "libarkbase/macros.h"
#include "unicode/locid.h"
#include "unicode/localebuilder.h"
#include "unicode/localematcher.h"
#include "stdlib_ani_helpers.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstring>
#include <memory>
#include <string>
#include <array>
#include <set>
#include <sstream>

namespace ark::ets::stdlib {

template <typename... Args>
static void ThrowRangeError(ani_env *env, Args &&...args)
{
    std::stringstream message;
    (message << ... << args);
    ThrowNewError(env, "std.core.RangeError", message.str().c_str(), "C{std.core.String}:");
}

std::vector<std::string> GetAvailableLocales()
{
    int32_t availableCount;
    std::vector<std::string> availableLocales;
    const icu::Locale *locales = icu::Locale::getAvailableLocales(availableCount);
    availableLocales.reserve(availableCount);
    for (int32_t i = 0; i < availableCount; ++i) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        availableLocales.emplace_back(intl::ToStdStringLanguageTag(locales[i]));
    }
    return availableLocales;
}

std::string BestAvailableLocale(const std::vector<std::string> &availableLocales, const std::string &locale)
{
    // 1. Let candidate be locale.
    std::string localeCandidate = locale;
    std::string undefined = std::string();
    // 2. Repeat,
    uint32_t length = availableLocales.size();
    while (!localeCandidate.empty()) {
        // a. If availableLocales contains an element equal to candidate, return candidate.
        for (uint32_t i = 0; i < length; ++i) {
            const std::string &itemStr = availableLocales[i];
            if (itemStr == localeCandidate) {
                return localeCandidate;
            }
        }
        // b. Let pos be the character index of the last occurrence of "-" (U+002D) within candidate.
        //    If that character does not occur, return undefined.
        size_t pos = localeCandidate.rfind('-');
        if (pos == std::string::npos) {
            return undefined;
        }
        // c. If pos ≥ 2 and the character "-" occurs at index pos-2 of candidate, decrease pos by 2.
        if (pos >= intl::INTL_INDEX_TWO && localeCandidate[pos - intl::INTL_INDEX_TWO] == '-') {
            pos -= intl::INTL_INDEX_TWO;
        }
        // d. Let candidate be the substring of candidate from position 0, inclusive, to position pos, exclusive.
        localeCandidate.resize(pos);
    }
    return undefined;
}

static std::string GetDefaultLocaleTag()
{
    icu::Locale defaultLocale;

    const char *defaultLocaleName = defaultLocale.getName();
    if (strcmp(defaultLocaleName, "en_US_POSIX") == 0 || strcmp(defaultLocaleName, "c") == 0) {
        return "en-US";
    }

    if (defaultLocale.isBogus() == TRUE) {
        return "und";
    }

    UErrorCode error = U_ZERO_ERROR;
    auto defaultLocaleTag = defaultLocale.toLanguageTag<std::string>(error);
    ANI_FATAL_IF(U_FAILURE(error));

    return defaultLocaleTag;
}

static std::vector<std::string> ToStringList(ani_env *env, ani_array aniList)
{
    ani_size len;
    ANI_FATAL_IF_ERROR(env->Array_GetLength(aniList, &len));

    std::vector<std::string> result;
    result.reserve(len);

    for (ani_size i = 0; i < len; i++) {
        ani_ref aniRef;
        ANI_FATAL_IF_ERROR(env->Array_Get(aniList, i, &aniRef));

        auto item = ConvertFromAniString(env, reinterpret_cast<ani_string>(aniRef));
        result.push_back(item);
    }
    return result;
}

static ani_array ToAniStrArray(ani_env *env, std::vector<std::string> strings)
{
    ani_class stringClass;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.String", &stringClass));

    ani_array array;
    if (strings.empty()) {
        ANI_FATAL_IF_ERROR(env->Array_New(0, nullptr, &array));
        return array;
    }
    auto first = intl::StdStrToAni(env, strings[0]);

    ANI_FATAL_IF_ERROR(env->Array_New(strings.size(), first, &array));
    for (size_t i = 1; i < strings.size(); ++i) {
        auto item = intl::StdStrToAni(env, strings[i]);
        ANI_FATAL_IF_ERROR(env->Array_Set(array, i, item));
    }
    return array;
}

ani_status CanonicalizeLocaleList(ani_env *env, std::vector<std::string> &seen,
                                  std::vector<std::string> &requestedLocales)
{
    if (seen.empty()) {
        return ANI_PENDING_ERROR;
    }
    auto len = seen.size();
    for (size_t i = 0; i < len; i++) {
        std::string localeCStr = seen[i];
        if (!intl::IsStructurallyValidLanguageTag(localeCStr)) {
            ThrowRangeError(env, "invalid locale");
            return ANI_PENDING_ERROR;
        }
        if (localeCStr.length() == 0) {
            ThrowRangeError(env, "invalid locale");
            return ANI_PENDING_ERROR;
        }

        std::transform(localeCStr.begin(), localeCStr.end(), localeCStr.begin(), intl::AsciiAlphaToLower);
        UErrorCode success = U_ZERO_ERROR;
        icu::Locale formalLocale = icu::Locale::forLanguageTag(seen[i], success);
        if ((U_FAILURE(success) != 0) || (formalLocale.isBogus() != 0)) {
            ThrowRangeError(env, "invalid locale");
            return ANI_PENDING_ERROR;
        }
        // Resets the LocaleBuilder to match the locale.
        // Returns an instance of Locale created from the fields set on this builder.
        formalLocale = icu::LocaleBuilder().setLocale(formalLocale).build(success);
        // Canonicalize the locale ID of this object according to CLDR.
        formalLocale.canonicalize(success);
        if ((U_FAILURE(success) != 0) || (formalLocale.isBogus() != 0)) {
            ThrowRangeError(env, "invalid locale");
            return ANI_PENDING_ERROR;
        }
        std::string languageTag = intl::ToStdStringLanguageTag(formalLocale);
        if (std::find(requestedLocales.begin(), requestedLocales.end(), languageTag) == requestedLocales.end()) {
            requestedLocales.push_back(languageTag);
        }
    }
    return ANI_OK;
}

static icu::LocaleMatcher BuildLocaleMatcher(UErrorCode &success)
{
    UErrorCode error = U_ZERO_ERROR;

    const icu::Locale defaultLocale = icu::Locale::forLanguageTag(GetDefaultLocaleTag(), error);
    ANI_FATAL_IF(U_FAILURE(error));

    icu::LocaleMatcher::Builder builder;
    builder.setDefaultLocale(&defaultLocale);

    int32_t count;
    const icu::Locale *availableLocales = icu::Locale::getAvailableLocales(count);
    for (int32_t i = 0; i < count; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        builder.addSupportedLocale(availableLocales[i]);
    }

    return builder.build(success);
}

icu::Locale GetLocale(ani_env *env, std::string &locTag)
{
    UErrorCode status = U_ZERO_ERROR;
    icu::Locale locale = icu::Locale::forLanguageTag(icu::StringPiece(locTag.c_str()), status);
    if (UNLIKELY(U_FAILURE(status))) {
        const auto errorMessage = std::string("Language tag '").append(locTag).append("' is invalid or not supported");
        ThrowNewError(env, "std.core.RuntimeError", errorMessage.c_str(), "C{std.core.String}:");
        return nullptr;
    }
    return locale;
}

ani_string StdCoreIntlBestFitLocale(ani_env *env, [[maybe_unused]] ani_class klass, ani_array locales)
{
    auto tags = ToStringList(env, locales);
    for (const auto &tag : tags) {
        if (!intl::IsStructurallyValidLanguageTag(tag)) {
            ThrowRangeError(env, "Incorrect locale information provided");
        }
    }
    auto success = UErrorCode::U_ZERO_ERROR;
    auto matcher = BuildLocaleMatcher(success);
    if (UNLIKELY(U_FAILURE(success))) {
        ThrowNewError(env, "std.core.RuntimeError", "Unable to build locale matcher", "C{std.core.String}:");
        return nullptr;
    }
    auto it = intl::LanguageTagListIterator(tags);
    auto bestfit = matcher.getBestMatchResult(it, success);
    if (UNLIKELY(U_FAILURE(success))) {
        ThrowNewError(env, "std.core.RuntimeError", "Unable to get best match result", "C{std.core.String}:");
        return nullptr;
    }
    auto locale = bestfit.makeResolvedLocale(success);
    if (UNLIKELY(U_FAILURE(success))) {
        ThrowNewError(env, "std.core.RuntimeError", "Unable to make resolved locale", "C{std.core.String}:");
        return nullptr;
    }
    auto tag = locale.toLanguageTag<std::string>(success);
    if (UNLIKELY(U_FAILURE(success))) {
        ThrowNewError(env, "std.core.RuntimeError", "Unable to convert locale into language tag",
                      "C{std.core.String}:");
        return nullptr;
    }
    if (tag == "en_US_POSIX" || tag == "c") {
        tag = "en-US";
    }
    return intl::StdStrToAni(env, tag);
}

std::vector<std::string> LookupLocales(std::vector<std::string> &availableLocales,
                                       std::vector<std::string> &requestedLocales)
{
    auto length = requestedLocales.size();
    std::vector<std::string> convertedLocales;
    convertedLocales.reserve(length);
    std::vector<uint32_t> indexAvailableLocales;
    indexAvailableLocales.reserve(length);
    for (uint32_t i = 0; i < length; ++i) {
        convertedLocales.push_back(requestedLocales[i]);
    }
    // 1. For each element locale of requestedLocales in List order, do
    //    a. Let noExtensionsLocale be the String value that is locale with all Unicode locale extension sequences
    //       removed.
    //    b. Let availableLocale be BestAvailableLocale(availableLocales, noExtensionsLocale).
    //    c. If availableLocale is not undefined, append locale to the end of result.

    for (uint32_t i = 0; i < length; ++i) {
        intl::ParsedLocale foundationResult = intl::HandleLocale(convertedLocales[i]);
        std::string availableLocale = BestAvailableLocale(availableLocales, foundationResult.base);
        if (!availableLocale.empty()) {
            indexAvailableLocales.push_back(i);
        }
    }
    // 2. Let result be a new empty List.
    std::vector<std::string> result;
    result.reserve(length);
    for (unsigned int indexAvailableLocale : indexAvailableLocales) {
        result.push_back(requestedLocales[indexAvailableLocale]);
    }
    // 3. Return result.
    return result;
}

ani_array StdCoreIntlBestFitLocales(ani_env *env, [[maybe_unused]] ani_class klass, ani_array locales)
{
    auto tags = ToStringList(env, locales);

    auto success = UErrorCode::U_ZERO_ERROR;
    auto matcher = BuildLocaleMatcher(success);
    if (UNLIKELY(U_FAILURE(success))) {
        ThrowNewError(env, "std.core.RuntimeError", "Unable to build locale matcher", "C{std.core.String}:");
        return nullptr;
    }

    auto result = std::vector<std::string>();
    for (const auto &tag : tags) {
        if (!intl::IsStructurallyValidLanguageTag(tag)) {
            ThrowRangeError(env, "Incorrect locale information provided");
            return nullptr;
        }
        success = UErrorCode::U_ZERO_ERROR;
        auto desired = icu::Locale::forLanguageTag(tag, success);
        auto matched = matcher.getBestMatchResult(desired, success);
        if (UNLIKELY(U_FAILURE(success))) {
            continue;
        }
        if (matched.getSupportedIndex() < 0) {
            continue;
        }
        auto bestfit = desired.toLanguageTag<std::string>(success);
        if (UNLIKELY(U_FAILURE(success))) {
            continue;
        }
        result.push_back(bestfit);
    }
    return ToAniStrArray(env, result);
}

static std::string LookupLocale(const std::string &locTag, const icu::Locale *availableLocales, const int32_t count)
{
    UErrorCode success = U_ZERO_ERROR;
    auto locP = icu::StringPiece(locTag.c_str());
    icu::Locale requestedLoc = icu::Locale::forLanguageTag(locP, success);
    if (UNLIKELY(U_FAILURE(success))) {
        return std::string();
    }
    for (int32_t i = 0; i < count; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (LIKELY(availableLocales[i] == requestedLoc)) {
            return requestedLoc.toLanguageTag<std::string>(success);
        }
    }
    return std::string();
}

ani_string StdCoreIntlLookupLocale(ani_env *env, [[maybe_unused]] ani_class klass, ani_array locales)
{
    UErrorCode success = U_ZERO_ERROR;
    ani_ref locale;
    int32_t availableCount;
    std::string bestLoc;
    const icu::Locale *availableLocales = icu::Locale::getAvailableLocales(availableCount);
    if (UNLIKELY(U_FAILURE(success))) {
        return CreateUtf8String(env, "", 0);
    }

    ani_size len;
    ANI_FATAL_IF_ERROR(env->Array_GetLength(locales, &len));

    for (ani_size i = 0; i < len; i++) {
        ANI_FATAL_IF_ERROR(env->Array_Get(locales, i, &locale));

        auto locTag = ConvertFromAniString(env, reinterpret_cast<ani_string>(locale));
        if (!intl::IsStructurallyValidLanguageTag(locTag)) {
            ThrowRangeError(env, "Incorrect locale information provided");
        }

        bestLoc = LookupLocale(locTag, availableLocales, availableCount);
        if (!bestLoc.empty()) {
            return intl::StdStrToAni(env, bestLoc);
        }
    }
    return intl::StdStrToAni(env, GetDefaultLocaleTag());
}

ani_array StdCoreIntlLookupLocales(ani_env *env, [[maybe_unused]] ani_class klass, ani_array locales)
{
    auto tags = ToStringList(env, locales);
    auto availableLocales = GetAvailableLocales();
    std::vector<std::string> requestedLocales;
    ani_status status = CanonicalizeLocaleList(env, tags, requestedLocales);
    if (status != ANI_OK) {
        return nullptr;
    }
    auto result = LookupLocales(availableLocales, requestedLocales);
    return ToAniStrArray(env, result);
}

ani_status RegisterIntlLocaleMatch(ani_env *env)
{
    const auto methods = std::array {ani_native_function {"bestFitLocale", "C{std.core.Array}:C{std.core.String}",
                                                          reinterpret_cast<void *>(StdCoreIntlBestFitLocale)},
                                     ani_native_function {"lookupLocale", "C{std.core.Array}:C{std.core.String}",
                                                          reinterpret_cast<void *>(StdCoreIntlLookupLocale)},
                                     ani_native_function {"bestFitLocales", "C{std.core.Array}:C{std.core.Array}",
                                                          reinterpret_cast<void *>(StdCoreIntlBestFitLocales)},
                                     ani_native_function {"lookupLocales", "C{std.core.Array}:C{std.core.Array}",
                                                          reinterpret_cast<void *>(StdCoreIntlLookupLocales)}};

    ani_class localeMatchClass;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.Intl.LocaleMatch", &localeMatchClass));

    return env->Class_BindStaticNativeMethods(localeMatchClass, methods.data(), methods.size());
}

}  // namespace ark::ets::stdlib
