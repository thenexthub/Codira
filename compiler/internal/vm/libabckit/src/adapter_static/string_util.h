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

#ifndef LIBABCKIT_SRC_ADAPTER_STATIC_STRING_UTIL_H
#define LIBABCKIT_SRC_ADAPTER_STATIC_STRING_UTIL_H

#include <string>
#include <vector>

#include "libabckit/src/metadata_inspect_impl.h"

namespace libabckit {

/**
 * @brief StringUtil
 */
class StringUtil {
public:
    /**
     * @brief Check if the string matches the substring starting from end position
     * @param str main string
     * @param subStr sub string
     * @return true: match, false: not match
     */
    static bool IsEndWith(const std::string &str, const std::string_view &subStr);

    /**
     * @brief Remove square brackets suffix from string
     * @param str string
     * @return std::string without square brackets suffix
     */
    static std::string RemoveBracketsSuffix(const std::string &str);

    /**
     * @brief Traverse types and assemble their names
     * @param type AbckitType *
     * @return std::string assembled types name
     */
    static std::string GetTypeNameStr(const AbckitType *type);

    /**
     * @brief Get function name with square brackets
     * @param name function name
     * @return std::string transformed function name
     */
    static std::string GetFuncNameWithSquareBrackets(const char *name);

    /**
     * @brief Replaces all occurrences of a substring within a string.
     * @param str The original string to be processed.
     * @param from The substring to be replaced.
     * @param to The substring to replace all occurrences of 'from'.
     * @return A new string with all occurrences of 'from' replaced by 'to'.
     *
     * @note If 'from' is an empty string, the original string is returned unchanged.
     * @warning If 'to' contains 'from', this may lead to infinite loops. Use with caution.
     */
    static std::string ReplaceAll(const std::string &str, const std::string &from, const std::string &to);
};
}  // namespace libabckit

#endif  // LIBABCKIT_SRC_ADAPTER_STATIC_STRING_UTIL_H
