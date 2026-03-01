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

#ifndef PANDA_GUARD_UTIL_STRING_UTIL_H
#define PANDA_GUARD_UTIL_STRING_UTIL_H

#include <string>
#include <tuple>
#include <vector>

namespace panda::guard {

class StringUtil {
public:
    /**
     * Split str by delimiter
     * @param str split str
     * @param delimiter split delimiter
     * @return split tokens, if split failed return empty vector
     * e.g. Split("##a#", "#"); => ["a"]
     */
    static std::vector<std::string> Split(const std::string &str, const std::string &delimiter);

    /**
     * Strictly split str by delimiter, which will gives empty part
     * @param str split str
     * @param delimiter split delimiter
     * @return split tokens, if split failed return empty vector
     * e.g. Split("##a#", "#"); => ["", "", "a", ""]
     */
    static std::vector<std::string> StrictSplit(const std::string &str, const std::string &delimiter);

    /**
     * Split from right by delimiter
     * @param str split str
     * @param delimiter split delimiter
     * @return (left, right) if split failed return tuple is ("", "")
     * e.g. RSplitOnce("com.test.main.js", ".") => ["com.test.main", "js"]
     */
    static std::tuple<std::string, std::string> RSplitOnce(const std::string &str, const std::string &delimiter);

    /**
     * Is Anonymous Function name
     * 1. empty name
     * 2. ^number or lowercase letters a-f
     * @param functionName function name
     */
    static bool IsAnonymousFunctionName(const std::string &functionName);

    /**
     * Is Anonymous NameSpace name
     * 1. =ensnumber
     * @param name nameSpace name
     * e.g. =ens0, =ens1...
     */
    static bool IsAnonymousNameSpaceName(const std::string &name);

    /**
     * Is str has specific prefix
     * @param start calc from specific position, default is 0
     */
    static bool IsPrefixMatched(const std::string &str, const std::string &prefix, size_t start = 0);

    /**
     * Is str has specific suffix
     */
    static bool IsSuffixMatched(const std::string &str, const std::string &suffix);

    static std::string UnicodeEscape(std::string_view string);

    /**
     * The slash/in JavaScript is the delimiter for regular expressions, indicating the beginning and
     * end of the expression. After converting to C++, it needs to be removed.
     * Usage scenario: str is a regular expression in JS format
     */
    static void RemoveSlashFromBothEnds(std::string &str);

    /**
     * Check string is all of numbers
     * Usage scenario: Attribute value is a number
     */
    static bool IsNumber(const std::string &str);

    /**
     * Split str by '^'
     * @param str split str
     * @return (left, right) if not contain '^', return tuple is (str, "")
     * e.g.
     *  SplitAnonymousName("foo^1) => ["foo", "^1"]
     *  SplitAnonymousName("foo) => ["foo", ""]
     */
    static std::tuple<std::string, std::string> SplitAnonymousName(const std::string &str);
};

}  // namespace panda::guard

#endif  // PANDA_GUARD_UTIL_STRING_UTIL_H
