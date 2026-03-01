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

#include "plugins/ets/stdlib/native/core/IntlSegmenter.h"
#include "plugins/ets/stdlib/native/core/IntlLocaleMatch.h"
#include "plugins/ets/stdlib/native/core/IntlCommon.h"
#include "stdlib_ani_helpers.h"
#include "libarkbase/macros.h"
#include "unicode/unistr.h"
#include "unicode/brkiter.h"
#include "unicode/ubrk.h"

#include <cstring>
#include <string>
#include <vector>
#include <array>
#include <cassert>
#include <optional>

namespace ark::ets::stdlib::intl {

using BreakerFactory = icu::BreakIterator *(*)(const icu::Locale &, UErrorCode &);

/// @brief Helper function that converts an ETS string to ICU Unicode string
icu::UnicodeString AniToUnicode(ani_env *env, ani_string etsStr)
{
    auto str = ConvertFromAniString(env, etsStr);
    icu::UnicodeString unicodeStr = icu::UnicodeString::fromUTF8(str);
    return unicodeStr;
}

/// @brief Converts an ETS string containing a BCP47 language tag to ICU Locale
std::optional<icu::Locale> EtsToLocale(ani_env *env, const ani_string &bcp47Locale)
{
    UErrorCode status = U_ZERO_ERROR;
    std::string stdStr = ConvertFromAniString(env, bcp47Locale);

    icu::Locale locale = icu::Locale::forLanguageTag(stdStr, status);
    if (UNLIKELY(U_FAILURE(status))) {
        std::cout << "ICU error: " << u_errorName(status) << std::endl;
        return std::nullopt;
    }
    if (locale.isBogus() != 0) {
        std::cout << "Created locale is bogus" << std::endl;
        return std::nullopt;
    }
    return std::optional<icu::Locale> {locale};
}

/**
 * @brief Creates a new IntlCluster object with the specified properties
 * @param env Pointer to ANI environment
 * @param klass ANI class reference (unused)
 * @param cluster String containing the cluster text
 * @param index Starting index of the cluster in original string
 * @param isWordLike Boolean indicating if cluster represents a word-like segment
 * @return Pointer to newly created IntlCluster object, nullptr if creation fails
 * @throws RuntimeError if class/constructor/field lookups fail
 */
ani_object StdCoreIntlCreateClusterObject(ani_env *env, [[maybe_unused]] ani_class klass, ani_string cluster,
                                          ani_int index, ani_boolean isWordLike)
{
    // Find the cluster class
    ani_class clusterClass;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.Intl.Cluster", &clusterClass));

    // Find the constructor method
    ani_method constructorMethod;
    ANI_FATAL_IF_ERROR(env->Class_FindMethod(clusterClass, "<ctor>", ":", &constructorMethod));

    // Create a new instance
    ani_object clusterObj;

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    ANI_FATAL_IF_ERROR(env->Object_New(clusterClass, constructorMethod, &clusterObj));

    // Get field IDs
    ani_field clusterField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(clusterClass, "cluster", &clusterField));

    ani_field indexField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(clusterClass, "index", &indexField));

    ani_field isWordLikeField;
    ANI_FATAL_IF_ERROR(env->Class_FindField(clusterClass, "isWordLike", &isWordLikeField));

    // Set field values
    ANI_FATAL_IF_ERROR(env->Object_SetField_Ref(clusterObj, clusterField, cluster));
    ANI_FATAL_IF_ERROR(env->Object_SetField_Int(clusterObj, indexField, index));
    ANI_FATAL_IF_ERROR(env->Object_SetField_Boolean(clusterObj, isWordLikeField, isWordLike));

    return clusterObj;
}

/**
 * @brief Determines if current cluster segment represents a word-like unit
 * @param break_iterator Reference to ICU break iterator
 * @return true if current cluster is word-like, false otherwise
 */
ani_boolean IntlCurrentClusterIsWordLike(std::unique_ptr<icu::BreakIterator> &breakIterator)
{
    auto ruleStatus = static_cast<int>(breakIterator->getRuleStatus());
    // Number-type words (digits, numbers)
    bool result = (ruleStatus >= UBRK_WORD_NUMBER && ruleStatus < UBRK_WORD_NUMBER_LIMIT);
    // Letter-based words (alphabetic characters)
    result = result || (ruleStatus >= UBRK_WORD_LETTER && ruleStatus < UBRK_WORD_LETTER_LIMIT);
    // Kana words (Japanese Hiragana/Katakana characters)
    result = result || (ruleStatus >= UBRK_WORD_KANA && ruleStatus < UBRK_WORD_KANA_LIMIT);
    // Ideographic words (Chinese/Japanese/Korean characters)
    result = result || (ruleStatus >= UBRK_WORD_IDEO && ruleStatus < UBRK_WORD_IDEO_LIMIT);
    return static_cast<ani_boolean>(result);
}

/**
 * @brief Generic function to segment text into clusters using specified ICU break iterator
 * @param env Pointer to ANI environment
 * @param klass ANI class reference
 * @param factory Function pointer to create specific type of ICU break iterator
 * @param str Input string to segment
 * @param localeStr BCP47 language tag string for locale-specific segmentation
 * @return Array of IntlCluster objects representing the segments, nullptr if operation fails
 * @throws RuntimeError if locale creation or break iterator initialization fails
 */
