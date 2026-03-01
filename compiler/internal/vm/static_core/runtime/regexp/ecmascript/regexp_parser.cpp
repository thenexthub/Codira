/**
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "runtime/regexp/ecmascript/regexp_parser.h"
#include "runtime/regexp/ecmascript/regexp_opcode.h"
#include "runtime/include/coretypes/line_string.h"

#include "libarkbase/utils/utils.h"
#include "libarkbase/utils/utf.h"

#include "securec.h"
#include "unicode/uchar.h"
#include "unicode/uniset.h"

namespace {
constexpr int INVALID_UNICODE_FROM_UTF8 = -1;

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
constexpr int UICODE_FROM_UTF8[] = {
    0x80, 0xc0, 0xdf, 0xe0, 0xef, 0xf0, 0xf7, 0xf8, 0xfb, 0xfc, 0xfd,
};

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
constexpr int UTF8_MIN_CODE[] = {
    0x80, 0x800, 0x10000, 0x00200000, 0x04000000,
};

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
constexpr char UTF8_FIRST_CODE[] = {
    0x1f, 0xf, 0x7, 0x3, 0x1,
};

int FromUtf8(int c, int l, const uint8_t *p, const uint8_t **pp)
{
    uint32_t b;
    uint32_t cc = c;
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    cc &= UTF8_FIRST_CODE[l - 1];
    for (int i = 0; i < l; i++) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        b = *p++;
        if (b < ark::utf::UTF8_2B_SECOND || b >= ark::utf::UTF8_2B_FIRST) {
            return INVALID_UNICODE_FROM_UTF8;
        }
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        cc = (cc << 6) | (b & ark::utf::UTF8_2B_THIRD);  // 6: Maximum Unicode range
    }
    if (cc < static_cast<uint32_t>(UTF8_MIN_CODE[l - 1])) {
        return INVALID_UNICODE_FROM_UTF8;
    }
    *pp = p;
    return static_cast<int>(cc);
}

int UnicodeFromUtf8(const uint8_t *p, int maxLen, const uint8_t **pp)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    int c = *p++;
    if (c < UICODE_FROM_UTF8[0]) {
        *pp = p;
        return c;
    }
    int l = 0;
    if (c >= UICODE_FROM_UTF8[1U] && c <= UICODE_FROM_UTF8[2U]) {         // 1 - 2: 0000 0080 - 0000 07FF
        l = 1;                                                            // 1: 0000 0080 - 0000 07FF Unicode
    } else if (c >= UICODE_FROM_UTF8[3U] && c <= UICODE_FROM_UTF8[4U]) {  // 3 - 4: 0000 0800 - 0000 FFFF
        l = 2;                                                            // 2: 0000 0800 - 0000 FFFF Unicode
    } else if (c >= UICODE_FROM_UTF8[5U] && c <= UICODE_FROM_UTF8[6U]) {  // 5 - 6: 0001 0000 - 0010 FFFF
        l = 3;                                                            // 3: 0001 0000 - 0010 FFFF Unicode
    } else if (c >= UICODE_FROM_UTF8[7U] && c <= UICODE_FROM_UTF8[8U]) {  // 7 - 8: 0020 0000 - 03FF FFFF
        l = 4;                                                            // 4: 0020 0000 - 03FF FFFF Unicode
        // NOLINTNEXTLINE(readability-magic-numbers)
    } else if (c == UICODE_FROM_UTF8[9U] || c == UICODE_FROM_UTF8[10U]) {  // 9 - 10: 0400 0000 - 7FFF FFFF
        l = 5;                                                             // 5: 0400 0000 - 7FFF FFFF Unicode
    } else {
        return INVALID_UNICODE_FROM_UTF8;
    }
    /* check that we have enough characters */
    if (l > (maxLen - 1)) {
        return INVALID_UNICODE_FROM_UTF8;
    }
    return FromUtf8(c, l, p, pp);
}
}  // namespace

namespace ark {
static constexpr uint32_t CACHE_SIZE = 128;
static constexpr uint32_t CHAR_MAXS = 128;
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
static constexpr uint32_t ID_START_TABLE_ASCII[4] = {
    /* $ A-Z _ a-z */
    0x00000000, 0x00000010, 0x87FFFFFE, 0x07FFFFFE};
static RangeSet g_gRangeD(0x30, 0x39);  // NOLINT(fuchsia-statically-constructed-objects, readability-magic-numbers)
// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static RangeSet g_gRangeS({
    std::pair<uint32_t, uint32_t>(0x0009, 0x000D),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x0020, 0x0020),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x00A0, 0x00A0),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x1680, 0x1680),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x2000, 0x200A),  // NOLINT(readability-magic-numbers)
    /* 2028;LINE SEPARATOR;Zl;0;WS;;;;;N;;;;; */
    /* 2029;PARAGRAPH SEPARATOR;Zp;0;B;;;;;N;;;;; */
    std::pair<uint32_t, uint32_t>(0x2028, 0x2029),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x202F, 0x202F),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x205F, 0x205F),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x3000, 0x3000),  // NOLINT(readability-magic-numbers)
    /* FEFF;ZERO WIDTH NO-BREAK SPACE;Cf;0;BN;;;;;N;BYTE ORDER MARK;;;; */
    std::pair<uint32_t, uint32_t>(0xFEFF, 0xFEFF),  // NOLINT(readability-magic-numbers)
});

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static RangeSet g_gRangeW({
    std::pair<uint32_t, uint32_t>(0x0030, 0x0039),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x0041, 0x005A),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x005F, 0x005F),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x0061, 0x007A),  // NOLINT(readability-magic-numbers)
});

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static RangeSet g_gRegexpIdentifyStart({
    std::pair<uint32_t, uint32_t>(0x0024, 0x0024),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x0041, 0x005A),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x0061, 0x007A),  // NOLINT(readability-magic-numbers)
});

// NOLINTNEXTLINE(fuchsia-statically-constructed-objects)
static RangeSet g_gRegexpIdentifyContinue({
    std::pair<uint32_t, uint32_t>(0x0024, 0x0024),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x0030, 0x0039),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x0041, 0x005A),  // NOLINT(readability-magic-numbers)
    std::pair<uint32_t, uint32_t>(0x0061, 0x007A),  // NOLINT(readability-magic-numbers)
});

