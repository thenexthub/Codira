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

#include "URI.h"
#include "../languageserver/common/Utils.h"

namespace ark {
static bool ShouldEscape(unsigned char c)
{
    // Unreserved characters.
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9')) {
        return false;
    }
    switch (c) {
        case '-':
        case '_':
        case '.':
        case '~':
        case '/': // '/' is only reserved when parsing.
            return false;
        // ':' should return true as our moveFile data'll be compared with vscode.URI %3A
        // eg: editor.document.uri.toString() == docEdit.textDocument.uri
        default:
            return true;
    }
}

static char Hexdigit(unsigned x)
{
    const unsigned char hexChar = 'A';
    const unsigned numRange = 10;
    // num => char: 0-9 => 0-9; 10- => a=>f
    unsigned int res;
    if (x < numRange) {
        res = static_cast<unsigned int>('0') + static_cast<unsigned int>(x);
    } else {
        res = static_cast<unsigned int>(hexChar) + static_cast<unsigned int>(x - numRange);
    }
    return static_cast<char>(res);
}

static std::optional<unsigned> HexDigitValue(char c)
{
    if (c >= '0' && c <= '9') {
        return static_cast<unsigned>(static_cast<int>(c) - static_cast<int>('0'));
    } else if (c >= 'a' && c <= 'z') {
        return static_cast<unsigned>(static_cast<int>(c) - static_cast<int>('a')) + 10U;
    } else if (c >= 'A' && c <= 'Z') {
        return static_cast<unsigned>(static_cast<int>(c) - static_cast<int>('A')) + 10U;
    } else {
        return std::nullopt;
    }
}

static bool IsHexDigit(char c)
{
    return HexDigitValue(c).has_value();
}

static uint8_t HexFromNibbles(char msb, char lsb)
{
    unsigned u1 = HexDigitValue(msb).value();
    unsigned u2 = HexDigitValue(lsb).value();
    return static_cast<uint8_t>((u1 << 4) | u2); // Hexadecimal Shift left 4 bits
}

static void PercentEncode(std::string content, std::string &out)
{
    for (auto it = content.begin(); it != content.end(); ++it) {
        if (ShouldEscape(static_cast<unsigned char>(*it))) {
            out.push_back('%');
            out.push_back(Hexdigit(static_cast<unsigned char>(*it) / HEXADECIMAL)); // Hexadecimal
            out.push_back(Hexdigit(static_cast<unsigned char>(*it) % HEXADECIMAL));
        } else {
            out.push_back(*it);
        }
    }
}

static std::string PercentDecode(std::string content)
{
    std::string result;
    auto e = content.end();
    for (auto it = content.begin(); it != e; ++it) {
        if (*it != '%') {
            result += *it;
            continue;
        }
        if (*it == '%' && it + URI_SECOND_POS < e && IsHexDigit(*(it + 1)) &&
            IsHexDigit(*(it + URI_SECOND_POS))) {
            // Get the character on certain position
            result.push_back(static_cast<char>(HexFromNibbles(*(it + 1), *(it + URI_SECOND_POS))));
            it += URI_SECOND_POS;
        } else {
            result.push_back(*it);
        }
    }
    return result;
}

std::string URI::GetAbsolutePath(std::string bodyPath)
{
    if (bodyPath.find_first_of(CONSTANTS::SLASH) != 0) {
        return "File scheme: expect body to be an absolute path starting with '/': " + bodyPath;
    }
    // For Windows paths e.g. /X:
    if (bodyPath.length() > URI_SECOND_POS && bodyPath[0] == '/' && bodyPath[URI_SECOND_POS] == ':') {
        // Get the character on certain position
        bodyPath = bodyPath.substr(1);
    }
    return bodyPath;
}

inline std::string PathToGBK(const std::string &originStr)
{
    auto strGBK = originStr;
#ifdef _WIN32
    strGBK = Codira::StringConvertor::NormalizeStringToGBK(strGBK).value();
#endif
    return strGBK;
}
inline std::string PathToUtf8(const std::string &originStr)
{
    auto strUTF8 = originStr;
#ifdef _WIN32
    strUTF8 = Codira::StringConvertor::NormalizeStringToUTF8(strUTF8).value();
#endif
    return strUTF8;
}

URI URI::URIFromAbsolutePath(const std::string absolutePath)
{
    auto path = ark::PathWindowsToLinux(absolutePath);
    std::string uriBody;
    // For Windows paths e.g. X:
    if (path.length() > 1 && path[1] == ':') {
        uriBody = CONSTANTS::SLASH;
    }
    uriBody += path;
    uriBody = PathToUtf8(uriBody);
    return URI("file", "", uriBody); // second arg is Authority
}

URI::URI(const std::string &scheme, const std::string &authority, const std::string &body)
    : scheme(scheme), authority(authority), body(body) {
}

std::string URI::ToString() const
{
    std::string result;
    PercentEncode(scheme, result);
    result.push_back(':');
    if (authority.empty() && body.empty()) {
        return result;
    }
    
    if (!authority.empty() || (!body.empty() && body[0] == '/')) {
        (void)result.append("//");
        PercentEncode(authority, result);
    }
    PercentEncode(body, result);
    return result;
}

URI URI::Parse(const std::string& origUri)
{
    URI u;
    std::string uri = origUri;

    auto pos = uri.find(':');
    if (pos == std::string::npos) {
        // error handling
        return u;
    }
    auto schemeStr = uri.substr(0, pos);
    u.scheme = PercentDecode(schemeStr);
    uri = uri.substr(pos + 1);
    if (uri.find_first_of("//") == 0) {
        uri = uri.substr(URI_SECOND_POS); // URI Format file:///%d
        pos = uri.find('/');
        u.authority = PercentDecode(uri.substr(0, pos));
        uri = uri.substr(pos);
    }
    u.body = PercentDecode(uri);
    return u;
}

std::string URI::Resolve(const URI &u)
{
    return GetAbsolutePath(u.body);
}

std::string URI::Resolve(const std::string &fileURI)
{
    auto uri = URI::Parse(fileURI);
    auto path = URI::Resolve(uri);
    return PathToGBK(path);
}
} // namespace ark
