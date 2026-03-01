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

#include "word_reader.h"

#include <algorithm>
#include <sstream>

#include "error.h"
#include "util/assert_util.h"

namespace {
constexpr char COMMENT_CHARACTER = '#';
constexpr char SINGLE_QUOTES = '\'';
constexpr char DOUBLE_QUOTES = '\"';
constexpr char CR = '\r';
constexpr char LF = '\n';
constexpr std::string_view WHITESPACE_CHARS = " \t";

constexpr char CHARACTER_AT = '@';

bool IsPureCommentLine(const std::string &line)
{
    const size_t pos = line.find_first_not_of(WHITESPACE_CHARS);
    return pos != std::string::npos && line[pos] == COMMENT_CHARACTER;
}

bool IsInsideQuotes(const std::string &line, size_t commentPos)
{
    bool insideSingleQuotes = false;
    bool insideDoubleQuotes = false;
    for (size_t i = 0; i < commentPos; ++i) {
        if (line[i] == SINGLE_QUOTES) {
            insideSingleQuotes = !insideSingleQuotes;
        } else if (line[i] == DOUBLE_QUOTES) {
            insideDoubleQuotes = !insideDoubleQuotes;
        }
    }
    return insideSingleQuotes || insideDoubleQuotes;
}

std::string RemoveTrailingComment(const std::string &line)
{
    size_t commentPos = line.find(COMMENT_CHARACTER);
    if (commentPos != std::string::npos && !IsInsideQuotes(line, commentPos)) {
        return line.substr(0, commentPos);
    }
    return line;
}

std::vector<std::string> StringToLines(const std::string &input)
{
    std::vector<std::string> lines;
    std::istringstream stream(input);
    std::string line;
    while (std::getline(stream, line)) {
        line = RemoveTrailingComment(line);
        line.erase(std::remove(line.begin(), line.end(), LF), line.end());
        line.erase(std::remove(line.begin(), line.end(), CR), line.end());
        if (!line.empty() && !IsPureCommentLine(line)) {
            lines.emplace_back(line);
        }
    }
    return lines;
}

bool IsStartDelimiter(const char character)
{
    return character == CHARACTER_AT;
}

bool IsNonStartDelimiter(const char character)
{
    return character == '{' || character == '}' || character == '(' || character == ')' || character == ',' ||
           character == ';' || character == ':';
}

bool IsDelimiter(const char character)
{
    return IsStartDelimiter(character) || IsNonStartDelimiter(character);
}

bool IsQuote(const char character)
{
    return character == SINGLE_QUOTES || character == DOUBLE_QUOTES;
}
}  // namespace

ark::guard::WordReader::WordReader(const std::string &content)
{
    lineIndex_ = 0;
    lines_ = StringToLines(content);
    if (!lines_.empty()) {
        currentLine_ = lines_[lineIndex_];
    }
    currentLinePos_ = 0;
}

std::string ark::guard::WordReader::NextWord()
{
    currentWord_ = "";

    RefreshCurrentLine();
    if (currentLine_.empty()) {
        return currentWord_;
    }

    const char startChar = currentLine_[currentLinePos_];
    if (IsQuote(startChar)) {
        currentWord_ = HandleQuotedWord(startChar);
    } else if (IsDelimiter(startChar)) {
        currentWord_ = HandleDelimiter();
    } else {
        currentWord_ = HandleSimpleWord();
    }
    return currentWord_;
}

void ark::guard::WordReader::SkipWhitespace()
{
    while (currentLinePos_ < currentLine_.length() && std::isspace(currentLine_[currentLinePos_])) {
        currentLinePos_++;
    }
}

std::string ark::guard::WordReader::NextLine()
{
    return ++lineIndex_ < lines_.size() ? lines_[lineIndex_] : "";
}

void ark::guard::WordReader::RefreshCurrentLine()
{
    if (!currentLine_.empty()) {
        SkipWhitespace();
    }

    while (currentLine_.empty() || currentLinePos_ >= currentLine_.length()) {
        currentLine_ = NextLine();
        if (currentLine_.empty()) {
            return;
        }
        currentLinePos_ = 0;
        SkipWhitespace();
    }
}

std::string ark::guard::WordReader::HandleQuotedWord(char startCharacter)
{
    size_t startPos = currentLinePos_ + 1;
    // the next word is start with a quote character, skip the opening quote, then find the closing quote
    do {
        currentLinePos_++;
        GUARD_ASSERT(currentLinePos_ == currentLine_.length(), ErrorCode::CLASS_SPECIFICATION_FORMAT_ERROR,
                         "ClassSpecification parsing failed: missing closing quote");
    } while (currentLinePos_ < currentLine_.length() && currentLine_[currentLinePos_] != startCharacter);
    currentWord_ = currentLine_.substr(startPos, currentLinePos_ - startPos);
    currentLinePos_++;
    return currentWord_;
}

std::string ark::guard::WordReader::HandleDelimiter()
{
    // the next word is a delimiter character.
    currentWord_ = currentLine_.substr(currentLinePos_, 1);
    currentLinePos_++;
    return currentWord_;
}

std::string ark::guard::WordReader::HandleSimpleWord()
{
    size_t startPos = currentLinePos_;
    // the next word is a simple character string. find the end of line, the first delimiter, or the first white space.
    while (currentLinePos_ < currentLine_.length()) {
        char currentChar = currentLine_[currentLinePos_];
        if (IsNonStartDelimiter(currentChar) || std::isspace(currentChar)) {
            break;
        }
        currentLinePos_++;
    }
    return currentLine_.substr(startPos, currentLinePos_ - startPos);
}