void RegExpParser::Parse()
{
    // dynbuffer head init [size,capture_count,statck_count,flags]
    buffer_.EmitU32(0);
    buffer_.EmitU32(0);
    buffer_.EmitU32(0);
    buffer_.EmitU32(0);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    PrintF("Parse Pattern------\n");
    // Pattern[U, N]::
    //      Disjunction[?U, ?N]
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    Advance();
    SaveStartOpCode saveStartOp;
    int captureIndex = captureCount_++;
    saveStartOp.EmitOpCode(&buffer_, captureIndex);
    ParseDisjunction(false);
    if (c0_ != KEY_EOF) {
        ParseError("extraneous characters at the end");
        return;
    }
    SaveEndOpCode saveEndOp;
    saveEndOp.EmitOpCode(&buffer_, captureIndex);
    MatchEndOpCode matchEndOp;
    matchEndOp.EmitOpCode(&buffer_, 0);
    // dynbuffer head assignments
    buffer_.PutU32(0, buffer_.size_);
    buffer_.PutU32(NUM_CAPTURE__OFFSET, captureCount_);
    buffer_.PutU32(NUM_STACK_OFFSET, stackCount_);
    buffer_.PutU32(FLAGS_OFFSET, flags_);
}

void RegExpParser::ParseDisjunction(bool isBackward)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    PrintF("Parse Disjunction------\n");
    size_t start = buffer_.size_;
    ParseAlternative(isBackward);
    if (isError_) {
        return;
    }
    do {
        if (c0_ == '|') {
            SplitNextOpCode splitOp;
            uint32_t len = buffer_.size_ - start;
            GotoOpCode gotoOp;
            splitOp.InsertOpCode(&buffer_, start, len + gotoOp.GetSize());
            uint32_t pos = gotoOp.EmitOpCode(&buffer_, 0) - gotoOp.GetSize();
            Advance();
            ParseAlternative(isBackward);
            gotoOp.UpdateOpPara(&buffer_, pos, buffer_.size_ - pos - gotoOp.GetSize());
        }
    } while (c0_ != KEY_EOF && c0_ != ')');
}

uint32_t RegExpParser::ParseOctalLiteral()
{
    // For compatibility with some other browsers (not all), we parse
    // up to three octal digits with a value below 256.
    // ES#prod-annexB-LegacyOctalEscapeSequence
    uint32_t value = c0_ - '0';
    Advance();
    if (c0_ >= '0' && c0_ <= '7') {
        value = value * OCTAL_VALUE + c0_ - '0';
        Advance();
        if (value < OCTAL_VALUE_RANGE && c0_ >= '0' && c0_ <= '7') {
            value = value * OCTAL_VALUE + c0_ - '0';
            Advance();
        }
    }
    return value;
}

bool RegExpParser::ParseUnlimitedLengthHexNumber(uint32_t maxValue, uint32_t *value)
{
    uint32_t x = 0;
    int d = static_cast<int>(HexValue(c0_));
    if (d < 0) {
        return false;
    }
    while (d >= 0) {
        if (UNLIKELY(x > (std::numeric_limits<uint32_t>::max() - static_cast<uint32_t>(d)) / HEX_VALUE)) {
            LOG(FATAL, COMMON) << "value overflow";
            return false;
        }
        x = x * HEX_VALUE + static_cast<uint32_t>(d);
        if (x > maxValue) {
            return false;
        }
        Advance();
        d = static_cast<int>(HexValue(c0_));
    }
    *value = x;
    return true;
}

// This parses RegExpUnicodeEscapeSequence as described in ECMA262.
bool RegExpParser::ParseUnicodeEscape(uint32_t *value)
{
    // Accept both \uxxxx and \u{xxxxxx} (if allowed).
    // In the latter case, the number of hex digits between { } is arbitrary.
    // \ and u have already been read.
    if (c0_ == '{' && IsUtf16()) {
        uint8_t *start = pc_ - 1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        Advance();
        if (ParseUnlimitedLengthHexNumber(0x10FFFF, value)) {  // NOLINT(readability-magic-numbers)
            if (c0_ == '}') {
                Advance();
                return true;
            }
        }
        pc_ = start;
        Advance();
        return false;
    }
    // \u but no {, or \u{...} escapes not allowed.
    bool result = ParseHexEscape(UNICODE_HEX_VALUE, value);
    if (result && IsUtf16() && U16_IS_LEAD(*value) && c0_ == '\\') {
        // Attempt to read trail surrogate.
        uint8_t *start = pc_ - 1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (*pc_ == 'u') {
            Advance(UNICODE_HEX_ADVANCE);
            uint32_t trail;
            if (ParseHexEscape(UNICODE_HEX_VALUE, &trail) && U16_IS_TRAIL(trail)) {
                *value = U16_GET_SUPPLEMENTARY((*value), (trail));  // NOLINT(hicpp-signed-bitwise)
                return true;
            }
        }
        pc_ = start;
        Advance();
    }
    return result;
}

bool RegExpParser::ParseHexEscape(int length, uint32_t *value)
{
    uint8_t *start = pc_ - 1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    uint32_t val = 0;
    for (int i = 0; i < length; ++i) {
        uint32_t c = c0_;
        int d = static_cast<int>(HexValue(c));
        if (d < 0) {
            pc_ = start;
            Advance();
            return false;
        }
        val = val * HEX_VALUE + static_cast<uint32_t>(d);
        Advance();
    }
    *value = val;
    return true;
}

void RegExpParser::ParseAlternativeEscape(bool isBackward, bool &isAtom)
{
    switch (c0_) {
        case 'b': {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("Assertion %c \n", c0_);
            WordBoundaryOpCode wordBoundaryOp;
            wordBoundaryOp.EmitOpCode(&buffer_, 0);
            Advance();
            break;
        }
        case 'B': {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("Assertion %c \n", c0_);
            NotWordBoundaryOpCode notWordBoundaryOp;
            notWordBoundaryOp.EmitOpCode(&buffer_, 0);
            Advance();
            break;
        }
        default: {
            isAtom = true;
            int atomValue = ParseAtomEscape(isBackward);
            if (atomValue != -1) {
                ParseAlternativeEscapeDefault(atomValue);
            }
            break;
        }
    }
}

