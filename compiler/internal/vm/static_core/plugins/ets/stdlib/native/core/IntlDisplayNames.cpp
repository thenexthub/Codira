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

#include "plugins/ets/stdlib/native/core/IntlDisplayNames.h"
#include "plugins/ets/stdlib/native/core/IntlCommon.h"
#include "plugins/ets/stdlib/native/core/IntlState.h"
#include "plugins/ets/stdlib/native/core/stdlib_ani_helpers.h"
#include "libarkbase/macros.h"
#include "unicode/numberformatter.h"
#include "unicode/numberrangeformatter.h"
#include "unicode/locid.h"
#include "unicode/unistr.h"
#include "unicode/uloc.h"
#include "unicode/ucurr.h"
#include "unicode/uldnames.h"
#include "unicode/udisplaycontext.h"
#include "IntlCommon.h"

#include <string>
#include <string_view>
#include <cstdlib>
#include <array>
#include <optional>
#include <algorithm>
#include <memory>
#include <cctype>

namespace ark::ets::stdlib::intl {

/**
 * @enum CodeType
 * @brief Enumeration of supported display name types
 *
 * This enum defines the different types of codes that can be used for localized
 * display names, following international standards.
 */
enum class CodeType {
    /** Language code (ISO 639) */
    LANGUAGE,
    /** Script code (ISO 15924) */
    SCRIPT,
    /** Region/Country code (ISO 3166) */
    REGION,
    /** Currency code (ISO 4217) */
    CURRENCY
};

constexpr int32_t ISO_SCRIPT_CODE_LENGTH = 4;
constexpr int32_t ISO_REGION_ALPHA_LENGTH = 2;
constexpr int32_t ISO_REGION_NUMERIC_LENGTH = 3;
constexpr int32_t ISO_CURRENCY_CODE_LENGTH = 3;
constexpr int32_t BUFFER_CAPACITY_SMALL = 100;
constexpr int32_t BUFFER_CAPACITY_LARGE = 200;

constexpr size_t FIRST_CHAR_INDEX = 0;
constexpr size_t SECOND_CHAR_INDEX = 1;
constexpr size_t THIRD_CHAR_INDEX = 2;
constexpr size_t FOURTH_CHAR_INDEX = 3;

/**
 * @brief Validates a script code according to ISO 15924 standard
 *
 * Script codes should be 4 letters with the first letter uppercase and
 * the remaining letters lowercase (e.g., "Latn", "Cyrl", "Arab").
 *
 * @param code The script code to validate
 * @return true if the code is a valid ISO 15924 script code, false otherwise
 */
[[nodiscard]] constexpr bool IsValidScriptCode(std::string_view code) noexcept
{
    // Script codes should be 4 letters according to ISO-15924
    if (code.length() != ISO_SCRIPT_CODE_LENGTH) {
        return false;
    }

    // First letter should be uppercase, the rest lowercase
    return (std::isupper(code[FIRST_CHAR_INDEX]) != 0) && (std::islower(code[SECOND_CHAR_INDEX]) != 0) &&
           (std::islower(code[THIRD_CHAR_INDEX]) != 0) && (std::islower(code[FOURTH_CHAR_INDEX]) != 0);
}

/**
 * @brief Validates a region code according to ISO 3166 or UN M49 standards
 * Valid region codes are either:
 * - Two uppercase letters (ISO 3166-1 alpha-2, e.g., "US", "JP", "DE")
 * - Three digits (UN M49 numeric code, e.g., "840", "392", "276")
 *
 * @param code The region code to validate
 * @return true if the code is a valid region code, false otherwise
 */
[[nodiscard]] constexpr bool IsValidRegionCode(std::string_view code) noexcept
{
    // Region codes are either 2 uppercase letters (ISO-3166) or 3 digits (UN M49)
    if (code.length() == ISO_REGION_ALPHA_LENGTH) {
        return (std::isupper(code[FIRST_CHAR_INDEX]) != 0) && (std::isupper(code[SECOND_CHAR_INDEX]) != 0);
    }

    if (code.length() == ISO_REGION_NUMERIC_LENGTH) {
        return (std::isdigit(code[FIRST_CHAR_INDEX]) != 0) && (std::isdigit(code[SECOND_CHAR_INDEX]) != 0) &&
               (std::isdigit(code[THIRD_CHAR_INDEX]) != 0);
    }

    return false;
}

/**
 * @brief Validates a currency code according to ISO 4217 standard
 *
 * Currency codes should be 3 uppercase letters (e.g., "USD", "EUR", "JPY").
 *
 * @param code The currency code to validate
 * @return true if the code is a valid ISO 4217 currency code, false otherwise
 */
[[nodiscard]] constexpr bool IsValidCurrencyCode(std::string_view code) noexcept
{
    // Currency codes should be 3 uppercase letters according to ISO-4217
    if (code.length() != ISO_CURRENCY_CODE_LENGTH) {
        return false;
    }

    return (std::isupper(code[FIRST_CHAR_INDEX]) != 0) && (std::isupper(code[SECOND_CHAR_INDEX]) != 0) &&
           (std::isupper(code[THIRD_CHAR_INDEX]) != 0);
}

/**
 * @brief Validates a language code according to BCP 47 standard
 *
 * Performs basic validation for language codes according to BCP 47.
 * Language codes should be 2-3 letters (ISO 639) or 5-8 characters for language-extlang.
 *
 * @param code The language code to validate
 * @return true if the code appears to be a valid BCP 47 language code, false otherwise
 */
[[nodiscard]] bool IsValidLanguageCode(std::string_view code)
{
    // Basic validation for language codes according to BCP 47
    // Language codes should be 2-3 letters (ISO 639) or 5-8 characters for language-extlang
    if (code.empty()) {
        return false;
    }

    // Check if the code uses valid characters
    if (!std::all_of(code.begin(), code.end(), [](char c) { return (std::isalpha(c) != 0) || c == '-' || c == '_'; })) {
        return false;
    }

    // Additional validation: try canonicalizing with ICU and check for errors
    UErrorCode status = U_ZERO_ERROR;
    std::array<char, BUFFER_CAPACITY_SMALL> canonical {};

    uloc_canonicalize(std::string(code).c_str(), canonical.data(), BUFFER_CAPACITY_SMALL, &status);

    return (U_SUCCESS(status) != 0);
}

/**
 * @brief Canonicalizes a language code according to BCP 47 standard
 *
 * Attempts to convert a language code to its canonical form using ICU.
 * If canonicalization fails, the original code is returned.
 *
 * @param code The language code to canonicalize
 * @return The canonicalized language code, or the original if canonicalization fails
 */
[[nodiscard]] std::string CanonicalizeLanguageCode(std::string_view code)
{
    UErrorCode status = U_ZERO_ERROR;
    std::array<char, BUFFER_CAPACITY_SMALL> canonical {};

    uloc_canonicalize(std::string(code).c_str(), canonical.data(), BUFFER_CAPACITY_SMALL, &status);

    if ((U_FAILURE(status) != 0)) {
        return std::string(code);  // Return original if canonicalization fails
    }

    return std::string(canonical.data());
}

/**
 * @brief Converts a string type identifier to the corresponding CodeType enum
 *
 * @param typeStr The type string to convert ("language", "script", "region", or "currency")
 * @param isValid Reference to a boolean that will be set to false if the type is invalid
 * @return The corresponding CodeType enum value
 */
[[nodiscard]] CodeType StringToCodeType(std::string_view typeStr, bool &isValid) noexcept
{
    isValid = true;

    if (typeStr == "language") {
        return CodeType::LANGUAGE;
    }
    if (typeStr == "script") {
        return CodeType::SCRIPT;
    }
    if (typeStr == "region") {
        return CodeType::REGION;
    }
    if (typeStr == "currency") {
        return CodeType::CURRENCY;
    }

    isValid = false;
    return CodeType::LANGUAGE;  // Default value, not used when isValid is false
}

/**
 * @brief Generic template function to retrieve a display name using ICU
 *
 * This function handles the common pattern of calling an ICU function to get a
 * display name, dealing with buffer allocation, error checking, and result conversion.
 *
 * @tparam Func Type of the function to call (usually a function pointer or lambda)
 * @param displayNames The ICU ULocaleDisplayNames object to use
 * @param code The code to get the display name for
 * @param getNameFunc The function to call to get the display name
 * @return An optional containing the Unicode string result, or std::nullopt if the operation failed
 */
template <typename Func>
std::optional<icu::UnicodeString> GetDisplayName(ULocaleDisplayNames *displayNames, const char *code, Func getNameFunc)
{
    UErrorCode status = U_ZERO_ERROR;
    std::array<UChar, BUFFER_CAPACITY_LARGE> buffer {};

    int32_t length = getNameFunc(displayNames, code, buffer.data(), BUFFER_CAPACITY_LARGE, &status);
    if ((U_SUCCESS(status) != 0) && length > 0) {
        return icu::UnicodeString(buffer.data(), length);
    }
    return std::nullopt;
}

/**
 * @brief Validates code and returns canonicalized form if valid
 *
 * @param env The environment
 * @param codeType The type of code to validate
 * @param codeStr The code string to validate
 * @param fallbackStr The fallback behavior if validation fails
 * @param isValid Reference to a boolean that will be set to the validation result
 * @return The canonicalized code if valid, or the original code otherwise
 */
static std::string ValidateAndCanonicalizeCode(ani_env *env, CodeType codeType, const std::string &codeStr,
                                               const std::string &fallbackStr, bool &isValid)
{
    std::string canonicalizedCode = codeStr;

    switch (codeType) {
        case CodeType::LANGUAGE:
            isValid = IsValidLanguageCode(codeStr);
            if (isValid) {
                canonicalizedCode = CanonicalizeLanguageCode(codeStr);
            }
            break;
        case CodeType::SCRIPT:
            isValid = IsValidScriptCode(codeStr);
            break;
        case CodeType::REGION:
            isValid = IsValidRegionCode(codeStr);
            break;
        case CodeType::CURRENCY:
            isValid = IsValidCurrencyCode(codeStr);
            break;
    }

    if (!isValid) {
        if (fallbackStr == "code") {
            return codeStr;
        }
        ThrowNewError(env, "std.core.RangeError", ("Invalid code: " + codeStr).c_str(), "C{std.core.String}:");
    }

    return canonicalizedCode;
}

/**
 * @brief Gets the display name based on the code type
 *
 * @param displayNames The ICU ULocaleDisplayNames object to use
 * @param codeType The type of code
 * @param canonicalizedCode The canonicalized code to get the display name for
 * @return An optional containing the Unicode string result
 */
static std::optional<icu::UnicodeString> GetDisplayNameByType(ULocaleDisplayNames *displayNames, CodeType codeType,
                                                              const std::string &canonicalizedCode)
{
    std::optional<icu::UnicodeString> result;

    switch (codeType) {
        case CodeType::LANGUAGE:
            result = GetDisplayName(displayNames, canonicalizedCode.c_str(), uldn_localeDisplayName);
            break;
        case CodeType::SCRIPT:
            result = GetDisplayName(displayNames, canonicalizedCode.c_str(), uldn_scriptDisplayName);
            break;
        case CodeType::REGION:
            result = GetDisplayName(displayNames, canonicalizedCode.c_str(), uldn_regionDisplayName);
            break;
        case CodeType::CURRENCY: {
            result = GetDisplayName(
                displayNames, canonicalizedCode.c_str(),
                [](ULocaleDisplayNames *ldn, const char *code, UChar *dest, int32_t destCapacity, UErrorCode *uStatus) {
                    return uldn_keyValueDisplayName(ldn, "currency", code, dest, destCapacity, uStatus);
                });
            if (!result) {
                // Fallback: just use the currency code
                result = icu::UnicodeString(canonicalizedCode.c_str(), static_cast<int32_t>(canonicalizedCode.length()),
                                            "UTF-8");
            }
            break;
        }
    }

    return result;
}

/// @brief Safely closes a ULocaleDisplayNames object
static inline void SafeCloseDisplayNames(ULocaleDisplayNames *displayNames)
{
    if (displayNames != nullptr) {
        uldn_close(displayNames);
    }
}

/**
 * @brief Native implementation of DisplayNames.of() method
 *
 * Retrieves a localized display name for a given code based on the specified type and locale.
 *
 * @param env The environment
 * @param klass The class from which this method was called (unused)
 * @param locale The BCP 47 language tag for the desired locale
 * @param type The type of code ("language", "script", "region", or "currency")
 * @param code The code to get the display name for
 * @param style The style of the display name ("long" or "short")
 * @param fallback The fallback behavior if the display name cannot be found ("code" or "none")
 * @param languageDisplay How to display language names with dialects ("dialect" or "standard")
 * @return The localized display name as an ani_string, or nullptr if not found and fallback is "none"
 */
static ani_string StdCoreIntlDisplayNamesOf(ani_env *env, [[maybe_unused]] ani_class klass, ani_string locale,
                                            ani_string type, ani_string code, ani_string style, ani_string fallback,
                                            ani_string languageDisplay)
{
    auto localeStr = ConvertFromAniString(env, locale);
    auto typeStr = ConvertFromAniString(env, type);
    auto codeStr = ConvertFromAniString(env, code);
    auto styleStr = ConvertFromAniString(env, style);
    auto fallbackStr = ConvertFromAniString(env, fallback);
    ani_boolean isUndefined = ANI_FALSE;
    [[maybe_unused]] auto aniStatus = env->Reference_IsUndefined(languageDisplay, &isUndefined);
    ASSERT(aniStatus == ANI_OK);
    auto languageDisplayStr = (isUndefined == ANI_FALSE) ? ConvertFromAniString(env, languageDisplay) : "dialect";

    bool isValidCodeType = true;
    auto codeType = StringToCodeType(typeStr, isValidCodeType);

    if (!isValidCodeType) {
        ThrowNewError(env, "std.core.RangeError", ("Invalid type: " + typeStr).c_str(), "C{std.core.String}:");
        return nullptr;
    }

    bool isValidCode = true;
    std::string canonicalizedCode = ValidateAndCanonicalizeCode(env, codeType, codeStr, fallbackStr, isValidCode);
    if (!isValidCode) {
        return (fallbackStr == "code") ? StdStrToAni(env, codeStr) : nullptr;
    }

    // Set up ICU locale
    UErrorCode status = U_ZERO_ERROR;
    auto icuLocale = icu::Locale::forLanguageTag(localeStr, status);
    if ((U_FAILURE(status) != 0)) {
        ThrowNewError(env, "std.core.RangeError", ("Invalid locale tag: " + localeStr).c_str(), "C{std.core.String}:");
        return nullptr;
    }

    // Determine ICU dialect handling and open display names
    auto dialectHandling = (styleStr == "short") ? ULDN_DIALECT_NAMES : ULDN_STANDARD_NAMES;
    ULocaleDisplayNames *displayNames = uldn_open(localeStr.c_str(), dialectHandling, &status);

    // Handle failure to create display names
    if ((U_FAILURE(status) != 0) || displayNames == nullptr) {
        SafeCloseDisplayNames(displayNames);
        return (fallbackStr == "code") ? StdStrToAni(env, codeStr) : nullptr;
    }

    std::optional<icu::UnicodeString> result = GetDisplayNameByType(displayNames, codeType, canonicalizedCode);

    ani_string retVal = nullptr;
    bool hasValidResult = result.has_value() && (result->isEmpty() == 0);
    if (hasValidResult) {
        retVal = UnicodeToAniStr(env, *result);
    } else if (fallbackStr == "code") {
        retVal = StdStrToAni(env, codeStr);
    } else {
        ThrowNewError(env, "std.core.RangeError", ("Invalid code: " + codeStr).c_str(), "C{std.core.String}:");
    }

    SafeCloseDisplayNames(displayNames);
    return retVal;
}

/**
 * @brief Registers the native methods for the Intl.DisplayNames class
 *
 * This function binds the native C++ implementation of the DisplayNames methods
 *
 * @param env The environment
 * @return ani_status indicating success or failure of the registration
 */
ani_status RegisterIntlDisplayNames(ani_env *env)
{
    const auto methods = std::array {
        ani_native_function {"ofNative",
                             "C{std.core.String}C{std.core.String}C{std.core.String}C{std.core.String}C{std.core."
                             "String}C{std.core.String}:C{std.core.String}",
                             reinterpret_cast<void *>(StdCoreIntlDisplayNamesOf)},
    };

    ani_class displayNamesClass;
    ANI_FATAL_IF_ERROR(env->FindClass("std.core.Intl.DisplayNames", &displayNamesClass));

    return env->Class_BindStaticNativeMethods(displayNamesClass, methods.data(), methods.size());
}

}  // namespace ark::ets::stdlib::intl