ani_array IntlClusters(ani_env *env, [[maybe_unused]] ani_class klass, BreakerFactory factory, ani_string str,
                       ani_string localeStr)
{
    std::optional<icu::Locale> locale = EtsToLocale(env, localeStr);
    if (!locale) {
        std::string message = "Unable to create ICU locale for specified tag (bcp47): ";
        message += ConvertFromAniString(env, localeStr);
        ThrowNewError(env, "std.core.RuntimeError", message.c_str(), "C{std.core.String}:");
        return nullptr;
    }
    icu::Locale breakLocale = locale.value();

    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<icu::BreakIterator> breaker(factory(breakLocale, status));
    if (UNLIKELY(U_FAILURE(status))) {
        std::string message = "Unable to create break iterator";
        ThrowNewError(env, "std.core.RuntimeError", message.c_str(), "C{std.core.String}:");
        return nullptr;
    }

    icu::UnicodeString uniStr = AniToUnicode(env, str);
    breaker->setText(uniStr);

    std::vector<ani_object> clusters;
    int32_t current = breaker->first();
    int32_t next = breaker->next();

    // Process each segment
    while (next != icu::BreakIterator::DONE) {
        icu::UnicodeString cluster = uniStr.tempSubStringBetween(current, next);
        std::string utf8Cluster;
        cluster.toUTF8String(utf8Cluster);

        ani_string clusterStr = StdStrToAni(env, utf8Cluster);
        ani_boolean isWordLike = IntlCurrentClusterIsWordLike(breaker);
        ani_object clusterObject = StdCoreIntlCreateClusterObject(env, klass, clusterStr, current, isWordLike);

        clusters.push_back(clusterObject);
        current = next;
        next = breaker->next();
    }

    // Find cluster class for array creation
    ani_class clusterClass;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.Intl.Cluster", &clusterClass));

    // Create array of the correct size
    ani_ref undefined {};
    ANI_FATAL_IF_ERROR(env->GetUndefined(&undefined));
    ani_array resultArray;
    ANI_FATAL_IF_ERROR(env->Array_New(clusters.size(), undefined, &resultArray));

    // Fill the array with cluster objects
    for (size_t i = 0; i < clusters.size(); ++i) {
        ANI_FATAL_IF_ERROR(env->Array_Set(resultArray, i, clusters[i]));
    }

    return resultArray;
}

/**
 * @brief Segments text into grapheme clusters
 * @param env Pointer to ANI environment
 * @param klass ANI class reference
 * @param str Input string to segment
 * @param localeStr BCP47 language tag string for locale-specific segmentation
 * @return Array of IntlCluster objects representing grapheme clusters
 */
ani_array StdCoreIntlGraphemeClusters(ani_env *env, ani_class klass, ani_string str, ani_string localeStr)
{
    return IntlClusters(env, klass, icu::BreakIterator::createCharacterInstance, str, localeStr);
}

/**
 * @brief Segments text into word clusters
 * @param env Pointer to ANI environment
 * @param klass ANI class reference
 * @param str Input string to segment
 * @param localeStr BCP47 language tag string for locale-specific segmentation
 * @return Array of IntlCluster objects representing word clusters
 */
ani_array StdCoreIntlWordClusters(ani_env *env, ani_class klass, ani_string str, ani_string localeStr)
{
    return IntlClusters(env, klass, icu::BreakIterator::createWordInstance, str, localeStr);
}

/**
 * @brief Segments text into sentence clusters
 * @param env Pointer to ANI environment
 * @param klass ANI class reference
 * @param str Input string to segment
 * @param localeStr BCP47 language tag string for locale-specific segmentation
 * @return Array of IntlCluster objects representing sentence clusters
 */
ani_array StdCoreIntlSentenceClusters(ani_env *env, ani_class klass, ani_string str, ani_string localeStr)
{
    return IntlClusters(env, klass, icu::BreakIterator::createSentenceInstance, str, localeStr);
}

/**
 * @brief Registers native methods for IntlSegmenter class
 * @param env Pointer to ETS environment
 * @return Status code indicating success/failure of registration
 */
ani_status RegisterIntlSegmenter(ani_env *env)
{
    const auto methods = std::array {
        ani_native_function {"graphemeClusters", "C{std.core.String}C{std.core.String}:C{std.core.Array}",
                             reinterpret_cast<void *>(StdCoreIntlGraphemeClusters)},
        ani_native_function {"wordClusters", "C{std.core.String}C{std.core.String}:C{std.core.Array}",
                             reinterpret_cast<void *>(StdCoreIntlWordClusters)},
        ani_native_function {"sentenceClusters", "C{std.core.String}C{std.core.String}:C{std.core.Array}",
                             reinterpret_cast<void *>(StdCoreIntlSentenceClusters)},
    };

    ani_class segmenterClass;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.Intl.Segmenter", &segmenterClass));

    return env->Class_BindStaticNativeMethods(segmenterClass, methods.data(), methods.size());
}

}  // namespace ark::ets::stdlib::intl