void RegExpParser::ParseAlternativeEscapeDefault(int atomValue)
{
    if (IsIgnoreCase()) {
        if (!IsUtf16()) {
            atomValue = Canonicalize(atomValue, false);
        } else {
            icu::UnicodeSet set(atomValue, atomValue);
            set.closeOver(USET_CASE_INSENSITIVE);
            set.removeAllStrings();
            int32_t size = set.size();
            RangeOpCode rangeOp;
            RangeSet rangeResult;
            for (int32_t idx = 0; idx < size; idx++) {
                int32_t uc = set.charAt(idx);
                RangeSet curRange(uc);
                rangeResult.Insert(curRange);
            }
            rangeOp.InsertOpCode(&buffer_, rangeResult);
            return;
        }
    }
    if (atomValue <= UINT16_MAX) {
        CharOpCode charOp;
        charOp.EmitOpCode(&buffer_, atomValue);
    } else {
        Char32OpCode charOp;
        charOp.EmitOpCode(&buffer_, atomValue);
    }
}

void RegExpParser::ParsePatternCharacter(bool isBackward)
{
    PrevOpCode prevOp;
    if (isBackward) {
        prevOp.EmitOpCode(&buffer_, 0);
    }
    uint32_t matchedChar = c0_;
    if (c0_ > (INT8_MAX + 1)) {
        Prev();
        int i = 0;
        UChar32 c = 0;
        int32_t length = end_ - pc_ + 1;
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        U8_NEXT(pc_, i, length, c);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        matchedChar = static_cast<uint32_t>(c);
        pc_ += i;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    if (IsIgnoreCase()) {
        matchedChar = static_cast<uint32_t>(Canonicalize(static_cast<int>(matchedChar), IsUtf16()));
    }
    if (matchedChar > UINT16_MAX) {
        Char32OpCode charOp;
        charOp.EmitOpCode(&buffer_, matchedChar);
    } else {
        CharOpCode charOp;
        charOp.EmitOpCode(&buffer_, matchedChar);
    }
    if (isBackward) {
        prevOp.EmitOpCode(&buffer_, 0);
    }
}

void RegExpParser::ParseAlternativeAny(bool isBackward)
{
    PrevOpCode prevOp;
    if (isBackward) {
        prevOp.EmitOpCode(&buffer_, 0);
    }
    if (IsDotAll()) {
        AllOpCode allOp;
        allOp.EmitOpCode(&buffer_, 0);
    } else {
        DotsOpCode dotsOp;
        dotsOp.EmitOpCode(&buffer_, 0);
    }
    if (isBackward) {
        prevOp.EmitOpCode(&buffer_, 0);
    }
}

void RegExpParser::ParseAlternativeRange(bool isBackward)
{
    PrevOpCode prevOp;
    Advance();
    if (isBackward) {
        prevOp.EmitOpCode(&buffer_, 0);
    }
    bool isInvert = false;
    if (c0_ == '^') {
        isInvert = true;
        Advance();
    }
    RangeSet rangeResult;
    if (!ParseClassRanges(&rangeResult)) {
        return;
    }
    if (isInvert) {
        rangeResult.Invert(IsUtf16());
    }
    uint32_t highValue = rangeResult.HighestValue();
    if (highValue <= UINT16_MAX) {
        RangeOpCode rangeOp;
        rangeOp.InsertOpCode(&buffer_, rangeResult);
    } else {
        Range32OpCode rangeOp;
        rangeOp.InsertOpCode(&buffer_, rangeResult);
    }

    if (isBackward) {
        prevOp.EmitOpCode(&buffer_, 0);
    }
}

// CC-OFFNXT(G.FUN.01, huge_method) solid logic
void RegExpParser::ParseAlternativeImpl(bool isBackward, bool &isAtom, int &captureIndex)
{
    switch (c0_) {
        case '^': {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("Assertion %c line start \n", c0_);
            LineStartOpCode lineStartOp;
            lineStartOp.EmitOpCode(&buffer_, 0);
            Advance();
            break;
        }
        case '$': {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("Assertion %c line end \n", c0_);
            LineEndOpCode lineEndOp;
            lineEndOp.EmitOpCode(&buffer_, 0);
            Advance();
            break;
        }
        case '\\': {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("Escape %c \n", c0_);
            Advance();
            ParseAlternativeEscape(isBackward, isAtom);
            break;
        }
        case '(': {
            Advance();
            isAtom = ParseAssertionCapture(&captureIndex, isBackward);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            Advance();
            break;
        }
        case '.': {
            ParseAlternativeAny(isBackward);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("Atom %c match any \n", c0_);
            isAtom = true;
            Advance();
            break;
        }
        case '[': {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("Atom %c match range \n", c0_);
            isAtom = true;
            ParseAlternativeRange(isBackward);
            break;
        }
        case '*':
        case '+':
        case '?':
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            ParseError("nothing to repeat");
            return;
        case '{': {
            uint8_t *begin = pc_ - 1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            int dummy;
            if (ParserIntervalQuantifier(&dummy, &dummy)) {
                ParseError("nothing to repeat");
                return;
            }
            pc_ = begin;
            Advance();
        }
            [[fallthrough]];
        case '}':
        case ']':
            if (IsUtf16()) {
                ParseError("syntax error");
                return;
            }
            [[fallthrough]];
        default: {
            // PatternCharacter
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("PatternCharacter %c\n", c0_);
            isAtom = true;
            ParsePatternCharacter(isBackward);
            Advance();
            break;
        }
    }
}

void RegExpParser::ParseAlternative(bool isBackward)
{
    size_t start = buffer_.size_;
    while (c0_ != '|' && c0_ != KEY_EOF && c0_ != ')') {
        if (isError_) {
            return;
        }
        size_t atomBcStart = buffer_.GetSize();
        int captureIndex = 0;
        bool isAtom = false;
        ParseAlternativeImpl(isBackward, isAtom, captureIndex);
        if (isAtom && !isError_) {
            ParseQuantifier(atomBcStart, captureIndex, captureCount_ - 1);
        }
        if (isBackward) {
            size_t end = buffer_.GetSize();
            size_t termSize = end - atomBcStart;
            size_t moveSize = end - start;
            buffer_.Expand(end + termSize);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (memmove_s(buffer_.buf_ + start + termSize, moveSize, buffer_.buf_ + start, moveSize) != EOK) {
                LOG(FATAL, COMMON) << "memmove_s failed";
                UNREACHABLE();
            }
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (memcpy_s(buffer_.buf_ + start, termSize, buffer_.buf_ + end, termSize) != EOK) {
                LOG(FATAL, COMMON) << "memcpy_s failed";
                UNREACHABLE();
            }
        }
    }
}

int RegExpParser::FindGroupName(const PandaString &name)
{
    size_t len;
    size_t nameLen = name.size();
    const char *p = reinterpret_cast<char *>(groupNames_.buf_);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    const char *bufEnd = reinterpret_cast<char *>(groupNames_.buf_) + groupNames_.size_;
    int captureIndex = 1;
    while (p < bufEnd) {
        len = strlen(p);
        if (len == nameLen && memcmp(name.c_str(), p, nameLen) == 0) {
            return captureIndex;
        }
        p += len + 1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        captureIndex++;
    }
    return -1;
}

template <typename OpCodeT>
void RegExpParser::InsertMatchAheadOpCode(bool isBackward)
{
    Advance();
    uint32_t start = buffer_.size_;
    ParseDisjunction(isBackward);
    MatchOpCode matchOp;
    matchOp.EmitOpCode(&buffer_, 0);
    OpCodeT matchAheadOp;
    uint32_t len = buffer_.size_ - start;
    matchAheadOp.InsertOpCode(&buffer_, start, len);
}

bool RegExpParser::HandleGroupName()
{
    PandaString name;
    auto **pp = const_cast<const uint8_t **>(&pc_);
    if (!ParseGroupSpecifier(pp, name)) {
        ParseError("GroupName Syntax error.");
        return false;
    }
    if (FindGroupName(name) > 0) {
        ParseError("Duplicate GroupName error.");
        return false;
    }
    groupNames_.EmitStr(name.c_str());
    newGroupNames_.push_back(name);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    PrintF("group name %s", name.c_str());

    return true;
}

bool RegExpParser::ParseAssertion(bool isBackward, bool &isAtom, bool &parseCapture)
{
    switch (c0_) {
        // (?=Disjunction[?U, ?N])
        case '=': {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("Assertion(?= Disjunction)\n");
            InsertMatchAheadOpCode<MatchAheadOpCode>(isBackward);
            break;
        }
        // (?!Disjunction[?U, ?N])
        case '!': {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("Assertion(?! Disjunction)\n");
            InsertMatchAheadOpCode<NegativeMatchAheadOpCode>(isBackward);
            break;
        }
        case '<': {
            Advance();
            // (?<=Disjunction[?U, ?N])
            if (c0_ == '=') {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                PrintF("Assertion(?<= Disjunction)\n");
                InsertMatchAheadOpCode<MatchAheadOpCode>(true);
                return true;
                // (?<!Disjunction[?U, ?N])
            }
            if (c0_ == '!') {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                PrintF("Assertion(?<! Disjunction)\n");
                InsertMatchAheadOpCode<NegativeMatchAheadOpCode>(true);
                return true;
            }

            Prev();
            if (!HandleGroupName()) {
                return false;
            }
            Advance();
            parseCapture = true;
            break;
        }
        // (?:Disjunction[?U, ?N])
        case ':':
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("Atom(?<: Disjunction)\n");
            isAtom = true;
            Advance();
            ParseDisjunction(isBackward);
            if (isError_) {
                return false;
            }
            break;
        default:
            Advance();
            ParseError("? Syntax error.");
            return false;
    }

    return true;
}

bool RegExpParser::ParseAssertionCapture(int *captureIndex, bool isBackward)
{
    bool isAtom = false;
    do {
        bool parseCapture = false;
        if (c0_ == '?') {
            Advance();
            if (!ParseAssertion(isBackward, isAtom, parseCapture)) {
                return false;
            }
        } else {
            groupNames_.EmitChar(0);
            parseCapture = true;
        }
        if (parseCapture) {
            isAtom = true;
            *captureIndex = captureCount_++;
            SaveEndOpCode saveEndOp;
            SaveStartOpCode saveStartOp;
            if (isBackward) {
                saveEndOp.EmitOpCode(&buffer_, *captureIndex);
            } else {
                saveStartOp.EmitOpCode(&buffer_, *captureIndex);
            }
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("capture start %d \n", *captureIndex);
            ParseDisjunction(isBackward);
            if (isError_) {
                return false;
            }
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("capture end %d \n", *captureIndex);
            if (isBackward) {
                saveStartOp.EmitOpCode(&buffer_, *captureIndex);
            } else {
                saveEndOp.EmitOpCode(&buffer_, *captureIndex);
            }
        }
    } while (c0_ != ')' && c0_ != KEY_EOF);
    if (c0_ != ')') {
        ParseError("capture syntax error");
        return false;
    }
    return isAtom;
}

int RegExpParser::ParseDecimalDigits()
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    PrintF("Parse DecimalDigits------\n");
    uint32_t result = 0;
    bool overflow = false;
    while (true) {
        if (c0_ < '0' || c0_ > '9') {
            break;
        }
        if (!overflow) {
            if (UNLIKELY(result > (INT32_MAX - c0_ + '0') / DECIMAL_DIGITS_ADVANCE)) {
                overflow = true;
            } else {
                result = result * DECIMAL_DIGITS_ADVANCE + c0_ - '0';
            }
        }
        Advance();
    }
    if (overflow) {
        return INT32_MAX;
    }
    return result;
}

