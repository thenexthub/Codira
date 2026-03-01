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
#include "plugins/ets/stdlib/native/core/IntlLocale.h"
#include "plugins/ets/stdlib/native/core/IntlLanguageTag.h"
#include "plugins/ets/stdlib/native/core/IntlCommon.h"
#include "plugins/ets/stdlib/native/core/stdlib_ani_helpers.h"
#include <unicode/locid.h>
#include <unicode/numsys.h>
#include <unicode/localpointer.h>
#include "libarkbase/macros.h"

#include <cassert>
#include <cstring>
#include <set>
#include <array>
#include <sstream>
#include <vector>
#include <algorithm>
#include "libarkbase/os/mutex.h"

namespace ark::ets::stdlib::intl {

enum LocaleInfo { LANG, SCRIPT, COUNTRY, U_CA, U_KF, U_CO, U_HC, U_NU, U_KN, COUNT };

struct LocaleCheckInfo {
    std::set<std::string> langs;
    std::set<std::string> regions;
    std::set<std::string> scripts;
    // Cached comma-separated string representations for performance
    // These avoid repeated string concatenation when queried from ETS layer
    std::string langsList;
    std::string regionsList;
    std::string scriptsList;
    bool isPopulated = false;
};

static LocaleCheckInfo &GetLocaleCheckInfo()
{
    static LocaleCheckInfo gCheckInfo;
    return gCheckInfo;
}

static os::memory::Mutex localeCheckInfoMutex;

static std::vector<std::string> &GetLocaleInfo()
{
    static std::vector<std::string> gLocaleInfo(LocaleInfo::COUNT, "");
    std::fill(gLocaleInfo.begin(), gLocaleInfo.end(), "");
    return gLocaleInfo;
}

ParsedLocale HandleLocale(const std::string &localeString)
{
    size_t len = localeString.size();
    ParsedLocale parsedResult;

    // a. The single-character subtag ’x’ as the primary subtag indicates
    //    that the language tag consists solely of subtags whose meaning is
    //    defined by private agreement.
    // b. Extensions cannot be used in tags that are entirely private use.
    if (intl::IsPrivateSubTag(localeString, len)) {
        parsedResult.base = localeString;
        return parsedResult;
    }
    // If cannot find "-u-", return the whole string as base.
    size_t foundExtension = localeString.find("-u-");
    if (foundExtension == std::string::npos) {
        parsedResult.base = localeString;
        return parsedResult;
    }
    // Let privateIndex be Call(%StringProto_indexOf%, foundLocale, « "-x-" »).
    size_t privateIndex = localeString.find("-x-");
    if (privateIndex != std::string::npos && privateIndex < foundExtension) {
        parsedResult.base = localeString;
        return parsedResult;
    }
    const std::string basis = localeString.substr(0, foundExtension);
    size_t extensionEnd = len;
    ASSERT(len > INTL_INDEX_TWO);
    size_t start = foundExtension + 1;
    HandleLocaleExtension(start, extensionEnd, localeString, len);
    const std::string end = localeString.substr(extensionEnd);
    parsedResult.base = basis + end;
    parsedResult.extension = localeString.substr(foundExtension, extensionEnd - foundExtension);
    return parsedResult;
}

void HandleLocaleExtension(size_t &start, size_t &extensionEnd, const std::string &result, size_t len)
{
    while (start < len - INTL_INDEX_TWO) {
        if (result[start] != '-') {
            start++;
            continue;
        }
        if (result[start + INTL_INDEX_TWO] == '-') {
            extensionEnd = start;
            break;
        }
        start += INTL_INDEX_THREE;
    }
}

static std::string Set2String(std::set<std::string> &inSet);

void StdCoreIntlLocaleFillCheckInfo()
{
    // Cache reference to avoid repeated calls
    LocaleCheckInfo &checkInfo = GetLocaleCheckInfo();

    // First check: Fast path without lock - avoids lock overhead after initialization
    if (checkInfo.isPopulated) {
        return;
    }

    // Lock mutex to prevent race conditions in multithreaded environments
    os::memory::LockHolder lock(localeCheckInfoMutex);

    // Second check: Verify again after acquiring lock - prevents multiple initializations
    if (checkInfo.isPopulated) {
        return;
    }

    int32_t availableCount;
    const icu::Locale *availableLocales = icu::Locale::getAvailableLocales(availableCount);

    for (int i = 0; i < availableCount; ++i) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto &locale = availableLocales[i];
        const std::string lang = locale.getLanguage();
        const std::string region = locale.getCountry();
        const std::string script = locale.getScript();
        if (!lang.empty()) {
            checkInfo.langs.insert(lang);
        }
        if (!region.empty()) {
            checkInfo.regions.insert(region);
        }
        if (!script.empty()) {
            checkInfo.scripts.insert(script);
        }
    }

