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

#ifndef GUARD_UTIL_STRING_UTIL_H
#define GUARD_UTIL_STRING_UTIL_H

#include <string>
#include <vector>

namespace ark::guard {

/**
 * @brief StringUtil
 */
class StringUtil {
public:
    /**
     * @brief replace slash with dot
     * @param str string with slash
     * @return string with dot
     * e.g. "a/b/c" ==> "a.b.c"
     */
    static std::string ReplaceSlashWithDot(const std::string &str);

    /**
     * @brief replace slash with dot
     * @param strs vector string with slash
     * @return vector string with dot
     * e.g. {"a/b/c", "a/b/d"} ==> {"a.b.c", "a.b.d"}
     */
    static std::vector<std::string> ReplaceSlashWithDot(const std::vector<std::string> &strs);

    /**
     * @brief Check if the string matches the substring starting from a fixed position
     * @param str main string
     * @param subStr sub string
     * @param start the starting position specified in the main string, defaults to 0
     * @return true: match, false: not match
     */
    static bool IsSubStrMatched(const std::string &str, const std::string &subStr, size_t start = 0);

    /**
     * @brief Checks if two strings match according to the pattern
     * @param inputStr The input string to be checked
     * @param pattern The pattern string (can be either plain string or regex pattern)
     * @return true if strings match, false otherwise
     */
    static bool IsMatch(const std::string &inputStr, const std::string &pattern);

    /**
     * @brief Convert wildcard patterns to C++ regular expressions. Used for name conversion (class name/member name).
     * Support wildcard characters:
     * - ? Match a single non-package delimiter character
     * - *: Match multiple non-package delimiter characters
     * - **: Match any character (including package separators)
     * -<n>: Reference the nth wildcard matching result (reverse reference)
     *  @param input The string to be transformed
     *  @return transformed string
     */
    static std::string ConvertWildcardToRegex(const std::string &input);

    /**
     * @brief Convert wildcard patterns to C++ regular expressions. Used to description conversions (types/parameters).
     * Support wildcard characters:
     * - ? Match a single non-package delimiter character
     * - *: Match multiple non-package delimiter characters
     * - **: Match any character (including package separators)
     * - %: Match any primitive type ("string", "int", etc.) or "void" type
     * - ***: Match any type (primitive or non-primitive, array or non-array)
     * - ...: Match any number of arguments of any type
     * -<n>: Reference the nth wildcard matching result (reverse reference)
     *  @param input The string to be transformed
     *  @return transformed string
     */
    static std::string ConvertWildcardToRegexEx(const std::string &input);

    /**
     * @brief removes a specified prefix from a string if it matches the beginning
     * @param inputStr the string to be processed
     * @param prefix prefix part;
     * @attention prefix cannot be empty
     * @return if inputStr matched the prefix, return a string with prefix-removed,
     * @return if inputStr not match the prefix, return the origin inputString content
     */
    static std::string RemovePrefixIfMatches(const std::string& inputStr, const std::string& prefix);

    /**
     * @brief ensure inputStr start with specific prefix
     * @param inputStr the string to be processed
     * @param prefix prefix part
     * @return check if the string start with prefix; if not, add prefix.
     */
    static std::string EnsureStartWithPrefix(const std::string& inputStr, const std::string& prefix);

    /**
     * @brief ensure inputStr end with specific suffix
     * @param inputStr the string to be processed
     * @param suffix suffix part
     * @return check if the string ends with suffix; if not, append suffix.
     */
    static std::string EnsureEndWithSuffix(const std::string& inputStr, const std::string& suffix);

    /**
     * @brief find the length of the longest prefix that matches the input string.
     * @param prefixes the list of prefixes to be matched
     * @param input the string to be processed
     * @return the length of the longest prefix
     */
    static size_t FindLongestMatchedPrefix(const std::vector<std::string>& prefixes, const std::string& input);

    /**
     * @brief find the length of the longest regular expression prefix that matches the input string.
     * @param prefixes the list of regular expression prefixes to be matched;
     * @param input the string to be processed
     * @attention ensure expression start with '^'
     * @return the length of the longest prefix
     */
    static size_t FindLongestMatchedPrefixReg(const std::vector<std::string>& prefixes, const std::string& input);

     /**
     * @brief remove all spaces in the string
     * @param input the string to be processed
     * @return the string with no spaces
     */
    static std::string RemoveAllSpaces(const std::string &input);
};
}  // namespace ark::guard

#endif  // GUARD_UTIL_STRING_UTIL_H
