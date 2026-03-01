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

/**
 * @file
 *
 * This file implements the Lexer Diagnostic.
 */

#include "LexerImpl.h"
#include "Codira/Basic/Display.h"

namespace Codira {
const int ESCAPE_NUM_OF_CHAR_STRING_LITERAL = 12;
const int ESCAPE_NUM_OF_CHAR_STRING_BYTE_LITERAL = 11;  // ruled out \$

void LexerImpl::DiagUnexpectedDecimalPoint(const char* reasonPoint)
{
    auto baseChar = std::tolower(*reasonPoint);
    auto prefix = PrefixName(static_cast<char>(baseChar));
    auto builder = diag.DiagnoseRefactor(DiagKindRefactor::lex_unexpected_decimal_point, GetPos(pCurrent), prefix);
    auto r = MakeRange(GetPos(reasonPoint - 1), GetPos(reasonPoint + 1));
    builder.AddHint(r, prefix);

    auto note = SubDiagnostic("only decimal or hexadecimal number can support fractional part");
    builder.AddNote(note);

    auto help = DiagHelp("if you expect a decimal fraction, could remove the prefix");
    help.AddSubstitution(r, " ");
    builder.AddHelp(help);

    help = DiagHelp("if you expect a hexadecimal fraction, could use hexadecimal prefix");
    help.AddSubstitution(r, "0x");
    builder.AddHelp(help);
}

void LexerImpl::DiagExpectedDigit(const char base)
{
    auto name = PrefixName(base);
    auto begin = GetPos(pCurrent);
    auto range = MakeRange(begin, begin + Position{0, 0, static_cast<int>(pNext - pCurrent)});
    auto builder = diag.DiagnoseRefactor(DiagKindRefactor::lex_expected_digit, range, name, ConvertCurrentChar());
    builder.AddMainHintArguments(name);
}

void LexerImpl::DiagUnexpectedDigit(const int& base, const char* reasonPoint)
{
    auto baseChar = static_cast<char>(std::tolower(*reasonPoint));
    static std::unordered_map<char, const char*> map = {
        {'b', "0~1"}, {'o', "0~7"}, {'d', "0~9"}, {'x', "0~9 or a~f"}
    };
    auto name = PrefixName(baseChar);
    auto builder =
        diag.DiagnoseRefactor(DiagKindRefactor::lex_unexpected_digit, GetPos(pCurrent), std::string{*pCurrent}, name);
    char c;
    if (base != DEC_BASE) {
        auto r = MakeRange(GetPos(reasonPoint - std::string("0").size()), GetPos(reasonPoint) + Position{0, 0, 1});
        builder.AddHint(r, "of this " + std::string{name} + " prefix");
        c = baseChar;
    } else if (baseChar == 'e' || baseChar == 'p') { // Means exponent part.
        builder.AddHint(GetPos(reasonPoint), "the exponent part base is decimal");
        c = 'd';
    } else {
        auto r = MakeRange(GetPos(reasonPoint), GetPos(pCurrent));
        builder.AddHint(r, "the default base is decimal");
        c = 'd';
    }

    CODEC_ASSERT(map.find(c) != map.end());
    std::string sub = std::string{name} + " may only contain digit within " + map[c];
    auto note = SubDiagnostic(sub);
    builder.AddNote(note);
}

void LexerImpl::DiagUnexpectedExponentPart(const char exp, const char prefix, const char* reasonPoint)
{
    auto preName = PrefixName(prefix);
    if (prefix != 'd' && prefix != 'x') {
        auto reaRange =
            MakeRange(GetPos(reasonPoint - std::string("0").size()), GetPos(reasonPoint) + Position{0, 0, 1});
        auto builder = diag.DiagnoseRefactor(
            DiagKindRefactor::lex_unexpected_exponent_part, GetPos(pCurrent), std::string{*pCurrent}, preName);
        builder.AddHint(reaRange, "of this " + std::string{preName} + " prefix");

        auto note = SubDiagnostic("only decimal or hexadecimal number can support exponent part");
        builder.AddNote(note);
        return;
    }
    if (exp == 'e' && prefix == 'x') {
        auto reaRange =
            MakeRange(GetPos(reasonPoint - std::string("0").size()), GetPos(reasonPoint) + Position{0, 0, 1});
        auto builder = diag.DiagnoseRefactor(
            DiagKindRefactor::lex_unexpected_exponent_part, GetPos(pCurrent), std::string{*pCurrent}, preName);
        builder.AddHint(reaRange, "of this " + std::string{preName} + " prefix");

        auto help = DiagHelp("could try to modify it to hexadecimal exponent type");
        help.AddSubstitution(GetPos(pCurrent), "p");
        builder.AddHelp(help);
        return;
    }
    if (exp == 'p' && prefix == 'd') {
        auto reaRange = MakeRange(GetPos(reasonPoint), GetPos(pCurrent));
        auto builder = diag.DiagnoseRefactor(
            DiagKindRefactor::lex_unexpected_exponent_part, GetPos(pCurrent), std::string{*pCurrent}, preName);
        builder.AddHint(reaRange, "the default base is decimal");

        auto help = DiagHelp("could try to modify it to " + std::string{preName} + " exponent type");
        help.AddSubstitution(GetPos(pCurrent), "e");
        builder.AddHelp(help);
    }
}

void LexerImpl::DiagExpectedExponentPart(const char* reasonPoint)
{
    auto start = reasonPoint - std::string("0").size();
    Range r = MakeRange(GetPos(start), GetPos(pCurrent));
    auto builder = diag.DiagnoseRefactor(
        DiagKindRefactor::lex_expected_exponent_part, GetPos(pCurrent), std::string{start, pCurrent});
    builder.AddHint(r);
}

void LexerImpl::DiagUnexpectedDollarIdentifier(const Token& t)
{
    bool foundWildcard = t.kind == TokenKind::WILDCARD;
    std::string foundMsg = (foundWildcard ? "symbol '" : "keyword '") + t.Value() + "'";
    auto builder = diag.DiagnoseRefactor(DiagKindRefactor::lex_expected_identifier_after_dollar, t, foundMsg);
    if (foundWildcard) {
        builder.diagnostic.mainHint.str = "expected a Unicode XID_Continue after underscore";
        return;
    }

    auto help = DiagHelp("you could escape keyword as identifier using '`'");
    help.AddSubstitution(t, "`" + t.Value() + "`");
    builder.AddHelp(help);
}

void LexerImpl::DiagIllegalSymbol(const char* pStart)
{
    do {
        ReadUTF8Char();
    } while (currentChar > ASCII_BASE);
    Range r = MakeRange(GetPos(pStart), GetPos(pCurrent));
    diag.DiagnoseRefactor(DiagKindRefactor::lex_unrecognized_symbol, r, std::string{pStart, pCurrent});
}

void LexerImpl::DiagUnterminatedSingleLineString(const char* pStart, bool isMatchEnd, bool isJString)
{
    auto builder = diag.DiagnoseRefactor(DiagKindRefactor::lex_unterminated_single_line_string,
        MakeRange(GetPos(pStart), GetPos(pCurrent) + Position{0, 0, 1}));

    if (!interpolations.empty() && interpolations.back() < pStart && !stringStarts.back().second) {
        auto p = interpolations.back();
        builder.AddHint(MakeRange(GetPos(p), GetPos(p + std::string("${").size())));
    }

    if (!isMatchEnd && !isJString) {
        auto note = SubDiagnostic("consider using multi-line string if you intended to include '\\n' in string");
        builder.AddNote(note);
    }
}

void LexerImpl::DiagUnterminatedMultiLineString(const char* pStart)
{
    auto builder = diag.DiagnoseRefactor(
        DiagKindRefactor::lex_unterminated_multi_line_string, MakeRange(GetPos(pStart), GetPos(pCurrent)));

    if (!interpolations.empty() && interpolations.back() < pStart && stringStarts.back().second) {
        auto p = interpolations.back();
        builder.AddHint(MakeRange(GetPos(p), GetPos(p + std::string("${").size())));
    }
}

void LexerImpl::DiagUnterminatedRawString(const char* pStart)
{
    auto builder = diag.DiagnoseRefactor(
        DiagKindRefactor::lex_unterminated_raw_string, MakeRange(GetPos(pStart), GetPos(pCurrent)));

    if (!interpolations.empty() && interpolations.back() < pStart && !stringStarts.back().second) {
        auto p = interpolations.back();
        builder.AddHint(MakeRange(GetPos(p), GetPos(p + std::string("${").size())));
    }
}

void LexerImpl::DiagUnrecognizedEscape(const char* pStart, bool isInString, bool isByteLiteral)
{
    std::string target = isInString ? "string" : "rune";
    if (isByteLiteral) {
        target += " byte";
    }
    if (isInString && isByteLiteral) {
        target = "byte array";
    }
    auto c = ConvertCurrentChar();
    auto builder = diag.DiagnoseRefactor(DiagKindRefactor::lex_unrecognized_escape,
        MakeRange(GetPos(pCurrent - 1), GetPos(pCurrent) + c.size()), "\\" + c, target);

    if (*pStart == '"' && *(pStart + 1) == '"') {
        builder.AddHint(MakeRange(GetPos(pStart), GetPos(pStart + std::string(R"(""")").size())), target);
    } else {
        builder.AddHint(GetPos(pStart), target);
    }

    // Escape number, there are 12 escapes in string/char literal, 11 escapes in char byte or byte array literal
    std::string noteMsg = "found " +
        std::to_string(isByteLiteral ? ESCAPE_NUM_OF_CHAR_STRING_BYTE_LITERAL : ESCAPE_NUM_OF_CHAR_STRING_LITERAL) +
        " possible escapes: ";
    for (auto& [f, s] : escapePrintMap) {
        noteMsg += "'" + s + "', ";
    }
    // Extra escapes comparing to normal ASCII.
    noteMsg += "'\\0', ";
    noteMsg += "'\\\\', ";
    noteMsg += "'\\'', ";
    noteMsg += "'\\\"', ";
    noteMsg += "'\\u{__}'";

    if (!isByteLiteral) {
        noteMsg += ", '\\$'";
    }
    noteMsg += " in " + target + " literal";
    auto note = SubDiagnostic(noteMsg);
    builder.AddNote(note);
}

void LexerImpl::DiagUnexpectedIntegerLiteralTypeSuffix(
    const char* pSuffixStart, const std::string& signednessType, const std::string& suffix)
{
    auto builder = diag.DiagnoseRefactor(DiagKindRefactor::lex_illegal_integer_suffix,
        MakeRange(GetPos(pSuffixStart), GetPos(pSuffixStart + suffix.size() + 1)), signednessType + suffix);
    auto note = SubDiagnostic(
        "integer literal type suffix can only be 'u8', 'u16', 'u32', 'u64', 'i8', 'i16', 'i32', 'i64'");
    builder.AddNote(note);
}


void LexerImpl::DiagUnexpectedFloatLiteralTypeSuffix(const char* pSuffixStart, const std::string& suffix)
{
    auto builder = diag.DiagnoseRefactor(DiagKindRefactor::lex_illegal_float_suffix,
        MakeRange(GetPos(pSuffixStart), GetPos(pSuffixStart + suffix.size() + 1)), "f" + suffix);
    auto note = SubDiagnostic("float literal type suffix can only be 'f16', 'f32', 'f64'");
    builder.AddNote(note);
}

void LexerImpl::DiagExpectedRightBracket(const char* pStart)
{
    const size_t escapeLens = 2;
    auto builder =
        diag.DiagnoseRefactor(DiagKindRefactor::lex_expected_right_bracket, GetPos(pCurrent), ConvertCurrentChar());
    builder.AddHint(GetPos(pStart + escapeLens));

    auto note = SubDiagnostic("unicode escape may contain 8 hexadecimal digits at most");
    builder.AddNote(note);
}

void LexerImpl::DiagExpectedRightBracketOrHexadecimal(const char* pStart)
{
    const size_t escapeLens = 2;
    auto builder = diag.DiagnoseRefactor(
        DiagKindRefactor::lex_expected_right_bracket_or_hexadecimal, GetPos(pCurrent), ConvertCurrentChar());
    builder.AddHint(MakeRange(GetPos(pStart), GetPos(pStart + escapeLens)));

    auto note = SubDiagnostic("unicode escape may only contain hexadecimal digits");
    builder.AddNote(note);
}

void LexerImpl::DiagCharactersOverflow(const char* pStart)
{
    auto builder = diag.DiagnoseRefactor(
        DiagKindRefactor::lex_characters_overflow, MakeRange(GetPos(pStart), GetPos(pCurrent + 1)));

    auto help = DiagHelp("if you intended to write a string literal, use '\"'");
    help.AddSubstitution(MakeRange(GetPos(pStart), GetPos(pCurrent + 1)),
        "\"" + std::string{pStart + 1 + 1, pCurrent} + "\"");
    builder.AddHelp(help);
}

bool LexerImpl::CheckUnicodeSecurity(const int32_t& c) const
{
    // Special areas and format characters
    bool isControlPictures = (c >= 0x2400 && c <= 0x243F);
    bool isPrivateUseAreas =
        (c >= 0xE000 && c <= 0xF8FF) || (c >= 0xF0000 && c <= 0xFFFFD) || (c >= 0x100000 && c <= 0x10FFFD);
    bool isSpecials = (c >= 0xFFF0 && c <= 0xFFFB);
    bool isTags = (c >= 0xE0000 && c <= 0xE007F);
    bool isBidirectionalNeutralFormatting = (c == 0x061C || c == 0x200E || c == 0x200F);
    bool isBidirectionalGeneralFormatting = (c >= 0x202A && c <= 0x202E) || (c >= 0x2066 && c <= 0x2069);
    bool isInterlinearAnnotationChar = (c == 0xFFF9 || c == 0xFFFA || c == 0xFFFB);
    bool isPrefixedFormatControl = ((c >= 0x0600 && c <= 0x0605) || c == 0x06DD || c == 0x070F || c == 0x0890 ||
        c == 0x0891 || c == 0x110BD || c == 0x110CD);
    bool isEgyptianHieroglyphs = (c >= 0x13430 && c <= 0x13438);
    bool isBrahmi = (c == 0x1107F || c == 0x094D || c == 0x09CD || c == 0x0A4D || c == 0x0ACD || c == 0x0B4D ||
        c == 0x0BCD || c == 0x0C4D || c == 0x0CCD || c == 0x0D3B || c == 0x0D3C || c == 0x0D4D || c == 0x0DCA ||
        c == 0x0E3A || c == 0x0E4E || c == 0x0EBA || c == 0x1039 || c == 0x1714 || c == 0x1715 || c == 0x1734 ||
        c == 0x17D1 || c == 0x17D2 || c == 0x1A60 || c == 0x1A7A || c == 0x1B44 || c == 0x1BAA || c == 0x1BAB ||
        c == 0x1BF2 || c == 0x1BF3 || c == 0xA806 || c == 0xA82C || c == 0xA8C4 || c == 0xA953 || c == 0xA9C0 ||
        c == 0xAAF6 || c == 0x10A3F || c == 0x11046 || c == 0x11070 || c == 0x110B9 || c == 0x11133 || c == 0x111C0 ||
        c == 0x11235 || c == 0x112EA || c == 0x1134D || c == 0x11442 || c == 0x114C2 || c == 0x115BF || c == 0x1163F ||
        c == 0x116B6 || c == 0x1172B || c == 0x11839 || c == 0x1193D || c == 0x1193E || c == 0x119E0 || c == 0x11A34 ||
        c == 0x11A47 || c == 0x11A99 || c == 0x11C3F || c == 0x11D44 || c == 0x11D45 || c == 0x11D97);
    bool isHistoricalViramas = (c == 0x0F84 || c == 0x103A || c == 0x193B || c == 0xABED || c == 0x11134);
    bool isMongolianVariation = (c >= 0x180B && c <= 0x180E);
    bool isGenericVariation = (c >= 0xFE00 && c <= 0xFE0F) || (c >= 0xE0100 && c <= 0xE01EF);
    bool isTagCharacters = (c >= 0xE0020 && c <= 0xE007F) || c == 0xE0001 || c == 0x2D7F || c == 0x1680;
    bool isIdeographic = (c >= 0x2FF0 && c <= 0x2FFB) || c == 0x303E;
    bool isMusicalFormatControl = (c >= 0x1D173 && c <= 0x1D17A);
    bool isShorthandFormat = (c >= 0x1BCA0 && c <= 0x1BCA3);
    bool isDeprecatedAlternate = (c >= 0x206A && c <= 0x206F);
    bool isOther = (c == 0xFFFC || c == 0xFFFD);
    bool isSpecialPurposeCharacters = isBidirectionalNeutralFormatting || isBidirectionalGeneralFormatting ||
        isInterlinearAnnotationChar || isPrefixedFormatControl || isEgyptianHieroglyphs || isBrahmi ||
        isHistoricalViramas || isMongolianVariation || isGenericVariation || isTagCharacters || isIdeographic ||
        isMusicalFormatControl || isShorthandFormat || isDeprecatedAlternate || isOther;
    bool isSecurity = isControlPictures || isPrivateUseAreas || isSpecials || isTags || isSpecialPurposeCharacters;
    return !isSecurity;
}

/// Check whether \param ch has an unsecure unicode value.
/// This indicates a unicode value that can help trigger security bugs, e.g. a unicode char that looks like a comment
/// punctuation when rendered, but actually not.
void LexerImpl::CheckUnsecureUnicodeValue(const int32_t& ch)
{
    if (!CheckUnicodeSecurity(ch)) {
        auto args = ConvertUnicode(ch);
        auto builder = diag.DiagnoseRefactor(DiagKindRefactor::lex_unsecure_unicode, GetPos(pCurrent), args);
        builder.AddMainHintArguments(args);
        success = false;
    }
}

void LexerImpl::DiagUnterminatedInterpolation()
{
    auto builder = diag.DiagnoseRefactor(DiagKindRefactor::lex_unterminated_interpolation,
        MakeRange(GetPos(interpolations.back()), GetPos(pCurrent) + Position{0, 0, 1}));

    builder.AddHint(GetPos(stringStarts.back().first));
}

void LexerImpl::DiagUnknownStartOfToken(const Position curPos)
{
    auto agr = ConvertCurrentChar();
    auto builder = diag.DiagnoseRefactor(DiagKindRefactor::lex_unknown_start_of_token, curPos, agr);
    builder.AddMainHintArguments(agr);
}

void LexerImpl::DiagUnrecognizedCharInByte(const int32_t& c, const std::string& str, const char* pStart,
    const std::pair<Position, Position>& range)
{
    CODEC_ASSERT(c > ASCII_BASE);
    auto builder = diag.DiagnoseRefactor(DiagKindRefactor::lex_unrecognized_char_in_binary_string,
                                         MakeRange(range.first, range.second), ConvertChar(c), str);

    auto hintPos = GetPos(pStart);
    builder.AddHint(MakeRange(hintPos, hintPos + std::string("b\"").size()), str);
    builder.AddNote("character in " + str + " must be ASCII");
}
}
