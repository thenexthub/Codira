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

#ifndef GUARD_CONFIGURATION_WORD_READER_H
#define GUARD_CONFIGURATION_WORD_READER_H

#include <string>
#include <vector>

namespace ark::guard {

/**
 * @brief Word Reader
 * Word Separation: Words are separated by spaces or delimiters.
 * Special Character Handling: If a word contains spaces or delimiters, it can be enclosed in single or double quotes.
 * Comment Handling: Content starting with '#' will be ignored and treated as a comment. If it is within quotes, it
 * will not be deleted.
 */
class WordReader {
public:
    /**
     * @brief Read the content, split the string by newline characters, convert it to vector<string>, and remove
     * comments.
     * @param content content
     */
    explicit WordReader(const std::string &content);

    /**
     * @brief Reads a word from this WordReader.
     * @return the read word.
     */
    std::string NextWord();

private:
    void SkipWhitespace();

    std::string NextLine();

    void RefreshCurrentLine();

    std::string HandleQuotedWord(char startCharacter);

    std::string HandleDelimiter();

    std::string HandleSimpleWord();

    std::vector<std::string> lines_;

    size_t lineIndex_ = 0;

    std::string currentLine_;  // lines_[lineIndex_]

    size_t currentLinePos_ = 0;

    std::string currentWord_;
};
}  // namespace ark::guard

#endif  // GUARD_CONFIGURATION_WORD_READER_H
