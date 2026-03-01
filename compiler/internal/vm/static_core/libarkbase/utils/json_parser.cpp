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

#include "json_parser.h"
#include <ios>
#include "logger.h"
#include "utils.h"

// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define LOG_JSON(level) LOG(level, COMMON) << "JsonParser: " << std::string(logRecursionLevel_, '\t')

namespace ark {

bool JsonObject::Parser::Parse(const std::string &text)
{
    std::istringstream iss(text);
    istream_.rdbuf(iss.rdbuf());
    return Parse();
}

bool JsonObject::Parser::Parse(std::streambuf *streamBuf)
{
    ASSERT(streamBuf != nullptr);
    istream_.rdbuf(streamBuf);
    return Parse();
}

bool JsonObject::Parser::Parse()
{
    ASSERT(currentObj_ != nullptr);
    if (GetJsonObject(currentObj_) && TryGetSymbol('\0')) {
        LOG_JSON(INFO) << "Successfully parsed JSON-object";
        return true;
    }
    LOG_JSON(ERROR) << "Parsing failed";
    return false;
}

bool JsonObject::Parser::GetJsonObject(JsonObject *emptyObj)
{
    LOG_JSON(DEBUG) << "Parsing object";
    logRecursionLevel_++;
    ASSERT(emptyObj != nullptr);
    ASSERT(emptyObj->valuesMap_.empty());
    if (!TryGetSymbol('{')) {
        return false;
    }

    if (TryGetSymbol('}')) {
        emptyObj->isValid_ = true;
        return true;
    }

    while (true) {
        if (!InsertKeyValuePairIn(emptyObj)) {
            return false;
        }
        if (TryGetSymbol(',')) {
            LOG_JSON(DEBUG) << "Got a comma-separator, getting a new \"key-value\" pair";
            continue;
        }
        break;
    }

    logRecursionLevel_--;
    return (emptyObj->isValid_ = TryGetSymbol('}'));
}

bool JsonObject::Parser::InsertKeyValuePairIn(JsonObject *obj)
{
    ASSERT(obj != nullptr);
    // Get key:
    if (!GetJsonString()) {
        LOG_JSON(ERROR) << "Error while getting a key";
        return false;
    }
    if (!TryGetSymbol(':')) {
        LOG_JSON(ERROR) << "Expected ':' between key and value:";
        return false;
    }
    ASSERT(parsedTemp_.Get<StringT>() != nullptr);
    Key key(std::move(*parsedTemp_.Get<StringT>()));

    if (!GetValue()) {
        return false;
    }

    // Get value:
    Value value(std::move(parsedTemp_));
    ASSERT(obj != nullptr);

    // Insert pair:
    bool isInserted = obj->valuesMap_.try_emplace(key, std::move(value)).second;
    if (!isInserted) {
        LOG_JSON(ERROR) << "Key \"" << key << "\" must be unique";
        return false;
    }
    // Save string representation as a "source" of scalar values. For non-scalar types, string_temp_ is "":
    obj->stringMap_.try_emplace(key, std::move(stringTemp_));
    obj->keys_.push_back(key);

    LOG_JSON(DEBUG) << "Added entry with key \"" << key << "\"";
    LOG_JSON(DEBUG) << "Parsed `key: value` pair:";
    LOG_JSON(DEBUG) << "- key: \"" << key << '"';
    ASSERT(obj->GetValueSourceString(key) != nullptr);
    LOG_JSON(DEBUG) << "- value: \"" << *obj->GetValueSourceString(key) << '"';

    return true;
}

bool JsonObject::Parser::GetNull()
{
    if (!TryGetSymbol('n') || !TryGetSymbol('u') || !TryGetSymbol('l') || !TryGetSymbol('l')) {
        LOG_JSON(ERROR) << "Error while reading null";
        return false;
    }

    parsedTemp_.SetValue(JsonObjPointer {});
    LOG_JSON(DEBUG) << "Got null";
    return true;
}

bool JsonObject::Parser::GetJsonString()
{
    if (!TryGetSymbol('"')) {
        LOG_JSON(ERROR) << "Expected '\"' at the start of the string";
        return false;
    }
    return GetString('"');
}

static bool UnescapeStringChunk(std::string *result, const std::string &chunk, char delim, bool *finished)
{
    for (size_t start = 0; start < chunk.size();) {
        size_t end = chunk.find('\\', start);
        *result += chunk.substr(start, end - start);

        if (end == std::string::npos) {
            // No more escapes.
            break;
        }

        if (end == chunk.size() - 1) {
            // Chunk ends with an unfinished escape sequence.
            *result += delim;
            *finished = false;
            return true;
        }

        ++end;
        start = end + 1;

        constexpr unsigned ULEN = 4;

        switch (chunk[end]) {
            case '"':
            case '\\':
            case '/':
                *result += chunk[end];
                break;
            case 'b':
                *result += '\b';
                break;
            case 'f':
                *result += '\f';
                break;
            case 'n':
                *result += '\n';
                break;
            case 'r':
                *result += '\r';
                break;
            case 't':
                *result += '\t';
                break;
            case 'u':
                if (end + ULEN < chunk.size()) {
                    // Char strings cannot include multibyte characters, ignore top byte.
                    *result += static_cast<char>((HexValue(chunk[end + ULEN - 1]) << 4U) | HexValue(chunk[end + ULEN]));
                    start += ULEN;
                    break;
                }
                [[fallthrough]];
            default:
                // Invalid escape sequence.
                return false;
        }
    }

    *finished = true;
    return true;
}

bool JsonObject::Parser::GetString(char delim)
{
    std::string string;

    for (bool finished = false; !finished;) {
        std::string chunk;
        if (!std::getline(istream_, chunk, delim) || !UnescapeStringChunk(&string, chunk, delim, &finished)) {
            LOG_JSON(ERROR) << "Error while reading a string";
            return false;
        }
    }

    LOG_JSON(DEBUG) << "Got a string: \"" << string << '"';
    stringTemp_ = string;
    parsedTemp_.SetValue(std::move(string));

    return true;
}

bool JsonObject::Parser::GetNum()
{
    NumT num = 0;
    istream_ >> num;
    if (istream_.fail()) {
        LOG_JSON(ERROR) << "Failed to read a num";
        return false;
    }
    parsedTemp_.SetValue(num);
    LOG_JSON(DEBUG) << "Got an number: " << num;
    return true;
}

bool JsonObject::Parser::GetBool()
{
    BoolT boolean {false};
    istream_ >> std::boolalpha >> boolean;
    if (istream_.fail()) {
        LOG_JSON(ERROR) << "Failed to read a boolean";
        return false;
    }
    parsedTemp_.SetValue(boolean);
    LOG_JSON(DEBUG) << "Got a boolean: " << std::boolalpha << boolean;
    return true;
}

void JsonObject::Parser::SaveSourceString(std::streampos posStart)
{
    auto posEnd = istream_.tellg();
    auto size = static_cast<size_t>(posEnd - posStart);
    stringTemp_.resize(size, '\0');
    istream_.seekg(posStart);
    istream_.read(&stringTemp_[0], static_cast<std::streamsize>(size));
    ASSERT(istream_);
}

bool JsonObject::Parser::GetValue()
{
    auto symbol = PeekSymbol();
    auto posStart = istream_.tellg();
    bool res = false;
    switch (symbol) {
        case 'n':
            res = GetNull();
            break;

        case 't':
        case 'f':
            res = GetBool();
            break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case '-':
        case '+':
        case '.':
            res = GetNum();
            break;

        case '"':
            return GetJsonString();
        case '[':
            stringTemp_ = "";
            return GetArray();
        case '{': {
            stringTemp_ = "";
            auto innerObjPtr = std::make_unique<JsonObject>();
            if (!GetJsonObject(innerObjPtr.get())) {
                return false;
            }
            LOG_JSON(DEBUG) << "Got an inner JSON-object";
            parsedTemp_.SetValue(std::move(innerObjPtr));
            return true;
        }
        default:
            LOG_JSON(ERROR) << "Unexpected character when trying to get value: '" << PeekSymbol() << "'";
            return false;
    }

    // Save source string of parsed value:
    SaveSourceString(posStart);
    return res;
}

bool JsonObject::Parser::GetArray()
{
    if (!TryGetSymbol('[')) {
        LOG_JSON(ERROR) << "Expected '[' at the start of an array";
        return false;
    }

    ArrayT temp;

    if (TryGetSymbol(']')) {
        parsedTemp_.SetValue(std::move(temp));
        return true;
    }

    while (true) {
        if (!GetValue()) {
            return false;
        }
        temp.push_back(std::move(parsedTemp_));
        if (TryGetSymbol(',')) {
            LOG_JSON(DEBUG) << "Got a comma-separator, getting the next array element";
            continue;
        }
        break;
    }
    parsedTemp_.SetValue(std::move(temp));
    return TryGetSymbol(']');
}

char JsonObject::Parser::PeekSymbol()
{
    istream_ >> std::ws;
    if (istream_.peek() == std::char_traits<char>::eof()) {
        return '\0';
    }
    return static_cast<char>(istream_.peek());
}

char JsonObject::Parser::GetSymbol()
{
    if (!istream_) {
        return '\0';
    }
    istream_ >> std::ws;
    if (istream_.peek() == std::char_traits<char>::eof()) {
        istream_.get();
        return '\0';
    }
    return static_cast<char>(istream_.get());
}

bool JsonObject::Parser::TryGetSymbol(int symbol)
{
    ASSERT(!IsWhitespace(symbol));
    if (static_cast<char>(symbol) != GetSymbol()) {
        istream_.unget();
        return false;
    }
    return true;
}

bool JsonObject::Parser::IsWhitespace(int symbol)
{
    return bool(std::isspace(static_cast<unsigned char>(symbol)));
}
}  // namespace ark

#undef LOG_JSON