    // Cache the string representations
    checkInfo.langsList = Set2String(checkInfo.langs);
    checkInfo.regionsList = Set2String(checkInfo.regions);
    checkInfo.scriptsList = Set2String(checkInfo.scripts);
    checkInfo.isPopulated = true;
}

std::string Set2String(std::set<std::string> &inSet)
{
    const int lastCommaLength = 1;
    std::string outList;
    for (const auto &item : inSet) {
        outList += item;
        outList += ",";
    }
    if (outList.length() > lastCommaLength) {
        outList.erase(outList.length() - lastCommaLength);
    }
    return outList;
}

ani_string StdCoreIntlLocaleDefaultLang(ani_env *env, [[maybe_unused]] ani_class klass)
{
    const std::string lang = icu::Locale::getDefault().getLanguage();
    return StdStrToAni(env, lang);
}

ani_string StdCoreIntlLocaleRegionList(ani_env *env, [[maybe_unused]] ani_class klass)
{
    const std::string &regionsList = GetLocaleCheckInfo().regionsList;
    return StdStrToAni(env, regionsList);
}

ani_string StdCoreIntlLocaleLangList(ani_env *env, [[maybe_unused]] ani_class klass)
{
    const std::string &langsList = GetLocaleCheckInfo().langsList;
    return StdStrToAni(env, langsList);
}

ani_string StdCoreIntlLocaleScriptList(ani_env *env, [[maybe_unused]] ani_class klass)
{
    const std::string &scriptsList = GetLocaleCheckInfo().scriptsList;
    return StdStrToAni(env, scriptsList);
}

ani_string StdCoreIntlLocaleNumberingSystemList(ani_env *env, [[maybe_unused]] ani_class klass)
{
    UErrorCode success = U_ZERO_ERROR;
    std::unique_ptr<icu::StringEnumeration> numberingSystemList(icu::NumberingSystem::getAvailableNames(success));
    if (UNLIKELY(U_FAILURE(success))) {
        ASSERT(true);
    }
    std::string nsList;
    while (const icu::UnicodeString *ns = numberingSystemList->snext(success)) {
        if (UNLIKELY(U_FAILURE(success))) {
            ASSERT(true);
        }
        std::string str;
        ns->toUTF8String(str);
        nsList += str;
    }

    return StdStrToAni(env, nsList);
}

ani_string StdCoreIntlLocaleInfo(ani_env *env, [[maybe_unused]] ani_class klass, ani_string lang)
{
    std::string langKey = ConvertFromAniString(env, lang);

    UErrorCode success = U_ZERO_ERROR;
    auto l = icu::Locale::forLanguageTag(langKey, success);
    if (UNLIKELY(U_FAILURE(success))) {
        ASSERT(true);
    }
    l.addLikelySubtags(success);
    if (UNLIKELY(U_FAILURE(success))) {
        ASSERT(true);
    }

    std::string result;
    result += l.getCountry();
    result += ",";
    result += l.getScript();

    return StdStrToAni(env, result);
}

ani_int StdCoreIntlLocaleIsTagValid(ani_env *env, [[maybe_unused]] ani_class klass, ani_string tag)
{
    std::string tagKey = ConvertFromAniString(env, tag);

    UErrorCode success = U_ZERO_ERROR;
    icu::Locale::forLanguageTag(tagKey, success);
    if (UNLIKELY(U_FAILURE(success))) {
        return 1;
    }
    return 0;
}

static int UnicodeKeyToLocaleInfo(const std::string &unicodeKey)
{
    int keyIndex = -1;
    if (unicodeKey == "ca") {
        keyIndex = LocaleInfo::U_CA;
    } else if (unicodeKey == "co") {
        keyIndex = LocaleInfo::U_CO;
    } else if (unicodeKey == "kf") {
        keyIndex = LocaleInfo::U_KF;
    } else if (unicodeKey == "hc") {
        keyIndex = LocaleInfo::U_HC;
    } else if (unicodeKey == "nu") {
        keyIndex = LocaleInfo::U_NU;
    } else if (unicodeKey == "kn") {
        keyIndex = LocaleInfo::U_KN;
    }
    return keyIndex;
}