bool RegExpParser::ParserIntervalQuantifier(int *pmin, int *pmax)
{
    // Quantifier::
    //     QuantifierPrefix
    //     QuantifierPrefix?
    // QuantifierPrefix::
    // *
    // +
    // ?
    // {DecimalDigits}
    // {DecimalDigits,}
    // {DecimalDigits,DecimalDigits}
    Advance();
    *pmin = ParseDecimalDigits();
    *pmax = *pmin;
    switch (c0_) {
        case ',': {
            Advance();
            if (c0_ == '}') {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                PrintF("QuantifierPrefix{DecimalDigits,}\n");
                *pmax = INT32_MAX;
                Advance();
            } else {
                *pmax = ParseDecimalDigits();
                if (c0_ == '}') {
                    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
                    PrintF("QuantifierPrefix{DecimalDigits,DecimalDigits}\n");
                    Advance();
                } else {
                    return false;
                }
            }
            break;
        }
        case '}':
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("QuantifierPrefix{DecimalDigits}\n");
            Advance();
            break;
        default:
            Advance();
            return false;
    }
    return true;
}

bool RegExpParser::ParseQuantifierPrefix(int &min, int &max, bool &isGreedy)
{
    switch (c0_) {
        case '*':
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("QuantifierPrefix %c\n", c0_);
            min = 0;
            max = INT32_MAX;
            Advance();
            break;
        case '+':
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("QuantifierPrefix %c\n", c0_);
            min = 1;
            max = INT32_MAX;
            Advance();
            break;
        case '?':
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("QuantifierPrefix %c\n", c0_);
            Advance();
            min = 0;
            max = 1;
            break;
        case '{': {
            uint8_t *start = pc_ - 1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            if (!ParserIntervalQuantifier(&min, &max)) {
                pc_ = start;
                Advance();  // back to '{'
                return false;
            }
            if (min > max) {
                ParseError("Invalid repetition count");
                return false;
            }
            break;
        }
        default:
            break;
    }
    if (c0_ == '?') {
        isGreedy = false;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        PrintF("Quantifier::QuantifierPrefix?\n");
        Advance();
    } else if (c0_ == '?' || c0_ == '+' || c0_ == '*' || c0_ == '{') {
        ParseError("nothing to repeat");
        return false;
    }
    return true;
}

