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

#include "IntlCollator.h"
#include "IntlCommon.h"
#include "IntlState.h"
#include "IntlCollatorCache.h"
#include "ani.h"
#include "plugins/ets/runtime/ets_exceptions.h"
#include "libarkbase/macros.h"
#include "stdlib_ani_helpers.h"
#include <unicode/coll.h>
#include <unicode/locid.h>
#include <unicode/stringpiece.h>
#include <unicode/translit.h>

#include <algorithm>
#include <string>
#include <array>
#include "IntlLocaleMatch.h"

namespace ark::ets::stdlib::intl {

// https://stackoverflow.com/questions/2992066/code-to-strip-diacritical-marks-using-icu
std::string RemoveAccents(ani_env *env, const std::string &str)
{
    // UTF-8 std::string -> UTF-16 UnicodeString
    icu::UnicodeString source = icu::UnicodeString::fromUTF8(icu::StringPiece(str));

    // Transliterate UTF-16 UnicodeString
    UErrorCode status = U_ZERO_ERROR;
    icu::Transliterator *accentsConverter =
        icu::Transliterator::createInstance("NFD; [:M:] Remove; NFC", UTRANS_FORWARD, status);
    accentsConverter->transliterate(source);
    delete accentsConverter;
    if (UNLIKELY(U_FAILURE(status))) {
        ThrowNewError(env, "std.core.RuntimeError", "Removing accents failed, transliterate failed",
                      "C{std.core.String}:");
        return std::string();
    }

    // UTF-16 UnicodeString -> UTF-8 std::string
    std::string result;
    source.toUTF8String(result);

    return result;
}

void RemovePunctuation(std::string &text)
{
    text.erase(std::remove_if(text.begin(), text.end(), ispunct), text.end());
}

ani_string StdCoreIntlCollatorRemoveAccents(ani_env *env, [[maybe_unused]] ani_class klass, ani_string etsText)
{
    std::string text = ConvertFromAniString(env, etsText);
    text = RemoveAccents(env, text);

    ani_boolean unhandledExc;
    ANI_FATAL_IF_ERROR(env->ExistUnhandledError(&unhandledExc));
    if (unhandledExc != 0U) {
        return nullptr;
    }

    return StdStrToAni(env, text);
}

ani_string StdCoreIntlCollatorRemovePunctuation(ani_env *env, [[maybe_unused]] ani_class klass, ani_string etsText)
{
    std::string text = ConvertFromAniString(env, etsText);
    RemovePunctuation(text);
    return StdStrToAni(env, text);
}

ani_double StdCoreIntlCollatorLocaleCmp(ani_env *env, [[maybe_unused]] ani_class klass, ani_string collationIn,
                                        ani_string langIn, ani_string firstStr, ani_string secondStr,
                                        ani_string caseFirstIn)
{
    auto collation = ConvertFromAniString(env, collationIn);
    auto lang = ConvertFromAniString(env, langIn);
    auto str1 = ConvertFromAniString(env, firstStr);
    auto str2 = ConvertFromAniString(env, secondStr);
    auto caseFirst = ConvertFromAniString(env, caseFirstIn);

    icu::Collator *collator = g_intlState->collatorCache.GetOrCreateCollator(env, lang, collation, caseFirst);
    if (collator == nullptr) {
        ThrowNewError(env, "std.core.RuntimeError", "Failed to create Collator instance for comparison",
                      "C{std.core.String}:");
        return 0;
    }

    UErrorCode status = U_ZERO_ERROR;
    icu::UnicodeString source = StdStrToUnicode(str1);
    icu::UnicodeString target = StdStrToUnicode(str2);
    auto res = collator->compare(source, target, status);
    if (UNLIKELY(U_FAILURE(status))) {
        ThrowNewError(env, "std.core.RuntimeError", "Comparison failed", "C{std.core.String}:");
    }
    return res;
}

ani_status RegisterIntlCollator(ani_env *env)
{
    const auto methods =
        std::array {ani_native_function {"removePunctuation", "C{std.core.String}:C{std.core.String}",
                                         reinterpret_cast<void *>(StdCoreIntlCollatorRemovePunctuation)},
                    ani_native_function {"removeAccents", "C{std.core.String}:C{std.core.String}",
                                         reinterpret_cast<void *>(StdCoreIntlCollatorRemoveAccents)},
                    ani_native_function {
                        "compareByCollation",
                        "C{std.core.String}C{std.core.String}C{std.core.String}C{std.core.String}C{std.core.String}:d",
                        reinterpret_cast<void *>(StdCoreIntlCollatorLocaleCmp)}};

    ani_class collatorClass;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.Intl.Collator", &collatorClass));
    return env->Class_BindNativeMethods(collatorClass, methods.data(), methods.size());
}

}  // namespace ark::ets::stdlib::intl
