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

#ifndef CODIRA_CHIR_STRING_WRAPPER_H
#define CODIRA_CHIR_STRING_WRAPPER_H

#include <string>

namespace Codira::CHIR {

/**
 * @brief Extend api for std::string
 */
class StringWrapper {
public:
    explicit StringWrapper(const std::string& initVal = "");

    /**
     * @brief Return current object as a string.
     *
     * @return The content of current object.
     */
    const std::string& Str() const;

    /**
     * @brief Append new content to the old one.
     *
     * @param newValue The new content.
     */
    void Append(const std::string& newValue);

    /**
     * @brief Append new content to the old one.
     *
     * @param newValue The new content.
     * @param delimiter The delimiter.
     */
    void Append(const std::string& newValue, const std::string& delimiter);

    /**
     * @brief Remove the last N characters.
     *
     * @param n The number of characters.
     */
    void RemoveLastNChars(const size_t n);

    /**
     * @brief If current object has content, then append delimiter to the content, otherwise, not append.
     *
     * @param delimiter The delimiter.
     * @return an object which has already appended delimiter.
     */
    StringWrapper& AddDelimiterOrNot(const std::string& delimiter);

    /**
     * @brief If the `newValue` is empty, then clear current object's content,
     * if not, append the `newValue` to current object's content.
     *
     * @param newValue The new content.
     * @return an object which has already appended or cleared.
     */
    StringWrapper& AppendOrClear(const std::string& newValue);

private:
    std::string value;
};
} // namespace Codira::CHIR

#endif