void RegExpParser::ParseQuantifier(size_t atomBcStart, int captureStart, int captureEnd)
{
    int min = -1;
    int max = -1;
    bool isGreedy = true;
    if (!ParseQuantifierPrefix(min, max, isGreedy)) {
        return;
    }
    if (min != -1 && max != -1) {
        stackCount_++;
        PushOpCode pushOp;
        pushOp.InsertOpCode(&buffer_, atomBcStart);
        atomBcStart += pushOp.GetSize();

        if (captureStart != 0) {
            SaveResetOpCode saveResetOp;
            saveResetOp.InsertOpCode(&buffer_, atomBcStart, captureStart, captureEnd);
        }

        // zero advance check
        if (max == INT32_MAX) {
            stackCount_++;
            PushCharOpCode pushCharOp;
            pushCharOp.InsertOpCode(&buffer_, atomBcStart);
            CheckCharOpCode checkCharOp;
            // NOLINTNEXTLINE(readability-magic-numbers)
            checkCharOp.EmitOpCode(&buffer_, RegExpOpCode::GetRegExpOpCode(RegExpOpCode::OP_LOOP)->GetSize());
        }

        if (isGreedy) {
            LoopGreedyOpCode loopOp;
            loopOp.EmitOpCode(&buffer_, atomBcStart - buffer_.GetSize() - loopOp.GetSize(), min, max);
        } else {
            LoopOpCode loopOp;
            loopOp.EmitOpCode(&buffer_, atomBcStart - buffer_.GetSize() - loopOp.GetSize(), min, max);
        }

        if (min == 0) {
            if (isGreedy) {
                SplitNextOpCode splitNextOp;
                splitNextOp.InsertOpCode(&buffer_, atomBcStart, buffer_.GetSize() - atomBcStart);
            } else {
                SplitFirstOpCode splitFirstOp;
                splitFirstOp.InsertOpCode(&buffer_, atomBcStart, buffer_.GetSize() - atomBcStart);
            }
        }

        PopOpCode popOp;
        popOp.EmitOpCode(&buffer_);
    }
}

bool RegExpParser::ParseGroupSpecifier(const uint8_t **pp, PandaString &name)
{
    const uint8_t *p = *pp;
    uint32_t c;
    std::array<char, CACHE_SIZE> buffer {};
    char *q = buffer.data();
    while (true) {
        if (p <= end_) {
            c = *p;
        } else {
            c = KEY_EOF;
        }
        if (c == '\\') {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            p++;
            if (*p != 'u') {
                return false;
            }
            if (!ParseUnicodeEscape(&c)) {
                return false;
            }
        } else if (c == '>') {
            break;
        } else if (c > CACHE_SIZE && c != KEY_EOF) {
            c = static_cast<uint32_t>(UnicodeFromUtf8(p, UTF8_CHAR_LEN_MAX, &p));
        } else if (c != KEY_EOF) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            p++;
        } else {
            return false;
        }
        if (q == buffer.data()) {
            if (IsIdentFirst(c) == 0) {
                return false;
            }
        } else {
            if (!u_isIDPart(c)) {
                return false;
            }
        }
        if (q != nullptr) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            *q++ = c;
        }
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    p++;
    *pp = p;
    name = buffer.data();
    return true;
}

bool RegExpParser::CalculateCaptureIndex(const uint8_t *p, int &captureIndex, const char *groupName, PandaString &name)
{
    if (p[1] == '?') {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (p[CAPTURE_CONUT_ADVANCE - 1] == '<' && p[CAPTURE_CONUT_ADVANCE] != '!' &&
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            p[CAPTURE_CONUT_ADVANCE] != '=') {
            hasNamedCaptures_ = 1;
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            p += CAPTURE_CONUT_ADVANCE;
            if (groupName != nullptr && ParseGroupSpecifier(&p, name) && strcmp(name.c_str(), groupName) == 0) {
                return true;
            }
            captureIndex++;
        }
    } else {
        captureIndex++;
    }

    return false;
}

