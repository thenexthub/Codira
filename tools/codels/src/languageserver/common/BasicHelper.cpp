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

#include <iostream>
#include "../logger/Logger.h"
#include "BasicHelper.h"

namespace ark {
constexpr int SMALL_BOUNDARY = 2;
constexpr int MAX_BOUNDARY = 4;

using namespace Codira;

enum class OffsetEncoding {
    UTF8,
    UTF16,
    UTF32,
};

size_t CountLeadingOnes(unsigned char c)
{
    size_t count = 0;
    while ((c & 0x80) != 0) {
        count++;
        c <<= 1;
    }
    return count;
}

bool IsProxyPairs(const std::string& utf8Str, size_t& i)
{
    unsigned int oneByte = 1;
    unsigned int twoByte = 2;
    unsigned int threeByte = 3;
    unsigned int twoByteOffset = 6;
    unsigned int threeByteOffset = 12;
    unsigned int fourByteOffset = 18;

    auto ch = static_cast<unsigned char>(utf8Str[i]);
    unsigned int codepoint = 0;

    if (ch < 0x80) {
        // 1 byte char (ASCII)
        codepoint = ch;
    } else if ((ch & 0xE0) == 0xC0) {
        // 2 byte char
        codepoint = ((ch & 0x1F) << twoByteOffset) | (static_cast<unsigned char>(utf8Str[i + oneByte]) & 0x3F);
    } else if ((ch & 0xF0) == 0xE0) {
        // 3 byte char
        codepoint = ((ch & 0x0F) << threeByteOffset) |
                    ((static_cast<unsigned char>(utf8Str[i + oneByte]) & 0x3F) << twoByteOffset) |
                    (static_cast<unsigned char>(utf8Str[i + twoByte]) & 0x3F);
    } else if ((ch & 0xF8) == 0xF0) {
        // 4 byte char
        codepoint = ((ch & 0x07) << fourByteOffset) |
                    ((static_cast<unsigned char>(utf8Str[i + oneByte]) & 0x3F) << threeByteOffset) |
                    ((static_cast<unsigned char>(utf8Str[i + twoByte]) & 0x3F) << twoByteOffset) |
                    (static_cast<unsigned char>(utf8Str[i + threeByte]) & 0x3F);
    }

    // Proxy pairs count as 2 characters
    if (codepoint > 0xFFFF) {
        return true;
    }
    return false;
}

bool IterCodePointsOfUTF8(const std::string &curLine, const std::function<bool(int, int)> &callback)
{
    if (curLine.empty()) {
        return true;
    }
    for (size_t i = 0; i < curLine.size();) {
        auto c = static_cast<unsigned char>(curLine[i]);
        int units = 1;
        // 0xxx is ASCII
        if (!(c & 0x80)) {
            if (callback(1, units)) {
                return true;
            }
            i++;
            continue;
        }
        // 10xxx, 110xxx, ...
        size_t length = CountLeadingOnes(c);
        if (IsProxyPairs(curLine, i)) {
            units += 1;
        }
        if (length < SMALL_BOUNDARY || length > MAX_BOUNDARY) {
            if (callback(1, units)) {
                return true;
            }
            i++;
            continue;
        }
        i += length;
        if (callback(static_cast<int>(length), units)) {
            return true;
        }
    }
    return false;
}

size_t MeasureUnits(const std::string &curLine, int col, OffsetEncoding enc, bool &valid)
{
    if (col < 0) {
        valid = false;
        return 0;
    }
    size_t result = 0;
    switch (enc) {
        case OffsetEncoding::UTF8:
            valid = IterCodePointsOfUTF8(curLine, [&](int len, int units) {
                if (col < units) {
                    return true;
                }
                result += static_cast<size_t>(len);
                col -= units;
                return col == 0;
            });
            if (col < 0) {
                valid = false;
            }
            break;
        case OffsetEncoding::UTF16:
        case OffsetEncoding::UTF32:
        default:
            Logger::Instance().LogMessage(MessageType::MSG_LOG, "Encoding is not support.");
            valid = false;
    }
    if (result > curLine.size()) {
        valid = false;
        return curLine.size();
    }
    return result;
}

int32_t GetOffsetFromPosition(const std::string &codeText, const Position pos)
{
    Logger& logger = Logger::Instance();
    if (pos.line < 0 || pos.column < 0) {
        logger.LogMessage(MessageType::MSG_WARNING, "Line value or character value is negative.");
        return -1;
    }
    size_t offsetAtCurLine = 0;
    for (int i = 0; i < pos.line; i++) {
        size_t next = codeText.find('\n', offsetAtCurLine);
        if (next == std::string::npos) {
            logger.LogMessage(MessageType::MSG_WARNING, "Line value is out of range.");
            return -1;
        }
        offsetAtCurLine = next + 1;
    }
    size_t curLineLF = codeText.find('\n', offsetAtCurLine);
    if (curLineLF == std::string::npos) {
        curLineLF = codeText.size();
    }
    std::string curLine = codeText.substr(offsetAtCurLine, (curLineLF - offsetAtCurLine) + 1);
    bool valid = true;
    size_t byteInCurLine = MeasureUnits(curLine, pos.column, OffsetEncoding::UTF8, valid);
    if (!valid) {
        return -1;
    }

    return static_cast<int32_t>(offsetAtCurLine + byteInCurLine);
}
} // namespace ark