ani_ref StdCoreIntlLocaleParseTag(ani_env *env, [[maybe_unused]] ani_class klass, ani_string tag)
{
    std::vector<std::string> data = GetLocaleInfo();
    std::string tagKey = ConvertFromAniString(env, tag);
    UErrorCode success = U_ZERO_ERROR;

    auto l = icu::Locale::forLanguageTag(tagKey, success);
    if (UNLIKELY(U_FAILURE(success))) {
        std::string message = "Failed to find locale from tag";
        ThrowNewError(env, "std.core.RuntimeError", message.c_str(), "C{std.core.String}:");
        return nullptr;
    }

    data[LocaleInfo::LANG] = l.getLanguage();
    data[LocaleInfo::SCRIPT] = l.getScript();
    data[LocaleInfo::COUNTRY] = l.getCountry();

    std::set<std::string> keyWordsSet;
    l.getUnicodeKeywords<std::string>(std::insert_iterator<decltype(keyWordsSet)>(keyWordsSet, keyWordsSet.begin()),
                                      success);

    for (auto &unicodeKey : keyWordsSet) {
        auto unicodeKeywordValue = l.getUnicodeKeywordValue<std::string>(unicodeKey, success);
        if (UNLIKELY(U_FAILURE(success))) {
            std::string message = "Failed to getUnicodeKeywordValue";
            ThrowNewError(env, "std.core.RuntimeError", message.c_str(), "C{std.core.String}:");
            return nullptr;
        }
        int keyIndex = UnicodeKeyToLocaleInfo(unicodeKey);
        if (keyIndex >= LocaleInfo::U_CA && keyIndex < LocaleInfo::COUNT) {
            data[keyIndex] = unicodeKeywordValue;
        }
    }

    std::stringstream s;
    for (int i = 0; i < LocaleInfo::COUNT; ++i) {
        s << (data[i]) << " ";
    }
    std::string res = s.str();
    if (res.length() > 1) {
        res.resize(res.length() - 1);
    }

    return StdStrToAni(env, res);
}

ani_string StdCoreIntlLocaleDefaultTag(ani_env *env)
{
    auto defaultLocale = icu::Locale::getDefault();
    auto defaultLocaleName = defaultLocale.getName();
    if (strcmp(defaultLocaleName, "en_US_POSIX") == 0 || strcmp(defaultLocaleName, "c") == 0) {
        return StdStrToAni(env, "en-US");
    }
    auto status = UErrorCode::U_ZERO_ERROR;
    auto tag = defaultLocale.toLanguageTag<std::string>(status);
    if (UNLIKELY(U_FAILURE(status))) {
        std::string message = "Error receiving default locale language tag: ";
        message += u_errorName(status);
        ThrowNewError(env, "std.core.RuntimeError", message.c_str(), "C{std.core.String}:");
        return nullptr;
    }
    return StdStrToAni(env, tag);
}

ani_status RegisterIntlLocaleNativeMethods(ani_env *env)
{
    const auto methods = std::array {
        ani_native_function {"initLocale", ":", reinterpret_cast<void *>(StdCoreIntlLocaleFillCheckInfo)},
        ani_native_function {"maximizeInfo", "C{std.core.String}:C{std.core.String}",
                             reinterpret_cast<void *>(StdCoreIntlLocaleInfo)},
        ani_native_function {"regionList", ":C{std.core.String}",
                             reinterpret_cast<void *>(StdCoreIntlLocaleRegionList)},
        ani_native_function {"langList", ":C{std.core.String}", reinterpret_cast<void *>(StdCoreIntlLocaleLangList)},
        ani_native_function {"scriptList", ":C{std.core.String}",
                             reinterpret_cast<void *>(StdCoreIntlLocaleScriptList)},
        ani_native_function {"numberingSystemList", ":C{std.core.String}",
                             reinterpret_cast<void *>(StdCoreIntlLocaleNumberingSystemList)},
        ani_native_function {"defaultLang", ":C{std.core.String}",
                             reinterpret_cast<void *>(StdCoreIntlLocaleDefaultLang)},
        ani_native_function {"isTagValid", "C{std.core.String}:i",
                             reinterpret_cast<void *>(StdCoreIntlLocaleIsTagValid)},
        ani_native_function {"parseTagImpl", "C{std.core.String}:C{std.core.String}",
                             reinterpret_cast<void *>(StdCoreIntlLocaleParseTag)},
    };

    ani_class localeClass;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.Intl.Locale", &localeClass));
    ani_status status = env->Class_BindNativeMethods(localeClass, methods.data(), methods.size());
    if (status != ANI_OK) {
        return status;
    }

    const std::array staticMethods = {
        ani_native_function {"defaultTag", ":C{std.core.String}",
                             reinterpret_cast<void *>(StdCoreIntlLocaleDefaultTag)},
    };

    return env->Class_BindStaticNativeMethods(localeClass, staticMethods.data(), staticMethods.size());
}

ani_string StdCoreIntlLocaleDefaultBaseName(ani_env *env, [[maybe_unused]] ani_class klass)
{
    const std::string name = icu::Locale::getDefault().getBaseName();
    return StdStrToAni(env, name);
}

}  // namespace ark::ets::stdlib::intl