static inline void ShiftPointerToClosingBracket(const uint8_t *p, const uint8_t *end)
{
    while (p < end && *p != ']') {
        if (*p == '\\') {
            p++;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        }
        p++;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
}

bool RegExpParser::ParseCaptureCountImpl(const uint8_t *p, int &captureIndex, const char *groupName, PandaString &name)
{
    switch (*p) {
        case '(': {
            if (CalculateCaptureIndex(p, captureIndex, groupName, name)) {
                return true;
            }
            break;
        }
        case '\\':
            p++;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            break;
        case '[': {
            ShiftPointerToClosingBracket(p, end_);
            break;
        }
        default:
            break;
    }

    return false;
}

int RegExpParser::ParseCaptureCount(const char *groupName)
{
    const uint8_t *p = nullptr;
    int captureIndex = 1;
    PandaString name;
    hasNamedCaptures_ = 0;
    for (p = base_; p < end_; p++) {  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (ParseCaptureCountImpl(p, captureIndex, groupName, name)) {
            return captureIndex;
        }
    }
    return captureIndex;
}

void RegExpParser::ParseLookBehind(DynChunk &buffer, PrevOpCode &prevOp, bool isBackward)
{
    if (isBackward) {
        prevOp.EmitOpCode(&buffer, 0);
    }
    Advance();
}

void RegExpParser::InsertRangeOpCode(DynChunk &buffer, RangeSet &rangeSet, PrevOpCode &prevOp, bool isBackward)
{
    RangeOpCode rangeOp;
    if (isBackward) {
        prevOp.EmitOpCode(&buffer, 0);
    }
    rangeOp.InsertOpCode(&buffer, rangeSet);
    ParseLookBehind(buffer, prevOp, isBackward);
}

void RegExpParser::InsertRange32OpCode(DynChunk &buffer, RangeSet &rangeSet, PrevOpCode &prevOp, bool isBackward)
{
    RangeSet atomRange(rangeSet);
    atomRange.Invert(IsUtf16());
    Range32OpCode rangeOp;
    if (isBackward) {
        prevOp.EmitOpCode(&buffer, 0);
    }
    rangeOp.InsertOpCode(&buffer, atomRange);
    ParseLookBehind(buffer, prevOp, isBackward);
}

int RegExpParser::ParseGroupName()
{
    Advance();
    if (c0_ != '<') {
        if (!IsUtf16() || HasNamedCaptures()) {
            ParseError("expecting group name.");
            return -1;
        }
    }
    Advance();
    Prev();
    PandaString name;
    auto **pp = const_cast<const uint8_t **>(&pc_);
    if (!ParseGroupSpecifier(pp, name)) {
        ParseError("GroupName Syntax error.");
        return -1;
    }
    int postion = FindGroupName(name);
    if (postion < 0) {
        postion = ParseCaptureCount(name.c_str());
        if (postion < 0 && (!IsUtf16() || HasNamedCaptures())) {
            ParseError("group name not defined");
            return -1;
        }
    }

    return postion;
}

static void EmitRefOpCode(DynChunk &buffer, uint32_t para, bool isBackward)
{
    if (isBackward) {
        BackwardBackReferenceOpCode backReferenceOp;
        backReferenceOp.EmitOpCode(&buffer, para);
    } else {
        BackReferenceOpCode backReferenceOp;
        backReferenceOp.EmitOpCode(&buffer, para);
    }
}

// CC-OFFNXT(G.FUN.01, huge_method) big switch case
// NOLINTNEXTLINE(readability-function-size)
int RegExpParser::ParseAtomEscape(bool isBackward)
{
    // AtomEscape[U, N]::
    //     DecimalEscape
    //     CharacterClassEscape[?U]
    //     CharacterEscape[?U]
    //     [+N]kGroupName[?U]
    int result = -1;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    PrintF("Parse AtomEscape------\n");
    PrevOpCode prevOp;
    switch (c0_) {
        case KEY_EOF:
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            ParseError("unexpected end");
            break;
        // DecimalEscape
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("NonZeroDigit %c\n", c0_);
            int capture = ParseDecimalDigits();
            if (capture > captureCount_ - 1 && capture > ParseCaptureCount(nullptr) - 1) {
                ParseError("invalid backreference count");
                break;
            }
            EmitRefOpCode(buffer_, capture, isBackward);
            break;
        }
        // CharacterClassEscape
        case 'd': {
            // [0-9]
            InsertRangeOpCode(buffer_, g_gRangeD, prevOp, isBackward);
            break;
        }
        case 'D': {
            // [^0-9]
            InsertRange32OpCode(buffer_, g_gRangeD, prevOp, isBackward);
            break;
        }
        case 's': {
            // [\f\n\r\t\v]
            InsertRangeOpCode(buffer_, g_gRangeS, prevOp, isBackward);
            break;
        }
        case 'S': {
            InsertRange32OpCode(buffer_, g_gRangeS, prevOp, isBackward);
            break;
        }
        case 'w': {
            // [A-Za-z0-9]
            InsertRangeOpCode(buffer_, g_gRangeW, prevOp, isBackward);
            break;
        }
        case 'W': {
            // [^A-Za-z0-9]
            InsertRange32OpCode(buffer_, g_gRangeW, prevOp, isBackward);
            break;
        }
        // P{UnicodePropertyValueExpression}
        // p{UnicodePropertyValueExpression}
        case 'P':
        case 'p': {
            RangeSet atom;
            if (ParseUnicodePropertyValueCharacters(&atom, c0_ == 'P') == -1) {
                // If there is an internal parsing error, interrupt
                break;
            }
            if (c0_ != '}') {
                ParseError("Unterminated Unicode property escape");
                break;
            }
            Advance();
            // Generate corresponding Range OpCode based on the filled atom
            if (atom.HighestValue() <= UINT16_MAX) {
                RangeOpCode rangeOp;
                rangeOp.InsertOpCode(&buffer_, atom);
            } else {
                Range32OpCode rangeOp;
                rangeOp.InsertOpCode(&buffer_, atom);
            }
            break;
        }
        case 'k': {
            int postion = ParseGroupName();
            if (postion < 0) {
                break;
            }
            EmitRefOpCode(buffer_, postion, isBackward);
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            Advance();
            break;
        }
        default:
            result = ParseCharacterEscape();
            break;
    }
    return result;
}

int RegExpParser::RecountCaptures()
{
    if (totalCaptureCount_ < 0) {
        const char *name = reinterpret_cast<const char *>(groupNames_.buf_);
        totalCaptureCount_ = ParseCaptureCount(name);
    }
    return totalCaptureCount_;
}
bool RegExpParser::HasNamedCaptures()
{
    if (hasNamedCaptures_ < 0) {
        RecountCaptures();
    }
    return false;
}

