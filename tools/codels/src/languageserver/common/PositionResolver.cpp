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

#include "PositionResolver.h"
#include <codecvt>
#include "../CompilerCodiraProject.h"

namespace ark {
bool IsUTF8(const std::string &str)
{
    int count;
    auto length = str.length();
    size_t i = 0;
    while (i < length) {
        auto ch = static_cast<unsigned char>(str[i]);
        if (ch <= 0x7f) {
            count = static_cast<int>(UTF8Kind::BYTE_ONE);
        } else if ((ch & 0xE0) == 0xC0) {
            count = static_cast<int>(UTF8Kind::BYTE_TWO);
        } else if (ch == 0xED && i < (length - 1) && (static_cast<unsigned char>(str[i + 1]) & 0xA0) == 0xA0) {
            // U+d800 to U+DFFF
            return false;
        } else if ((ch & 0xF0) == 0xE0) {
            count = static_cast<int>(UTF8Kind::BYTE_THREE);
        } else if ((ch & 0xF8) == 0xF0) {
            count = static_cast<int>(UTF8Kind::BYTE_FOUR);
        } else {
            return false;
        }
        int j = 0;
        while (j < count && i < length) {
            i++;
            if (i == length || ((static_cast<unsigned char>(str[i]) & 0xC0) != 0x80)) {
                return false;
            }
            j++;
        }
        i++;
    }
    return true;
}

std::basic_string<char32_t> UTF8ToChar32(const std::string &str)
{
    try {
        std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
        return conv.from_bytes(str);
    } catch (const std::exception& e) {
        // deal with illegal utf-8 string
        return std::u32string();
    }
}

std::string Char32ToUTF8(const std::basic_string<char32_t>& str)
{
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.to_bytes(str);
}

int GetFirstTokenOnCurLine(const std::vector<Codira::Token> &tokens, int declLine)
{
    if (tokens.empty()) {
        return -1;
    }
    int left = 0;
    int right = static_cast<int>(tokens.size()) - 1;
    while (left < right) {
        int mid = (left + right) / 2;   // Dichotomy, not magic number.
        if (declLine == tokens[mid].Begin().line) {
            right = mid;
        } else if (declLine > tokens[mid].Begin().line) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    if (right < 0 || right >= static_cast<int>(tokens.size())) {
        return -1;
    }
    if (tokens[right].Begin().line == declLine) {
        return right;
    }
    return -1;
}

int GetLastTokenOnCurLine(const std::vector<Codira::Token> &tokens, int declLine)
{
    if (tokens.empty()) {
        return -1;
    }
    int left = 0;
    int right = static_cast<int>(tokens.size()) - 1;
    while (left < right) {
        int mid = (left + right + 1) / 2;   // Dichotomy, not magic number.
        if (declLine == tokens[mid].Begin().line) {
            left = mid;
        } else if (declLine > tokens[mid].Begin().line) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    if (left < 0 || left >= static_cast<int>(tokens.size())) {
        return -1;
    }
    if (tokens[left].Begin().line == declLine) {
        return left;
    }
    return -1;
}

Codira::Position TransformFromChar2IDE(Codira::Position pos)
{
    if (pos.line == 0 || pos.column == 0) {
        return pos;
    }
    pos.line -= 1;
    pos.column -= 1;
    return pos;
}

Codira::Position PosFromIDE2Char(Codira::Position pos)
{
    pos.line += 1;
    pos.column += 1;
    return pos;
}

Range TransformFromChar2IDE(Range range)
{
    range.start = TransformFromChar2IDE(range.start);
    range.end = TransformFromChar2IDE(range.end);
    return range;
}

Range TransformFromIDE2Char(Range range)
{
    range.start = PosFromIDE2Char(range.start);
    range.end = PosFromIDE2Char(range.end);
    return range;
}

bool PositionInCurToken(int line, int column, const Codira::Token &token)
{
    std::string lastString;
    if (token.kind == Codira::TokenKind::MULTILINE_STRING ||
        token.kind == Codira::TokenKind::MULTILINE_RAW_STRING) {
        int beginLine = token.Begin().line;
        int endLine = LineOfCommentEnd(token, lastString);
        if (beginLine < endLine && line > beginLine && line <= endLine) {
            return true;
        }
    }
    return line == token.Begin().line && column >= token.Begin().column &&
           column <= token.End().column;
}

int LineOfCommentEnd(const Codira::Token &token, std::string &lastString)
{
    std::string comment = token.Value();
    int endLine = token.Begin().line;
    std::string separateCharacter = "\n";
    size_t separateCharacterLen = separateCharacter.size();
    size_t lastPosition = 0;
    size_t index = 0;
    while ((index = comment.find(separateCharacter, lastPosition)) != std::string::npos) {
        endLine++;
        lastPosition = index + separateCharacterLen;
    }
    lastString = comment.substr(lastPosition);
    if (token.kind == Codira::TokenKind::MULTILINE_STRING) {
        endLine++;
    }
    return endLine;
}

void InterpStringInMultiString(const std::vector<Codira::Token> &tokens, Codira::Position &pos,
                               const Codira::AST::Node &node, bool isIDEToUTF8)
{
    std::string path = CompilerCodiraProject::GetInstance()->GetPathBySource(node, pos.fileID);
    ArkAST *input = CompilerCodiraProject::GetInstance()->GetArkAST(path);
    if (input == nullptr) {
        return;
    }
    int index = input->GetCurTokenByPos(pos, 0, static_cast<int>(input->tokens.size()) - 1);
    if (index < 0 || index > static_cast<int>(tokens.size() - 1)) {
        return;
    }
    if (tokens[index].kind == Codira::TokenKind::MULTILINE_STRING &&
        PositionInCurToken(pos.line, pos.column, tokens[index])) {
        auto splitStr = Split(tokens[index].Value());
        int indexOfSplitStr = (pos.line - tokens[index].Begin().line) - 1;
        if (indexOfSplitStr < 0 || indexOfSplitStr > static_cast<int>(splitStr.size() - 1)) {
            return;
        }
        std::string strCurPosition = splitStr[indexOfSplitStr];
        if (!IsUTF8(strCurPosition)) {
            return;
        }
        if (isIDEToUTF8) {
            auto char32ForToken = UTF8ToChar32(strCurPosition);
            auto validStr = char32ForToken.substr(0, pos.column);
            pos.column = static_cast<int>(Char32ToUTF8(validStr).size());
            return;
        }
        std::string validStr = strCurPosition.substr(0, pos.column);
        pos.column = static_cast<int>(CountUnicodeCharacters(validStr));
    }
}

bool IsMultiLine(const Codira::Token &token)
{
    return token.kind == Codira::TokenKind::COMMENT || token.kind == Codira::TokenKind::MULTILINE_STRING ||
           token.kind == Codira::TokenKind::MULTILINE_RAW_STRING;
}

int RedundantCharacterOfMultiLineString(const std::vector<Codira::Token> &tokens, const Codira::Position &pos,
                                        int preBegin)
{
    std::string lastString;
    if (!IsUTF8(lastString) || preBegin < 0) {
        return 0;
    }
    int endLine = LineOfCommentEnd(tokens[preBegin], lastString);
    if (endLine == pos.line && !lastString.empty()) {
        return static_cast<int>(static_cast<int>(lastString.size()) - CountUnicodeCharacters(lastString));
    }
    return 0;
}

void PositionIDEToUTF8(const std::vector<Codira::Token> &tokens, Codira::Position &pos,
                       const Codira::AST::Node &node)
{
    if (tokens.empty()) {
        return;
    }
    int begin = GetFirstTokenOnCurLine(tokens, pos.line);
    int end = GetLastTokenOnCurLine(tokens, pos.line);
    int preBegin = begin - 1;
    bool isInvalid = begin < 0 || end < 0 || (preBegin >= 0 && IsMultiLine(tokens[preBegin]));
    if (isInvalid) {
        if (begin < 0 || end < 0) {
            InterpStringInMultiString(tokens, pos, node, true);
            return;
        }
        Codira::Position posRes = pos;
        posRes.column = posRes.column + RedundantCharacterOfMultiLineString(tokens, posRes, preBegin);
        if (posRes < tokens[begin].Begin()) {
            InterpStringInMultiString(tokens, pos, node, true);
            return;
        }
    }
    int redundantCharacters = 0;
    if (preBegin >= 0 && IsMultiLine(tokens[preBegin])) {
        redundantCharacters += RedundantCharacterOfMultiLineString(tokens, pos, preBegin);
    }
    while (begin <= end) {
        if (!IsUTF8(tokens[begin].Value())) {
            break;
        }
        if (tokens[begin].Begin().column - redundantCharacters > pos.column) {
            if (begin - 1 >= 0 &&
                tokens[begin - 1].Begin().column + static_cast<int>(tokens[begin - 1].Value().size()) >
                pos.column + redundantCharacters) {
                // token end > trigger pos, the trigger position in on the [begin - 1] token
                auto offset = static_cast<int>(static_cast<int>(tokens[begin - 1].Value().size()) -
                                               CountUnicodeCharacters(tokens[begin - 1].Value()));
                auto validStr = tokens[begin - 1].Value().substr(0,  pos.column - tokens[begin - 1].Begin().column +
                                                                      redundantCharacters);
                redundantCharacters = redundantCharacters - offset +
                                      static_cast<int>(
                                          static_cast<int>(validStr.size()) - CountUnicodeCharacters(validStr));
            }
            break;
        }
        auto offset = static_cast<int>(static_cast<int>(tokens[begin].Value().size()) -
                                       CountUnicodeCharacters(tokens[begin].Value()));
        redundantCharacters += offset;
        begin++;
    }
    pos.column = pos.column + redundantCharacters;
}

void PositionIDEToUTF8ForC(const ArkAST &input, Codira::Position &pos)
{
    auto tokens = input.tokens;
    if (tokens.empty()) {
        return;
    }
    int begin = GetFirstTokenOnCurLine(tokens, pos.line);
    int end = GetLastTokenOnCurLine(tokens, pos.line);
    int characters = 0;
    int preBegin = begin - 1;
    bool isInvalid = begin < 0 || end < 0 || pos < tokens[begin].Begin();
    if (isInvalid) {
        InterpStringInMultiString(tokens, pos, *input.file, false);
        return;
    }
    if (preBegin >= 0 && IsMultiLine(tokens[preBegin])) {
        characters += RedundantCharacterOfMultiLineString(tokens, pos, preBegin);
    }
    // If the cursor is at the interpolated string, a cut calculation is required.
    int idxOfCurTok = input.GetCurToken(pos, begin, end);
    if (idxOfCurTok == -1) {
        return;
    }
    Token curToken = tokens[idxOfCurTok];
    int idxOfStringLiteral = -1;
    int offsetOfStringLiteral = 0;
    if (curToken.Begin() <= pos && pos <= curToken.End()) {
        if (curToken.kind == TokenKind::STRING_LITERAL) {
            idxOfStringLiteral = idxOfCurTok;
            Lexer lexer = Lexer(input.tokens, input.packageInstance->diag, *input.sourceManager);
            auto stringParts = lexer.GetStrParts(curToken);
            for (auto &part : stringParts) {
                auto offset =
                    static_cast<int>(static_cast<int>(part.value.size()) - CountUnicodeCharacters(part.value));
                if (part.begin.column - offsetOfStringLiteral <= pos.column &&
                    pos.column <= part.begin.column - offsetOfStringLiteral + CountUnicodeCharacters(part.value)) {
                    auto validStr = part.value.substr(0, pos.column - part.begin.column + offsetOfStringLiteral);
                    offsetOfStringLiteral = offsetOfStringLiteral - offset +
                                            (static_cast<int>(validStr.size()) - CountUnicodeCharacters(validStr));
                    break;
                } else {
                    offsetOfStringLiteral += offset;
                }
            }
        }
    }

    while (begin <= end) {
        if (begin == idxOfStringLiteral) {
            characters += offsetOfStringLiteral;
            break;
        }
        if (!IsUTF8(tokens[begin].Value())) {
            break;
        }
        if (tokens[begin].Begin().column - characters > pos.column) {
            if (begin - 1 >= 0 &&
                tokens[begin - 1].Begin().column + static_cast<int>(tokens[begin - 1].Value().size()) >
                pos.column + characters) {
                // token end > trigger pos, the trigger position in on the [begin - 1] token
                auto offset = static_cast<int>(static_cast<int>(tokens[begin - 1].Value().size()) -
                                               CountUnicodeCharacters(tokens[begin - 1].Value()));
                auto validStr = tokens[begin - 1].Value().substr(0, pos.column - tokens[begin - 1].Begin().column +
                                                                        characters);
                characters =
                    characters - offset +
                    static_cast<int>(static_cast<int>(validStr.size()) - CountUnicodeCharacters(validStr));
            }
            break;
        }
        auto offset = static_cast<int>(static_cast<int>(tokens[begin].Value().size()) -
                                       CountUnicodeCharacters(tokens[begin].Value()));
        characters += offset;
        begin++;
    }
    pos.column = pos.column + characters;
}

void PositionUTF8ToIDE(const std::vector<Codira::Token> &tokens, Codira::Position &pos,
                       const Codira::AST::Node &node)
{
    if (tokens.empty()) {
        return;
    }
    int begin = GetFirstTokenOnCurLine(tokens, pos.line);
    int end = GetLastTokenOnCurLine(tokens, pos.line);
    int redundantCharacters = 0;
    int preBegin = begin - 1;
    bool isInvalid = begin < 0 || end < 0 || pos < tokens[begin].Begin();
    if (isInvalid) {
        InterpStringInMultiString(tokens, pos, node, false);
        return;
    }
    if (preBegin >= 0 && IsMultiLine(tokens[preBegin]))  {
        redundantCharacters += RedundantCharacterOfMultiLineString(tokens, pos, preBegin);
    }
    while (begin <= end) {
        if (!IsUTF8(tokens[begin].Value())) {
            return;
        }
        if (!PositionInCurToken(pos.line, pos.column, tokens[begin])) {
            redundantCharacters += static_cast<int>(static_cast<int>(tokens[begin].Value().size()) -
                                                    CountUnicodeCharacters(tokens[begin].Value()));
        }
        if (PositionInCurToken(pos.line, pos.column, tokens[begin])) {
            std::string validStr = tokens[begin].Value().substr(0, pos.column - tokens[begin].Begin().column);
            redundantCharacters += static_cast<int>(static_cast<int>(validStr.size()) -
                                                    CountUnicodeCharacters(validStr));
            pos.column = pos.column - redundantCharacters;
            return;
        }
        begin++;
    }
}

int CountUnicodeCharacters(const std::string& utf8Str)
{
    int length = 0;
    int i = 0;
    int oneByte = 1;
    int twoByte = 2;
    int threeByte = 3;
    int fourByte = 4;
    int twoByteOffset = 6;
    int threeByteOffset = 12;
    int fourByteOffset = 18;

    while (i < static_cast<int>(utf8Str.size())) {
        auto ch = static_cast<unsigned char>(utf8Str[i]);
        unsigned int codepoint = 0;

        if (ch < 0x80) {
            // 1 byte char (ASCII)
            codepoint = ch;
            i += oneByte;
        } else if ((ch & 0xE0) == 0xC0) {
            // 2 byte char
            codepoint = ((ch & 0x1F) << twoByteOffset) | (static_cast<unsigned char>(utf8Str[i + oneByte]) & 0x3F);
            i += twoByte;
        } else if ((ch & 0xF0) == 0xE0) {
            // 3 byte char
            codepoint = ((ch & 0x0F) << threeByteOffset) |
                        ((static_cast<unsigned char>(utf8Str[i + oneByte]) & 0x3F) << twoByteOffset) |
                        (static_cast<unsigned char>(utf8Str[i + twoByte]) & 0x3F);
            i += threeByte;
        } else if ((ch & 0xF8) == 0xF0) {
            // 4 byte char
            codepoint = ((ch & 0x07) << fourByteOffset) |
                        ((static_cast<unsigned char>(utf8Str[i + oneByte]) & 0x3F) << threeByteOffset) |
                        ((static_cast<unsigned char>(utf8Str[i + twoByte]) & 0x3F) << twoByteOffset) |
                        (static_cast<unsigned char>(utf8Str[i + threeByte]) & 0x3F);
            i += fourByte;
        }

        // Proxy pairs count as 2 characters
        if (codepoint <= 0xFFFF) {
            length += 1;
        } else {
            int proxyPairsCount = 2;
            length += proxyPairsCount;
        }
    }
    return length;
}

void UpdateRange(const std::vector<Codira::Token> &tokens, Range &range, const Codira::AST::Node &node,
                 bool needUpdateByName)
{
    if (range.end.column < range.start.column) {
        return;
    }
    int length = range.end.column - range.start.column;
    if (needUpdateByName && node.symbol && !node.symbol->name.empty()) {
        auto relName = node.symbol->name;
        auto decl = dynamic_cast<const Decl *>(&node);
        if (decl) {
            if (!decl->identifierForLsp.empty()) {
                relName = decl->identifierForLsp;
            }
            if (decl->TestAttr(Codira::AST::Attribute::PRIMARY_CONSTRUCTOR) ||
                decl->TestAttr(Codira::AST::Attribute::CONSTRUCTOR)) {
                relName = GetConstructorIdentifier(*decl).empty() ? relName : GetConstructorIdentifier(*decl);
            }
        }
        length = CountUnicodeCharacters(relName);
    }

    PositionUTF8ToIDE(tokens, range.start, node);
    range.end.column = range.start.column + length;
}
} // namespace ark