// CC-OFFNXT(G.FUN.01, huge_cyclomatic_complexity, huge_method) big switch case
int RegExpParser::ParseCharacterEscape()
{
    // CharacterEscape[U]::
    //     ControlEscape
    //     c ControlLetter
    //     0 [lookahead ∉ DecimalDigit]
    //     HexEscapeSequence
    //     RegExpUnicodeEscapeSequence[?U]
    //     IdentityEscape[?U]
    uint32_t result = 0;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    switch (c0_) {
        // ControlEscape
        case 'f':
            result = '\f';
            PrintControlEscapeAndAdvance();
            break;
        case 'n':
            result = '\n';
            PrintControlEscapeAndAdvance();
            break;
        case 'r':
            result = '\r';
            PrintControlEscapeAndAdvance();
            break;
        case 't':
            result = '\t';
            PrintControlEscapeAndAdvance();
            break;
        case 'v':
            result = '\v';
            PrintControlEscapeAndAdvance();
            break;
        // c ControlLetter
        case 'c': {
            ParseControlLetter(result);
            break;
        }
        case '0': {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("CharacterEscape 0 [lookahead ∉ DecimalDigit]\n");
            if (IsUtf16() && !(*pc_ >= '0' && *pc_ <= '9')) {  // NOLINT(readability-magic-numbers)
                Advance();
                result = 0;
                break;
            }
            [[fallthrough]];
        }
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7': {
            if (IsUtf16()) {
                // With /u, decimal escape is not interpreted as octal character code.
                ParseError("Invalid class escape");
                return 0;
            }
            result = ParseOctalLiteral();
            break;
        }
        // ParseHexEscapeSequence
        // ParseRegExpUnicodeEscapeSequence
        case 'x': {
            Advance();
            if (ParseHexEscape(UNICODE_HEX_ADVANCE, &result)) {
                return result;
            }
            if (IsUtf16()) {
                ParseError("Invalid class escape");
                return -1;
            }
            result = 'x';
            break;
        }
        case 'u': {
            Advance();
            if (ParseUnicodeEscape(&result)) {
                return result;
            }
            if (IsUtf16()) {
                // With /u, invalid escapes are not treated as identity escapes.
                ParseError("Invalid unicode escape");
                return 0;
            }
            // If \u is not followed by a two-digit hexadecimal, treat it
            // as an identity escape.
            result = 'u';
            break;
        }
        // IdentityEscape[?U]
        case '$':
        case '(':
        case ')':
        case '*':
        case '+':
        case '.':
        case '/':
        case '?':
        case '[':
        case '\\':
        case ']':
        case '^':
        case '{':
        case '|':
        case '}':
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("IdentityEscape %c\n", c0_);
            result = c0_;
            Advance();
            break;
        default: {
            ParseCharacterEscapeDefault(result);
            break;
        }
    }
    return static_cast<int>(result);
}

void RegExpParser::ParseCharacterEscapeDefault(uint32_t &result)
{
    if (IsUtf16()) {
        ParseError("Invalid unicode escape");
        result = 0;
        return;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    PrintF("SourceCharacter %c\n", c0_);
    result = c0_;
    if (result < CHAR_MAXS) {
        Advance();
    }
}

void RegExpParser::ParseControlLetter(uint32_t &result)
{
    Advance();
    if ((c0_ >= 'A' && c0_ <= 'Z') || (c0_ >= 'a' && c0_ <= 'z')) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        PrintF("ControlLetter %c\n", c0_);
        result = static_cast<uint32_t>(c0_) & 0x1f;  // NOLINT(readability-magic-numbers, hicpp-signed-bitwise)
        Advance();
    } else {
        if (!IsUtf16()) {
            pc_--;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            result = '\\';
        } else {
            ParseError("Invalid control letter");
            result = -1;
        }
    }
}

void RegExpParser::PrintControlEscapeAndAdvance()
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    PrintF("ControlEscape %c\n", c0_);
    Advance();
}

bool RegExpParser::ParseClassRangesImpl(RangeSet *result)
{
    RangeSet s1;
    uint32_t c1 = ParseClassAtom(&s1);
    if (c1 == UINT32_MAX) {
        ParseError("invalid class range");
        return false;
    }

    int nextC0 = *pc_;
    if (c0_ == '-' && nextC0 != ']') {
        if (c1 == CLASS_RANGE_BASE) {
            if (IsUtf16()) {
                ParseError("invalid class range");
                return false;
            }
            result->Insert(s1);
            return true;
        }
        Advance();
        RangeSet s2;
        uint32_t c2 = ParseClassAtom(&s2);
        if (c2 == UINT32_MAX) {
            ParseError("invalid class range");
            return false;
        }
        if (c2 == CLASS_RANGE_BASE) {
            if (IsUtf16()) {
                ParseError("invalid class range");
                return false;
            }
            result->Insert(s2);
            return true;
        }
        if (c1 < INT8_MAX) {
            if (c1 > c2) {
                ParseError("invalid class range");
                return false;
            }
        }
        if (IsIgnoreCase()) {
            c1 = static_cast<uint32_t>(Canonicalize(c1, IsUtf16()));
            c2 = static_cast<uint32_t>(Canonicalize(c2, IsUtf16()));
        }

        result->Insert(c1, c2);
    } else {
        result->Insert(s1);
    }

    return true;
}

bool RegExpParser::ParseClassRanges(RangeSet *result)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    PrintF("Parse ClassRanges------\n");
    while (c0_ != ']') {
        if (!ParseClassRangesImpl(result)) {
            return false;
        }
    }
    Advance();
    return true;
}

uint32_t RegExpParser::ParseClassAtom(RangeSet *atom)
{
    uint32_t ret = UINT32_MAX;
    switch (c0_) {
        case '\\': {
            Advance();
            ret = static_cast<uint32_t>(ParseClassEscape(atom));
            break;
        }
        case KEY_EOF:
            break;
        case 0: {
            if (pc_ >= end_) {
                return UINT32_MAX;
            }
            [[fallthrough]];
        }
        default: {
            uint32_t value = c0_;
            size_t u16Size;
            if (c0_ > INT8_MAX) {
                pc_ -= 1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                auto u16Result = utf::ConvertUtf8ToUtf16Pair(pc_, true);
                value = u16Result.first;
                u16Size = u16Result.second;
                Advance(u16Size + 1);
            } else {
                Advance();
            }
            if (IsIgnoreCase()) {
                value = static_cast<uint32_t>(Canonicalize(value, IsUtf16()));
            }
            atom->Insert(RangeSet(value));
            ret = value;
            break;
        }
    }
    return ret;
}

void RegExpParser::InsertRangeBase(RangeSet *atom, RangeSet &rangeSet, bool invert)
{
    atom->Insert(rangeSet);
    if (invert) {
        atom->Invert(IsUtf16());
    }
}

// CC-OFFNXT(G.FUN.05, huge_method)
// NOLINTNEXTLINE(readability-function-size)
int RegExpParser::ParseClassEscape(RangeSet *atom)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    PrintF("Parse ClassEscape------\n");
    int result = -1;
    switch (c0_) {
        case 'b':
            Advance();
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("ClassEscape %c", 'b');
            result = '\b';
            atom->Insert(RangeSet(static_cast<uint32_t>('\b')));
            break;
        case '-':
            Advance();
            result = '-';
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("ClassEscape %c", '-');
            atom->Insert(RangeSet(static_cast<uint32_t>('-')));
            break;
        // CharacterClassEscape
        case 'd':
        case 'D':
            result = CLASS_RANGE_BASE;
            InsertRangeBase(atom, g_gRangeD, c0_ == 'D');
            Advance();
            break;
        case 's':
        case 'S':
            result = CLASS_RANGE_BASE;
            InsertRangeBase(atom, g_gRangeS, c0_ == 'S');
            Advance();
            break;
        case 'w':
        case 'W':
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
            PrintF("ClassEscape::CharacterClassEscape %c\n", c0_);
            result = CLASS_RANGE_BASE;
            InsertRangeBase(atom, g_gRangeW, c0_ == 'W');
            Advance();
            break;
        // P{UnicodePropertyValueExpression}
        // p{UnicodePropertyValueExpression}
        case 'P':
        case 'p':
            if (ParseUnicodePropertyValueCharacters(atom, c0_ == 'P') == -1) {
                break;
            }
            if (c0_ != '}') {
                ParseError("Unterminated Unicode property escape");
                break;
            }
            Advance();
            result = CLASS_RANGE_BASE;
            break;
        default:
            result = ParseCharacterEscape();
            int value = result;
            if (IsIgnoreCase()) {
                value = Canonicalize(value, IsUtf16());
            }
            atom->Insert(RangeSet(static_cast<uint32_t>(value)));
            break;
    }
    return result;
}

int RegExpParser::ParseUnicodePropertyValueCharacters(RangeSet *atom, bool invert)
{
    if (!IsUtf16()) {
        ParseError("Unicode property escapes require the 'u' flag");
        return -1;
    }

    Advance();
    if (c0_ != '{') {
        ParseError("Invalid Unicode property escape");
        return -1;
    }
    Advance();
    if (c0_ == '}') {
        ParseError("Empty Unicode property escape");
        return -1;
    }
    // 1. Parsing attribute names/values between { and }
    uint8_t *start = pc_ - 1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    while (c0_ != '}' && c0_ != KEY_EOF) {
        Advance();
    }
    if (c0_ == KEY_EOF) {
        ParseError("Unterminated Unicode property escape");
        return -1;
    }
    uint8_t *propEnd = pc_ - 1;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    PandaString property(reinterpret_cast<const char *>(start), propEnd - start);

    // 2. Creating a UnicodeSet from attribute strings using ICU,
    // ICU's pattern is typically `[:<property>:]` or `[<key>=<value>]`
    PandaString icuPattern;
    if (property.find('=') != PandaString::npos) {
        icuPattern = "[\\p{" + property + "}]";
    } else {
        icuPattern = "[:" + property + ":]";
    }
    UErrorCode status = U_ZERO_ERROR;
    icu::UnicodeSet icuSet(icu::UnicodeString::fromUTF8(icuPattern.c_str()), status);

    if (U_FAILURE(status) != 0) {
        ParseError("Invalid or unsupported Unicode property");
        return -1;
    }
    // 3. If it is \P, then invert the set
    if (invert) {
        icuSet.complement();
    }
    // 4. Convert icu:: UnicodeSet to RangeSet
    int32_t rangeCount = icuSet.getRangeCount();
    for (int32_t i = 0; i < rangeCount; i++) {
        UChar32 startRange = icuSet.getRangeStart(i);
        UChar32 endRange = icuSet.getRangeEnd(i);
        atom->Insert(startRange, endRange);
    }
    // Return CLASS_SHANGEBASE to inform the caller that atom has been filled and is a complex character set
    return CLASS_RANGE_BASE;
}

void RegExpParser::ParseUnicodePropertyValueCharactersImpl(bool *isValue)
{
    if ((c0_ >= 'A' && c0_ <= 'Z') || (c0_ >= 'a' && c0_ <= 'z')) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        PrintF("UnicodePropertyCharacter::ControlLetter %c\n", c0_);
    } else if (c0_ == '_') {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        PrintF("UnicodePropertyCharacter:: _ \n");
    } else if (c0_ >= '0' && c0_ <= '9') {
        *isValue = true;
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
        PrintF("UnicodePropertyValueCharacter::DecimalDigit %c\n", c0_);
    } else {
        return;
    }
    Advance();
    ParseUnicodePropertyValueCharactersImpl(isValue);
}

// NOLINTNEXTLINE(cert-dcl50-cpp)
void RegExpParser::PrintF(const char *fmt, ...)
{
    (void)fmt;
}

void RegExpParser::ParseError(const char *errorMessage)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    PrintF("error: ");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    PrintF(errorMessage);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg)
    PrintF("\n");
    SetIsError();
    size_t length = strlen(errorMessage) + 1;
    if (memcpy_s(errorMsg_, length, errorMessage, length) != EOK) {
        LOG(FATAL, COMMON) << "memcpy_s failed";
        UNREACHABLE();
    }
}

int RegExpParser::IsIdentFirst(uint32_t c)
{
    if (c < CACHE_SIZE) {
        // NOLINTNEXTLINE(hicpp-signed-bitwise
        return (ID_START_TABLE_ASCII[c >> 5] >> (c & 31)) & 1;  // 5: Shift five bits 31: and operation binary of 31
    }
    return static_cast<int>(u_isIDStart(c));
}
}  // namespace ark
